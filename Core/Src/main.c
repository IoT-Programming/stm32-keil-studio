#include "stm32f4xx_hal.h"
#include "PulseSensorPlayground.h"

// 핸들러 선언
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim2;

// 함수 프로토타입
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_TIM2_Init(void);
void MX_USART2_UART_Init(void);
void Error_Handler(void);

PulseSensor mySensor; // PulseSensor 객체
int _write(int file, char *data, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t *)data, len, HAL_MAX_DELAY);
    return len;
}
int main(void) {
    HAL_Init(); // HAL 초기화
    SystemClock_Config(); // 시스템 클럭 설정

    // 주변 장치 초기화
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();

    // PulseSensor 초기화
    PulseSensor_Init(&mySensor, ADC_CHANNEL_0, GPIO_PIN_13, 3100);
    PulseSensor_Begin(&mySensor);

    // 타이머 인터럽트 시작
    HAL_TIM_Base_Start_IT(&htim2);

    while (1) {
    	if (PulseSensor_SawStartOfBeat(&mySensor)) {
    	    int bpm = PulseSensor_GetBeatsPerMinute(&mySensor);
    	    printf("♥ A Heartbeat Happened!\n"); // 심박 이벤트 메시지
    	    printf("BPM: %d\n", bpm); // BPM 출력
    	} else {
    	    printf("No Beat Detected. Current ADC Value: %lu\n", PulseSensor_ReadADC(&mySensor));
    	}

        HAL_Delay(1000); // 20ms 지연
    }
}


void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // PLL 설정을 위한 구조체
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI; // HSI 오실레이터 사용
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;                  // HSI 활성화
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;              // PLL 활성화
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;      // PLL 입력 소스 HSI
    RCC_OscInitStruct.PLL.PLLM = 16;                          // PLLM 설정
    RCC_OscInitStruct.PLL.PLLN = 336;                         // PLLN 설정
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;               // PLLP 설정
    RCC_OscInitStruct.PLL.PLLQ = 7;                           // PLLQ 설정

    // RCC 오실레이터 초기화
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // 클럭 설정
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // 시스템 클럭 소스 PLL
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // AHB 클럭 분주비
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;         // APB1 클럭 분주비
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;         // APB2 클럭 분주비

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
}

void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        PulseSensor_OnSampleTime(&mySensor);
        printf("Timer interrupt triggered.\n"); // 타이머 인터럽트 확인
    }
}

void Error_Handler(void) {
    while (1) {
        // 에러 발생 시 멈춤
    }
}
