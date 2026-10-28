/*
 * KodyBledow.h
 *
 *  Created on: 26 paź 2026
 *      Author: Piotr
 */

#ifndef INC_KODYBLEDOW_H_
#define INC_KODYBLEDOW_H_
#include "stm32u3xx_hal.h"

#define BLAD_OK				0	//brak błędu
#define BLAD_HAL			1	//błąd zwrócony przez HAL
#define BLAD_ZAJETY			2	//błąd zajętości zasobu zgodny z HAL
#define BLAD_TIMEOUT		3	//bład timeoutu zgodny z HAL
#define BLAD_NIC_DO_ROBOTY	4	//
#define BLAD_BRAK_KARTY		5	//nie wykryto obecności karty w gnieździ
#define BLAD_OTWARCIA_KARTY	6
#define BLAD_ZAPISU_KARTY	7


#endif /* INC_KODYBLEDOW_H_ */
