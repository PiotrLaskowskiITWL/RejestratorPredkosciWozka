//////////////////////////////////////////////////////////////////////////////
//
// Obsługa pomiaru czasów obrotu rolek precgwytywana timerem 2
//
// Utworzone: 28 paź 2026
// Autor: Piotr
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "Pomiar.h"
#include "Czas.h"
#include "KodyBledow.h"


extern TIM_HandleTypeDef htim2;
extern stPomiary_t stPomiary[ROZMIAR_BUF_POMIAROW];
uint8_t cIndeksPomiaru[4];
uint8_t cStatusPomiaru = SP_PIERWSZY_POMIAR;
uint32_t nPomiar, nPoprzedniPomiar[4];

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
	for (uint8_t n=0; n<ROZMIAR_BUF_POMIAROW; n++)
	{
		stPomiary[n].nCzas1 = 0;
		stPomiary[n].nCzas2 = 0;
		stPomiary[n].nCzas3 = 0;
		stPomiary[n].nCzas4 = 0;
	}

	for (uint8_t n=0; n<4; n++)
	{
		cIndeksPomiaru[n] = 0;
		nPoprzedniPomiar[n] = 0;
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
	if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
	{
		nPomiar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		if (cIndeksPomiaru[0] > 0)
			stPomiary[cIndeksPomiaru[0]].nCzas1 = MinalCzas2(nPoprzedniPomiar[0], nPomiar);
		nPoprzedniPomiar[0] = nPomiar;
		cIndeksPomiaru[0]++;
		if (cIndeksPomiaru[0] == ROZMIAR_BUF_POMIAROW)
		{
			HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_1);
			cStatusPomiaru |= SP_GOTOWY;
		}
	}
	if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2))
	{
		nPomiar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		if (cIndeksPomiaru[1] > 0)
			stPomiary[cIndeksPomiaru[1]].nCzas2 = MinalCzas2(nPoprzedniPomiar[1], nPomiar);
		nPoprzedniPomiar[1] = nPomiar;
		cIndeksPomiaru[1]++;
		if (cIndeksPomiaru[1] == ROZMIAR_BUF_POMIAROW)
		{
			HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_2);
			cStatusPomiaru |= SP_GOTOWY;
		}
	}
	if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3))
	{
		nPomiar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		if (cIndeksPomiaru[2] > 0)
			stPomiary[cIndeksPomiaru[2]].nCzas3 = MinalCzas2(nPoprzedniPomiar[2], nPomiar);
		nPoprzedniPomiar[2] = nPomiar;
		cIndeksPomiaru[2]++;
		if (cIndeksPomiaru[2] == ROZMIAR_BUF_POMIAROW)
		{
			HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_3);
			cStatusPomiaru |= SP_GOTOWY;
		}
	}
	if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4))
	{
		nPomiar = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		if (cIndeksPomiaru[3] > 0)
			stPomiary[cIndeksPomiaru[3]].nCzas4 = MinalCzas2(nPoprzedniPomiar[3], nPomiar);
		nPoprzedniPomiar[3] = nPomiar;
		cIndeksPomiaru[3]++;
		if (cIndeksPomiaru[3] == ROZMIAR_BUF_POMIAROW)
		{
			HAL_TIM_IC_Stop_IT(&htim2,  TIM_CHANNEL_4);
			cStatusPomiaru |= SP_GOTOWY;
		}
	}
	cStatusPomiaru |= SP_TRWA | SP_NOWY_POMIAR;
}
