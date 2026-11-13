/*
 * Enkoder.h
 *
 *  Created on: 13 lis 2026
 *      Author: Piotr
 */

#ifndef INC_ENKODER_H_
#define INC_ENKODER_H_

#include "stm32u3xx_hal.h"


typedef struct
{
	uint8_t cOstatnioAktywnyCzujnik;
	uint8_t cStanCzujnika[4];
	uint8_t cPoprzedniStanCzujnika[4];
	uint8_t cStanEnkodera[2];
	uint8_t cPoprzedniStanEnkodera[2];
	uint8_t cKierunek[2];
} stEnkoder_t;

int8_t SprawdźKierunek(uint8_t cStanBieżący, uint8_t *cStanPoprzedni);
void SprawdźKierunekCzujników(uint8_t cNumerCzujnika, stEnkoder_t *stEnkoder);

#endif /* INC_ENKODER_H_ */
