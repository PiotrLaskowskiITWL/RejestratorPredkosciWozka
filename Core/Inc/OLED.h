/*
 * OLED.h
 *
 *  Created on: Jul 29, 2026
 *      Author: dorota
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_
#include "stm32u3xx_hal.h"
#include <ssd1306_fonts.h>

#define OLED_SZEROKOSC 128
#define OLED_WYSOKOSC 64

typedef struct {
    uint16_t sPozX;
    uint16_t sPozY;
    uint8_t cNegatyw;
    uint8_t cZainicjowany;
} SSD1306_t;

void OLED_WyslijKomende(uint8_t bajt);
void OLED_WyslijDane(uint8_t* buffor, uint8_t rozmiar);
void OLED_Init(void);
void OLED_OdswiezEkran(void);
void OLED_RysujPiksel(uint8_t cPozX, uint8_t cPozY, uint8_t cŚwieci);
void OLED_RysujLiniePoziomą(uint8_t cStartX, uint8_t cStartY, uint8_t cDługość);
char OLED_RysujZnak(char cZnak, FontDef *Font);
char OLED_RysujNapis(char *cNapis, uint8_t cPozX, uint8_t cPozY, FontDef *Font);
void RysujBitmape(uint8_t cStartX, uint8_t cStartY, uint8_t cSzer, uint8_t cWys, uint8_t *cBitmapa);
void OLED_CzyśćBuforEkranu(void);

#endif /* INC_OLED_H_ */
