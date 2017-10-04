/*
 * usart.c
 *
 *  Created on: Dec 6, 2015
 *      Author: Kamil
 */

#include <stdio.h>

#include "main.h"
#include "usart.h"
#include "console.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_usart.h"

char g_char;

/*
unsigned char g_USART1_Rx_buf[RX_BUFFER_LENGTH];
unsigned int g_USART1_Rx_buf_index = 0;



void USART_Rx(const char USART1_Rx_char)
{
	g_USART1_Rx_buf[g_USART1_Rx_buf_index] = USART1_Rx_char;
	g_USART1_Rx_buf_index++;

	if (g_USART1_Rx_buf_index == RX_BUFFER_LENGTH - 1)
		g_USART1_Rx_buf_index = 0;

	if (g_USART1_Rx_buf_index > 0)
		USART1_To_Buf(USART1_Rx_char); // funkcja wysyla znaki do dalszego przetwarzania w innym pliku
}
*/

void USART1_Init(unsigned long baud_rate)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitTypeDef USART_InitStruct;

	USART_InitStruct.USART_BaudRate = 				baud_rate;
	USART_InitStruct.USART_HardwareFlowControl = 	USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = 					USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = 				USART_Parity_No;
	USART_InitStruct.USART_StopBits = 				USART_StopBits_1;
	USART_InitStruct.USART_WordLength = 			USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);

	USART_Cmd(USART1, ENABLE);

	USART1_GPIO_Init();
	USART_Init_IRQ();

	//USART_Tx("Uruchomiono USART1 do obslugi konsoli \r\n");

}


void USART1_GPIO_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;

	/*	Tx	*/

	GPIO_InitStruct.GPIO_Pin = USART1_Tx; //PA9, PC4
	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_3;
	GPIO_Init(USART1_port, &GPIO_InitStruct);

	/*	Rx	*/

	GPIO_InitStruct.GPIO_Pin = USART1_Rx; //PA10, PC5
	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_3;
	GPIO_Init(USART1_port, &GPIO_InitStruct);

	/*	Podlaczenie pinow do funkcji alternatywnej USART1	*/

	GPIO_PinAFConfig(USART1_port, USART1_Tx_source, USART1_AF);
	GPIO_PinAFConfig(USART1_port, USART1_Rx_source, USART1_AF);
}

void USART_Init_IRQ()
{
	/*	Przerwania z rejestru RXNE wlaczone na stale.
	 * 	Natomiast przerwania z rejestru TXE wlaczane tylko na czas wysylania danych przez USART.	*/
	//	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStruct;

	/*	Przerwania od UART1	*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init( &NVIC_InitStruct);

	NVIC_EnableIRQ(USART1_IRQn); //niby wlaczone przerwanie EXTI0 w NVIC



	/*	Przerwania od UART2	*/
}


void USART_Tx(const char *pUSART_Tx_string) //dziala
{
	/*
	 * Funckcja wysyla caly string do USART
	 */

	while (*pUSART_Tx_string)
		USART_Tx_char(*pUSART_Tx_string++);

	//USART_Tx_char('\n');

	//*USART_Tx_string==0;s
}

void USART_Tx_char(char USART_Tx_char) //dziala
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, USART_Tx_char);
}

//------------------------------------------------------------------------------
struct __FILE {
    int dummy;
};

/* Potrzebny jesli potrzebna funkcja printf */
/* Struktura FILE jest zaimplementowana w stdio.h */
FILE __stdout;

void putch(int ch, FILE *f)
{
	//if (ch=='\n')			//jesli wykryto \n
		//USART_Tx_char('\r');	// wyslij \r

	USART_Tx_char(ch);		// wyslij char

//	return ch;
}
//------------------------------------------------------------------------------


void Echo(const char Echo_char)
{
	USART_SendData(USART1, Echo_char); // Echo Char
}


void USART1_IRQHandler()
{
	//USART_Rx();
	//char *znak = (char *) g_char;

	while(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // Wait for Char
	{
		const char znak = USART_ReceiveData(USART1); // Collect Char

		Console_Input1(znak);

/*		if (znak <= 127)
		{
			Echo(znak);
			//USART_SendData(USART1, Data); // Echo Char
			//USART_Rx(znak);
			//ChangeStateLED();
			//USART1_To_Buff(znak);
			Console_Input1(znak);
		}
*/
	}

//	while(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) // Wait for Empty
//		USART_SendData(USART1, Data); // Echo Char
}


