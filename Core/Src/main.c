/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "MAX30100.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* We are doing heat monitioring git testing  */

/* We are now in feature temp sensor we will checking this code in featuresensor  */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */
uint16_t heart_rate = 0;
uint8_t spo2 = 0;

//adding global varables


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define WARNING_THRESHOLD 120
#define CRITICAL_THRESHOLD 150


//defining Pins that recieve signal from MCU1 (D6) to MCU 2
#define SIGNAL_PIN GPIO_PIN_6
#define PIEZO_PIN GPIO_PIN_10

/**
  * @brief  The application entry point.
  * @retval int
  */

//adding power amangement for the code

//adding a function to turn the seson on and off
void power_Sensors_On(void){
	//THIS CODE TURNS ON THE PULSE SENSOR
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	//THIS CODE TURNS ON THE LM35
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

}
//this turns the sensors off
//
void Power_Sensors_Off(void){
	//turns off pulse sensors
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	//turns on pulse sensor
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

}

void check_signal_and_activate_piezo(void) {
    if (HAL_GPIO_ReadPin(GPIOB, SIGNAL_PIN) == GPIO_PIN_SET) {
    	//turn on piezo::
             HAL_GPIO_WritePin(GPIOB, PIEZO_PIN, GPIO_PIN_SET);
    } else {
    	//turn off piezo::
        HAL_GPIO_WritePin(GPIOB, PIEZO_PIN, GPIO_PIN_RESET);  // Turn off piezo
    }
}


//defining the function which will read the temperature from the LM35
double Read_Temperature(void){

	unit32_t adc_value = 0;

	//for the conversion:
	//samples input value
	HAL_ADC_Start(&hadc1);
	//checks if the conversion is succesfull
	if(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY)==HAL_OK){
		//this retrieves the value of the ADC conversion if succesfull
		adc_value = HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
	//convert

	double voltage = (adc_value/4095.0)*3.3;
	double temperature = voltage*100.0;

	return temperature;
}
//
//this function is meant to initlzied the MAX30100 and set the sensor to heart rate & SPO2 MODE!!!!
	void MAX30100_Init(void) {
	    MAX30100_SetMode(MAX30100_MODE_HR_SPO2); // Set sensor to Heart Rate & SpO2 mode
	    MAX30100_SetLEDs(MAX30100_LED_CURRENT_27MA, MAX30100_LED_CURRENT_27MA); // LED currents
	}

//reads data and SPO2
	void MAX30100_ReadData(void) {
	    MAX30100_ReadFIFO(&heart_rate, &spo2);
	}


int main(void)
{


  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  uint8_t redLED, irLED;
  MAX30100_Init();

  while (1)

  {
	  power_Sensors_On();



	  MAX30100_ReadData();


	  /* USER CODE BEGIN 1 */

	  /*we're creating a major design change!
	   * we realized that just having a temperature above 36.6 degrees celcius wouldn't be helpful for the workers. depending on how much heat they have
	   * it will be useful for them to have levels to how hot they are
	   * so therefore, they can have the following
	   *
	   * a flashing LED light if theres a low alert (36.6 to 38 deg celcius)
	   * a high alert if (38 deg - 39 deg)
	   * a critical alert if (>39) so the led just stays off and sends a signal to MCU2
	   * */

	  //writing code that checks our data from the MAX30100 and fifo data and extracts RED LED
	  uint8_t fifoData[4];
	  MAX30100_ReadFifo(fifoData, 4);
	  HAL_Delay(100);

	  //adding the HEART RATE LOGIC
	  //keeps the green LED Light on
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	  //keeps green LED off
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

	  //within the while loop, we're going to read the temperature using the Read_Temperature() function
	  double temperature = Read_Temperature();
	  //out threshold is that if its above a certain temperature, then we can turn the LED light on and Off
	  if(temperature > 36.6 && temperature <= 38.0 && heart_rate < 120 && heart_rate > 100){
		  //here, the LED will flash
		  //green LED off
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_Delay(500);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_Delay(500);
	  }

	  //added a condition for if between 38 to 39 then the LED just stays on
	  else if(temperature > 38.0 && temperature <= 39.0 && heart_rate < 120){
		  //green LED off
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	  }
	  else if(temperature > 39 && heart_rate > 120){
		  //sending the signal to MCU 2

		  //to send the signal, heres what wer're doing:
		  //we're using the function HAL_UART_TRANSMIT to give it out temp and heart rate
		  //creating an array
		  char data_to_send[50];
		  sprintf(data_to_send, "", Read_Temperature(), MAX30100_ReadData());
		  //acc transmiting the data
		  HAL_UART_Transmit(&huart1, (uint8_t*)data_to_send, strlen(data_to_send), HAL_MAX_DELAY);
		  HAL_Delay(5000);
		  //currently the light stays off
		  //green LED off
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	  }

//added
	  //we're assuming that if not, the LED light will stay off.

	  //turning the sensor off
	  if(heart_rate < 80){
		  power_Sensors_Off();
	  }

	  HAL_Delay(1000);
  }
  /* USER CODE END 1 */


}




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
