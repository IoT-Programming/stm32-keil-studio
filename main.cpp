#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dht11_driver.h"
/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA
#define RF_FREQUENCY    915000000 // Hz
#define TX_OUTPUT_POWER 14        // 14 dBm
#define LORA_BANDWIDTH  0         // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 5
#define LORA_FIX_LENGTH_PAYLOAD_ON                      false
#define LORA_FHSS_ENABLED                               false  
#define LORA_NB_SYMB_HOP                                4     
#define LORA_IQ_INVERSION_ON                            false
#define LORA_CRC_ENABLED                                true
#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 32
#define Rx_ID 10



typedef enum
{
    LOWPOWER = 0,
    IDLE,

    RX,
    RX_TIMEOUT,
    RX_ERROR,

    TX,
    TX_TIMEOUT,

    CAD,
    CAD_DONE
}AppStates_t;

volatile AppStates_t State = LOWPOWER;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";


uint8_t HelloMsg[] = "Hum : ";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2, SUM;
int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

    
float Temperature, Humidity;

TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
char str1[100];
uint8_t Rxdata[750];
char Txdata[750];
char GPS_Payyload[100];
uint8_t Flag=0;
static int Msgindex;
char *ptr;
float Latitude,Longitude;
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

void get_location(void);



int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch,1,HAL_MAX_DELAY);
    return ch;
}




int main(void) {
    uint8_t TEMP;
    uint8_t RH;
    uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2, SUM;
    float Temperature, Humidity;
    HAL_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
    
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_UART_Receive_DMA(&huart1,(uint8_t*)Rxdata, 700);//위성에서 데이터 받아서 Rxdata에 저장
    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    Radio.Init( &RadioEvents );

    // verify the connection with the board
    while( Radio.Read( REG_VERSION ) == 0x00  )
    {
        debug( "Radio could not be detected!\n\r", NULL );
        wait( 1000 );
    }
    
    debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1272MB2XAS ) ), "\n\r > Board Type: SX1272MB2xAS < \n\r" );

    Radio.SetChannel(RF_FREQUENCY);
    phymac_id = 1;
    debug("[PHYMAC] ID : %i\n", phymac_id);

    debug_if( LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r" );
    debug_if( !LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r" );


    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                         LORA_IQ_INVERSION_ON, 2000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                         LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                         LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                         LORA_IQ_INVERSION_ON, true );

    debug_if( DEBUG_MESSAGE, "Starting Ping-Pong loop\r\n" );

    
    State = IDLE;
    Buffer[0] = Rx_ID;

    while (1) {
        DHT11_Start();
        get_location();
        if (Check_Response()) {
            // debug("DHT11 응답 성공\n");
            Rh_byte1 = DHT11_ReadData ();
            Rh_byte2 = DHT11_ReadData ();
            Temp_byte1 = DHT11_ReadData ();
            Temp_byte2 = DHT11_ReadData ();
            SUM = DHT11_ReadData ();
        }
        else {
            debug("DHT11 Error: No response\n");
        }

        switch( State )
        {
        case IDLE:
            wait_ms( 1000 );

            // sprintf((char*)HelloMsg, "Hum: %d.%d\n", Rh_byte1, Rh_byte2);
            sprintf((char*)HelloMsg, "Temp: %d.%d\n", Temp_byte1, Temp_byte2);

            strcpy( ( char* )Buffer+1, ( char* )HelloMsg );

            debug("Hum : %d.%d\tTemp : %d.%d\n",Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2);
            Radio.Send( Buffer, BufferSize );
            debug( "...Ping\r\n" );

            // strcpy( ( char* )Buffer+1, ( char* )str1 );

            // debug("Hum : %d.%d\tTemp : %d.%d\n",Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2);
            // Radio.Send( Buffer, BufferSize );
            debug( "...Ping\r\n" );

            State = TX;
            break;
        case TX:
            break;
        case RX:
            break;

        default:
        //     State = TX;
            break;
        }
        wait_ms( 1000 );
        
    }
}


