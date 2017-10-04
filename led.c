/*
 * led_buttons.c
 *
 *  Created on: Feb 24, 2015
 *      Author: Kamil
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "led.h"
#include "main.h"

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include <stm32f30x_gpio.h>



void DefGPIO_out()
{
	/*	Domyslne stany wyjsc sterujacych kluczami tranzystorowymi	*/

	GPIO_WriteBit(Key_power_port, Power_out_pin, Bit_RESET);
	GPIO_WriteBit(Key_power_port, GPS_power_pin, Bit_SET);

}

void ChangeStateLED()
{
	/*	Funkcja wlacza i wylacza diody
	 * 	Przerobic, tak by mozna bylo obsluzyc przy jej pomocy wszystkie wykorzystywane diody
	 * 	*/

	if(GPIO_ReadOutputData(LED_port) & LED_ready)
		GPIO_WriteBit(LED_port, LED_ready, Bit_RESET);
	else
		GPIO_WriteBit(LED_port, LED_ready, Bit_SET);
}

void ChangeStateLEDswitch(uint16_t *LED)
{


	if(GPIO_ReadOutputData(LED_port) & *LED)
		GPIO_SetBits(LED_port, *LED);
	else
		GPIO_ResetBits(LED_port, *LED);
}

void PowerSupply()
{
	/*	Funkcja wlacza i wylacza klucz mocy z paneli PV	*/

	if(GPIO_ReadOutputData(Key_power_port) & Power_out_pin)
		GPIO_WriteBit(Key_power_port, Power_out_pin, Bit_RESET);
	else
		GPIO_WriteBit(Key_power_port, Power_out_pin, Bit_SET);
}

void GPS_on_off()
{
	/*	Funkcja wlacza i wylacza klucz zasilajacy modul GPS
	 * 	Funkcja ma za zadanie ograniczyc zuzycie energii przez sterownik,
	 * 	wiec modul jest wlaczany cyklicznie co pewien czas np. raz dziennie o stalej godzinie
	 * 	przed wschodem Slonca
	 * 	*/

	if(GPIO_ReadOutputData(Key_power_port) & GPS_power_pin)
		GPIO_WriteBit(Key_power_port, GPS_power_pin, Bit_RESET);
	else
		GPIO_WriteBit(Key_power_port, GPS_power_pin, Bit_SET);

}
