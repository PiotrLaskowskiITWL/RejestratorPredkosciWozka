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
extern uint8_t cLiczbaPomiarów;

////////////////////////////////////////////////////////////////////////////////
// Funkcja wysyła dane telemetryczne przez bluetooth aby mogły być odebrane dowolnym terminalem
// Parametry: *stPomiary - wskaźnik na tablicę ze strukturą pomiarową
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t WyślijTelemetrieBluetooth(stPomiary_t *stPomiary)
{
	uint8_t cBłąd = BLAD_OK;
	int nRozmiar;

	nRozmiar = FormatujNagłówekDanych(cBuforTelemetrii);	//nagłówek danych taki sam jak na karcie
	HAL_UART_Transmit(&huart1, (uint8_t*)cBuforTelemetrii, nRozmiar, 10);

	//wykonaj obliczenia prędkości dla wszystkich danych
	for (uint8_t n=0; n<cLiczbaPomiarów; n++)
	{
		nRozmiar = FormatujDanePomiarowe(cBuforTelemetrii, n+1, stPomiary + n);
		HAL_UART_Transmit(&huart1, (uint8_t*)cBuforTelemetrii, nRozmiar, 10);
	}
	return cBłąd;
}
