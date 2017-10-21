/*
 * konsola.c
 *
 *  Created on: Mar 6, 2015
 *      Author: Kamil
 */


/*
 * BUGS:
 * 1. Wpisujac pusty parametr, np. "set pos" bez wartosci nie wykrywa tego jako blad chociaz powinno <= mozna to rozwiazac zliczajac znaki i porownujac z iloscia jaka powinna byc
 */



#include "console.h"

#include "main.h"
#include "usart.h"
#include "calculations.h"
#include "rtc.h"



#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_usart.h"


static char g_USART1_RX_buff[USART1_RX_BUFFER_LENGTH];
static char g_pars_buff[PARSER_BUFFER_LENGHT];


// Inicjacja zmiennych indeksow buforow
static uint32_t g_USART1_RX_buff_index = 0;
static uint32_t g_USART1_RX_buff_head = 0;
static uint32_t g_USART1_RX_buff_tail = 0;


// Flagi
uint32_t g_USART1_BuffANALIZE = 0; // flaga sygnalizowana po wystapieniu znaku EOL i oznaczajaca gotowosc do "parsowania"
uint32_t g_USART1_BuffRELOAD = 0; // flaga sygnalizowana po zakonczonym "parsowaniu" (zeruje index bufora Rx)
uint32_t g_USART1_BuffERR = 0; // flaga sygnalizowana jezeli tablica grozi przepelnieniem
uint32_t g_ParsERR = 0; // flaga ogolnego bledu parsera

uint32_t g_USART1_STRLenght = 0;
//uint32_t g_DOT = 0;
//uint32_t g_COMMA = 0;
//uint32_t g_SPACE = 0;


/*	Zmienne potrzebne do obliczen	*/
extern float g_latitude; // zmienne laczace wartosc z oznaczeniem kierunku (N i S, E i W)
extern float g_longitude;
extern float g_dir_ang;
extern uint32_t g_CalcTRIGG;
extern uint32_t g_CalcTriggTable;
extern uint32_t g_TrackerTEST;
extern int g_ADCTest;

extern uint32_t g_Time_s;

void Console_Flags_Check()
{
/*
 * Sprawdzanie flag
 */

	if (g_USART1_RX_buff_index == 0 || g_USART1_BuffRELOAD == ON)
		g_USART1_RX_buff_head = 0;


	if (g_USART1_RX_buff_index == USART1_RX_BUFFER_LENGTH - 3)
	{
		g_USART1_BuffERR = ON;
		g_USART1_RX_buff_index = 0;
	}


	if (g_USART1_BuffANALIZE == ON) // analiza
	{
		g_USART1_BuffRELOAD = ON;

		Parse();

		g_USART1_BuffANALIZE = OFF;
	}


	if (g_USART1_BuffANALIZE == OFF && g_USART1_BuffRELOAD == ON) // przeladowanie bufora odbiorczego i kasowanie bufora parsera
	{
		USART1_Buff_Reload();
		Pars_Buff_Erase();

		g_USART1_BuffRELOAD = OFF;
	}

	if (g_ParsERR == ON)
	{
		USART_Tx("\rOgolny blad parsera. Sprawdz, czy parametry, ktore wpisales sa na pewno poprawne.\n\r");

		g_ParsERR = OFF;
	}

}


void Console_Input1(const char znak) // odbior z USART1
{
	USART1_To_Buff(znak);
}


void Console_Output1(const char tekst) // nadawanie po USART1
{
	USART_Tx(tekst);
}


void USART1_To_Buff(const char znak) // bufor kolowy == WYWOLYWANY PRZERWANIAMI PO ODEBRANIU ZNAKU Z USART1! <== TEORETYCZNIE JUZ POWINIEN BYC NAPISANY POPRAWNI
{
	//uint32_t *buff_index = (uint32_t *) g_USART1_RX_buff_index;

	if (znak == 127) // ok
		Delete_Char();


	else if (g_USART1_RX_buff_index != USART1_RX_BUFFER_LENGTH - 3 && g_USART1_BuffANALIZE == OFF)
		g_USART1_RX_buff[g_USART1_RX_buff_index] = g_pars_buff[g_USART1_RX_buff_index] = znak;

	else if (g_USART1_RX_buff_index != USART1_RX_BUFFER_LENGTH - 3 && g_USART1_BuffANALIZE == ON)
		g_USART1_RX_buff[g_USART1_RX_buff_index] = znak;


	if (znak < 127 && znak != 252 && znak != 27) // znak 252 to przypadkowy znak ktory dostawal sie do bufora podczas uruchamiania

	{
		Echo(znak);
		g_USART1_RX_buff_index++;
	}





	if(znak == 13) // powrot karetki CR czyli enter
		Enter_Char();

//	return g_USART1_RX_buff_index;

}


