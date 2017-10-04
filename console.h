/*
 * console.h
 *
 *  Created on: Mar 8, 2015
 *      Author: Kamil
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{

	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t hour;
	uint32_t min;
	uint32_t sec;
	float lat;
	char lat_dir;
	float lon;
	char lon_dir;
	float dir_ang;

} ConsoleInitTypeDef;


#define USART1_RX_BUFFER_LENGTH 1024 // dlugosc bufora kolowego

#define PARSER_BUFFER_LENGHT 128 // dlugosc bufora liniowego

#define COMUNICATE_MAX 9

#define ON 1
#define OFF 0


void Console_Flags_Check();

void Console_Input1(const char znak);
void Console_Output1(const char tekst);

void USART1_To_Buff(const char ch);
void Parse();
void USART1_Help();
void USART1_Buff_Reload();
void Pars_Buff_Erase(); // Mozna w przyszlosci ograniczyc obszar zerowania. Aktualnie zeruje caly bufor.
void Enter_Char();
void Delete_Char();

void Set_Time(uint32_t hh, uint32_t mm, uint32_t ss); // ustawia czas RTC
void Set_Date (uint32_t yyyy, uint32_t mm, uint32_t dd); // ustawia date RTC
void Set_Possition (float latitude, char lat_dir,float longitude, char lon_dir);
void Set_Direction_Angle(float dir_ang);

void Get_Time();
void Get_Date();
void Get_Position();
void Get_Direction_Angle();

void NMEA_Parse();

#endif /* CONSOLE_H_ */
