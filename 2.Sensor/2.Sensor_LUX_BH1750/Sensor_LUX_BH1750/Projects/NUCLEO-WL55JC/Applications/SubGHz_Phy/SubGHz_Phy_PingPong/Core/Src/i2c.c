#include "i2c.h"

#define I2C_INIT 1
I2C_HandleTypeDef hi2c3;

void MX_I2C3_Init(void)
{
	#if I2C_INIT
		/* USER CODE BEGIN I2C3_Init 0 */

		/* USER CODE END I2C3_Init 0 */

		/* USER CODE BEGIN I2C3_Init 1 */

		/* USER CODE END I2C3_Init 1 */
		hi2c3.Instance = I2C3;
		hi2c3.Init.Timing = 0x0070D8D9;
		hi2c3.Init.OwnAddress1 = 0x00;
		hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c3.Init.OwnAddress2 = 0;
		hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
		hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

		if (HAL_I2C_Init(&hi2c3) != HAL_OK)
		{
			Error_Handler();
		}
		/** Configure Analogue filter
		*/
		if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		{
			Error_Handler();
		}
		/** Configure Digital filter
		*/
		if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN I2C3_Init 2 */

		/* USER CODE END I2C3_Init 2 */
	#endif
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	#if I2C_INIT
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
		if(hi2c->Instance==I2C3)
		{
		/* USER CODE BEGIN I2C1_MspInit 0 */
			
		/* USER CODE END I2C1_MspInit 0 */
		/** Initializes the peripherals clocks
		*/
			PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
			PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
			if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
			{
				Error_Handler();
			}

			__HAL_RCC_GPIOC_CLK_ENABLE();
			/**I2C1 GPIO Configuration
			PB7     ------> I2C1_SDA
			PB8     ------> I2C1_SCL
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

			/* Peripheral clock enable */
			__HAL_RCC_I2C3_CLK_ENABLE();
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
		}
	#endif
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
	#if I2C_INIT
		if(hi2c->Instance==I2C3)
		{
		/* USER CODE BEGIN I2C1_MspDeInit 0 */

		/* USER CODE END I2C1_MspDeInit 0 */
			/* Peripheral clock disable */
			__HAL_RCC_I2C3_CLK_DISABLE();

			/**I2C1 GPIO Configuration
			PB7     ------> I2C1_SDA
			PB8     ------> I2C1_SCL
			*/
			HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

			HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);

		/* USER CODE BEGIN I2C1_MspDeInit 1 */

		/* USER CODE END I2C1_MspDeInit 1 */
		}
	#endif
}


