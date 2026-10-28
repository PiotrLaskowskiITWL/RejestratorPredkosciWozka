//////////////////////////////////////////////////////////////////////////////
//
// Obsługa przesyłania telemetrii UART-em przez moduł bluettoth
//
// Utworzone: 28 paź 2026
// Autor: Piotr
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "Bluetooth.h"
#include "KartaSD.h"
#include "KodyBledow.h"


extern UART_HandleTypeDef huart1;
char cBuforTelemetrii[ROZMIAR_BUFORA_TELEMETRII];


////////////////////////////////////////////////////////////////////////////////
// Funkcja wysyła dane telemetryczne przez bluetooth aby mogły być odebrane dowolnym terminalem
// Parametry: *stPomiary - wskaźnik na tablicę ze strukturą pomiarową
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t WyślijTelemetrieBluetooth(stPomiary_t *stPomiary)
{
	uint8_t cBłąd = BLAD_OK;
	int nRozmiar;
	float fDroga;
	float fCzas1, fCzas2, fCzas3, fCzas4;

	nRozmiar = FormatujNagłówekDanych(cBuforTelemetrii);	//nagłówek danych taki sam jak na karcie
	HAL_UART_Transmit(&huart1, (uint8_t*)cBuforTelemetrii, nRozmiar, 10);

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
		nRozmiar = FormatujDanePomiarowe(cBuforTelemetrii, n, stPomiary + n);
		HAL_UART_Transmit(&huart1, (uint8_t*)cBuforTelemetrii, nRozmiar, 10);
	}
	return cBłąd;
}
