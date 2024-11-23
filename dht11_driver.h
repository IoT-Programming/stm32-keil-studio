/*
 * dht11_driver.h
 *
 *  Created on: Sep 17, 2024
 *      Author: ksshin
 */

#ifndef INC_DHT11_DRIVER_H_
#define INC_DHT11_DRIVER_H_

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DHT11_Pin GPIO_PIN_11
#define DHT11_GPIO_Port GPIOA

void Set_Pin_Output (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Input (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void DHT11_Start (void);
uint8_t DHT11_ReadData (void);
uint8_t Check_Response (void);

#ifdef __cplusplus
}
#endif

#endif /* INC_DHT11_DRIVER_H_ */