void USART1_Buff_Reload() // funkcja poprawiona --- SPRAWDZIC
{
	//uint32_t *buff_index = &g_USART1_RX_buff_index;
	uint32_t i;

	for ( i = 0; i == g_USART1_RX_buff_index - g_USART1_RX_buff_head; i++)
		g_USART1_RX_buff [i] = g_pars_buff [i] = g_USART1_RX_buff[g_USART1_RX_buff_head + i];

	g_USART1_RX_buff_index = 0; // blad bo nadpisze dopiero co skopiowane dane

}


void Pars_Buff_Erase()
{
	int32_t i;

	for (i=0; i<=PARSER_BUFFER_LENGHT; i++)
	g_pars_buff[i] = 0;
}


void Enter_Char()
{
	USART_Tx("\n\r");

	g_USART1_RX_buff [g_USART1_RX_buff_index] = g_pars_buff [g_USART1_RX_buff_index] = 0; // zabezpieczenie

	g_USART1_RX_buff_tail = g_USART1_RX_buff_index; // tail jest tam gdzie string skonczyl sie '0' --- niekoniecznie potrzebne

	g_USART1_STRLenght = g_USART1_RX_buff_tail - g_USART1_RX_buff_head; // --- niekoniecznie potrzebne

	g_USART1_RX_buff_head = g_USART1_RX_buff_index + 1; // head 2

	g_ADCTest = OFF; // enter wylacza podawanie wartosci z ADC

	g_USART1_BuffANALIZE = ON;

}


void Delete_Char()
{
	g_USART1_RX_buff [g_USART1_RX_buff_index-1] = g_pars_buff [g_USART1_RX_buff_index-1] = 0;


	g_USART1_RX_buff_index--;

	USART_Tx("\b");




}


