//////////////////////////////////////////////////////////////////////////////
//
// Obsługa wyświetlacza OLED 128x64 po magistrali SPI
//
// Utworzone: Jul 29, 2026
// Autor: Dorota
// (c) ITWL 2026
//////////////////////////////////////////////////////////////////////////////
#include "OLED.h"
#include "main.h"
#include <stdio.h>




extern SPI_HandleTypeDef hspi3;
uint8_t OLED_Buffor[OLED_WYSOKOSC * OLED_SZEROKOSC / 8];
static SSD1306_t SSD1306;



void OLED_WyslijKomende(uint8_t bajt){

	//HAL_GPIO_WritePin(CHIPSELECT_GPIO_Port, CHIPSELECT_Pin, GPIO_PIN_RESET);	//Wybierz OLED
	HAL_GPIO_WritePin(D_C_GPIO_Port, D_C_Pin, GPIO_PIN_RESET); 					//DC = 0: polecenie
	HAL_SPI_Transmit(&hspi3, &bajt, 1, 10);										//transmisja bajtu polecenia
	//HAL_GPIO_WritePin(CHIPSELECT_GPIO_Port, CHIPSELECT_Pin, GPIO_PIN_SET); 		//zwolnij OLED
}

void OLED_WyslijDane(uint8_t* buffor, uint8_t rozmiar){

	//HAL_GPIO_WritePin(CHIPSELECT_GPIO_Port, CHIPSELECT_Pin, GPIO_PIN_RESET); 	//Wybierz OLED
	HAL_GPIO_WritePin(D_C_GPIO_Port, D_C_Pin, GPIO_PIN_SET); 					//DC = 1: dane do wyświetlenia
	HAL_SPI_Transmit(&hspi3, buffor, rozmiar, 10);								//transmisja danych do wyświetlenia
	//HAL_GPIO_WritePin(CHIPSELECT_GPIO_Port, CHIPSELECT_Pin, GPIO_PIN_SET); 		//zwolnij OLED
}


