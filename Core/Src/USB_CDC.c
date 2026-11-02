//////////////////////////////////////////////////////////////////////////////
//
// Obsługa przesyłania telemetrii UART-em przez port CDC emulowany w USB
//
// Utworzone: 30 paź 2026
// Autor: Piotr Laskowski
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "USB_CDC.h"
#include "KartaSD.h"
#include "KodyBledow.h"
#include <stdio.h>
#include "Bluetooth.h"
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"


char cBuforCDC[ROZMIAR_BUFORA_TELEMETRII];
extern uint8_t cLiczbaPomiarów;
extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm_global;
uint8_t cCDC_Aktywne;

////////////////////////////////////////////////////////////////////////////////
// Funkcja wysyła dane telemetryczne przez CDC aby mogły być odebrane dowolnym terminalem
// Parametry: *stPomiary - wskaźnik na tablicę ze strukturą pomiarową
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t WyślijTelemetrieCDC(stPomiary_t *stPomiary)
{
	uint8_t cBłąd = BLAD_OK;
	uint32_t nStatus;
	int32_t nRozmiar;
	uint32_t nWysłane;

	if ((cCDC_Aktywne) && (cdc_acm_global != NULL))
	{
		nRozmiar = FormatujNagłówekDanych(cBuforCDC);	//nagłówek danych taki sam jak na karcie
		nStatus = ux_device_class_cdc_acm_write(cdc_acm_global, (uint8_t*)cBuforCDC, (uint32_t)nRozmiar, &nWysłane);
		cBłąd = (uint8_t)nStatus;

		//wykonaj obliczenia prędkości dla wszystkich danych
		for (uint8_t n=0; n<cLiczbaPomiarów; n++)
		{
			nRozmiar = FormatujDanePomiarowe(cBuforCDC, n+1, stPomiary + n);
			nStatus = ux_device_class_cdc_acm_write(cdc_acm_global, (uint8_t*)cBuforCDC, (uint32_t)nRozmiar, &nWysłane);
			cBłąd = (uint8_t)nStatus;
		}
	}
	return cBłąd;
}
