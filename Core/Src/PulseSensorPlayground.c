#include "PulseSensorPlayground.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

extern ADC_HandleTypeDef hadc1;

// PulseSensor 초기화 함수
void PulseSensor_Init(PulseSensor *sensor, uint16_t analogPin, uint16_t ledPin, int threshold) {
    sensor->analogPin = analogPin;
    sensor->ledPin = ledPin;
    sensor->threshold = threshold;
    sensor->bpm = 0;
    sensor->lastBeatTime = 0;
    sensor->beatDetected = 0;
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

        // BPM 계산
        if (sensor->lastBeatTime != 0) { // 이전 비트 감지 시간이 있는 경우
            sensor->bpm = 60000 / (currentTime - sensor->lastBeatTime);
        }
        sensor->lastBeatTime = currentTime; // 마지막 비트 감지 시간 업데이트

        // LED 깜박임 (비트 감지 시)
        HAL_GPIO_WritePin(GPIOC, sensor->ledPin, GPIO_PIN_SET);

        // 디버깅 메시지 출력
        printf("ADC Value: %lu, Threshold: %d, Beat Detected: %d, BPM: %d\n",
               value, sensor->threshold, sensor->beatDetected, sensor->bpm);

    } else if (value <= sensor->threshold && sensor->beatDetected) {
        // 신호가 Threshold 아래로 내려가면 비트 감지 상태 초기화
        sensor->beatDetected = 0;

        // LED 끄기 (비트 감지 종료 시)
        HAL_GPIO_WritePin(GPIOC, sensor->ledPin, GPIO_PIN_RESET);

        // 디버깅 메시지 출력
        printf("ADC Value: %lu, Threshold: %d, Beat Detected: %d, BPM: %d\n",
               value, sensor->threshold, sensor->beatDetected, sensor->bpm);
    }

    // 추가 디버깅용 출력 (상시 확인)
    printf("ADC Value: %lu, Threshold: %d, Beat Detected: %d, BPM: %d\n",
           value, sensor->threshold, sensor->beatDetected, sensor->bpm);
}



uint32_t PulseSensor_ReadADC(PulseSensor *sensor) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = sensor->analogPin;
    sConfig.Rank = 1; // ADC 순서
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adcValue = HAL_ADC_GetValue(&hadc1);

    printf("ADC Value: %lu\n", adcValue); // UART를 통해 ADC 값을 출력
    return adcValue;
}


