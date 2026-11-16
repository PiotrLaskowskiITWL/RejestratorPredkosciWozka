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

stEnkoder_t stEnkoder;

////////////////////////////////////////////////////////////////////////////////
// Funkcja ustawia stan poczatkowy automatu stanów enkodera
// Parametry: brak
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void InicjujEnkoder(void)
{
	stEnkoder.cKierunek[0] = 1;
	stEnkoder.cKierunek[1] = 1;
	stEnkoder.cStanCzujnika[0] = 0;
	stEnkoder.cStanCzujnika[1] = 1;
}



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
	//jeżeli dane wejscie jest aktywowane dwukrotnie to oznacza zmianę kierunku
	if (cIndeksCzujnika == stEnkoder->cOstatnioAktywnyCzujnik)
	{
		switch (cIndeksCzujnika)
		{
		case 0:	stEnkoder->cKierunek[0] *= -1;	break;	//zmiana kierunku na wejściu 1
		case 1:	stEnkoder->cKierunek[0] *= -1;	break;	//zmiana kierunku na wejściu 2
		}
	}

/*	switch (cIndeksCzujnika)
	{
	case 0:	//impuls pojawił się na wejściu 1
		if (stEnkoder->cKierunek[0] == 1)
		{
			stEnkoder->cStanCzujnika[0] = 0;
			stEnkoder->cStanCzujnika[1] = 1;
		}
		else	//przeciwny kierunek
		{
			stEnkoder->cStanCzujnika[0] = 2;
			stEnkoder->cStanCzujnika[1] = 3;
		}
		stEnkoder->cStanEnkodera[0] = stEnkoder->cStanCzujnika[0];
		break;

	case 1:	//impuls pojawił się na wejściu 2
		if (stEnkoder->cKierunek[0] == 1)
		{
			stEnkoder->cStanCzujnika[1] = 1;
			stEnkoder->cStanCzujnika[0] = 2;
		}
		else
		{
			stEnkoder->cStanCzujnika[1] = 0;
			stEnkoder->cStanCzujnika[0] = 3;
		}
		stEnkoder->cStanEnkodera[0] = stEnkoder->cStanCzujnika[1];
		break;
	}*/

/*	for (uint8_t e=0; e<2; e++)	//indeksowanie po enkoderach
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
	}*/



	stEnkoder->cOstatnioAktywnyCzujnik = cIndeksCzujnika;
}
