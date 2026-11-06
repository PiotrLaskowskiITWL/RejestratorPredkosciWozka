//////////////////////////////////////////////////////////////////////////////
//
// Obsługa zapisu danych pomiarowych na kartę SD z wykorzytaniem biblioteki FileX
//
// Utworzone: 26 paź 2026
// Autor: Piotr Laskowski
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "KartaSD.h"
#include "KodyBledow.h"
#include "app_filex.h"
#include <stdio.h>

uint8_t cStatusKarty = 0;
uint16_t sLicznikPomiarow;
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
extern SD_HandleTypeDef hsd1;
extern char cBuforNapisów[ROZMIAR_BUF_NAPISOW];	//zmienna do drukowania napisów funkcją sprintf()
extern uint16_t sLiczbaPomiarów;
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
//  *stPomiary - wskaźnik na tablice struktur przechowujących dane pomiarowe
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t ZapiszPomiarNaKarte(stPomiary_t *stPomiary)
{
	uint8_t cBłąd = BLAD_OK;
	UINT nStatus;
	int nRozmiar;

	cBłąd = CzyKartaObecna();
	if (cBłąd)
	{
		cStatusKarty = STATSD_BRAK_KARTY;	//wyjęta karta nie jest zamontowana, nawet jeżeli wcześniej była
		return cBłąd;
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

	sprintf(cBuforNapisów, "Pomiary%03d.csv", sLicznikPomiarow++);
	if (sLicznikPomiarow > 999)
		sLicznikPomiarow = 0;

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

	for (uint16_t n=0; n<sLiczbaPomiarów; n++)
	{
		nRozmiar = FormatujDanePomiarowe(cBuforNapisów, n+1, stPomiary + n);
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

	/*nStatus =  fx_media_close(&sdio_disk);
	if (nStatus)
		cBłąd = BLAD_ZAPISU_KARTY;
	else
		cStatusKarty = 0;	//karta nie jest już dłużej zamontowana*/
	return cBłąd;
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje do pliku na karcie indeks kolejnego pomiaru
// Parametry:
//  cIndeksPomiaru - Indeks kolejnego pomiaru
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t ZapiszIndeksPomiaruNaKarte(uint16_t sIndeksPomiaru)
{
	uint8_t cBłąd = BLAD_OK;
	UINT nStatus;
	int nRozmiar;

	cBłąd = CzyKartaObecna();
	if (cBłąd)
	{
		cStatusKarty = STATSD_BRAK_KARTY;	//wyjęta karta nie jest zamontowana, nawet jeżeli wcześniej była
		return BLAD_BRAK_KARTY;
	}

	if (cStatusKarty != STATSD_MEDIA_OTWARTE)
	{
		nStatus = fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));
		if (nStatus)
			return BLAD_OTWARCIA_KARTY;
		cStatusKarty = STATSD_MEDIA_OTWARTE;
	}

	//Gdy nie ma pliku to go utwórz. Jeżeli istnieje, to funkcja zwraca kod błędu FX_ALREADY_CREATED, więc nie traktuję tego jako błąd
	nStatus = fx_file_create(&sdio_disk, "IndeksPomiaru.txt");
	if ((nStatus != FX_ALREADY_CREATED) && (nStatus != FX_SUCCESS))
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_file_open(&sdio_disk, &fx_file, "IndeksPomiaru.txt", FX_OPEN_FOR_WRITE);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_file_seek(&fx_file, 0);
	if (nStatus)
		return BLAD_OTWARCIA_KARTY;

	nRozmiar = sprintf(cBuforNapisów, "%d", sIndeksPomiaru);

	nStatus = fx_file_write(&fx_file, cBuforNapisów, nRozmiar);
	if (nStatus)
	return BLAD_ZAPISU_KARTY;

	nStatus =  fx_file_close(&fx_file);
	if (nStatus)
		return BLAD_ZAPISU_KARTY;

	nStatus = fx_media_flush(&sdio_disk);
	if (nStatus)
		cBłąd = BLAD_ZAPISU_KARTY;

	return cBłąd;
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja pobiera z karty ostatnio zapisany indeks pomiaru
// Parametry:
//  *cIndeksPomiaru - wskaźnika na Indeks kolejnego pomiaru
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t CzytajIndeksPomiaruZKarty(uint16_t *sIndeksPomiaru)
{
	uint8_t cBłąd = BLAD_OK;
	UINT nStatus;
	int nRozmiar;

	cBłąd = CzyKartaObecna();
	if (cBłąd)
	{
		cStatusKarty = STATSD_BRAK_KARTY;	//wyjęta karta nie jest zamontowana, nawet jeżeli wcześniej była
		return BLAD_BRAK_KARTY;
	}

	if (cStatusKarty != STATSD_MEDIA_OTWARTE)
	{
		nStatus = fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));
		if (nStatus)
			return BLAD_OTWARCIA_KARTY;
		cStatusKarty = STATSD_MEDIA_OTWARTE;
	}

	nStatus = fx_file_open(&sdio_disk, &fx_file, "IndeksPomiaru.txt", FX_OPEN_FOR_WRITE);
	if (nStatus)
		return BLAD_OTWARCIA_KARTY;

	nStatus = fx_file_seek(&fx_file, 0);
	if (nStatus)
		return BLAD_OTWARCIA_KARTY;

	nStatus = fx_file_read(&fx_file, cBuforNapisów, 3, (ULONG*)&nRozmiar);
	if (nRozmiar)
	{
		sscanf(cBuforNapisów, "%d", &nRozmiar);
		*sIndeksPomiaru = (uint16_t)nRozmiar;
	}
	else
		*sIndeksPomiaru = 0;

	nStatus =  fx_file_close(&fx_file);
	if (nStatus)
		cBłąd = BLAD_ZAPISU_KARTY;

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
	for (uint8_t n=0; n<ROZMIAR_BUF_NAPISOW; n++)
		*(cBufor + n) = 0;

	return sprintf(cBufor, "Nr\t t1[us]\t v1[m/s]\t t2[us]\t v2[m/s]\t t3[us]\t v3[m/s]\t t4[us]\t v4[m/s]\t L[m]\t v[m/s]\t a[m/s^2]\t PoslizgL1[m]\t PoslizgL2[m]\t PoslizgL3[m]\t PoslizgL4[m]\t MahalanobisV1[sigma]\t MahalanobisV2[sigma]\t MahalanobisV3[sigma]\t MahalanobisV4[sigma]\r\n");		//wersja z filtrem Kalmana, odległoscią mahalonobisa dla czujnika 1, tabulacją i spacją
	//return sprintf(cBufor, "Nr\tt1[us]\tv1[m/s]\tt2[us]\tv2[m/s]\tt3[us]\tv3[m/s]\tt4[us]\tv4[m/s]\tL[m]\tv[m/s]\ta[m/s^2]\tPoslizgL1[m]\tPoslizgL2[m]\tPoslizgL3[m]\tPoslizgL4[m]\tMahalanobisV1[sigma]\tMahalanobisV2[sigma]\tMahalanobisV3[sigma]\tMahalanobisV4[sigma]\r\n");		//wersja z filtrem Kalmana, odległoscią mahalonobisa dla czujnika 1, tabulacją bez spacji
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
	int nRozmiar;

	for (uint8_t n=0; n<ROZMIAR_BUF_NAPISOW; n++)
		*(cBufor + n) = 0;

	nRozmiar = sprintf(cBufor, "%u\t %lu\t %.2f\t %lu\t %.2f\t %lu\t %.2f\t %lu\t %.2f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t %.3f\t %.2f\t %.2f\t %.2f\t %.2f\r\n", cIndeks,
				stPomiary->nCzas1, stPomiary->fPrędkość1, stPomiary->nCzas2, stPomiary->fPrędkość2, stPomiary->nCzas3, stPomiary->fPrędkość3, stPomiary->nCzas4, stPomiary->fPrędkość4,
				stPomiary->fKalmanDroga, stPomiary->fKalmanPredkość, stPomiary->fKalmanPrzyspieszenie, stPomiary->fBłądDrogi1, stPomiary->fBłądDrogi2, stPomiary->fBłądDrogi3, stPomiary->fBłądDrogi4,
				stPomiary->fOdlMahalanobisaV1, stPomiary->fOdlMahalanobisaV2, stPomiary->fOdlMahalanobisaV3, stPomiary->fOdlMahalanobisaV4);

	/*nRozmiar = sprintf(cBufor, "%u\t%lu\t%.2f\t%lu\t%.2f\t%lu\t%.2f\t%lu\t%.2f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.2f\t%.2f\t%.2f\t%.2f\r\n", cIndeks,
					stPomiary->nCzas1, stPomiary->fPrędkość1, stPomiary->nCzas2, stPomiary->fPrędkość2, stPomiary->nCzas3, stPomiary->fPrędkość3, stPomiary->nCzas4, stPomiary->fPrędkość4,
					stPomiary->fKalmanDroga, stPomiary->fKalmanPredkość, stPomiary->fKalmanPrzyspieszenie, stPomiary->fBłądDrogi1, stPomiary->fBłądDrogi2, stPomiary->fBłądDrogi3, stPomiary->fBłądDrogi4,
					stPomiary->fOdlMahalanobisaV1, stPomiary->fOdlMahalanobisaV2, stPomiary->fOdlMahalanobisaV3, stPomiary->fOdlMahalanobisaV4); */

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
