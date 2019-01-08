/**
 * @file main.c
 * @brief MLX90632 example with implementation of driver with i2c communication
 * @internal
 *
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @endinternal
 *
 * @details
 *
 * @addtogroup mlx90632_private MLX90632 Internal library functions
 * @{
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"

#include "mlx90632.h"

I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/* Definition of I2C address of MLX90632 */
#define CHIP_ADDRESS 0x3a << 1
/* HAL_I2C_Mem_Read()/Write() are used instead of Master_Transmit()/Receive() because repeated start condition is needed */
/* Implementation of I2C read for 16-bit values */
int32_t mlx90632_i2c_read(int16_t register_address, uint16_t *value)
{
	uint8_t data[2];
	int32_t ret;
	ret = HAL_I2C_Mem_Read(&hi2c1, CHIP_ADDRESS, register_address, 2, data, sizeof(data), 100);
	//Endianness
	*value = data[1]|(data[0]<<8);
	return ret;
}

/* Implementation of I2C read for 32-bit values */
int32_t mlx90632_i2c_read32(int16_t register_address, uint32_t *value)
{
	uint8_t data[4];
	int32_t ret;
	ret = HAL_I2C_Mem_Read(&hi2c1, CHIP_ADDRESS, register_address, 2, data, sizeof(data), 100);
	//Endianness
	*value = data[2]<<24|data[3]<<16|data[0]<<8|data[1];
	return ret;
}

/* Implementation of I2C write for 16-bit values */
int32_t mlx90632_i2c_write(int16_t register_address, uint16_t value) {
	uint8_t data[2];
	data[0] = value >> 8;
	data[1] = value;
	return HAL_I2C_Mem_Write(&hi2c1, CHIP_ADDRESS, register_address, 2, data, 2, 100);
}

/* Implementation of reading all calibration parameters for calucation of Ta and To */
static int mlx90632_read_eeprom(int32_t *PR, int32_t *PG, int32_t *PO, int32_t *PT, int32_t *Ea, int32_t *Eb, int32_t *Fa, int32_t *Fb, int32_t *Ga, int16_t *Gb, int16_t *Ha, int16_t *Hb, int16_t *Ka)
{
	int32_t ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_P_R, (uint32_t *) PR);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_P_G, (uint32_t *) PG);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_P_O, (uint32_t *) PO);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_P_T, (uint32_t *) PT);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_Ea, (uint32_t *) Ea);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_Eb, (uint32_t *) Eb);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_Fa, (uint32_t *) Fa);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_Fb, (uint32_t *) Fb);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read32(MLX90632_EE_Ga, (uint32_t *) Ga);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read(MLX90632_EE_Gb, (uint16_t *) Gb);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read(MLX90632_EE_Ha, (uint16_t *) Ha);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read(MLX90632_EE_Hb, (uint16_t *) Hb);
	if(ret < 0)
		return ret;
	ret = mlx90632_i2c_read(MLX90632_EE_Ka, (uint16_t *) Ka);
	if(ret < 0)
		return ret;
	return 0;
}

void usleep(int min_range, int max_range) {
	while(--min_range);
}

double pre_ambient, pre_object, ambient, object;

int main(void)
{
	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();

	/* Check the internal version and prepare a clean start */
	mlx90632_init();

	/* Definition of MLX90632 calibration parameters */
	int16_t ambient_new_raw;
	int16_t ambient_old_raw;
	int16_t object_new_raw;
	int16_t object_old_raw;
	int32_t PR = 0x00587f5b;
	int32_t PG = 0x04a10289;
	int32_t PT = 0xfff966f8;
	int32_t PO = 0x00001e0f;
	int32_t Ea = 4859535;
	int32_t Eb = 5686508;
	int32_t Fa = 53855361;
	int32_t Fb = 42874149;
	int32_t Ga = -14556410;
	int16_t Ha = 16384;
	int16_t Hb = 0;
	int16_t Gb = 9728;
	int16_t Ka = 10752;

	/* Read EEPROM calibration parameters */
	mlx90632_read_eeprom(&PR, &PG, &PO, &PT, &Ea, &Eb, &Fa, &Fb, &Ga, &Gb, &Ha, &Hb, &Ka);
	while (1)
	{
		/* Get raw data from MLX90632 */
		mlx90632_read_temp_raw(&ambient_new_raw, &ambient_old_raw, &object_new_raw, &object_old_raw);
		/* Pre-calculations for ambient and object temperature calculation */
		pre_ambient = mlx90632_preprocess_temp_ambient(ambient_new_raw, ambient_old_raw, Gb);
		pre_object = mlx90632_preprocess_temp_object(object_new_raw, object_old_raw, ambient_new_raw, ambient_old_raw, Ka);
		/* Set emissivity = 1 */
		mlx90632_set_emissivity(1.0);
		/* Calculate ambient and object temperature */
		ambient = mlx90632_calc_temp_ambient(ambient_new_raw, ambient_old_raw, PT, PR, PG, PO, Gb);
		object = mlx90632_calc_temp_object(pre_object, pre_ambient, Ea, Eb, Ga, Fa, Fb, Ha, Hb);
	}
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x0000020B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Pinout Configuration
*/
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
