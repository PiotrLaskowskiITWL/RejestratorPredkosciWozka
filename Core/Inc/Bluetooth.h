/*
 * Bluetooth.h
 *
 *  Created on: 28 paź 2026
 *      Author: Piotr
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_
#include "stm32u3xx_hal.h"
#include "main.h"



uint8_t WyślijTelemetrieBluetooth(stPomiary_t *stPomiary);
uint8_t PrzełączBTwTrybDanych(void);

#endif /* BLUETOOTH_H_ */