void OnTxDone( void )
{
    Radio.Sleep( );
    State = IDLE;
    debug_if( DEBUG_MESSAGE, "> OnTxDone\n\r" );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    uint8_t dataIndFlag = 0;
    Radio.Sleep( );
    State = IDLE;
    
    if (Buffer[PHYMAC_PDUOFFSET_TYPE] == PHYMAC_PDUTYPE_DATA &&
        Buffer[PHYMAC_PDUOFFSET_DSTID] == phymac_id)
    {
        dataIndFlag = 1;
    }

}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    // State = TX_TIMEOUT;
    State = IDLE;
    debug_if( DEBUG_MESSAGE, "> OnTxTimeout\n\r" );
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    Buffer[BufferSize] = 0;
    // State = RX_TIMEOUT;
    State = RX;
    debug_if( DEBUG_MESSAGE, "> OnRxTimeout\n\r" );
}

void OnRxError( void )
{
    Radio.Sleep( );
    // State = RX_ERROR;
    State = RX;
    debug_if( DEBUG_MESSAGE, "> OnRxError\n\r" );
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Flag=1;
}
void get_location(void)
{
    if (Flag == 1)
    {
        Msgindex = 0;
        strcpy(Txdata, (char *)(Rxdata));


        ptr = strstr(Txdata, "GPGLL");

        if (ptr != NULL)
        {

            while (1)
            {
                GPS_Payyload[Msgindex] = *ptr;
                Msgindex++;
                ptr++;
                if (*ptr == '\n' || *ptr == '\r') // 메시지 끝
                {
                    GPS_Payyload[Msgindex] = '\0';
                    break;
                }
            }


            //HAL_UART_Transmit(&huart2, (uint8_t *)GPS_Payyload, 33, HAL_MAX_DELAY);
            //HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);


            char latitude_str[15], longitude_str[15];
            float latitude, longitude;


            strncpy(latitude_str, &GPS_Payyload[6], 10);  // "3749.05976"
            latitude_str[10] = '\0';

            latitude = atof(latitude_str);  // atof로 변환
            //HAL_UART_Transmit(&huart2, (char*)latitude_str, strlen(latitude_str), HAL_MAX_DELAY);

            strncpy(longitude_str, &GPS_Payyload[19], 11);  // "12703.05842"
            longitude_str[11] = '\0';
            //HAL_UART_Transmit(&huart2, (char*)longitude_str, strlen(longitude_str), HAL_MAX_DELAY);
            longitude = atof(longitude_str);  // atof로 변환
            // 실수형 경도 값을 문자열로 변환
            float lat_decimal = (latitude / 100);

            
            strcpy(str1, "\nlat:");
            strcat(str1, latitude_str);
            strcat(str1, ",");
            strcat(str1, "long:");
            strcat(str1, longitude_str);
            //HAL_UART_Transmit(&huart2, (char*)str1, strlen(str1), HAL_MAX_DELAY);
            //sprintf(longitude_str1,  "%.5f", lat_decimal);



            // 이제 이 문자열을 UART로 전송
            //HAL_UART_Transmit(&huart2, (uint8_t *)longitude_str1, strlen(longitude_str1), HAL_MAX_DELAY);




        }
        else
        {
           // HAL_UART_Transmit(&huart2, (uint8_t *)"GPGLL not found\r\n", 18, HAL_MAX_DELAY);
        }

        Flag = 0; // 플래그 초기화
    }
}

static void MX_TIM6_Init(void)
{

/* USER CODE BEGIN TIM6_Init 0 */
    __HAL_RCC_TIM6_CLK_ENABLE();
/* USER CODE END TIM6_Init 0 */

    TIM_MasterConfigTypeDef sMasterConfig = {0};

/* USER CODE BEGIN TIM6_Init 1 */

/* USER CODE END TIM6_Init 1 */
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 90-1;

    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 65535;
    // htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
    // Error_Handler();
        printf("Error clocking1\n");
    }
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
// Error_Handler();
        printf("Error clocking2\n");
    }
/* USER CODE BEGIN TIM6_Init 2 */

/* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
 HAL_GPIO_WritePin(GPIOA, LD2_Pin|DHT11_Pin, GPIO_PIN_SET);
/* USER CODE END MX_GPIO_Init_2 */
}
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