void OLED_Init(void){

	HAL_Delay(100);

	OLED_WyslijKomende(0xAE); //display off
	OLED_WyslijKomende(0x20); //Set Memory Addressing Mode
	OLED_WyslijKomende(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
    						  // 10b,Page Addressing Mode (RESET); 11b,Invalid

	OLED_WyslijKomende(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
#ifdef SSD1306_MIRROR_VERT
	OLED_WyslijKomende(0xC0); // Mirror vertically
#else
	OLED_WyslijKomende(0xC8); //Set COM Output Scan Direction
#endif

	OLED_WyslijKomende(0x00); //---set low column address
	OLED_WyslijKomende(0x10); //---set high column address
	OLED_WyslijKomende(0x40); //--set start line address - CHECK
	OLED_WyslijKomende(0x81); //--set contrast control register - CHECK
	OLED_WyslijKomende(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
	OLED_WyslijKomende(0xA0); // Mirror horizontally
#else
	OLED_WyslijKomende(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
	OLED_WyslijKomende(0xA7); //--set inverse color
#else
	OLED_WyslijKomende(0xA6); //--set normal color
#endif

	//OLED_WyslijKomende(0xA8); //--set multiplex ratio(1 to 64) - CHECK
	OLED_WyslijKomende(0xFF);

	//OLED_WyslijKomende(0x3F); // ustawia wysokosc = 64
	OLED_WyslijKomende(0x1F); // ustawia wysokosc = 32

	OLED_WyslijKomende(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	OLED_WyslijKomende(0xD3); //-set display offset - CHECK
	OLED_WyslijKomende(0x00); //-not offset

	OLED_WyslijKomende(0xD5); //--set display clock divide ratio/oscillator frequency
	OLED_WyslijKomende(0xF0); //--set divide ratio

	OLED_WyslijKomende(0xD9); //--set pre-charge period
	OLED_WyslijKomende(0x22); //

	OLED_WyslijKomende(0xDA); //--set com pins hardware configuration - CHECK

	OLED_WyslijKomende(0x12); // wysokość = 64

	OLED_WyslijKomende(0xDB); //--set vcomh
	OLED_WyslijKomende(0x20); //0x20,0.77xVcc

	OLED_WyslijKomende(0x8D); //--set DC-DC enable
	OLED_WyslijKomende(0x14); //
	OLED_WyslijKomende(0xAF); //--turn on SSD1306 panel

	OLED_CzyśćBuforEkranu();	//wyczyść bufor

	OLED_OdswiezEkran();
	SSD1306.cZainicjowany = 1;
	SSD1306.sPozX = 0;
	SSD1306.sPozY = 0;
}

void OLED_OdswiezEkran(void){

	for(uint8_t i = 0; i < OLED_WYSOKOSC / 8; i++){
		OLED_WyslijKomende(0xB0 + i); //Set Page Start Address for Page Addressing Mode
		OLED_WyslijKomende(0x00); //Set Lower Column Start Address for Page Addressing Mode
		OLED_WyslijKomende(0x10); //Set Higher Column Start Address for Page Addressing	Mode
		OLED_WyslijDane(&OLED_Buffor[OLED_SZEROKOSC*i], OLED_SZEROKOSC);
	}
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje na wyświetlaczu pojedynczy piksel w punkcie o podanych współrzednych ekranowych
// Parametry:
//  cPozX, cPozY - współrzedne ekranowe w pikselach
//  cŚwieci: 1=piksel zaświecony, 0=piksel zgaszony
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_RysujPiksel(int16_t sPozX, int16_t sPozY, uint8_t cŚwieci)
{
	if(sPozX >= OLED_SZEROKOSC || sPozX < 0 || sPozY >= OLED_WYSOKOSC || sPozY < 0) return;

	if (cŚwieci)
		OLED_Buffor[sPozX + (sPozY / 8) * OLED_SZEROKOSC] |=   1 << (sPozY % 8);	//piksel zaświecony
	else
		OLED_Buffor[sPozX + (sPozY / 8) * OLED_SZEROKOSC] &= ~(1 << (sPozY % 8));	//piksel zgaszony
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje pozioma linię
// Parametry:
//  sStartX, sStartY - współrzędne ekranowe początku linii w pikselach
//  sDługość -  długość linii w pikselach
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_RysujLiniePoziomą(int16_t sStartX, int16_t sStartY, int16_t sDługość)
{
	if (sDługość < 0)
	{
		sDługość = -sDługość;
		sStartX -= sDługość;
	}

	for (uint16_t x=0; x< sDługość; x++)
		OLED_RysujPiksel(sStartX + x, sStartY, 1);
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje pozioma linię przerywaną
// Parametry:
//  sStartX, sStartY - współrzędne ekranowe początku linii w pikselach
//  sDługość -  długość linii w pikselach
//  cPrzerwa - ilość pikseli przerwy między rysowanymi punktami
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_RysujLiniePoziomąPrzerywaną(int16_t sStartX, int16_t sStartY, int16_t sDługość, uint8_t cPrzerwa)
{
	if (sDługość < 0)
	{
		sDługość = -sDługość;
		sStartX -= sDługość;
	}

	sDługość /= cPrzerwa;

	for (uint16_t x=0; x< sDługość; x++)
		OLED_RysujPiksel(sStartX + x * cPrzerwa, sStartY, 1);
}




////////////////////////////////////////////////////////////////////////////////
// Rysuje pionową linię
// Parametry:
//  sStartX, sStartY - współrzedne ekranowe początku linii w pikselach
//  sDługość -  długość linii w pikselach
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_RysujLiniePionową(int16_t sStartX, int16_t sStartY, int16_t sDługość)
{
	if (sDługość < 0)
	{
		sDługość = -sDługość;
		sStartY -= sDługość;
	}

	for (int16_t y=0; y< sDługość; y++)
		OLED_RysujPiksel(sStartX, sStartY + y, 1);
}



////////////////////////////////////////////////////////////////////////////////
// rysuj linię o współrzędnych x1,y1, x2,y2
// Parametry:
//  cStartX, cStartY - współrzędne poczatku linii
//  cStopX, cStopY - współrzędne końca linii
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_RysujLinię(int16_t sStartX, int16_t sStartY, int16_t sStopX, int16_t sStopY)
{
	if (sStartY == sStopY)
		OLED_RysujLiniePoziomą(sStartX, sStartY, sStopX - sStartX);
	else
	if (sStartX == sStopX)
		OLED_RysujLiniePionową(sStartX, sStartY, sStopY - sStartY);
	else
	{
		uint16_t	dx = (sStopX > sStartX ? sStopX - sStartX : sStartX - sStopX);
		int16_t	xstep =  sStopX > sStartX ? 1 : -1;
		uint16_t	dy = (sStopY > sStartY ? sStopY - sStartY : sStartY - sStopY);
		int16_t	ystep =  sStopY > sStartY ? 1 : -1;
		int16_t	col = sStartX, row = sStartY;

		if (dx < dy)
		{
			int16_t t = - (dy >> 1);
			while (1)
			{
				OLED_RysujPiksel(col, row, 1);
				if (row == sStopY)
					return;

				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t -= dy;
				}
			}
		}
		else
		{
			int16_t t = - (dx >> 1);
			while (1)
			{
				OLED_RysujPiksel(col, row, 1);
				if (col == sStopX)
					return;

				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t -= dx;
				}
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje na wyświetlaczu pojedynczy znak wybraną czcionką w bieżącym miejscy
// Parametry:
//  cZnak - kod ASCII znaku do wyświetlanie
//  *Font - wskaźnik na czcionkę
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
char OLED_RysujZnak(char cZnak, FontDef *Font)
{
    uint32_t i, b, j;

    //Wyjdź gdy znak nie ma reprezentacji graficznej
    if (cZnak < 32 || cZnak > 126)
        return 0;

    //Sprawdź czu jest zmiejsce na znak w bieżącej linii
    if ((OLED_SZEROKOSC < (SSD1306.sPozX + Font->cFontWidth)) || (OLED_WYSOKOSC < (SSD1306.sPozY + Font->cFontHeight)))
        return 0;

    for (i = 0; i < Font->cFontHeight; i++)
    {
        b = Font->sData[(cZnak - 32) * Font->cFontHeight + i];
        for (j = 0; j < Font->cFontWidth; j++)
        {
            if((b << j) & 0x8000)
            	OLED_RysujPiksel(SSD1306.sPozX + j, (SSD1306.sPozY + i), 1);	//zaświeć
             else
            	OLED_RysujPiksel(SSD1306.sPozX + j, (SSD1306.sPozY + i), 0);	//zgaś
        }
    }

    //Uaktualinij zajętą pozycję
    SSD1306.sPozX += Font->cFontWidth;
    return cZnak;
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje na wyświetlaczu napis wybraną czcionką w miejscu o podanych współrzędnych
// Parametry:
//  *cNapis - wskaźnik na napis do wyświetlenia
//  cPozX, cPozY - współrzedne ekranowe w pikselach
//  *Font - wskaźnik na czcionkę
// Zwraca: kod ostatnio rysowanego znaku lub 0 jeżeli blędny znak
////////////////////////////////////////////////////////////////////////////////
char OLED_RysujNapis(char *cNapis, uint8_t cPozX, uint8_t cPozY, FontDef *Font)
{
	uint8_t cRysowanyZnak;

	SSD1306.sPozX = cPozX;
	SSD1306.sPozY = cPozY;

	while (*cNapis)
	{
		cRysowanyZnak = OLED_RysujZnak(*cNapis, Font);
		if (cRysowanyZnak != *cNapis)
			return cRysowanyZnak;	//wyjdź jeżeli trafił się niedrukowalny znak
		cNapis++;	//następny znak
	}
	return cRysowanyZnak;
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje bitmapę w miejscu o podanych współrzędnych
// Pracuje z bitmapami generowanymi przez Advanced Image2C++ Converter: https://image2cpp.42web.io
// Parametry:
//  cStartX, cStartY - współrzedne ekranowe lewego górnego narożnika bitmapy w pikselach
//  cSzer, cWys - szerokość i wysokość bitmapy
//  *cBitmapa - wskaźnik na bitmapę
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void RysujBitmape(uint8_t cStartX, uint8_t cStartY, uint8_t cSzer, uint8_t cWys, uint8_t *cBitmapa)
{
	uint8_t cObraz;

	for (uint8_t y = 0; y < cWys; y++)	//iteracja po wierszach bitmapy
	{
		for (uint8_t x = 0; x < cSzer/8; x++)	//iteracja po bajtach wiersza
		{
			cObraz = cBitmapa[y * cSzer/8 + x];
			for (uint8_t b = 0; b < 8; b++)
			{
				if((cObraz << b) & 0x80)
					OLED_RysujPiksel(cStartX + x*8 + b, (cStartY + y), 1);	//zaświeć
				else
					OLED_RysujPiksel(cStartX + x*8 + b, (cStartY + y), 0);	//zgaś
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Czyści bufor ekranu
// Parametry: brak
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OLED_CzyśćBuforEkranu(void)
{
	for (uint16_t i=0; i<OLED_WYSOKOSC; i++)
	{
		for (uint16_t j=0; j<OLED_SZEROKOSC / 8; j++)
			OLED_Buffor[j + i*OLED_SZEROKOSC/8] = 0;
	}
}

