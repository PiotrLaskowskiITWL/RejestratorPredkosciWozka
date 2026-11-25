/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_4
#define LED_GPIO_Port GPIOC
#define CD_Pin GPIO_PIN_7
#define CD_GPIO_Port GPIOC
#define D_C_Pin GPIO_PIN_6
#define D_C_GPIO_Port GPIOB



/* USER CODE BEGIN Private defines */
#define NUMER_WERSJI_REPO		34		//numer wersji w repozytorium: https://github.com/PiotrLaskowskiITWL/RejestratorPredkosciWozka
#define ROZMIAR_BUF_NAPISOW		255
#define ROZMIAR_BUF_POMIAROW	3000
#define LICZBA_CZUJNIKOW		4		//liczba wejść czujnikiów
#define LICZBA_KANALOW			2		//liczba kanałów pomiarowych. Każdy kanał składa się z 2 czujników
#define MAGNESOW_NA_ROLCE		6U
#define	DROGA_CALKOWITA			9.500f	//długość bieżni wyrzutni [m]
#define	DROGA_ROZPEDZANIA		7.500f	//odcinek wyrzutni na której rozpędza się BSP [m]
#define OBWOD_ROLKI_M			0.2235f	//droga jednego obrotu rolki [m]
#define ODCINEK_ROLKI			(OBWOD_ROLKI_M / MAGNESOW_NA_ROLCE)
#define PRZYSP_STARTOWE			13.0f	//12-15 m/s^2, impuls startowy do 20 m/s^2
#define PRZYSP_HAMOWANIA		0.0f
#define LICZBA_SYMULACJI		(MAGNESOW_NA_ROLCE * (uint16_t)(DROGA_CALKOWITA / OBWOD_ROLKI_M))
#define SKOK_X_WYKRESU			(DISP_X_SIZE / LICZBA_POMIAROW)
#define MASA_BSP				78.0f	//masa BSP [kg]
#define SILA_TLOKA				(MASA_BSP * PRZYSP_STARTOWE)	// Zakładam siłę napędzajacą jako iloczyn masy BSP i zmierzonego przyspieszenia [N]
#define OKRES_PETLI				100	//czas obiegu pętli głównej programu [us]
#define LICZBA_PODZIALEK		3	//liczba podziałek dla wykresu na ekranie
#define SZEROKOSC_OPISU_PODZIALKI	19	//3 znaki po 6 pixeli
#define LICZBA_POMIAROW_DO_ZAPISU	10	//musi być przynajmniej tyle pomiarów aby zarejestrowało dane
#define MAX_PREDKOSC			100	//teoretyczna maksymalna prędkość [m/s] z jaką możemy wyobrazić sobie pracę
#define MIN_CZAS				(1000000 * OBWOD_ROLKI_M / (MAX_PREDKOSC * MAGNESOW_NA_ROLCE))	//minimalny czas jaki mozna zarejestrować [us]
#define ROLKA1					0
#define ROLKA2					1
#define WYSOKOSC_ZERA_WYKRESU	52

//znaczenia bitów zmiennej cStatusPomiaru
#define SP_UZBROJONY			0x01	//timer został uruchomiony i czeka na pomiar
#define SP_TRWA					0x02	//pojawiły się impulsy, które rozpoczeły pomiar, bez nich nie uruchomi się timeout kończący
#define SP_NOWY_POMIAR			0x04	//pojawił się nowy impuls, więc resetuj timeout i czyść bit przy każdym resecie. Ciągły strumień bitów wydłuża pomiar aż do zapełnienia zmiennej
#define SP_GOTOWY				0x08	//wystąpił timeout po tym jak zaczęły przychodzić impulsy
#define SP_POMIAR_CZUJNIKA1		0x10	//pojawił się nowy impuls od czujnika 1
#define SP_POMIAR_CZUJNIKA2		0x20	//pojawił się nowy impuls od czujnika 2
#define SP_POMIAR_CZUJNIKA3		0x40	//pojawił się nowy impuls od czujnika 3
#define SP_POMIAR_CZUJNIKA4		0x80	//pojawił się nowy impuls od czujnika 4

typedef struct
{
	uint32_t nZegar;			//32-bitowa wartość timera w chwili pomiaru
	uint8_t cAktywnyCzujnik;	//indeks czujnika, który wyzwolił pomiar
	float fDroga[LICZBA_CZUJNIKOW];		//droga widziana każdym czujnikiem
	float fPrędkość[LICZBA_CZUJNIKOW];			//prędkość obliczona dla obrotu rolki 1 [m/s]
	float fPrzyspieszenie[LICZBA_CZUJNIKOW];	//obliczone przyspieszenie
	float fKalmanDroga;			//droga wózka estymowana przez FK [m]
	float fKalmanPredkość;		//prędkość wózka estymowana przez FK [m/s]
	float fKalmanPrzyspieszenie;//przyspieszenie wózka estymowane przez FK [m/s^2]
	float fBłądDrogi[LICZBA_CZUJNIKOW];			//poślizg rolki 1 względem estymowanej drogi [m]
	float fOdlMahalanobisaV[LICZBA_CZUJNIKOW];	//odległość pomiaru prędkości czujnika 1 wyrażona w odchyleniach standardowych od jego estymaty
	int8_t cKierunekObrotów[LICZBA_KANALOW];			//kierunek -1..1 pary czujników 1 i 2
	uint8_t cAlgorytm[LICZBA_CZUJNIKOW];
} stPomiary_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
