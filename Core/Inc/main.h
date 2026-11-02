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
typedef struct
{
	uint32_t nCzas1;
	uint32_t nCzas2;
	uint32_t nCzas3;
	uint32_t nCzas4;
	float fPrędkość1;
	float fPrędkość2;
	float fPrędkość3;
	float fPrędkość4;
	float fKalmanDroga;
	float fKalmanPredkość;
	float fKalmanPrzyspieszenie;
	float fBłądDrogi1;
	float fBłądDrogi2;
	float fBłądDrogi3;
	float fBłądDrogi4;
	uint8_t cLiczbaPomiarów;
} stPomiary_t;
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
#define ROZMIAR_BUF_NAPISOW		150
#define ROZMIAR_BUF_POMIAROW	200
#define LICZBA_CZUJNIKOW	4
#define	DROGA_CALKOWITA		9500U	//długość bieżni wyrzutni [mm]
#define	DROGA_ROZPEDZANIA	7500U	//odcinek wyrzutni na której rozpędza się BSP [mm]
#define OBWOD_ROLKI			200U	//droga jednego obrotu rolki [mm]
#define PRZYSP_STARTOWE		13.0f	//12-15 m/s^2, impuls startowy do 20 m/s^2
#define PRZYSP_HAMOWANIA	0.0f
#define CZUJNIKOW_NA_ROLCE	1U
#define LICZBA_POMIAROW		(DROGA_CALKOWITA / OBWOD_ROLKI * CZUJNIKOW_NA_ROLCE)
#define SKOK_X_WYKRESU		(DISP_X_SIZE / LICZBA_POMIAROW)
#define MASA_BSP			78.0f	//masa BSP [kg]
#define SILA_TLOKA			(MASA_BSP * PRZYSP_STARTOWE)	// Zakładam siłę napędzajacą jako iloczyn masy BSP i zmierzonego przyspieszenia [N]
#define OKRES_PETLI			100	//czas obiegu pętli głównej programu [us]

//znaczenia bitów zmiennej cStatusPomiaru
#define SP_ROZPOCZETY		0x01	//pomiar rozpoczęty - timer został uruchomiony i czeka na pomiar
#define SP_TRWA				0x02	//pojawiły się impulsy, które rozpoczeły pomiar, bez nich nie uruchomi się timeout kończący
#define SP_NOWY_POMIAR		0x04	//pojawił się nowy impuls, więc resetuj timeout i czyść bit przy każdym resecie. Ciągły strumień bitów wydłuża pomiar
#define SP_GOTOWY			0x08	//wystąpił kilkusekundowy timeout po tym jak zaczęły przychodzić impulsy
#define SP_POMIAR_ROLKI1	0x10	//pojawił się nowy impuls od rolki 1
#define SP_POMIAR_ROLKI2	0x20	//pojawił się nowy impuls od rolki 2
#define SP_POMIAR_ROLKI3	0x30	//pojawił się nowy impuls od rolki 3
#define SP_POMIAR_ROLKI4	0x80	//pojawił się nowy impuls od rolki 4
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
