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
#define ROZMIAR_BUF_NAPISOW		64
#define ROZMIAR_BUF_POMIAROW	120
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

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
