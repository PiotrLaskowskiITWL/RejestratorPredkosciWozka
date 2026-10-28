/*
 * Czas.h
 *
 *  Created on: 7 cze 2026
 *      Author: PitLab
 */

#ifndef INC_CZAS_H_
#define INC_CZAS_H_
#include "stm32u3xx_hal.h"

uint32_t PobierzCzasT7(void);
uint32_t MinalCzasT7(uint32_t nPoczatek);
uint32_t MinalCzas2(uint32_t nPoczatek, uint32_t nCzasAkt);

#endif /* INC_CZAS_H_ */
