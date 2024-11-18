#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

#include "stm32f4xx_hal.h"

// PulseSensor 구조체 정의
typedef struct {
    uint16_t analogPin;           // ADC 입력 핀
    uint16_t ledPin;              // LED 출력 핀
    int threshold;                // 신호 감지 임계값
    int bpm;                      // BPM 값
    unsigned long lastBeatTime;   // 마지막 비트 시간
    int beatDetected;             // 비트 감지 여부 (0 또는 1)
} PulseSensor;

// 함수 프로토타입
void PulseSensor_Init(PulseSensor *sensor, uint16_t analogPin, uint16_t ledPin, int threshold);
void PulseSensor_Begin(PulseSensor *sensor);
int PulseSensor_SawStartOfBeat(PulseSensor *sensor);
int PulseSensor_GetBeatsPerMinute(PulseSensor *sensor);
void PulseSensor_OnSampleTime(PulseSensor *sensor);
uint32_t PulseSensor_ReadADC(PulseSensor *sensor);

#endif