void Parse ()
{
	ConsoleInitTypeDef ConsoleTypeDef;

	char *p = &g_pars_buff [0];

// Czas
	if (!strncmp(p, "set time -h" , 10))
	{
		USART_Tx("set time hh:mm:ss\n\r");
		//USART_Tx("\n\r");

	}

	else if (!strncmp(p, "set time " , 8)) // DZIALA!
	{

		ConsoleTypeDef.hour = (p[9] - '0') * 10 + p[10] - '0';
		ConsoleTypeDef.min = (p[12] - '0') * 10 + p[13] - '0';
		ConsoleTypeDef.sec = (p[15] - '0') * 10 + p[16] - '0';

		Set_Time(ConsoleTypeDef.hour, ConsoleTypeDef.min, ConsoleTypeDef.sec);
		USART_Tx("\n\r");

	}

// Data
	if (!strncmp(p, "set date -h" , 10)) // DZIALA!
		USART_Tx("set date yyyy-mm-dd\n\r");

	else if (!strncmp(p, "set date " , 8))
	{
		ConsoleTypeDef.year = (p[9]-'0')*1000 + (p[10]-'0')*100 + (p[11]-'0')*10 + (p[12]-'0');
		ConsoleTypeDef.month = (p[14]-'0')*10 + (p[15]-'0');
		ConsoleTypeDef.day = (p[17]-'0')*10 + (p[18]-'0');

		Set_Date(ConsoleTypeDef.year, ConsoleTypeDef.month, ConsoleTypeDef.day);
		USART_Tx("\n\r");

	}

// Pozycja
	if (!strncmp(p, "set pos -h", 10)) // musi byc 10 bo na 11 ostatniej pozycji (0-10) jest '0'
	{
		USART_Tx("set pos longitudeN/S latitudeW/E \n\r");
		USART_Tx("Pozycja podawana w katach wyrazonych dziesietnie \n\r");
		USART_Tx("Warszawa PL \n\r");
		USART_Tx("set pos 52.259N 21.020E \n\r");
		//USART_Tx("\n\r");

	}

	else if (!strncmp(p, "set pos " , 7)) // <= DZIALA
	{
		// te deklaracje to bardzo zly nawyk!
		char lat[7], lon[7];
		char *la = &lat[0];
		char *lo = &lon[0];
		int32_t i;

		for (i=0; i<=6; i++)
		{
			lat[i] = g_pars_buff[8+i];
			lon[i] = g_pars_buff[16+i];
		}

		ConsoleTypeDef.lat = atof(la);
		ConsoleTypeDef.lon = atof(lo);

		//ConsoleTypeDef.lat = (p[8] - '0')*10 + (p[9] - '0') + (p[11] - '0')*0,1 + (p[12] - '0')*0,01 + (p[13] - '0')*0,001;
		ConsoleTypeDef.lat_dir = p[14];
		//ConsoleTypeDef.lon = (p[16] - '0')*10 + (p[17] - '0') + (p[19] - '0')*0,1 + (p[20] - '0')*0,01 + (p[21] - '0')*0,001;
		ConsoleTypeDef.lon_dir = p[22];

		Set_Possition(ConsoleTypeDef.lat, ConsoleTypeDef.lat_dir, ConsoleTypeDef.lon, ConsoleTypeDef.lon_dir);
		//USART_Tx("\n\r");

	}


	if (!strncmp(p, "set dir -h \n\r" , 9))
	{
		USART_Tx("Pozycja trackera wzgleden poludnia S.\n\r");
		USART_Tx("Dla S jest 0 st.\n\r");
		USART_Tx("set dir angle \n\r");
		USART_Tx("set dir 180.00 \n\r");
		//USART_Tx("\n\r");
	}


	else if (!strncmp(p, "set dir " , 7))
	{
		char dir[6];
		char *p_dir = &dir[0];
		int32_t i;

		for(i=0; i<=5 ;i++)
			dir[i] = g_pars_buff[8+i];

		ConsoleTypeDef.dir_ang = atof(p_dir);

		Set_Direction_Angle(ConsoleTypeDef.dir_ang);

	}

// Zadania
	if (!strncmp(p, "get measure" , 10))
		MeasureToUSART();


	else if (!strncmp(p, "get time" , 7));


	else if (!strncmp(p, "get pos" , 6));

// Test obliczen
	if (!strncmp(p, "calc test" , 8))
		CalcTest();


// Test elementu wykonawczego trackera
	if (!strncmp(p, "tracker demo" , 11))
	{
		TrackerTest(0);
	}

	if (!strncmp(p, "tracker test on" , 14))
	{
		TrackerTest(1);

	}

	if (!strncmp(p, "tracker test off" , 15))
	{
		//SysTick_Div_Config (SYSTICK_STD_DIV);
		g_TrackerTEST = OFF;

	}

	if (!strncmp(p, "adc test" , 7))
		g_ADCTest = ON;


// Help
	if ((!strncmp(p, "help" , 4)) || (!strncmp(p, "-h" , 2)))
		{
			USART_Tx("\n------------------------------------------Pomoc--------------------------------------------\n\r");
			USART_Tx("Sun Tracker v1.0\n\r");
			USART_Tx("\nSterownik ze stalym krokiem sledzi ruch Slonca na horyzoncie\n\r");
			USART_Tx("\nKazda z komend ma oddzielny plik pomocy wg. ponizszych przykladow:\n\r");
			USART_Tx("set KOMENDA -h\n\r");
			USART_Tx("get KOMENDA -h\n\r");
			USART_Tx("\nLista mozliwych komend:\n\r");
			USART_Tx("set time     		- ustawia czas\n\r");
			USART_Tx("set date     		- ustawia date\n\r");
			USART_Tx("set pos      		- ustawia lokalizacje trackera\n\r");
			USART_Tx("set dir      		- ustawia kierunek bieguna magnetycznego Ziemi\n\r");
			USART_Tx("get measure  		- aktualna zawartosc danych pomiarowych\n\r");
			USART_Tx("get time     		- aktualny czas i date oraz ilosc czasu jaka uplynela od wlaczenia\n\r");
			USART_Tx("get pos      		- wprowadzone polozenie trackera i kierunek wzgleden N \n\r");
			USART_Tx("\nDostepne funkcje testowe: \n\r");
			USART_Tx("calc test    		- wykonanie testu obliczen\n\r");
			USART_Tx("tracker test demo \n\r");
			USART_Tx("tracker test on		- wykonanie testu elementu wykonawczego trackera\n\r");
			USART_Tx("tracker test off	- zakonczenie testu elementu wykonawczego trackera\n\r");
			USART_Tx("adc test	- podaje aktualne wartosci odczytane przez ADC\n\r");
			USART_Tx("---------------------------------------------------------------------------------------------\n\r");

			USART_Tx("\n\r");
		}

/*
	else
		g_ParsERR = ON;
*/

}


