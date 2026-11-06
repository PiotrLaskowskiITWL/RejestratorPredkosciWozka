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
extern stPomiary_t stPomiary[ROZMIAR_BUF_POMIAROW];
uint16_t sIndeksPomiaru[LICZBA_CZUJNIKOW];
uint8_t cJestPoprzedniPomiar[LICZBA_CZUJNIKOW];
uint8_t cStatusPomiaru;
uint32_t nBieżącyStanZegara, nPoprzedniStanZegara[LICZBA_CZUJNIKOW];
extern uint32_t nCzasPoczatkuPomiaru;



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

	//HAL_TIMEx_HallSensor_Start_IT(htim2);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_1);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_2);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_3);
	cBłąd |= HAL_TIM_IC_Start_IT(&htim2,  TIM_CHANNEL_4);

	//wyczyść dane
	for (uint16_t n=0; n<ROZMIAR_BUF_POMIAROW; n++)
	{
		stPomiary[n].nCzas1 = 0;
		stPomiary[n].nCzas2 = 0;
		stPomiary[n].nCzas3 = 0;
		stPomiary[n].nCzas4 = 0;
		stPomiary[n].fKalmanDroga = 0.0f;
		stPomiary[n].fKalmanPredkość = 0.0f;
		stPomiary[n].fKalmanPrzyspieszenie = 0.0f;
		stPomiary[n].fBłądDrogi1 = 0.0f;
		stPomiary[n].fBłądDrogi2 = 0.0f;
		stPomiary[n].fBłądDrogi3 = 0.0f;
		stPomiary[n].fBłądDrogi4 = 0.0f;
	}

	for (uint8_t n=0; n<LICZBA_CZUJNIKOW; n++)
	{
		sIndeksPomiaru[n] = 0;
		nPoprzedniStanZegara[n] = 0;
		cJestPoprzedniPomiar[n] = 0;
	}
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
			nBieżącyStanZegara = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			//if (nPoprzedniPomiar[0] > 0)
			if (cJestPoprzedniPomiar[0])
			{
				stPomiary[sIndeksPomiaru[0]].nCzas1 = MinalCzas2(nPoprzedniStanZegara[0], nBieżącyStanZegara);
				cStatusPomiaru |= SP_POMIAR_ROLKI1;			//pojawił się nowy impuls od rolki 1
				sIndeksPomiaru[0]++;
			}
			nPoprzedniStanZegara[0] = nBieżącyStanZegara;
			cJestPoprzedniPomiar[0] = 1;
			if (sIndeksPomiaru[0] == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_1);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			nBieżącyStanZegara = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			//if (nPoprzedniPomiar[1] > 0)
			if (cJestPoprzedniPomiar[1])
			{
				stPomiary[sIndeksPomiaru[1]].nCzas2 = MinalCzas2(nPoprzedniStanZegara[1], nBieżącyStanZegara);
				cStatusPomiaru |= SP_POMIAR_ROLKI2;		//pojawił się nowy impuls od rolki 2
				sIndeksPomiaru[1]++;
			}
			nPoprzedniStanZegara[1] = nBieżącyStanZegara;
			cJestPoprzedniPomiar[1] = 1;
			if (sIndeksPomiaru[1] == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_2);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{
			nBieżącyStanZegara = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			//if (nPoprzedniPomiar[2] > 0)
			if (cJestPoprzedniPomiar[2])
			{
				stPomiary[sIndeksPomiaru[2]].nCzas3 = MinalCzas2(nPoprzedniStanZegara[2], nBieżącyStanZegara);
				cStatusPomiaru |= SP_POMIAR_ROLKI3;		//pojawił się nowy impuls od rolki 3
				sIndeksPomiaru[2]++;
			}
			nPoprzedniStanZegara[2] = nBieżącyStanZegara;
			cJestPoprzedniPomiar[2] = 1;
			if (sIndeksPomiaru[2] == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_3);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
		{
			nBieżącyStanZegara = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
			//if (nPoprzedniPomiar[3] > 0)
			if (cJestPoprzedniPomiar[3])
			{
				stPomiary[sIndeksPomiaru[3]].nCzas4 = MinalCzas2(nPoprzedniStanZegara[3], nBieżącyStanZegara);
				cStatusPomiaru |= SP_POMIAR_ROLKI4;		//pojawił się nowy impuls od rolki 4
				sIndeksPomiaru[3]++;
			}
			nPoprzedniStanZegara[3] = nBieżącyStanZegara;
			cJestPoprzedniPomiar[3] = 1;
			if (sIndeksPomiaru[3] == ROZMIAR_BUF_POMIAROW)
			{
				HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_4);
				cStatusPomiaru |= SP_GOTOWY;
			}
		}

		//dowolny pomiar resetuje timeout kończący pomiar oraz ustawia flagi nowych danych
		cStatusPomiaru |= SP_NOWY_POMIAR | SP_TRWA;
		nCzasPoczatkuPomiaru = PobierzCzasT7();
	}
}
