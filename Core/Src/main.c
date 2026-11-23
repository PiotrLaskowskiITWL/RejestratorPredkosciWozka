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

extern FontDef Font_6x8;
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
extern const uint8_t logo_ITWL[];
extern const int logo_ITWL_width;
extern const int logo_ITWL_height;
char cBuforNapisów[ROZMIAR_BUF_NAPISOW];	//zmienna do drukowania napisów funkcją sprintf()
stPomiary_t stPomiary[ROZMIAR_BUF_POMIAROW];
uint8_t cBłąd;
uint16_t sLiczbaPomiarów = 0;
extern uint8_t cStatusPomiaru;
extern uint16_t sIndeksPomiaru;		//indeksuje kolejne pomiary
uint16_t sIndeksAnalizy;			//indeksuje analize pomiarów, gdyż możliwy jest jednoczesny pomiar wielu czujników i processing wtedy sie gubi
uint8_t cLicznikOczekiwania;
uint32_t nCzas, nCzasBiezący, nCzasPoprzedni;
uint32_t nCzasPetliGłównej, nCzasOstatniegoPomiaru;
uint32_t nT1, nT2;
float fCzas;	//czas w sekundach
float fMaxPrędkośćKalmana;
extern float32_t fX[KSTAN];						//wektor stanu: 0=droga, 1=prędkość, 2=przyspieszenie, 3..6=błąd drogi rolek 1..4
extern uint16_t sLicznikPomiarow;
float fNapięcieAku;
float fDroga[LICZBA_CZUJNIKOW] = {0.0f};
GPIO_PinState cStanWejścia;
uint16_t sIndeks = 0;
uint32_t nPoprzedniZegar[LICZBA_CZUJNIKOW];
float fPoprzedniaPrędkość[LICZBA_CZUJNIKOW];
int8_t cKierunek[LICZBA_KANALOW];
int8_t cPoprzedniKierunek[LICZBA_KANALOW];
uint8_t cPoprzedniCzujnik[LICZBA_KANALOW];
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
    RysujBitmape((OLED_SZEROKOSC - logo_ITWL_width), 0, logo_ITWL_width, logo_ITWL_height, (uint8_t*)logo_ITWL);
    OLED_RysujNapis("WS-150-3I", 0, 0, &Font_7x10);
    sprintf(cBuforNapisów, "ver.%d", NUMER_WERSJI_REPO);
    OLED_RysujNapis(cBuforNapisów, 0, 12, &Font_7x10);

    //zmierz i wyświetl napięcie akumulatora
    cBłąd = HAL_ADC_Start(&hadc1);
    cBłąd = HAL_ADC_PollForConversion(&hadc1, 100);
	uint32_t nAdcVal = HAL_ADC_GetValue(&hadc1);
	fNapięcieAku = 3.3f * nAdcVal / 4096;
	fNapięcieAku *= 113.0f / 13.0f;	//zewnętrzny dzielnik napięcia
    sprintf(cBuforNapisów, "Ub=%.1fV", fNapięcieAku);
    OLED_RysujNapis(cBuforNapisów, 0, 54, &Font_7x10);

    OLED_OdswiezEkran();
	CzytajIndeksPomiaruZKarty(&sLicznikPomiarow);	//odczytaj indeks ostatnio zapisywanego pomiaru, po to aby bez względu na wyłączanie zasilnia kontynuować numerację i nie nadpisywać swieżych pomiarów
    OLED_CzyśćBuforEkranu();
    while (1)
    {
	  /*/diagnostyka pomiaru czasu - sprawdzić po zmianie częstotliwości taktowania timera
	  for (uint8_t n=0; n<6; n++)
	  {
		  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		  nCzasStart = PobierzCzasT7();
		  do  nCzas = MinalCzas(nCzasStart);
		  while (nCzas < 100000);
	  }*/
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    	uint8_t cPozX, cPozY;
    	float fWspSkaliY;	//tyle m/s na wysokości ekranu
    	float fWspSkaliX;


    	cStanWejścia = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);	//czujnik 2 to PA1
    	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, !cStanWejścia);

    	//jeżeli układ pomiarowy nie jest uzbrojony to uzbrój go, czyli inicjuj dane wejdź w gotowość na rejestrację pomiaru
    	if ((cStatusPomiaru & SP_UZBROJONY) != SP_UZBROJONY)
    	{
    		InicjujFiltrKalmanaPrędkościRolek7X3Z();
    		cBłąd = RozpocznijPomiarTimerem();
    		cStatusPomiaru |= SP_UZBROJONY;
    		fMaxPrędkośćKalmana = 0.0f;
    		for (uint8_t n=0; n<LICZBA_CZUJNIKOW; n++)
    		{
    			fDroga[n] = 0.0f;
    			fPoprzedniaPrędkość[n] = 0;				//zerowa poprzednia prędkość jest potrzebna do policzenia pierwszego przyspieszenia
    			nPoprzedniZegar[n] = 0;
    		}
    		for (uint8_t n=0; n<LICZBA_KANALOW; n++)
    		{
    			cPoprzedniKierunek[n] = cKierunek[n] = 1;	//zakładam ża początkowy kierunek jest zawsze do przodu
    			cPoprzedniCzujnik[n] = 0;
    		}
    	}

    	nCzasBiezący = PobierzCzasT7();
    	//Pojawił się pierwszy impuls pomiaru wiec uznajemy proces za trwajacy i rusza predykcja filtra
    	if (cStatusPomiaru & SP_TRWA)
    	{
    		nCzasPetliGłównej = MinalCzas2(nCzasPoprzedni, nCzasBiezący);
    		PredykcjaFiltraKalmanaPrędkościRolek7X3Z((float)nCzasPetliGłównej / 1000000);
    	}
    	nCzasPoprzedni = nCzasBiezący;

    	//=============================================================================================================================================================
    	//Jeżeli w cyklu pomiarowym pojawił się więcej niż jeden impuls, to wykonaj dodatkowy obieg pętli dla każdego impulsu, tak aby nie tracić danych
    	if (sIndeksAnalizy < sIndeksPomiaru)
    	{
    		if (sIndeksAnalizy > 0)	//indeks pomiaru jest inkrementowany po pomiarze, więc piersze dane są dla indeksu = 1
    		{
    			sIndeks = sIndeksAnalizy - 1;	//indeks jest aktualizowany w przerwaniu CC timera i wskazuje na następny pomiar, więc trzeba odwołać się do poprzedniego elementu

    			//oblicz prędkość ostatniego pomiaru dla każdego czujnika
    			if (stPomiary[sIndeks].cAktywnyCzujnik == SP_POMIAR_CZUJNIKA1)
				{
					cStatusPomiaru &= ~SP_POMIAR_CZUJNIKA1;
					fDroga[0] += (float)cKierunek[ROLKA1] * ODCINEK_ROLKI;	//całkuj drogę jaką pokonał wózek mierzony rolką 1 [m]
					if (sIndeks > 1)	//są 2 impulsy, można okreslić prędkość
					{
						nCzas = MinalCzas2(nPoprzedniZegar[0], stPomiary[sIndeks].nZegar);
						fCzas = (float)nCzas / 1000000;
						stPomiary[sIndeks].fPrędkość[0] = (float)cKierunek[ROLKA1] * ODCINEK_ROLKI / fCzas;	//średnia prędkość na bieżącym odcinku

						if (sIndeks > 2)	//aby okreslić kierunek muszą być przechwycone co najmniej 3 impulsy
						{
							//Zmiana kierunku obrotów objawia się na dwa sposoby:
							//1)dwa kolejne impulsy wystąpiły na tym samym czujniku
							if (cPoprzedniCzujnik[ROLKA1] == SP_POMIAR_CZUJNIKA1)
							{
								cKierunek[ROLKA1] *= -1;	//zmień znak
								stPomiary[sIndeks].cAlgorytm[0] = 1;
							}
							else
							{
								//2) określenie kierunku przez porównanie czasów T1 i T2, gdzie:
								nT1 = MinalCzas2(nPoprzedniZegar[0], nPoprzedniZegar[1]);			//T1 = czas między poprzednim impulsem czujnika 1 a ostatnim impulsem czujnika 2 na tej rolce,
								nT2 = MinalCzas2(nPoprzedniZegar[1], stPomiary[sIndeks].nZegar);	//T2 = czas między ostatnim impulsem czujnika 2 a impulsem bieżącym bieżącego czujnika 1
								if (nT1 < nT2)
									cKierunek[ROLKA1] = 1;
								else
									cKierunek[ROLKA1] = -1;
								stPomiary[sIndeks].cAlgorytm[0] = 2;
							}

							if (cKierunek[ROLKA1] != cPoprzedniKierunek[ROLKA1])	//czy była zmiana kierunku
							{
								fDroga[0] -= (float)cPoprzedniKierunek[ROLKA1] * ODCINEK_ROLKI;	//przy zmianie kierunku cześć drogi (liczone przed nawiasem) jest ze znakiem + a cześć ze znakiem minus
								stPomiary[sIndeks].fPrędkość[0] = 0.0f;	//nastąpiła zmiana kierunku, więc prędkość przechodzi przez 0
							}
							cPoprzedniKierunek[ROLKA1] = cKierunek[ROLKA1];
						}
						stPomiary[sIndeks].fPrzyspieszenie[0] = (stPomiary[sIndeks].fPrędkość[0] - fPoprzedniaPrędkość[0]) / fCzas;
						fPoprzedniaPrędkość[0] = stPomiary[sIndeks].fPrędkość[0];
						AktulizacjaCzujnikiem1FiltraKalmanaPrędkości7X3Z(fDroga[0], stPomiary[sIndeks].fPrędkość[0], stPomiary[sIndeks].fPrzyspieszenie[0]);

						//znajdź maksymalną prędkość
						if (fMaxPrędkośćKalmana < fX[1])
							fMaxPrędkośćKalmana = fX[1];

						stPomiary[sIndeks].cKierunekObrotów[ROLKA1] = cKierunek[ROLKA1];
						nPoprzedniZegar[0] = stPomiary[sIndeks].nZegar;
					}
					cPoprzedniCzujnik[ROLKA1] = SP_POMIAR_CZUJNIKA1;
					stPomiary[sIndeks].fDroga[0] = fDroga[0];
				}


    			if (stPomiary[sIndeks].cAktywnyCzujnik == SP_POMIAR_CZUJNIKA2)
				{
					cStatusPomiaru &= ~SP_POMIAR_CZUJNIKA2;
					fDroga[1] += (float)cKierunek[ROLKA1] * ODCINEK_ROLKI;	//całkuj drogę jaką pokonał wózek mierzony rolką 2 [m]
					if (sIndeks > 1)	//do pomiaru prędkości musza być przynajmniej 2 impulsy
					{
						nCzas = MinalCzas2(nPoprzedniZegar[1], stPomiary[sIndeks].nZegar);
						fCzas = (float)nCzas / 1000000;
						stPomiary[sIndeks].fPrędkość[1] = (float)cKierunek[ROLKA1] * ODCINEK_ROLKI / fCzas;	//średnia prędkość na bieżącym odcinku

						if (sIndeks > 2)	//aby okreslić kierunek muszą być przechwycone co najmniej 3 impulsy
						{
							if (cPoprzedniCzujnik[ROLKA1] == SP_POMIAR_CZUJNIKA2)
							{
								cKierunek[ROLKA1] *= -1;	//zmień znak
								stPomiary[sIndeks].cAlgorytm[1] = 1;
							}
							else
							{
								//określenie kierunku przez porównanie czasów T1 i T2, gdzie:
								nT1 = MinalCzas2(nPoprzedniZegar[0], stPomiary[sIndeks].nZegar);	//T1 = czas między ostatnim impulsem czujnika 1 a impulsem bieżącym bieżącego czujnika 2
								nT2 = MinalCzas2(nPoprzedniZegar[1], nPoprzedniZegar[0]);			//T2 = czas między poprzednim impulsem czujnika 2 a ostatnim impulsem czujnika 1 na tej rolce,
								if (nT1 < nT2)
									cKierunek[ROLKA1] = 1;
								else
									cKierunek[ROLKA1] = -1;
								stPomiary[sIndeks].cAlgorytm[1] = 2;
							}

							if (cKierunek[ROLKA1] != cPoprzedniKierunek[ROLKA1])	//czy była zmiana kierunku
							{
								fDroga[1] -= (float)cPoprzedniKierunek[ROLKA1] * ODCINEK_ROLKI;	//przy zmianie kierunku cześć drogi (liczone przed nawiasem) jest ze znakiem + a cześć ze znakiem minus
								stPomiary[sIndeks].fPrędkość[1] = 0.0f;	//nastąpiła zmiana kierunku, więc prędkość przechodzi przez 0
							}
							cPoprzedniKierunek[ROLKA1] = cKierunek[ROLKA1];
						}
						stPomiary[sIndeks].fPrzyspieszenie[1] = (stPomiary[sIndeks].fPrędkość[1] - fPoprzedniaPrędkość[1]) / fCzas;
						fPoprzedniaPrędkość[1] = stPomiary[sIndeks].fPrędkość[1];
						AktulizacjaCzujnikiem2FiltraKalmanaPrędkości7X3Z(fDroga[1], stPomiary[sIndeks].fPrędkość[1], stPomiary[sIndeks].fPrzyspieszenie[1]);

						//znajdź maksymalną prędkość
						if (fMaxPrędkośćKalmana < fX[1])
							fMaxPrędkośćKalmana = fX[1];

						stPomiary[sIndeks].cKierunekObrotów[ROLKA1] = cKierunek[ROLKA1];
						nPoprzedniZegar[1] = stPomiary[sIndeks].nZegar;
					}
					cPoprzedniCzujnik[ROLKA1] = SP_POMIAR_CZUJNIKA2;
					stPomiary[sIndeks].fDroga[1] = fDroga[1];
				}


    			if (stPomiary[sIndeks].cAktywnyCzujnik == SP_POMIAR_CZUJNIKA3)
				{
					cStatusPomiaru &= ~SP_POMIAR_CZUJNIKA3;
					fDroga[2] += (float)cKierunek[ROLKA2] * ODCINEK_ROLKI;	//całkuj drogę jaką pokonał wózek mierzony rolką 1 [m]
					if (sIndeks > 1)	//do pomiaru prędkości musza być przynajmniej 2 impulsy
					{
						nCzas = MinalCzas2(nPoprzedniZegar[2], stPomiary[sIndeks].nZegar);
						fCzas = (float)nCzas / 1000000;
						stPomiary[sIndeks].fPrędkość[2] = (float)cKierunek[ROLKA2] * ODCINEK_ROLKI / fCzas;	//średnia prędkość na bieżącym odcinku

						if (sIndeks > 2)	//aby okreslić kierunek muszą być przechwycone co najmniej 3 impulsy
						{
							if (cPoprzedniCzujnik[ROLKA2] == SP_POMIAR_CZUJNIKA3)
							{
								cKierunek[ROLKA2] *= -1;	//zmień znak
								stPomiary[sIndeks].cAlgorytm[2] = 1;
							}
							else
							{
								//określenie kierunku przez porównanie czasów T1 i T2, gdzie:
								nT1 = MinalCzas2(nPoprzedniZegar[2], nPoprzedniZegar[3]);			//T1 = czas między poprzednim impulsem czujnika 3 a ostatnim impulsem czujnika 4 na tej rolce,
								nT2 = MinalCzas2(nPoprzedniZegar[3], stPomiary[sIndeks].nZegar);	//T2 = czas między ostatnim impulsem czujnika 4 a impulsem bieżącym bieżącego czujnika 3
								if (nT1 < nT2)
									cKierunek[ROLKA2] = 1;
								else
									cKierunek[ROLKA2] = -1;
								stPomiary[sIndeks].cAlgorytm[2] = 2;
							}
							if (cKierunek[ROLKA2] != cPoprzedniKierunek[ROLKA2])	//czy była zmiana kierunku
							{
								fDroga[2] -= (float)cPoprzedniKierunek[ROLKA2] * ODCINEK_ROLKI;	//przy zmianie kierunku cześć drogi (liczone przed nawiasem) jest ze znakiem + a cześć ze znakiem minus
								stPomiary[sIndeks].fPrędkość[2] = 0.0f;	//nastąpiła zmiana kierunku, więc prędkość przechodzi przez 0
							}
							cPoprzedniKierunek[ROLKA2] = cKierunek[ROLKA2];
						}
						stPomiary[sIndeks].fPrzyspieszenie[2] = (stPomiary[sIndeks].fPrędkość[2] - fPoprzedniaPrędkość[2]) / fCzas;
						fPoprzedniaPrędkość[2] = stPomiary[sIndeks].fPrędkość[2];
						AktulizacjaCzujnikiem3FiltraKalmanaPrędkości7X3Z(fDroga[2], stPomiary[sIndeks].fPrędkość[2], stPomiary[sIndeks].fPrzyspieszenie[2]);

						//znajdź maksymalną prędkość
						if (fMaxPrędkośćKalmana < fX[1])
							fMaxPrędkośćKalmana = fX[1];

						nPoprzedniZegar[2] = stPomiary[sIndeks].nZegar;
						stPomiary[sIndeks].cKierunekObrotów[ROLKA2] = cKierunek[ROLKA2];
					}
					cPoprzedniCzujnik[ROLKA2] = SP_POMIAR_CZUJNIKA3;
					stPomiary[sIndeks].fDroga[2] = fDroga[2];
				}


    			if (stPomiary[sIndeks].cAktywnyCzujnik == SP_POMIAR_CZUJNIKA4)
				{
					cStatusPomiaru &= ~SP_POMIAR_CZUJNIKA4;
					fDroga[3] += (float)cKierunek[ROLKA2] * ODCINEK_ROLKI;	//całkuj drogę jaką pokonał wózek mierzony rolką 4 [m]
					if (sIndeks > 1)	//do pomiaru prędkości musza być przynajmniej 2 impulsy
					{
						nCzas = MinalCzas2(nPoprzedniZegar[3], stPomiary[sIndeks].nZegar);
						fCzas = (float)nCzas / 1000000;
						stPomiary[sIndeks].fPrędkość[3] = (float)cKierunek[ROLKA2] * ODCINEK_ROLKI / fCzas;	//średnia prędkość na bieżącym odcinku
						if (sIndeks > 2)	//aby okreslić kierunek muszą być przechwycone co najmniej 3 impulsy
						{
							if (cPoprzedniCzujnik[ROLKA2] == SP_POMIAR_CZUJNIKA4)
							{
								cKierunek[ROLKA2] *= -1;	//zmień znak
								stPomiary[sIndeks].cAlgorytm[3] = 1;
							}
							else
							{
								//określenie kierunku przez porównanie czasów T1 i T2, gdzie:
								nT1 = MinalCzas2(nPoprzedniZegar[2], stPomiary[sIndeks].nZegar);	//T1 = czas między ostatnim impulsem czujnika 3 a impulsem bieżącym bieżącego czujnika 4
								nT2 = MinalCzas2(nPoprzedniZegar[3], nPoprzedniZegar[2]);			//T2 = czas między poprzednim impulsem czujnika 4 a ostatnim impulsem czujnika 3 na tej rolce,
								if (nT1 < nT2)
									cKierunek[ROLKA2] = 1;
								else
									cKierunek[ROLKA2] = -1;
								stPomiary[sIndeks].cAlgorytm[3] = 2;
							}

							if (cKierunek[ROLKA2] != cPoprzedniKierunek[ROLKA2])	//czy była zmiana kierunku
							{
								fDroga[3] -= (float)cPoprzedniKierunek[ROLKA2] * ODCINEK_ROLKI;	//przy zmianie kierunku cześć drogi (liczone przed nawiasem) jest ze znakiem + a cześć ze znakiem minus
								stPomiary[sIndeks].fPrędkość[3] = 0.0f;	//nastąpiła zmiana kierunku, więc prędkość przechodzi przez 0
							}
							cPoprzedniKierunek[ROLKA2] = cKierunek[ROLKA2];
						}
						stPomiary[sIndeks].fPrzyspieszenie[3] = (stPomiary[sIndeks].fPrędkość[3] - fPoprzedniaPrędkość[3]) / fCzas;
						fPoprzedniaPrędkość[3] = stPomiary[sIndeks].fPrędkość[3];
						AktulizacjaCzujnikiem4FiltraKalmanaPrędkości7X3Z(fDroga[3], stPomiary[sIndeks].fPrędkość[3], stPomiary[sIndeks].fPrzyspieszenie[3]);

						//znajdź maksymalną prędkość
						if (fMaxPrędkośćKalmana < fX[1])
							fMaxPrędkośćKalmana = fX[1];

						nPoprzedniZegar[3] = stPomiary[sIndeks].nZegar;
						stPomiary[sIndeks].cKierunekObrotów[ROLKA2] = cKierunek[ROLKA2];
					}
					cPoprzedniCzujnik[ROLKA2] = SP_POMIAR_CZUJNIKA4;
					stPomiary[sIndeks].fDroga[3] = fDroga[3];
				}

				//=============================================================================================================================================================
				//ustaw dane z filtra Kalmana. Nawet gdy nie ma pomiaru to wstaw dane z predykcji
				stPomiary[sIndeks].fKalmanDroga = fX[0];
				stPomiary[sIndeks].fKalmanPredkość = fX[1];
				stPomiary[sIndeks].fKalmanPrzyspieszenie = fX[2];
				stPomiary[sIndeks].fBłądDrogi[0] = fX[3];
				stPomiary[sIndeks].fBłądDrogi[1] = fX[4];
				stPomiary[sIndeks].fBłądDrogi[2] = fX[5];
				stPomiary[sIndeks].fBłądDrogi[3] = fX[6];

				//przyszły nowe dane, więc przerysuj wykres
				if (cStatusPomiaru & SP_NOWY_POMIAR)
				{
					//oblicz liczbę pomiarów
					float fMaxPrędkość = 0;
					sLiczbaPomiarów = sIndeksAnalizy;
					fWspSkaliX = (float)OLED_SZEROKOSC / sLiczbaPomiarów;	//skaluj szerokość wykresu do bieżącej liczby pomiarów

					//znajdź największą prędkość aby obliczyć współczynnik skalowania wykresu w Y
					for (uint16_t n=0; n<sLiczbaPomiarów; n++)
					{
						if (stPomiary[n].fPrędkość[0] > fMaxPrędkość)
							fMaxPrędkość = stPomiary[n].fPrędkość[0];
						if (stPomiary[n].fPrędkość[1] > fMaxPrędkość)
							fMaxPrędkość = stPomiary[n].fPrędkość[1];
						if (stPomiary[n].fPrędkość[2] > fMaxPrędkość)
							fMaxPrędkość = stPomiary[n].fPrędkość[2];
						if (stPomiary[n].fPrędkość[3] > fMaxPrędkość)
							fMaxPrędkość = stPomiary[n].fPrędkość[3];
						if (fMaxPrędkośćKalmana > fMaxPrędkość)
							fMaxPrędkość = fMaxPrędkośćKalmana;
					 }
					 fWspSkaliY = (float)WYSOKOSC_ZERA_WYKRESU / (fMaxPrędkość * 1.2f);	//tyle m/s na wysokości ekranu

					 OLED_CzyśćBuforEkranu();
					 //Jako pierwszy wyświetla napis aby można było na nim rysować wykresy
					 sprintf(cBuforNapisów, "V  =%.2f  ", fMaxPrędkośćKalmana);
					 OLED_RysujNapis(cBuforNapisów, 20, 45, &Font_11x18);
					 OLED_RysujNapis("max", 31, 45+18-10, &Font_7x10);		//przesunięcie w Y aby wyrównać do dołu
					 OLED_RysujNapis("m", 120, 45-3, &Font_7x10);
					 OLED_RysujNapis("s", 120, 45+8, &Font_7x10);
					 OLED_RysujLiniePoziomą(120, 45+7, 7);				//kreska ułamka jednostki [m/s]

					 //rysuj poziomą podziałkę wykresu
					 float fMaxPredkość = (float)WYSOKOSC_ZERA_WYKRESU / fWspSkaliY;
					 uint8_t cMaxPredkość = (uint8_t)(ceilf(fMaxPredkość) + 1.0f);
					 cMaxPredkość &= 0xFE;		//zamień na liczbę parzystą aby uniknąć podziału nieparzystych
					 float fSkokPodziałki = (float)cMaxPredkość / LICZBA_PODZIALEK;
					 for (uint8_t n=0; n<LICZBA_PODZIALEK; n++)
					 {
						 int16_t y = (int16_t)(fSkokPodziałki * (n+1) * fWspSkaliY);
						 if ((y > 0) && (y < OLED_WYSOKOSC))
						 {
							 OLED_RysujLiniePoziomąPrzerywaną(SZEROKOSC_OPISU_PODZIALKI, WYSOKOSC_ZERA_WYKRESU - y, OLED_SZEROKOSC - SZEROKOSC_OPISU_PODZIALKI, 3);
							 sprintf(cBuforNapisów, "%.1f", (n+1) * fSkokPodziałki);
							 OLED_RysujNapis(cBuforNapisów, 0, WYSOKOSC_ZERA_WYKRESU - y - 3, &Font_6x8);
						 }
					 }

					 //rysuj wykresy prędkości wózka ze wszystkich 4 rolek jako punkty pomiarowe
					 for (uint16_t x=0; x<sLiczbaPomiarów; x++)
					 {
						 cPozX = x * fWspSkaliX;
						 cPozY = WYSOKOSC_ZERA_WYKRESU - stPomiary[x].fPrędkość[0] * fWspSkaliY;
						 OLED_RysujPiksel(cPozX, cPozY, 1);
						 cPozY = WYSOKOSC_ZERA_WYKRESU - stPomiary[x].fPrędkość[1] * fWspSkaliY;
						 OLED_RysujPiksel(cPozX, cPozY, 1);
						 cPozY = WYSOKOSC_ZERA_WYKRESU - stPomiary[x].fPrędkość[2] * fWspSkaliY;
						 OLED_RysujPiksel(cPozX, cPozY, 1);
						 cPozY = WYSOKOSC_ZERA_WYKRESU - stPomiary[x].fPrędkość[3] * fWspSkaliY;
						 OLED_RysujPiksel(cPozX, cPozY, 1);
					 }

					 //rysuj wykres prędkości wózka z filtra Kalmana jako linię ciągłą
					 int16_t sStartX, sStartY, sStopX, sStopY;
					 sStartX = sStartY = 0;
					 for (uint16_t x=0; x<sLiczbaPomiarów; x++)
					 {
						 sStopX = x * fWspSkaliX;
						 sStopY = stPomiary[x].fKalmanPredkość * fWspSkaliY;
						 OLED_RysujLinię(sStartX, WYSOKOSC_ZERA_WYKRESU - sStartY, sStopX, WYSOKOSC_ZERA_WYKRESU - sStopY);
						 sStartX = sStopX;
						 sStartY = sStopY;
					 }

					 //rysuj kierunek obrotu
					 sprintf(cBuforNapisów, "k: %+d %+d", stPomiary[sIndeks].cKierunekObrotów[ROLKA1], stPomiary[sIndeks].cKierunekObrotów[ROLKA2]);
					 OLED_RysujNapis(cBuforNapisów, 60, 0, &Font_6x8);
					 //sprintf(cBuforNapisów, "t: %ld", nCzasPetliGłównej);
					 //OLED_RysujNapis(cBuforNapisów, 60, 15, &Font_6x8);
					 OLED_OdswiezEkran();
				}
    		}		//koniec warunku if (sIndeksAnalizy > 0)
    		if (sIndeksAnalizy < ROZMIAR_BUF_POMIAROW)
    			sIndeksAnalizy++;
    	}	//koniec warunku if (sIndeksAnalizy < sIndeksPomiaru)

		  //koniec pomiaru, zapisz na kartę i wyślij telemetrię
		  if (cStatusPomiaru & SP_GOTOWY)
		  {
			  if (sLiczbaPomiarów > LICZBA_POMIAROW_DO_ZAPISU)	//nie zapisuj bardzo krótkich pomiarów
			  {
				  cBłąd = WyślijTelemetrieBluetooth(stPomiary);
				  cBłąd = ZapiszPomiarNaKarte(stPomiary);
				  if (cBłąd == BLAD_OK)
				  {
					  cBłąd = ZapiszIndeksPomiaruNaKarte(sLicznikPomiarow);		//aktualizuj indeks. skoro zapis danych się udał to indeks też powinien się zapisać
				  }
				  sLiczbaPomiarów = 0;
				  sIndeksAnalizy = sIndeksPomiaru = 0;
			  }

			  cStatusPomiaru = 0;	//kasuj wszystkie flagi
			  cLicznikOczekiwania = 0;
		  }
		  else	//cStatusPomiaru nie jest SP_GOTOWY. Sprawdź czy wystąpił timeout
		  {
			  nCzas = MinalCzas(nCzasOstatniegoPomiaru);
			  if ((nCzas > 5000000) && (cStatusPomiaru & SP_UZBROJONY)  && (cStatusPomiaru & SP_TRWA) && (sIndeksPomiaru > 6))	//timeout 5 sekund
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

  /** Enable Epod Booster
  */
  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Set Flash latency before increasing MSIS
  */
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hadc1.Init.LowPowerAutoWait = ENABLE;
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
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
      Error_Handler();
  }
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
  htim2.Init.Prescaler = 47;
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
  htim7.Init.Prescaler = 47;
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
