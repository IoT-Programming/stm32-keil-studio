/*
 * dht11_driver.c
 *
 *  Created on: Sep 17, 2024
 *      Author: ksshin
 */

#include "dht11_driver.h"
#include "mbed_wait_api.h"


GPIO_InitTypeDef GPIO_InitStruct_dht11 = {0};


void Set_Pin_Output (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitStruct_dht11.Pin = GPIO_Pin;  // GPIO_Pin 핀 사용
	GPIO_InitStruct_dht11.Mode = GPIO_MODE_OUTPUT_PP;  // 푸시-풀 출력 모드
	GPIO_InitStruct_dht11.Pull = GPIO_NOPULL;  // 풀업/풀다운 없음
	GPIO_InitStruct_dht11.Speed = GPIO_SPEED_FREQ_LOW;  // 속도 설정
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct_dht11);
}

void Set_Pin_Input (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitStruct_dht11.Pin = GPIO_Pin;  // GPIO_Pin 핀 사용
	GPIO_InitStruct_dht11.Mode = GPIO_MODE_INPUT;  // 입력 모드로 전환
	GPIO_InitStruct_dht11.Pull = GPIO_NOPULL;  // 풀업/풀다운 없음
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct_dht11);
}

void DHT11_Start (void)
{
	Set_Pin_Output (DHT11_GPIO_Port, DHT11_Pin);  // set the pin as output
	HAL_GPIO_WritePin (DHT11_GPIO_Port, DHT11_Pin, 0);   // pull the pin low
	wait_us(18000);   // wait for 18ms
	Set_Pin_Input(DHT11_GPIO_Port, DHT11_Pin);    // set as input
}

uint8_t Check_Response (void)
{
	uint8_t Response = 0;
	wait_us(40);
	if (!(HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin)))
	{
		wait_us(80);
		if ((HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin))) Response = 1;
		else Response = 0;
	}
	while ((HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin)));   // wait for the pin to go low

	return Response;
}

uint8_t DHT11_ReadData (void)
{
	uint8_t i=0;
	uint8_t j;
	for (j=0;j<8;j++)
	{
		while (!(HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin)));   // wait for the pin to go high
		wait_us(40);   // wait for 40 us
		if (!(HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((HAL_GPIO_ReadPin (DHT11_GPIO_Port, DHT11_Pin)));  // wait for the pin to go low
	}
	return i;
}


