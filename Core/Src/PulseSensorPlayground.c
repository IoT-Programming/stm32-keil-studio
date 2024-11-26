#include "PulseSensorPlayground.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

extern ADC_HandleTypeDef hadc1;

// PulseSensor 초기화 함수
void PulseSensor_Init(PulseSensor *sensor, uint16_t analogPin, uint16_t ledPin, int threshold) {

    sensor->ledPin = ledPin;
    sensor->threshold = threshold;
    sensor->bpm = 0;
    sensor->lastBeatTime = 0;
    sensor->beatDetected = 0;
    sensor->startTime = HAL_GetTick(); // 시작 시간 설정
	sensor->beatCount = 0;
}

// 초기화 작업 시작
void PulseSensor_Begin(PulseSensor *sensor) {
    // 추가 초기화 작업 (필요 시 작성)
}

// 새로운 비트 감지 여부 확인
int PulseSensor_SawStartOfBeat(PulseSensor *sensor) {
    if (sensor->beatDetected) {
        sensor->beatDetected = 0; // 상태 초기화
        return 1;
    }
    return 0;
}

// 현재 BPM 값 반환
int PulseSensor_GetBeatsPerMinute(PulseSensor *sensor) {
    return sensor->bpm;
}

void PulseSensor_OnSampleTime(PulseSensor *sensor) {
    uint32_t value = PulseSensor_ReadADC(sensor); // ADC 값을 읽음
    unsigned long currentTime = HAL_GetTick();   // 현재 시간(ms)을 가져옴

    // 심박수 감지 로직
    if (value > sensor->threshold && !sensor->beatDetected) {
        sensor->beatDetected = 1; // 비트 감지 상태 설정
        sensor->beatCount++;      // 비트 수 증가
        sensor->lastBeatTime = currentTime; // 마지막 비트 감지 시간 업데이트

        // LED 깜박임 (비트 감지 시)
        HAL_GPIO_WritePin(GPIOC, sensor->ledPin, GPIO_PIN_SET);
    } else if (value <= sensor->threshold && sensor->beatDetected) {
        sensor->beatDetected = 0; // 비트 감지 상태 초기화
        HAL_GPIO_WritePin(GPIOC, sensor->ledPin, GPIO_PIN_RESET);
    }

    // 10초가 경과했는지 확인
    if (currentTime - sensor->startTime >= 15000) {
        // BPM 계산
        if (sensor->beatCount > 0) {
            sensor->bpm = sensor->beatCount * 6; // 비트 수를 10초에서 60초로 환산
        } else {
            sensor->bpm = 0;
        }

        // 10초마다만 출력
        printf("BPM: %d\n", sensor->bpm);

        // 초기화
        sensor->beatCount = 0;
        sensor->startTime = currentTime;
    }
}




uint32_t PulseSensor_ReadADC(PulseSensor *sensor) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Rank = 1; // ADC 순서
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adcValue = HAL_ADC_GetValue(&hadc1);

    return adcValue;
}