void Set_Time(uint32_t hh, uint32_t mm, uint32_t ss) // ustawia czas RTC <= DZIALA
{

	int8_t set = 0;

	if (hh >= 0 && hh <=23)
		set++;

	if (mm >= 0 && mm <= 59)
		set++;

	if (ss >= 0 && ss <= 59)
		set++;


	if (set != 3)
	{
		USART_Tx("Blad w wprowadzonych parametrach czasu!\n\r");

		g_ParsERR = ON;
	}

	else if (set == 3)
	{
		//RTC_Set_Time(hh, mm, ss);
		SetCalcTime(hh, mm, ss);
		CalcTriggTableIn (1, ON);
		CalcTriggTableCheck();
	}


}


void Set_Date (uint32_t yyyy, uint32_t mm, uint32_t dd) // ustawia date RTC
{
	int8_t set = 0;

	if (yyyy >= 2017)
		set++;

	if (mm >= 1 && mm <= 12)
		set++;

	if (dd >= 1 && dd <= 31)
		set++;


	if (set != 3)
	{
		USART_Tx("\n\r Blad w wprowadzonych parametrach daty!\n\r");

		g_ParsERR = ON;
	}

	else if (set == 3)
	{
		//RTC_Set_Date(yyyy, mm, dd);
		SetCalcCalendar(yyyy, mm, dd); // kalendarz oparty na taktowaniu systick
		CalcTriggTableIn (0, ON);
		CalcTriggTableCheck();
	}
}


void Set_Possition (float latitude, char lat_dir,float longitude, char lon_dir)
{
	int8_t set = 0;

	if (latitude > 0 || latitude <= 90)
		set++;

	if (longitude > 0 || longitude <= 180)
		set++;

	if (lat_dir == 'N' || lat_dir == 'S')
		set++;

	if (lon_dir == 'W' || lon_dir == 'E')
		set++;


	if (set != 4)
	{
		USART_Tx("\n\r Blad w wprowadzonych parametrach pozycji!\n\r");

		g_ParsERR = ON;
	}

	else if (set == 4) // <= uzupelnienie zmiennych globalnych
		{
		/*	Konwersja zmiennych float i char w pojedynczy float ze znakiem	*/
		if(lat_dir == 'N')
			g_latitude = latitude;
		if(lat_dir == 'S')
			g_latitude =  latitude * (-1);

		if(lon_dir == 'E')
			g_longitude = longitude * (-1);
		if(lon_dir == 'W')
			g_longitude = longitude;

		CalcTriggTableIn(2, ON);
		CalcTriggTableCheck();
		}



}

void Set_Direction_Angle(float dir_ang)
{
	int8_t set = 0;

	if(dir_ang >= 0 && dir_ang <= 359.99)
		set++;

	if(set != 1)
	{
		USART_Tx("\n\r Blad w wprowadzonych parametrach kierunku!\n\r");

		g_ParsERR = ON;
	}

	else if(set == 1) // <= Stworzyc przekierowanie!!!
	{
		g_dir_ang = dir_ang;
		CalcTriggTableIn(3, ON);
		CalcTriggTableCheck();
	}

}


void Get_Time()
{

}


void Get_Date()
{

	//int32_t i = 0;

}


void Get_Position()
{

}


void Get_DirectionAngle()
{

}


void NMEA_Parse()
{
	/*
	 * Funkcja rozkladajaca ramki z GPS
	 */


}


void USART1_Help() // nie uzywane
{
	/*
	 * 	Funkcja ktora wyrzuca mozliwe opcje
	 */

	int comunicate = 0;

	//char help_console[COMUNICATE_MAX];

	//help_console [0] = "set -h";

	for (comunicate = 0; comunicate <= COMUNICATE_MAX; comunicate++);


}
