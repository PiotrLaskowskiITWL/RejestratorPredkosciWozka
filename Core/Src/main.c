/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "KartaSD.h"
#include "KodyBledow.h"
#include "Bluetooth.h"
#include "Czas.h"
#include "Pomiar.h"
#include "Kalman.h"
#include <arm_math.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RNG_HandleTypeDef hrng;

SD_HandleTypeDef hsd1;

SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_DRD_FS;

/* USER CODE BEGIN PV */
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
extern const uint8_t logo_ITWL[];
extern const int logo_ITWL_width;
extern const int logo_ITWL_height;
char cBuforNapisów[ROZMIAR_BUF_NAPISOW];	//zmienna do drukowania napisów funkcją sprintf()
stPomiary_t stPomiary[ROZMIAR_BUF_POMIAROW];
uint8_t cBłąd;
uint8_t cLiczbaPomiarów;
extern uint8_t cStatusPomiaru;
extern uint8_t cIndeksPomiaru[LICZBA_CZUJNIKOW];
uint8_t cLicznikOczekiwania;
uint32_t nCzas, nCzasPoczatkuPomiaru, nCzasStart, nCzasStop;
uint32_t nCzasPetliGłównej, nPoprzedniCzasPetliGłównej;
float fPrędkośćMax;
extern float32_t fX[KSTAN];						//wektor stanu: 0=droga, 1=prędkość, 2=przyspieszenie, 3..6=błąd drogi rolek 1..4
extern uint8_t cLicznikPomiarow;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ICACHE_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_RNG_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ICACHE_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_RNG_Init();
  MX_SDMMC1_SD_Init();
  MX_FileX_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  InicjujPomiarTimerem();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //Podczas uruchomienia wyświetl logo
    RysujBitmape((OLED_SZEROKOSC - logo_ITWL_width)/2+8, 0, logo_ITWL_width, logo_ITWL_height, (uint8_t*)logo_ITWL);
	OLED_RysujNapis("WS-150", 0, 54, &Font_7x10);

	sprintf(cBuforNapisów, "v.%d", NUMER_WERSJI_REPO);
	OLED_RysujNapis(cBuforNapisów, 100, 54, &Font_7x10);

    OLED_OdswiezEkran();
	CzytajIndeksPomiaruZKarty(&cLicznikPomiarow);	//odczytaj indeks ostatnio zapisywanego pomiaru, po to aby bez względu na wyłączanie zasilnia kontynuować numeracje i nie nadpisywać swieżych pomiarów
    OLED_CzyśćBuforEkranu();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint8_t cPozX, cPozY;
	  float fWspSkaliY = (float)OLED_WYSOKOSC / 20.0f;	//tyle m/s na wysokości ekranu
	  float fWspSkaliX = (float)OLED_SZEROKOSC / LICZBA_POMIAROW;
	  float fDroga = 0.0f;
	  uint8_t cIndeks;

	  /*/symuluj strzał z wyrzutni aby wygenerować czasy obrotu 4 rolek
	  for (uint16_t n=0; n<LICZBA_POMIAROW; n++)
	  {
		  fPrzyrostPredkości = sqrtf(PRZYSP_STARTOWE * OBWOD_ROLKI / 1000);	//w każdym obiegu zwiększaj prędkość o zadane przyspieszenie
		  HAL_RNG_GenerateRandomNumber(&hrng, &nLiczbaLosowa);	//losuj liczbę błędu pomiaru
		  fPredkośćRolki1 += fPrzyrostPredkości + ((nLiczbaLosowa & 0xFF) / 800.0f) - 128/800;	//dodaj szum rozłożony równomiernie względem zera
		  stPomiary[n].nCzas1 = ((float)OBWOD_ROLKI / 1000) / fPredkośćRolki1 * 1000000;	//czas w mikrosekundach

		  HAL_RNG_GenerateRandomNumber(&hrng, &nLiczbaLosowa);	//losuj liczbę błędu pomiaru
		  fPredkośćRolki2 += fPrzyrostPredkości + ((nLiczbaLosowa & 0xFF) / 800.0f) - 128/800;	//dodaj szum rozłożony równomiernie względem zera
		  stPomiary[n].nCzas2 = ((float)OBWOD_ROLKI / 1000) / fPredkośćRolki2 * 1000000;	//czas w mikrosekundach

		  HAL_RNG_GenerateRandomNumber(&hrng, &nLiczbaLosowa);	//losuj liczbę błędu pomiaru
		  fPredkośćRolki3 += fPrzyrostPredkości + ((nLiczbaLosowa & 0xFF) / 800.0f) - 128/800;	//dodaj szum rozłożony równomiernie względem zera
		  stPomiary[n].nCzas3 = ((float)OBWOD_ROLKI / 1000) / fPredkośćRolki3 * 1000000;	//czas w mikrosekundach

		  HAL_RNG_GenerateRandomNumber(&hrng, &nLiczbaLosowa);	//losuj liczbę błędu pomiaru
		  fPredkośćRolki4 += fPrzyrostPredkości + ((nLiczbaLosowa & 0xFF) / 800.0f) - 128/800;	//dodaj szum rozłożony równomiernie względem zera
		  stPomiary[n].nCzas4 = ((float)OBWOD_ROLKI / 1000) / fPredkośćRolki4 * 1000000;	//czas w mikrosekundach
	  }*/

	  if ((cStatusPomiaru & SP_ROZPOCZETY) != SP_ROZPOCZETY)
	  {
		  InicjujFiltrKalmanaPrędkościRolek7X2Z();
		  cBłąd = RozpocznijPomiarTimerem();
		  nCzasPoczatkuPomiaru = PobierzCzasT7();
		  cStatusPomiaru |= SP_ROZPOCZETY;
		  cLiczbaPomiarów = 0;
		  fPrędkośćMax = 0.0f;
	  }

	  nCzas = PobierzCzasT7();
	  //Pojawił się piewszy impuls pomiaru
	  if (cStatusPomiaru & SP_TRWA)
	  {
		  nCzasPetliGłównej = MinalCzas2(nPoprzedniCzasPetliGłównej, nCzas);
		  PredykcjaFiltraKalmanaPrędkościRolek7X2Z((float)nCzasPetliGłównej / 1000000);
	  }
	  nPoprzedniCzasPetliGłównej = nCzas;

	//oblicz prędkość ostatniego pomiaru dla każdego czujnika
	if (cStatusPomiaru & SP_POMIAR_ROLKI1)
	{
		cStatusPomiaru &= ~SP_POMIAR_ROLKI1;
		cIndeks = cIndeksPomiaru[0] - 1;	//cIndeksPomiaru[] jest aktualizowany w przerwaniu CC timera i wskazuje na następny pomiar
		if (stPomiary[cIndeks].nCzas1)
		{
			stPomiary[cIndeks].fPrędkość1 = ((float)OBWOD_ROLKI / 1000) / ((float)stPomiary[cIndeks].nCzas1 / 1000000);	//średnia prędkość na bieżącym odcinku
			fDroga = (cIndeks + 1) * (float)OBWOD_ROLKI / 1000;	//taką drogę całkowitą w [m] pokonał wózek mierzony rolką 1
			AktulizacjaRolką1FiltraKalmanaPrędkości7X2Z(fDroga, stPomiary[cIndeks].fPrędkość1);
			stPomiary[cIndeks].fKalmanDroga = fX[0];
			stPomiary[cIndeks].fKalmanPredkość = fX[1];
			stPomiary[cIndeks].fKalmanPrzyspieszenie = fX[2];
			stPomiary[cIndeks].fBłądDrogi1 = fX[3];

			//znajdź maksymalną prędkość
			if (fPrędkośćMax < stPomiary[cIndeks].fKalmanPredkość)
				fPrędkośćMax = stPomiary[cIndeks].fKalmanPredkość;
		}
		else
			stPomiary[cIndeks].fPrędkość1 = 0.0f;
	}

	if (cStatusPomiaru & SP_POMIAR_ROLKI2)
	{
		cStatusPomiaru &= ~SP_POMIAR_ROLKI2;
		cIndeks = cIndeksPomiaru[1] - 1;
		if (stPomiary[cIndeks].nCzas2)
		{
			stPomiary[cIndeks].fPrędkość2 = ((float)OBWOD_ROLKI / 1000) / ((float)stPomiary[cIndeks].nCzas2 / 1000000);	//średnia prędkość na bieżącym odcinku
			fDroga = (cIndeks + 1) * (float)OBWOD_ROLKI / 1000;	//taką drogę całkowitą w [m] pokonał wózek mierzony rolką 2
			AktulizacjaRolką2FiltraKalmanaPrędkości7X2Z(fDroga, stPomiary[cIndeks].fPrędkość2);
			stPomiary[cIndeks].fKalmanDroga = fX[0];
			stPomiary[cIndeks].fKalmanPredkość = fX[1];
			stPomiary[cIndeks].fKalmanPrzyspieszenie = fX[2];
			stPomiary[cIndeks].fBłądDrogi2 = fX[4];

			//znajdź maksymalną prędkość
			if (fPrędkośćMax < stPomiary[cIndeks].fKalmanPredkość)
				fPrędkośćMax = stPomiary[cIndeks].fKalmanPredkość;
		}
		else
			stPomiary[cIndeks].fPrędkość2 = 0.0f;
	}

	if (cStatusPomiaru & SP_POMIAR_ROLKI3)
	{
		cStatusPomiaru &= ~SP_POMIAR_ROLKI3;
		cIndeks = cIndeksPomiaru[2] - 1;
		if (stPomiary[cIndeks].nCzas3)
		{
			stPomiary[cIndeks].fPrędkość3 = ((float)OBWOD_ROLKI / 1000) / ((float)stPomiary[cIndeks].nCzas3 / 1000000);	//średnia prędkość na bieżącym odcinku
			fDroga = (cIndeks + 1) * (float)OBWOD_ROLKI / 1000;	//taką drogę całkowitą w [m] pokonał wózek mierzony rolką 2
			AktulizacjaRolką3FiltraKalmanaPrędkości7X2Z(fDroga, stPomiary[cIndeks].fPrędkość3);
			stPomiary[cIndeks].fKalmanDroga = fX[0];
			stPomiary[cIndeks].fKalmanPredkość = fX[1];
			stPomiary[cIndeks].fKalmanPrzyspieszenie = fX[2];
			stPomiary[cIndeks].fBłądDrogi3 = fX[5];

			//znajdź maksymalną prędkość
			if (fPrędkośćMax < stPomiary[cIndeks].fKalmanPredkość)
				fPrędkośćMax = stPomiary[cIndeks].fKalmanPredkość;
		}
		else
			stPomiary[cIndeks].fPrędkość3 = 0.0f;
	}

	if (cStatusPomiaru & SP_POMIAR_ROLKI4)
	{
		cStatusPomiaru &= ~SP_POMIAR_ROLKI4;
		cIndeks = cIndeksPomiaru[3] - 1;
		if (stPomiary[cIndeks].nCzas4)
		{
			stPomiary[cIndeks].fPrędkość4 = ((float)OBWOD_ROLKI / 1000) / ((float)stPomiary[cIndeks].nCzas4 / 1000000);	//średnia prędkość na bieżącym odcinku
			fDroga = (cIndeks + 1) * (float)OBWOD_ROLKI / 1000;	//taką drogę całkowitą w [m] pokonał wózek mierzony rolką 2
			AktulizacjaRolką4FiltraKalmanaPrędkości7X2Z(fDroga, stPomiary[cIndeks].fPrędkość4);
			stPomiary[cIndeks].fKalmanDroga = fX[0];
			stPomiary[cIndeks].fKalmanPredkość = fX[1];
			stPomiary[cIndeks].fKalmanPrzyspieszenie = fX[2];
			stPomiary[cIndeks].fBłądDrogi4 = fX[6];

			//znajdź maksymalną prędkość
			if (fPrędkośćMax < stPomiary[cIndeks].fKalmanPredkość)
				fPrędkośćMax = stPomiary[cIndeks].fKalmanPredkość;
		}
		else
			stPomiary[cIndeks].fPrędkość4 = 0.0f;
	}

	  //przyszły nowe dane, więc przerysuj wykres
	  if (cStatusPomiaru & SP_NOWY_POMIAR)
	  {
		  //oblicz liczbę pomiarów
		  uint32_t nMinCzas = 0xFFFFFFFF;
		  for (uint8_t n=0; n<LICZBA_CZUJNIKOW; n++)
		  {
			  if (cIndeksPomiaru[n] > cLiczbaPomiarów)
				  cLiczbaPomiarów = cIndeksPomiaru[n];
		  }
		  fWspSkaliX = (float)OLED_SZEROKOSC / cLiczbaPomiarów;	//skaluj szerokość wykresu do bieżącej liczby pomiarów

		  //znajdź najmniejszy czas aby obliczyć współczynnik skalowania wykresu w Y
		  for (uint8_t n=0; n<cLiczbaPomiarów; n++)
		  {
			 if (stPomiary[n].nCzas1 && (stPomiary[n].nCzas1 < nMinCzas))
				 nMinCzas = stPomiary[n].nCzas1;
			 if (stPomiary[n].nCzas2 && (stPomiary[n].nCzas2 < nMinCzas))
				 nMinCzas = stPomiary[n].nCzas2;
			 if (stPomiary[n].nCzas3 && (stPomiary[n].nCzas3 < nMinCzas))
				 nMinCzas = stPomiary[n].nCzas3;
			 if (stPomiary[n].nCzas4 && (stPomiary[n].nCzas4 < nMinCzas))
				 nMinCzas = stPomiary[n].nCzas4;
		  }
		  fWspSkaliY = (float)OLED_WYSOKOSC / (((float)OBWOD_ROLKI / 1000) / ((float)nMinCzas / 1000000));	//tyle m/s na wysokości ekranu

		  OLED_CzyśćBuforEkranu();
		  //Jako pierwszy wyświetla napis aby można było na nim rysować wykresy
		  sprintf(cBuforNapisów, "V  =%.2f  ", fPrędkośćMax);
		  OLED_RysujNapis(cBuforNapisów, 20, 45, &Font_11x18);
		  OLED_RysujNapis("max", 31, 45+18-10, &Font_7x10);		//przesunięcie w Y aby wyrównać do dołu
		  OLED_RysujNapis("m", 120, 45-3, &Font_7x10);
		  OLED_RysujNapis("s", 120, 45+8, &Font_7x10);
		  OLED_RysujLiniePoziomą(120, 45+7, 7);				//kreska ułamka jednostki [m/s]

		  //rysuj wykresy prędkości wózka ze wszystkich 4 rolek
		  for (uint8_t x=0; x<cLiczbaPomiarów; x++)
		  {
			  cPozX = x * fWspSkaliX;
			  cPozY = OLED_WYSOKOSC - stPomiary[x].fPrędkość1 * fWspSkaliY;
			  OLED_RysujPiksel(cPozX, cPozY, 1);
			  cPozY = OLED_WYSOKOSC - stPomiary[x].fPrędkość2 * fWspSkaliY;
			  OLED_RysujPiksel(cPozX, cPozY, 1);
			  cPozY = OLED_WYSOKOSC - stPomiary[x].fPrędkość3 * fWspSkaliY;
			  OLED_RysujPiksel(cPozX, cPozY, 1);
			  cPozY = OLED_WYSOKOSC - stPomiary[x].fPrędkość4 * fWspSkaliY;
			  OLED_RysujPiksel(cPozX, cPozY, 1);
		  }
		  OLED_OdswiezEkran();
	  }

	  //koniec pomiaru, zapisz na kartę i wyślij telemetrię
	  if (cStatusPomiaru & SP_GOTOWY)
	  {
		  cBłąd = WyślijTelemetrieBluetooth(stPomiary);
		  if (cBłąd == BLAD_OK)
		  {
			  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);	//błyśnij LED gdy poprawnie wysłał telemetrię
			  HAL_Delay(100);
		  }
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);	//niski poziom gasi LED
		  HAL_Delay(100);


		  cBłąd = ZapiszPomiarNaKarte(stPomiary);
		  if (cBłąd == BLAD_OK)
		  {
			  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);	////błyśnij LED gdy poprawnie zapisał dane na kartę
			  HAL_Delay(100);
		  }
		  cBłąd = ZapiszIndeksPomiaruNaKarte(cLicznikPomiarow);
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);	//niski poziom gasi LED
		  HAL_Delay(100);
		  cStatusPomiaru = 0;	//kasuj wszystkie flagi
		  cLicznikOczekiwania = 0;
	  }
	  else	//cStatusPomiaru nie jest SP_GOTOWY. Sprawdź czy wystąpił timeout
	  {
		  nCzas = MinalCzas(nCzasPoczatkuPomiaru);
		  if ((nCzas > 5000000) && (cStatusPomiaru & SP_ROZPOCZETY)  && (cStatusPomiaru & SP_TRWA))	//timeout 5 sekund
		  {
			  cBłąd = ZakończPomiarTimerem();
			  cStatusPomiaru |= SP_GOTOWY;
		  }
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the CRS clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the common peripherals clock
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SYSTICK;
  PeriphClkInit.SystickClockSelection = RCC_SYSTICKCLKSOURCE_HCLK_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */
	if (CzyKartaObecna())		//jeżeli nie ma karty to pomiń inicjalizację
		return;
  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 0x7;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi3.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi3.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP2_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(&hspi3, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 23;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 23;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */
  HAL_TIM_Base_Start_IT(&htim7);		//rozpocznij liczenie czasu w trybie z przerwaniami
  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_DRD_FS.Instance = USB_DRD_FS;
  hpcd_USB_DRD_FS.Init.dev_endpoints = 8;
  hpcd_USB_DRD_FS.Init.speed = USBD_FS_SPEED;
  hpcd_USB_DRD_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_DRD_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.battery_charging_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.bulk_doublebuffer_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.iso_singlebuffer_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_DRD_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D_C_GPIO_Port, D_C_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CD_Pin */
  GPIO_InitStruct.Pin = CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : D_C_Pin */
  GPIO_InitStruct.Pin = D_C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D_C_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
