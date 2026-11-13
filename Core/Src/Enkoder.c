//////////////////////////////////////////////////////////////////////////////
//
// Obsługa enkodera
//
// Utworzone: 13 lis 2026
// Autor: Piotr Laskowski
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "Enkoder.h"
#include "main.h"

uint8_t cPoprzedniStanCzujnika[LICZBA_CZUJNIKOW] = {0,0,0,0};
uint8_t cStanEnkodera[2];
stEnkoder_t stEnkoder;

////////////////////////////////////////////////////////////////////////////////
// Funkcja określa kierunek obrotu enkodera na podstawie sekwencji stanów faz enkodera
// Parametry:
//  cStanBieżący - bieżący stan enkodera
//  *cStanPoprzedni - wskaźnik na stan poprzedni enkodera
// Zwraca: kierunek obrotu: +1 lub -1
////////////////////////////////////////////////////////////////////////////////
int8_t SprawdźKierunek(uint8_t cStanBieżący, uint8_t *cStanPoprzedni)
{
	int8_t cKierunek;

	if (((*cStanPoprzedni == 0) && (cStanBieżący == 1)) || ((*cStanPoprzedni == 1) && (cStanBieżący == 3)) || ((*cStanPoprzedni == 3) && (cStanBieżący == 2)) || ((*cStanPoprzedni == 2) && (cStanBieżący == 0)))
		cKierunek = 1;

	if (((*cStanPoprzedni == 0) && (cStanBieżący == 2)) || ((*cStanPoprzedni == 2) && (cStanBieżący == 3)) || ((*cStanPoprzedni == 3) && (cStanBieżący == 1)) || ((*cStanPoprzedni == 1) && (cStanBieżący == 0)))
		cKierunek = -1;

//	if (((*cStanPoprzedni == 0) && (cStanBieżący == 0)) || ((*cStanPoprzedni == 1) && (cStanBieżący == 1)) || ((*cStanPoprzedni == 2) && (cStanBieżący == 2)) || ((*cStanPoprzedni == 3) && (cStanBieżący == 3)))
	//	cKierunek = cBieżącyKierunek * -1;

	return cKierunek;
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja określa kierunek obrotu enkodera na podstawie sekwencji aktywacji czujników
// Parametry:
//  cIndeksCzujnika - indeks aktywowanego czujnika
//  *stEnkoder - wskaźnik na stan poprzedni enkodera
// Zwraca: kierunek obrotu: +1 lub -1
////////////////////////////////////////////////////////////////////////////////
void SprawdźKierunekCzujników(uint8_t cIndeksCzujnika, stEnkoder_t *stEnkoder)
{
	//jeżeli dane wejscie jest aktuwowane dwukrotnie to oznacza zmianę kierunku
	if (cIndeksCzujnika == stEnkoder->cOstatnioAktywnyCzujnik)
	{
		if ((cIndeksCzujnika == 0) || (cIndeksCzujnika == 1))
			stEnkoder->cKierunek[0] *= -1;
		if ((cIndeksCzujnika == 2) || (cIndeksCzujnika == 3))
			stEnkoder->cKierunek[1] *= -1;
	}
	else	//aktywowane są kolejne czujniki
	{
		switch (cIndeksCzujnika)
		{
		case 0:	//impuls pojawił się na wejsciu 1
			if (stEnkoder->cStanCzujnika[0] == 0)
				stEnkoder->cStanCzujnika[0] = 3;
			else
				if (stEnkoder->cStanCzujnika[0] == 3)
					stEnkoder->cStanCzujnika[0] = 0;
			stEnkoder->cStanEnkodera[0] = stEnkoder->cStanCzujnika[0];
			break;

		case 1:	//impuls pojawił się na wejsciu 2
			if (stEnkoder->cStanCzujnika[1] == 1)
				stEnkoder->cStanCzujnika[1] = 2;
			else
				if (stEnkoder->cStanCzujnika[1] == 2)
					stEnkoder->cStanCzujnika[1] = 1;
			stEnkoder->cStanEnkodera[0] = stEnkoder->cStanCzujnika[1];
			break;
		}
	}


	for (uint8_t e=0; e<2; e++)	//indeksowanie po enkoderach
	{
		if (((stEnkoder->cPoprzedniStanEnkodera[e] == 0) && (stEnkoder->cStanEnkodera[e] == 1)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 1) && (stEnkoder->cStanEnkodera[e] == 3)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 3) && (stEnkoder->cStanEnkodera[e] == 2)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 2) && (stEnkoder->cStanEnkodera[e] == 0)))
			stEnkoder->cKierunek[e] = 1;

		if (((stEnkoder->cPoprzedniStanEnkodera[e] == 1) && (stEnkoder->cStanEnkodera[e] == 0)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 0) && (stEnkoder->cStanEnkodera[e] == 2)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 2) && (stEnkoder->cStanEnkodera[e] == 3)) ||
			((stEnkoder->cPoprzedniStanEnkodera[e] == 3) && (stEnkoder->cStanEnkodera[e] == 1)))
			stEnkoder->cKierunek[e] = -1;

		stEnkoder->cPoprzedniStanEnkodera[e] = stEnkoder->cStanEnkodera[e];
	}



	stEnkoder->cOstatnioAktywnyCzujnik = cIndeksCzujnika;
}
