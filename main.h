/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|

    ( C )2014 Semtech

Description: Contains the callbacks for the IRQs and any application related details.

License: Revised BSD License, see LICENSE.TXT file included in the project.

Maintainer: Miguel Luis and Gregory Cristian

Copyright (c) 2024 STMicroelectronics.  
All rights reserved.  
This software is licensed under terms that can be found in the LICENSE file
in the root directory of this software component. If no LICENSE file comes
with this software, it is provided AS-IS.
*/

#ifndef __MAIN_H__
#define __MAIN_H__



/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* PHY MAC layer PDU offset definitions -------------------------------------*/
#define PHYMAC_PDUOFFSET_TYPE           0
#define PHYMAC_PDUOFFSET_SRCID          1
#define PHYMAC_PDUOFFSET_DSTID          2
#define PHYMAC_PDUOFFSET_DATA           4
#define PHYMAC_PDUTYPE_DATA             0

/* GPIO Pin Definitions -----------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define Vib_Pin GPIO_PIN_14
#define Vib_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define Prox_GPIO_Port GPIOB
#define Prox_Pin GPIO_PIN_13

/* Function Prototypes ------------------------------------------------------*/
void Error_Handler(void);
static void MX_GPIO_Init(void);

/* Callback Functions Prototypes --------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event.
 */
void OnTxDone(void);

/*!
 * @brief Function to be executed on Radio Rx Done event.
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

/*!
 * @brief Function executed on Radio Tx Timeout event.
 */
void OnTxTimeout(void);

/*!
 * @brief Function executed on Radio Rx Timeout event.
 */
void OnRxTimeout(void);

/*!
 * @brief Function executed on Radio Rx Error event.
 */
void OnRxError(void);

/*!
 * @brief Function executed on Radio FHSS Change Channel event.
 */
void OnFhssChangeChannel(uint8_t channelIndex);

/*!
 * @brief Function executed on CAD Done event.
 */
void OnCadDone(void);

/* User-defined Variables ---------------------------------------------------*/
static uint8_t phymac_id;



#endif /* __MAIN_H__ */
