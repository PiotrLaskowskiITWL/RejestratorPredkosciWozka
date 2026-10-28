//////////////////////////////////////////////////////////////////////////////
//
// Obsługa zapisu danych pomiarowych na kartę SD z wykorzytaniem biblioteki FileX
//
// Utworzone: 26 paź 2026
// Autor: Piotr
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "KartaSD.h"
#include "KodyBledow.h"
#include "app_filex.h"
#include <stdio.h>

uint8_t cStatusKarty = 0;
uint8_t cLicznikPomiarow = 1;
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
//extern UART_HandleTypeDef huart1;
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
//  *stPomiary - wskaźnik na tablice struktur przechowująceych dane pomiarowe
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t ZapiszPomiarNaKarte(stPomiary_t *stPomiary)
{
	uint8_t cBłąd = BLAD_OK;
	UINT nStatus;
	int nRozmiar;
	//float fPrędkość1, fPrędkość2, fPrędkość3, fPrędkość4, fDroga;
	float fDroga;
	float fCzas1, fCzas2, fCzas3, fCzas4;

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
	nRozmiar = FormatujNagłówekDanych(cBuforNapisów);
	nStatus = fx_file_write(&fx_file, cBuforNapisów, nRozmiar);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	//wykonaj obliczenia prędkości dla wszystkich danych
	fDroga = 0.0f;
	fCzas1 = fCzas2 = fCzas3 = fCzas4 = 0.0f;
	for (uint8_t n=0; n<LICZBA_POMIAROW; n++)
	{
		fDroga = (float)OBWOD_ROLKI * (n+1) / 1000;
		fCzas1 += ((float)(stPomiary + n)->nCzas1 / 1000000);
		fCzas2 += ((float)(stPomiary + n)->nCzas2 / 1000000);
		fCzas3 += ((float)(stPomiary + n)->nCzas3 / 1000000);
		fCzas4 += ((float)(stPomiary + n)->nCzas4 / 1000000);
		(stPomiary + n)->fPrędkość1 = fDroga / fCzas1;
		(stPomiary + n)->fPrędkość2 = fDroga / fCzas2;
		(stPomiary + n)->fPrędkość3 = fDroga / fCzas3;
		(stPomiary + n)->fPrędkość4 = fDroga / fCzas4;
		nRozmiar = FormatujDanePomiarowe(cBuforNapisów, n, stPomiary + n);
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



////////////////////////////////////////////////////////////////////////////////
// Funkcja tworzy nagłówek jaki będzie umieszczony w pierszym wierszu danych
// Parametry:
//  *cBufor wskaźnik na bufor danych
// Zwraca: ilość znaków nagłówka
////////////////////////////////////////////////////////////////////////////////
int FormatujNagłówekDanych(char *cBufor)
{
	return sprintf(cBufor, "\r\nNr\t t1[us]\t v1[m/s]\t t2[us]\t v2[m/s] t t3[us]\t v3[m/s] t t4[us]\t v4[m/s]\r\n");
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja tworzy ciąg znaków będący wpisem do logu
// Parametry:
//  *cBufor wskaźnik na bufor danych
//  cIndeks - numer pomiaru
//  *stPomiary - wskaźnik na tablicę struktur danych pomiarowych
// Zwraca: ilość znaków nagłówka
////////////////////////////////////////////////////////////////////////////////
int FormatujDanePomiarowe(char *cBufor, uint8_t cIndeks, stPomiary_t *stPomiary)
{
	char *cZnak;
	int nRozmiar = sprintf(cBufor, "%u\t %lu\t %.2f\t %lu\t %.2f %lu\t %.2f %lu\t %.2f\r\n", cIndeks, stPomiary->nCzas1, stPomiary->fPrędkość1, stPomiary->nCzas2, stPomiary->fPrędkość2, stPomiary->nCzas3, stPomiary->fPrędkość3, stPomiary->nCzas4, stPomiary->fPrędkość4);

	//zamień kropki dziesiętne na przecinki, aby Excell dał radę odczytać dane
	do
	{
		cZnak = strchr(cBufor, '.');
		if (cZnak)
			*cZnak = ',';
	}
	while (cZnak);

	return nRozmiar;
}
