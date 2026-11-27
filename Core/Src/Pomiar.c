//////////////////////////////////////////////////////////////////////////////
//
// Obsługa pomiaru czasów obrotu rolek precgwytywana timerem 2
//
// Utworzone: 28 paź 2026
// Autor: Piotr Laskowski
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "Pomiar.h"
#include "Czas.h"
#include "main.h"
#include "KodyBledow.h"



extern TIM_HandleTypeDef htim2;
stPomiary_t __attribute__ ((aligned (32))) __attribute__((section(".SekcjaRAM3")))  stPomiary[ROZMIAR_BUF_POMIAROW];
uint16_t sIndeksPomiaru = 0;
uint8_t cStatusPomiaru;
extern uint32_t nCzasOstatniegoPomiaru;



////////////////////////////////////////////////////////////////////////////////
// Funkcja konfiguruje timer 2 do pray z przechwytywaniam impulsów z hallotronów
// Parametry: brak
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t InicjujPomiarTimerem(void)
{
	return HAL_TIM_IC_Init(&htim2);
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja rozpoczyna pomiar timerem
// Parametry: brak
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t RozpocznijPomiarTimerem(void)
{
	uint8_t cBłąd = BLAD_OK;

	//wyczyść dane
	for (uint16_t n=0; n<ROZMIAR_BUF_POMIAROW; n++)
	{
		stPomiary[n].nZegar = 0;
		stPomiary[n].fKalmanDroga = 0.0f;
		stPomiary[n].fKalmanPredkość = 0.0f;
		stPomiary[n].fKalmanPrzyspieszenie = 0.0f;
		stPomiary[n].cKierunekObrotów[0] = 0;
		stPomiary[n].cKierunekObrotów[1] = 0;
		stPomiary[n].cAktywnyCzujnik = 0;

		for (uint16_t x=0; x<LICZBA_CZUJNIKOW; x++)
		{
			//stPomiary[n].fBłądDrogi[x] = 0.0f;
			stPomiary[n].fDroga[x] = 0.0f;
			stPomiary[n].fPrędkośćIndywi[x] = 0.0f;
			stPomiary[n].fPrędkośćWszyst[x] = 0.0f;
			stPomiary[n].fPrzyspieszenie[x] = 0.0f;
			stPomiary[n].fOdlMahalanobisaV[x] = 0.0f;
			stPomiary[n].cAlgorytm[x] = 0;
		}
	}
	sIndeksPomiaru = 0;

	//Włącz pomiar timerem
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_1);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_2);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_3);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_4);
	return cBłąd;
}



////////////////////////////////////////////////////////////////////////////////
// Funkcja kończy pomiar timerem. Uruchamiana po timeoucie
// Parametry: brak
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t ZakończPomiarTimerem(void)
{
	uint8_t cBłąd = BLAD_OK;

	//HAL_TIMEx_HallSensor_Start_IT(htim2);
	cBłąd |= HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_1);
	cBłąd |= HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_2);
	cBłąd |= HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_3);
	cBłąd |= HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_4);
	return cBłąd;
}



////////////////////////////////////////////////////////////////////////////////
// Callback przechwytujący czasy aktywacji wejść przechwytujących timera
// Parametry: *htim - wskaźnik na strukturę  timera
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2)
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			stPomiary[sIndeksPomiaru].nZegar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			stPomiary[sIndeksPomiaru].cAktywnyCzujnik = SP_POMIAR_CZUJNIKA1;
			cStatusPomiaru |= SP_POMIAR_CZUJNIKA1;			//pojawił się nowy impuls od rolki 1
			sIndeksPomiaru++;
			if (sIndeksPomiaru == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_1);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			stPomiary[sIndeksPomiaru].nZegar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			stPomiary[sIndeksPomiaru].cAktywnyCzujnik = SP_POMIAR_CZUJNIKA2;
			cStatusPomiaru |= SP_POMIAR_CZUJNIKA2;		//pojawił się nowy impuls od rolki 2
			sIndeksPomiaru++;
			if (sIndeksPomiaru == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_2);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{
			stPomiary[sIndeksPomiaru].nZegar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			stPomiary[sIndeksPomiaru].cAktywnyCzujnik = SP_POMIAR_CZUJNIKA3;
			cStatusPomiaru |= SP_POMIAR_CZUJNIKA3;		//pojawił się nowy impuls od rolki 3
			sIndeksPomiaru++;
			if (sIndeksPomiaru == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_3);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
		{
			stPomiary[sIndeksPomiaru].nZegar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
			stPomiary[sIndeksPomiaru].cAktywnyCzujnik = SP_POMIAR_CZUJNIKA4;
			cStatusPomiaru |= SP_POMIAR_CZUJNIKA4;		//pojawił się nowy impuls od rolki 4
			sIndeksPomiaru++;
			if (sIndeksPomiaru == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_4);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		//dowolny pomiar resetuje timeout kończący pomiar oraz ustawia flagi nowych danych
		cStatusPomiaru |= SP_NOWY_POMIAR | SP_TRWA;
		nCzasOstatniegoPomiaru = PobierzCzasT7();
	}
}
