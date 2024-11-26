#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

#include "stm32f4xx_hal.h"

// PulseSensor 구조체 정의
typedef struct {
    uint32_t threshold;
    uint8_t beatDetected;
    unsigned long lastBeatTime;
    unsigned long startTime;   // 10초 간격의 시작 시간
    uint32_t beatCount;        // 10초 동안 감지된 비트 수
    int bpm;                   // 계산된 BPM
    uint16_t ledPin;           // LED 핀 번호
} PulseSensor;
// 함수 프로토타입
void PulseSensor_Init(PulseSensor *sensor, uint16_t analogPin, uint16_t ledPin, int threshold);
void PulseSensor_Begin(PulseSensor *sensor);
int PulseSensor_SawStartOfBeat(PulseSensor *sensor);
int PulseSensor_GetBeatsPerMinute(PulseSensor *sensor);
void PulseSensor_OnSampleTime(PulseSensor *sensor);
uint32_t PulseSensor_ReadADC(PulseSensor *sensor);

#endif
