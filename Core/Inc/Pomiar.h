/*
 * Pomiar.h
 *
 *  Created on: 28 paź 2026
 *      Author: Piotr
 */

#ifndef INC_POMIAR_H_
#define INC_POMIAR_H_
#include "stm32u3xx_hal.h"
#include "main.h"

#define ZEGAR_TIMERA	24000000


uint8_t InicjujPomiarTimerem(void);
uint8_t ZakończPomiarTimerem(void);
uint8_t RozpocznijPomiarTimerem(void);


#endif /* INC_POMIAR_H_ */
