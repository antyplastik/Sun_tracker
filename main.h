/*
 * main.h
 *
 *  Created on: Nov 2, 2014
 *      Author: inz. Kamil Ptasinski
 */

#define SYSTICK_STD_DIV		1000
#define SYSTICK_TEST_DIV	10

//**********Sygnalizacja LED**********

#define LED_port	GPIOE
#define LED_Rx		GPIO_Pin_9
#define LED_Tx		GPIO_Pin_10
#define	LED_ready	GPIO_Pin_11


//**********Przyciski**********
#define Button_port GPIOA
#define Button GPIO_Pin_0

//**********Kompas cyfrowy**********
/*
 * Interfejs I2C
 *
 * PB6 - SCL	(I2C)
 * PB7 - SDA	(I2C)
 * PE5 - INT2	(I2C)
 * PE4 - INT1	(I2C)
 */

#define kmps_I2C	GPIOB
#define kmps_SCL	GPIO_Pin_6
#define kmps_SDA	GPIO_Pin_7
#define kmps_port	GPIOE
#define kmps_INT1	GPIO_Pin_4
#define kmps_INT2	GPIO_Pin_5

//**********Zyroskop cyfrowy (MEMS)**********
/*
 * Interfejs SPI
 *
 * PA5 - SCK	(SPI)
 * PA7 - MOSI	(SPI)
 * PA6 - MISO	(SPI)
 * PE3 - CS		(SPI)
 * PE1 - INT2
 * PE0 - INT1
 */

#define mems_SPI_port		GPIOA
#define mems_SCK			GPIO_Pin_5
#define mems_MISO			GPIO_Pin_6
#define mems_MOSI			GPIO_Pin_7
#define mems_przerw_port	GPIOE
#define mems_INT1			GPIO_Pin_0
#define mems_INT2			GPIO_Pin_1
#define mems_CS				GPIO_Pin_3



//**********Wyjscia sterujace kluczami	**********

#define Key_power_port	GPIOC

#define Power_out_pin	GPIO_Pin_0	//	Klucz wyjsciowy mocy z PV
#define GPS_power_pin	GPIO_Pin_1	//	Klucz zasilajacy GPS



//**********Zegar czasu rzeczywistego RTC**********



/*
 * Aktualnie USB nie uzywane do komunikacji z konsola
 * PA11 - D-
 * PA12 - D+
 */


/****************	Prototypy funkcji	****************/
void SysTick_Div_Config(int systick_div);
/*	Prototypy funkcji inicjacji GPIO w trybie normalnym i AF	*/
void LED_Init();
void Button_Init();
//czujniki cyfrowe
void SPI_Magnet_Axio_Init();
void SPI_GPIO_Init();
void I2C_Gyro_Init();
void I2C_GPIO_Init();


