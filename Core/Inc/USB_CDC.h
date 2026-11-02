/*
 * USB_CDC.h
 *
 *  Created on: 30 paź 2026
 *      Author: Piotr
 */

#ifndef INC_USB_CDC_H_
#define INC_USB_CDC_H_
#include "stm32u3xx_hal.h"
#include "main.h"

uint8_t WyślijTelemetrieCDC(stPomiary_t *stPomiary);

#endif /* INC_USB_CDC_H_ */
