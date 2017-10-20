/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**
**  Distribution: The file is distributed “as is,” without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

/* Includes */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "interrupts.h"
#include "usart.h"
#include "calculations.h"
#include "console.h"
#include "gps.h"
#include "led.h"
#include "servo.h"
#include "adc.h"
#include "rtc.h"

#include "stm32f30x.h"
#include "stm32f30x_conf.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_usart.h"


/* Private typedef */

/* Private define  */

/* Private macro */

/* Private variables */

/* Private function prototypes */

/* Private functions */

/* Global variables */
//uint32_t timerFlag;
//const int32_t LED[] = {LED_Rx, LED_Tx, LED_ready};

/*	Zmienne globalne do obslugi usart	*/
//unsigned char g_buforRx[17] = {0};
//unsigned char g_bufRxIndex = 0;
//bool odebranoDane = false;

/*	Zmienne globalne do okreslenia polozenia panelu PV	*/
//float g_latitude; // dlugosc geograficzna
//float g_longitude; //szerokosc geograficzna
//float g_altitude;	//wysokosc nppm

//float g_azimuth; //polozenie w plaszczyznie azymutu x
//float g_elevation; //nachylenie w plaszczyznie elewacji y

/*	Zmienne globalne potrzebne do obliczen	*/

//float g_delta_czasu; // +- sekundy w zaleznosci od dnia roku
//loat g_dekl_slon; // wysokosc slonca nad horyzontem w zaleznosci od dnia roku
//int g_dzien_roku;

/*	Zmienne globalne czasu	*/
//int g_year, g_month, g_day;
//float g_hour, g_minutes, g_seconds;

extern uint32_t g_CalcRDY;

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
// uint32_t ii;

  SysTick_Div_Config(SYSTICK_STD_DIV);

  Servo_Init();
  ADC1_PV_Init();
  USART1_Init(USART1_BAUDRATE);
  LED_Init();
  //Button_Init();
  RTC_PV_Init();
  Button_Init_IRQ();

  Calculations_Init();



  //GPIO_SetBits(LED_port, LED_Rx);

  //Servo_Ctrl(45,45);
  /* Example update ii when timerFlag has been set by SysTick interrupt */
  //ii = 0;
  while (1)
  {
/*	  if (1==timerFlag)
    {

    	timerFlag = 0;
    	ii++;

    }
*/

// --- Sprawdzanie flag i realizacja zadan poza przerwaniami ---

    Console_Flags_Check();

    if (g_CalcRDY == ON)
    	Calc_Scheduler_And_Flags_Check();

  }

  /* Program will never run to this line */
  return 0;
}


void SysTick_Div_Config(int systick_div)
{
	  /* Example use SysTick timer and read System core clock */
	  SysTick_Config(SystemCoreClock/systick_div);  /* 1 ms if clock frequency 72 MHz */

	  SystemCoreClockUpdate();
	 // ii = SystemCoreClock;   /* This is a way to read the System core clock */
}


void KEY_Init()
{

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Pin = Power_out_pin | GPS_power_pin;
	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_2;
	GPIO_Init(Key_power_port, &GPIO_InitStruct);

}

void LED_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Pin = LED_ready | LED_Rx | LED_Tx;
	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_2;
	GPIO_Init(LED_port, &GPIO_InitStruct);

}

void Button_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = 		Button;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_2;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}




void SPI_Magnet_Axio_Init()
{

}

void SPI_GPIO_Init()
{

}

void I2C_Gyro_Init()
{

}

void I2C_GPIO_Init()
{

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  while(1)
  {}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
}

#ifdef USE_SEE
#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval sEE_FAIL.
  */
uint32_t sEE_TIMEOUT_UserCallback(void)
{
  /* Return with error code */
  return sEE_FAIL;
}
#endif
#endif /* USE_SEE */


/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval sEE_FAIL.
  */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)

{
  return;
}
