/*
 * Pomiar.h
 *
 *  Created on: 28 paź 2026
 *      Author: Piotr
 */

#ifndef INC_POMIAR_H_
#define INC_POMIAR_H_
#include "stm32u3xx_hal.h"
#include "main.h"

#define ZEGAR_TIMERA	24000000


uint8_t InicjujPomiarTimerem(void);
uint8_t ZakończPomiarTimerem(void);
uint8_t RozpocznijPomiarTimerem(void);


typedef struct
{
	uint32_t nZegar;							//32-bitowa wartość timera w chwili pomiaru
	uint32_t nCzas[LICZBA_CZUJNIKOW];			//zmierzony czas między impulsami
	uint8_t cAktywnyCzujnik;					//indeks czujnika, który wyzwolił pomiar
	float fDroga[LICZBA_CZUJNIKOW];				//droga widziana każdym czujnikiem
	float fPrędkośćIndywi[LICZBA_CZUJNIKOW];	//prędkość obliczona dla obrotu rolki [m/s] dla indywidualnego magnesu (każdy z osobna)
	float fPrędkośćWszyst[LICZBA_CZUJNIKOW];	//prędkość obliczona dla obrotu rolki [m/s] dla wszystkich magnesów
	float fPrzyspieszenie[LICZBA_CZUJNIKOW];	//obliczone przyspieszenie
	float fKalmanDroga;							//droga wózka estymowana przez FK [m]
	float fKalmanPredkość;						//prędkość wózka estymowana przez FK [m/s]
	float fKalmanPrzyspieszenie;				//przyspieszenie wózka estymowane przez FK [m/s^2]
	//float fBłądDrogi[LICZBA_CZUJNIKOW];		//poślizg rolki 1 względem estymowanej drogi [m]
	float fOdlMahalanobisaV[LICZBA_CZUJNIKOW];	//odległość pomiaru prędkości czujnika 1 wyrażona w odchyleniach standardowych od jego estymaty
	int8_t cKierunekObrotów[LICZBA_KANALOW];	//kierunek -1..1 pary czujników 1 i 2
	uint8_t cAlgorytm[LICZBA_CZUJNIKOW];		//rodzaj algortymu użytego do obliczeń
	uint8_t cIndeksMagnesu[LICZBA_CZUJNIKOW];	//indeks magnesu używany do obliczenia prędkosci i przyspieszenia
} stPomiary_t;


#endif /* INC_POMIAR_H_ */
