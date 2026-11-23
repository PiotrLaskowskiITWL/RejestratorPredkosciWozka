/*
 * Wyrzutnia.h
 *
 *  Created on: 9 paź 2026
 *      Author: Piotr
 */
#ifndef INC_WYRZUTNIA_H_
#define INC_WYRZUTNIA_H_
#include "stm32u3xx_hal.h"



#define KSTAN	7	//rozmiar wektora stanu: droga, prędkość, przyspieszenie i błędy prędkości 4 rolek
#define KPOM	3	//rozmiar wektora pomiaru: droga i prędkość

//#define WARIANCJA_ZRYWUL	1000.0f		//określa jak szybko może zmieniać się zryw, czyli pochodna przyspieszenia. To determinuje dynamikę modelu
#define WARIANCJA_ZRYWU									1.0e9//określa jak szybko może zmieniać się zryw, czyli pochodna przyspieszenia. To determinuje dynamikę modelu
#define POCZATKOWA_WARIANCJA_PREDKCJI_DROGI				0.03f	//zakładam wartość: (obwód rolki / liczba magnesów)^2
#define POCZATKOWA_WARIANCJA_PREDKCJI_PREDKOSCI			0.05f	//zakładam: ((obwód rolki / liczba magnesów) / (4ms / liczba magnesów))^2
#define POCZATKOWA_WARIANCJA_PREDKCJI_PRZYSPIESZENIA	0.3f
#define POCZATKOWA_WARIANCJA_POSLIZGU					0.1f
#define WARIANCJA_POMIARU_DROGI 						0.03f;	//wariancja statycznego pomiaru drogi rolki 1 [m^2]
#define WARIANCJA_POMIARU_PREDKOSCI 					0.05f;	//wariancja statycznego pomiaru prędkości rolki 1 [m^2/s^2]
#define WARIANCJA_POMIARU_PRZYSPIESZENIA				0.30f;
#define WARIANCJA_POSLIZGU 								0.12f		//okresla jak szybko może zmieniać się poślizg (zmiana drogi) rolek
#define WARIANCJA_PREDKOSCI	2.5f		//służy do pomiaru jakości pojedynczego pomiaru metodą Mahalanobisa

uint8_t SymulujPrędkościRolek(float *fDrogaRolki1, float *fDrogaRolki2, float *fVRolki1, float *fVRolki2, float *fCzas1, float *fCzas2);
uint8_t InicjujFiltrKalmanaPrędkościRolek7X3Z(void);
uint8_t PredykcjaFiltraKalmanaPrędkościRolek7X3Z(float fDeltaCzasu);
uint8_t AktulizacjaCzujnikiem1FiltraKalmanaPrędkości7X3Z(float fDroga, float fPredkość, float fPrzyspieszenie);
uint8_t AktulizacjaCzujnikiem2FiltraKalmanaPrędkości7X3Z(float fDroga, float fPredkość, float fPrzyspieszenie);
uint8_t AktulizacjaCzujnikiem3FiltraKalmanaPrędkości7X3Z(float fDroga, float fPredkość, float fPrzyspieszenie);
uint8_t AktulizacjaCzujnikiem4FiltraKalmanaPrędkości7X3Z(float fDroga, float fPredkość, float fPrzyspieszenie);
float OdległośćMahalanobisa(float fPomiar, float fEstymata, float fWariancja);

#endif /* INC_WYRZUTNIA_H_ */
