/*
 * usart.h
 *
 *  Created on: Dec 6, 2015
 *      Author: Kamil
 */

#ifndef USART_H_
#define USART_H_

//**********USART**********
/*
 * GPS podpiety do USART2
 * Wykorzystywane tylko Rx i Tx
 * PA2 - Tx
 * PA3 - Rx
 */
#define USART2_port	GPIOA
#define USART2_Rx	GPIO_Pin_3
#define USART2_Tx	GPIO_Pin_2

/*
 * USART1 wykorzystywany do komunikacji z konsola
 * Wykorzystywane tylko Rx i Tx
 * PA9 - Tx
 * PA10 - Rx
 */
#define USART1_port	GPIOA
#define USART1_Rx	GPIO_Pin_10
#define USART1_Tx	GPIO_Pin_9

#define USART1_AF			GPIO_AF_7
#define USART1_Rx_source	GPIO_PinSource10
#define USART1_Tx_source	GPIO_PinSource9

/*
 * Opcjonalnie RS-485 zrealizowany na USART3
 * PB10 - Tx
 * PB11 - Rx
 */


//#define RX_BUFFER_LENGTH 1024
#define USART1_BAUDRATE 115200

void USART1_Init(unsigned long baud_rate);
void USART1_GPIO_Init();

void Echo(const char Echo_char);

void USART_Tx(const char *pUSART_Tx_string);
void USART_Tx_char(char USART_Tx_char); // funkcja do wysylania string
//void fputch(int ch, FILE *f); //funkcja potrzebna do obslugi funkcji printf
//void USART_Rx(const char USART1_Rx_char);

void USART_Init_IRQ(); // wlacza przerwania od uzywanych portow UART
void USART1_IRQHandler();

#endif /* USART_H_ */
