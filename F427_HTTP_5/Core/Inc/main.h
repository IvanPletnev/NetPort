/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TERM1_Pin GPIO_PIN_9
#define TERM1_GPIO_Port GPIOC
#define IFMODE1_Pin GPIO_PIN_8
#define IFMODE1_GPIO_Port GPIOA
#define DE1_Pin GPIO_PIN_11
#define DE1_GPIO_Port GPIOA
#define RE1_Pin GPIO_PIN_12
#define RE1_GPIO_Port GPIOA
#define SOCK1_Pin GPIO_PIN_0
#define SOCK1_GPIO_Port GPIOD
#define SOCK2_Pin GPIO_PIN_1
#define SOCK2_GPIO_Port GPIOD
#define TERM2_Pin GPIO_PIN_2
#define TERM2_GPIO_Port GPIOD
#define RE2_Pin GPIO_PIN_3
#define RE2_GPIO_Port GPIOD
#define DE2_Pin GPIO_PIN_4
#define DE2_GPIO_Port GPIOD
#define IFMODE2_Pin GPIO_PIN_7
#define IFMODE2_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
