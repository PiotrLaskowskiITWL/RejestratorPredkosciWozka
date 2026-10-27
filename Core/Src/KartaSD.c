/*
 * KartaSD.c
 *
 *  Created on: 26 paź 2026
 *      Author: Piotr
 */
#include "KartaSD.h"
#include "KodyBledow.h"
#include "main.h"
#include "app_filex.h"
#include <stdio.h>

uint8_t cStatusKarty = 0;
uint8_t cLicznikPomiarow = 1;
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
extern UART_HandleTypeDef huart1;
extern SD_HandleTypeDef hsd1;
extern char cBuforNapisów[ROZMIAR_BUF_NAPISOW];	//zmienna do drukowania napisów funkcją sprintf()
FX_MEDIA        sdio_disk;
FX_FILE         fx_file;

////////////////////////////////////////////////////////////////////////////////
// Funkcja sprawdza obecność karty w gniździe testując stan pinu CD
// Parametry: brak
// Zwraca: kod błędu: BLAD_OK = karta obecna lub BLAD_BRAK_KARTY
////////////////////////////////////////////////////////////////////////////////
uint8_t CzyKartaObecna(void)
{
	uint8_t cBłąd = BLAD_OK;

	if (HAL_GPIO_ReadPin(CD_GPIO_Port, CD_Pin) == GPIO_PIN_SET)
		cBłąd = BLAD_BRAK_KARTY;
	return cBłąd;
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje dane pomiarowe na karcie
// Parametry:
//  *nCzasRolki1, *nCzasRolki2 - wskaźniki na tablice danych pomiarowych
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t ZapiszPomiarNaKarte(uint32_t *nCzasRolki1, uint32_t *nCzasRolki2)
{
	uint8_t cBłąd = BLAD_OK;
	UINT nStatus;
	int nRozmiar;
	char *cZnak;

	cBłąd = CzyKartaObecna();
	if (cBłąd)
	{
		cStatusKarty = STATSD_BRAK_KARTY;	//wyjęta karta nie jest zamontowana, nawet jeżeli wcześniej była
		return BLAD_NIC_DO_ROBOTY;
	}
	else	//jeżeli karta jest to sprawdź czy nie została przed chwilę włożona
	{
		if (cStatusKarty == STATSD_BRAK_KARTY)
		{
			hsd1.Instance = SDMMC1;
			hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
			hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
			hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
			hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
			hsd1.Init.ClockDiv = 0;
			HAL_SD_Init(&hsd1);
			cStatusKarty &= ~STATSD_BRAK_KARTY;
		}
	}

	if (cStatusKarty != STATSD_MEDIA_OTWARTE)
	{
		nStatus = fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));
		if (nStatus)
			return BLAD_OTWARCIA_KARTY;
		cStatusKarty = STATSD_MEDIA_OTWARTE;
	}

	sprintf(cBuforNapisów, "Pomiary%02d.csv", cLicznikPomiarow++);

	//jeżeli istnieje poprzedni plik o tej samej nazwie to usuń go
	nStatus = fx_file_delete(&sdio_disk, cBuforNapisów);
	if ((nStatus != FX_SUCCESS) && (nStatus != FX_NOT_FOUND))
	    return BLAD_ZAPISU_KARTY;

	nStatus = fx_file_create(&sdio_disk, cBuforNapisów);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_file_open(&sdio_disk, &fx_file, cBuforNapisów, FX_OPEN_FOR_WRITE);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_file_seek(&fx_file, 0);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	//zapisz nagłówek w pierwszym wierszu
	nRozmiar = sprintf(cBuforNapisów, "\r\nNr\t t1[us]\t V1[m/s]\t t2[us]\t V2[m/s]\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)cBuforNapisów, nRozmiar, 10);
	nStatus = fx_file_write(&fx_file, cBuforNapisów, nRozmiar);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	float fPrędkość1, fPrędkość2, fDroga;
	float fCzas1, fCzas2;
	fPrędkość1 = fPrędkość2 = fDroga = 0.0f;
	fCzas1 = fCzas2 = 0.0f;
	for (uint8_t n=0; n<LICZBA_POMIAROW; n++)
	{
		fDroga = (float)OBWOD_ROLKI * (n+1) / 1000;
		fCzas1 += ((float)*(nCzasRolki1 + n) / 1000000);
		fCzas2 += ((float)*(nCzasRolki2 + n) / 1000000);
		fPrędkość1 = fDroga / fCzas1;
		fPrędkość2 = fDroga / fCzas2;
		nRozmiar = sprintf(cBuforNapisów, "%u\t %lu\t %.2f\t %lu\t %.2f\r\n", n, *(nCzasRolki1 + n), fPrędkość1, *(nCzasRolki2 + n), fPrędkość2);

		//zamień kropki dziesietne na przecinki, aby Excell dał radę odczytać dane
		do
		{
			cZnak = strchr(cBuforNapisów, '.');
			if (cZnak)
				*cZnak = ',';
		}
		while (cZnak);

		HAL_UART_Transmit(&huart1, (uint8_t*)cBuforNapisów, nRozmiar, 10);
		nStatus = fx_file_write(&fx_file, cBuforNapisów, nRozmiar);
		if (nStatus)
			return BLAD_ZAPISU_KARTY;
	}

	nStatus =  fx_file_close(&fx_file);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_media_flush(&sdio_disk);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus =  fx_media_close(&sdio_disk);
	if (nStatus)
		cBłąd = BLAD_ZAPISU_KARTY;
	else
		cStatusKarty = 0;	//karta nie jest już dłużej zamontowana
	return cBłąd;
}

