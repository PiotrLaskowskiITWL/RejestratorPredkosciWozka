/*
 * Wyrzutnia.h
 *
 *  Created on: 9 paź 2026
 *      Author: Piotr
 */
#ifndef INC_WYRZUTNIA_H_
#define INC_WYRZUTNIA_H_
#include "stm32u3xx_hal.h"


//#define OKRES_PETLI			100	//czas obiegu pętli głównej programu [us]

#define KSTAN	7	//rozmiar wektora stanu: droga, prędkość, przyspieszenie i błedy prędkości 4 rolek
#define KPOM	2	//rozmiar wektora pomiaru: droga i prędkość

#define WARIANCJA_ZRYWUL	5.0e-1f		//
#define WARIANCJA_POSLIZGU 	7.0e-9f;

uint8_t SymulujPrędkościRolek(float *fDrogaRolki1, float *fDrogaRolki2, float *fVRolki1, float *fVRolki2, float *fCzas1, float *fCzas2);
uint8_t InicjujFiltrKalmanaPrędkościRolek7X2Z(void);
uint8_t PredykcjaFiltraKalmanaPrędkościRolek7X2Z(float fDeltaCzasu);
uint8_t AktulizacjaRolką1FiltraKalmanaPrędkości7X2Z(float fDroga, float fPredkość);
uint8_t AktulizacjaRolką2FiltraKalmanaPrędkości7X2Z(float fDroga, float fPredkość);
uint8_t AktulizacjaRolką3FiltraKalmanaPrędkości7X2Z(float fDroga, float fPredkość);
uint8_t AktulizacjaRolką4FiltraKalmanaPrędkości7X2Z(float fDroga, float fPredkość);


#endif /* INC_WYRZUTNIA_H_ */
