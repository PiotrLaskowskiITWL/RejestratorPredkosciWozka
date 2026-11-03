/*
 * KartaSD.h
 *
 *  Created on: 26 paź 2026
 *      Author: Piotr
 */


#ifndef INC_KARTASD_H_
#define INC_KARTASD_H_
#include "stm32u3xx_hal.h"
#include "main.h"

#define STATSD_MEDIA_OTWARTE		0x01
#define STATSD_BRAK_KARTY			0x02

#ifndef FX_SD_VOLUME_NAME
  #define FX_SD_VOLUME_NAME "STM32_SDIO_DISK"
#endif

uint8_t CzyKartaObecna(void);
uint8_t ZapiszPomiarNaKarte(stPomiary_t *stPomiary);
uint8_t CzytajIndeksPomiaruZKarty(uint8_t *cIndeksPomiaru);
uint8_t ZapiszIndeksPomiaruNaKarte(uint8_t cIndeksPomiaru);
int FormatujNagłówekDanych(char *cBufor);
int FormatujDanePomiarowe(char *cBufor, uint8_t cIndeks, stPomiary_t *stPomiary);

#endif /* INC_KARTASD_H_ */
