/*
 * obliczenia.c
 *
 *  Created on: Feb 23, 2015
 *      Author: Kamil
 */

/*
 * PV Tracker Module V1.0
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "calculations.h"
#include "console.h"
#include "servo.h"
#include "rtc.h"
#include "adc.h"
#include "main.h"
#include "led.h"


// !!!!!!!!! ZAMIENIC ZMIENNE GLOBALNE FLAG NA POZYCJE BITOWE W 8-bit TABLICY !!!!!!!!!
uint32_t g_CalcTRIGG = 0; // flaga wyzwalajaca obliczenia <= ma 3 stany! ON, OFF i Change (wymuszone z powodu zmiany parametrow podanych przez konsole)

uint32_t g_CalcRDY; // flaga wystawiona jezeli zostaly wprowadzone parametry do obliczen ()
uint32_t g_CalcERR = 0;
uint32_t g_TrackerTEST = 0; // flaga testu trackera <= przyspiesza czas
uint32_t g_CalcLeapYear = 0; // flaga ma wartosc 1 jezeli rok jest przestepny
uint32_t g_CalcCalSumerTime = 1; // domyslnie flaga dla czasu letniego
uint32_t g_CalcMeasSTP = 0;
extern int g_ADCTest;

// zmienna laczaca wartosc z oznaczeniem kierunku (N i S, E i W)
float g_latitude = 52.259;	// szerokosc geo N lub S
float g_longitude = 21.020;	// dlugosc geo E lub W
float g_dir_ang = 0; // 0st to N

// !!!!!!!!! ZAMIENIC ZMIENNE GLOBALNE CZASU I DATY NA POZYCJE BITOWE W 8-bit TABLICY !!!!!!!!!
uint32_t g_Calc_Year; // <============================ gdzies zeruje sie  rok i przez to oblicza glupoty!!!!!!!!!!!!!!!!!!!!!
uint32_t g_Calc_month;
uint32_t g_Calc_day;
uint32_t g_Calc_hour;
uint32_t g_Calc_minute;
uint32_t g_Calc_second;

/*
extern uint32_t g_RTC_Year;
extern uint32_t g_RTC_month;
extern uint32_t g_RTC_day;
extern uint32_t g_RTC_hour;
extern uint32_t g_RTC_minute;
extern uint32_t g_RTC_second;
*/

//float g_pan_x = 0; // nachylenie na osi NS	+ N w gore a - N w dol
//float g_pan_y = 0; // nachylenie na osi EW	+ W w gore a - W w dol

uint32_t g_Calc_Counter = 0;

uint32_t g_Time_s = 0; // zmienna laczaca godziny, minuty i sekundy do obliczen <= podpiete do systick
//uint32_t g_ON_timer = 0;
uint32_t g_Time_ms = 0;
uint32_t g_TimeZone = 0;
uint32_t g_Test_time_simul = 0;
uint32_t g_Test_time_simul_prev_val = 0;

uint32_t g_measure_count = 0;
//uint32_t g_Measure_day = 1;	// licznik dni pomiaru

uint32_t g_day_of_year;
float g_delta_time = 0;
float g_sun_declination = 0;
float g_sunrise = 0;
float g_sunset = 0;
uint32_t g_zone_meridian = 0; // domyslnie 0st dla UTC
float g_Sun_H = 0;
float g_Sun_V = 0;

uint32_t g_Servo_H_val = 0;
uint32_t g_Servo_V_val = 0;

float g_SunHTableVal[TRACKER_MAX_TIME_STEPS];
float g_SunVTableVal[TRACKER_MAX_TIME_STEPS];
uint32_t g_ServoHTableVal[TRACKER_MAX_TIME_STEPS];
uint32_t g_ServoVTableVal[TRACKER_MAX_TIME_STEPS];
float g_SolarTimeVal[TRACKER_MAX_TIME_STEPS];
float g_HSAVal[TRACKER_MAX_TIME_STEPS];

uint32_t g_tracker_test_step_count = 0;
uint32_t g_tracker_test_measure_count = 0;

uint32_t g_CalcTriggTableIndex = 0;

float g_tracker_start_time, g_tracker_stop_time;

uint32_t g_Tracker_Step_Count = 0; // na ktorym kroku jest algorytm trackera
uint32_t g_Measure_Step_Count = 0; // na ktorym kroku jest algorytm zapisu pomiaru

uint32_t g_Tracker_Step_Value = 0; // zmienna przechowuje ilosc krokow jaka musi wykonac tracker w ciagu dnia
uint32_t g_Measure_Step_Value = 0; // zmienna przechowuje liczbe pomiarow jaka zostanie wykonana w ciagu dnia <= aktualnie 56 pomiarow w ciagu doby dla 14h

uint32_t g_CalcTriggTable [TRIGG_VEC];
uint32_t g_TrackerTimeStepsTable [TRACKER_MAX_TIME_STEPS];
//uint32_t g_TrackerTimeStepsTableH [TRACKER_MAX_STEPS_H];
//uint32_t g_TrackerTimeStepsTableV [TRACKER_MAX_STEPS_V];
uint32_t g_MeasureIntervalTable	[MEASURE_MAX_STEPS];

float g_ADC1MeasureTable [MEASURE_MAX_STEPS] [MEASURE_TABLE_COL]; // [g_measure_count,g_day_of_year, g_Time_s, V1, I1, V2, I2,g_Sun_H ,g_Sun_V]
volatile char g_Measure_To_USART_str[12] = {"0\0"}; // tabela do string zaciagnieta z pliku adc.c
extern uint32_t g_ADC_PV_value_conv [ADC1_DMA_CHANNELS];

/* ========================================= BLOK INICJACJI, WEJSC I WYJSC OBLICZEN ========================================= */

void Calculations_Init()
{
	/* Zerowanie tablic */

	CalcTriggTableClean();

	MeasureTableClean();
	MeasureIntervalTableClean();

	TrackerCountTableClean();

	CalculationOutputTableClean();

	g_CalcRDY = OFF;
}


void CalculationsInput()
{
	// zrodlo zegarowe dla zegarow obliczen <= sterowane bezposrednio z systick

	g_Time_ms++; //
	if (g_Time_ms == 500)
		ChangeStateLED();

	if (g_Time_ms == 1000)
	{
		g_Time_s++;
		//g_ON_timer++;
		g_Time_ms = 0;
		ChangeStateLED();

		g_CalcRDY = ON;

		if (g_ADCTest == ON)
			ADC_Test();

		if (g_TrackerTEST == ON)
		{
			TrackerTestScheduler(); // co [s] wczytywana wartosc czasu z tablicy
		}
	}
}


/*
void PositionConverter() // <= niepotrzebne
{
	//Funkcja przelicza stopnie, minuty i sekundy lokalizacji na wartosci dziesietne
	//http://geo.ur.krakow.pl/?show=poradnik&poradnik=1


}
*/


void CalculationsOutput()
{
	// Tablice przechowywujace obliczone wartosci polozenia Slonca i ustawienia serwomechanizmow
	uint32_t step_count;

	if (g_TrackerTEST == ON)
		step_count = g_tracker_test_step_count;
	else
		step_count = g_Tracker_Step_Count;

	g_SunHTableVal[step_count] = g_Sun_H;
	g_SunVTableVal[step_count] = g_Sun_V;
	g_ServoHTableVal[step_count] = g_Servo_H_val;
	g_ServoVTableVal[step_count] = g_Servo_V_val;

	if (step_count == g_Tracker_Step_Value) // plapka
		step_count = 0;

}


void CalculationOutputTableClean()
{
	int i;
	for(i = 0; i <= TRACKER_MAX_TIME_STEPS; i++)
	{
		g_SunHTableVal[i] = 0;
		g_SunVTableVal[i] = 0;
		g_ServoHTableVal[i] = 0;
		g_ServoVTableVal[i] = 0;
		g_SolarTimeVal[i] = 0;
		g_HSAVal[i] = 0;
	}
}


void MoveServos() // OK!!!
{
	/*
	 * Przekazanie wynikow obliczen sterownikowi urzadzen wykonawczych. W tym przypadku serwomotorów prostopadlych wzgledem siebie w ukladzie elewacja-azymut.
	 * Wartosci przekazywane sa w stopniach typu int
	 *
	 * Algorytm obliczania pozycji Slonca oblicza w zakresie -180 (E) do 180 (W)
	 * Kierunek wzgledem polnocy N jest w zakresie 0 - 359.99
	 * Powyzsze wartosci nalezy przekonwertowac do zakresu pracy serwa, czyli 0 - 180 dla kata azymutu
	 *
	 */
	float Sun_H;
	//uint32_t Sun_V = 0;

	float servo_H = 0, servo_V = 0;
	//float dir_ang = 0; // korekta kata kierunku 0st wzgledem N
//	float Sun_H; // korekta na zakres 0 - 359.99

	//g_latitude; // szerokosc geo N(+) lub S(-)
	//g_longitude; // dlugosc geo E(+) lub W(-)

	//Servo_Ctrl(0, 90);
/*
	if (g_Sun_H >= 0)
		Sun_H = roundf((g_Sun_H)/2);
	else if (g_Sun_H < 0)
	{
		Sun_H = ((g_Sun_H)/2) * (-1);
		Sun_H = roundf(Sun_H);
	}
*/

	Sun_H = roundf(g_Sun_H);
	servo_V = roundf(g_Sun_V);

// KONWERSJA SLEDZENIA SLONCA W ZALEZNOSCI OD POLKULI

	if (g_latitude >= 0) // zakres pracy korzystny dla polkuli N
	{
		// Algorytm obliczania pozycji Slonca wskazuje w zakresie -180st (E) do 180st (W)
		// Dla serwa E = 180st, S = 90st, W = 0st
		// Algorytm		-180		0		180

		if (Sun_H >= -180 && Sun_H <= 0) // od rana do poludnia
			servo_H = 90 - (Sun_H /2);
		//else if (Sun_H == 0)
		//	servo_H = 90;
		else if (Sun_H <= 180 && Sun_H > 0) // do wieczora
			servo_H = 90 - (Sun_H /2); //ok

	}

	else if (g_latitude < 0) // zakres pracy korzystny dla polkuli S
	{
		// Algorytm obliczania pozycji Slonca wskazuje w zakresie -180st (E) do 180st (W)
		// Dla serwa E = 0st, N = 90st, W = 180st
		// Algorytm		-180		0		180

		if (Sun_H >= -180 && Sun_H <= 0)
			servo_H = 90 + (Sun_H /2);
		else if (Sun_H <= 180 && Sun_H > 0)
			servo_H = 90 + (Sun_H /2);
	}



	g_Servo_H_val = servo_H;
	g_Servo_V_val = servo_V;

	if ((servo_H >= 0 && servo_H <= 180) && (servo_V >= 0 && servo_V <= 90))
		Servo_Ctrl((int32_t)servo_H, (int32_t)servo_V); // wejscie w deg

	CalculationsOutput();
}

/* ========================================= BLOK GENEROWANIA POMIAROW ========================================= */

void ADC1ToMeasureTable()
{
	// Funkcja generujaca rekord w tabeli pomiarowej

	// uint32_t g_ADC1MeasureTable [MEASURE_MAX_STEPS] [9]; // [g_measure_count,g_day_of_year, g_Time_s, V1, I1, V2, I2,g_Sun_H ,g_Sun_V]
	int32_t i, time;

	/*
	if (g_TrackerTEST == ON)
		time = g_TEST_Fast_Time;
	else
		time = g_Time_s;
	*/

	time = g_Time_s;

	if (g_measure_count <= MEASURE_MAX_STEPS)
	{
/*
		if (g_measure_count == 0) // kontrolka
			g_measure_count = 1;
*/

		g_ADC1MeasureTable [g_measure_count] [0] = g_measure_count;
		g_ADC1MeasureTable [g_measure_count] [1] = g_day_of_year;
		g_ADC1MeasureTable [g_measure_count] [2] = time;

		for (i=1; i <= ADC1_DMA_CHANNELS; i++)
			g_ADC1MeasureTable [g_measure_count][i+2] = g_ADC_PV_value_conv [i-1]; // przesuniecie pomiedzy poszczegolnymi kolumnami tablic

		g_ADC1MeasureTable [g_measure_count] [7] = g_Sun_H;
		g_ADC1MeasureTable [g_measure_count] [8] = g_Sun_V;

		if (g_measure_count == MEASURE_MAX_STEPS)
			g_measure_count = 0;
	}

}


void MeasureTableClean()
{
	uint32_t l, c; // line, column <= do zerowania tablicy pomiarowej

	for (l = 0; l <= MEASURE_MAX_STEPS -1; l++) // zerowae tablicy pomiarowej
	{
		for (c = 0; c <= MEASURE_TABLE_COL -1; c++)
			g_ADC1MeasureTable [l][c] = 0;
	}
}


void MeasureToUSART() // Teoretycznie OK!!!
{
	// Funkcja tworzy naglowek tabeli pomiarowej oraz wysyla jej zawartosc do portu USART1
	// Funkcja wywolywana przez konsole (USART1)

	// uint32_t g_ADC1MeasureTable [MEASURE_MAX_STEPS] [9]; // [g_measure_count,g_day_of_year, g_Time_s, V1, I1, V2, I2,g_Sun_H ,g_Sun_V]

	uint32_t w, k, i;

	for(i = 0; i <= MEASURE_TABLE_COL-1; i++) // generuje tytuly kolumn tablicy pomiarowej
	{
		if (i == 0)
			USART_Tx("No");
		if (i == 1)
			USART_Tx("Day");
		if (i == 2)
			USART_Tx("Time[s]");
		if (i == 3)
			USART_Tx("V1[mV]");
		if (i == 4)
			USART_Tx("I1[mA]");
		if (i == 5)
			USART_Tx("V2[mV]");
		if (i == 6)
			USART_Tx("I2[mA]");
		if (i == 7)
			USART_Tx("H[deg]");
		if (i == 8)
			USART_Tx("V[deg]");

		USART_Tx(g_Measure_To_USART_str);

		if (i < MEASURE_TABLE_COL-1)
			USART_Tx("   ");

		else if (i == MEASURE_TABLE_COL-1) // koniec linii
			USART_Tx("\n\r");
	}


	for (w = 0; w <= g_measure_count; w++)
	{
		for (k = 0; k <= MEASURE_TABLE_COL-1; k++) // druk wiersza
		{

			if (k == 0)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 1)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 2)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 3)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 4)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 5)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 6)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 7)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);

			if (k == 8)
				sprintf((char *)g_Measure_To_USART_str, "%d", g_ADC1MeasureTable[w][k]);


			USART_Tx(g_Measure_To_USART_str);

			if (k < MEASURE_TABLE_COL-1)
				USART_Tx("   ");

			else if (k == MEASURE_TABLE_COL-1) // koniec linii
				USART_Tx("\n\r");


			/*
			 * Znaki:
			 *  \r wyswietla dane w kolumnie (pierwszy znak string wedruje do poczatku transmitowanej linijki)
			 *  \n zaczyna od nowej linii
			 */
		}

	}

}


/* ========================================= BLOK TESTOWY ========================================= */

void CalcTest()
{
	USART_Tx("\nUstawiono warunki testowe:\n\r");
	USART_Tx("set date 2017-06-01\n\r");

	if (g_TrackerTEST == ON)
		USART_Tx("set time 00:00:01\n\r");
	else
		USART_Tx("set time 12:10:05\n\r");

	USART_Tx("set pos 52.259N 21.020E\n\r");
	USART_Tx("set dir 10\n\r");
	//USART_Tx("\n\r");

	// Data
	Set_Date(2017, 6, 1);
//	CalcTriggTableIn(0,ON);

	// Czas
	if (g_TrackerTEST == ON)
		Set_Time(0,0,1);
	else
		Set_Time(12,10,05); // sprawdzic dzialanie
//	CalcTriggTableIn(1,ON);

	// Pozycja
	Set_Possition(52.259, 'N', 21.020, 'E');
	//CalcTriggTableIn(2,ON);

	// Kierunek
	Set_Direction_Angle(10);
//	CalcTriggTableIn(3,ON);

	//CalcTriggTableCheck();

}


void TrackerTest(uint32_t arg)
{
	/*
	 * Test trackera
	 * Jezeli arg = 0 - demo
	 * Jezeli arg = 1 - test z parametrami wprowadzonymi przez uzytkownika
	 */

	USART_Tx("\nTest trackera\n\r");

	g_TrackerTEST = ON; // wlaczenie funkcji testowej trackera

	if (arg == 0 || arg == 1)
	{
		if (arg == 0)
			CalcTest(); // test z wartosciami poczatkowymi

		CalcTriggTableCheck();
		g_TrackerTEST = ON; // wlaczenie funkcji testowej trackera
	}

	//USART_Tx("\nCzas biegnie szybciej\n\r");
}


void TrackerTestScheduler()
{
	/*
	 * Funkcja wywolywana co 1s. Jedno wywolanie jest rowne zmianie pozycji odczytywanej tablicy. Funkcja laduje do zmiennej g_Test_time_simul wartosc z tablicy obliczonych stepli czasowych
	 */

	//g_tracker_test_step_count = g_Tracker_Step_Count;

	if (g_tracker_test_step_count <= g_Tracker_Step_Value && g_TrackerTimeStepsTable [g_tracker_test_step_count] != 0) // OK!
		g_Test_time_simul = g_TrackerTimeStepsTable [g_tracker_test_step_count];
/*
	if (g_tracker_test_measure_count <= g_Measure_Step_Value) // <= dopisac wyrzucanie rekordu tabeli przez USART
	{
		g_Test_time_simul = g_MeasureIntervalTable [g_tracker_test_measure_count];

		//SunPosition();
		//ADC1ToMeasureTable();

		if (g_tracker_test_measure_count == g_Measure_Step_Value)
			g_tracker_test_measure_count = 0;
		else
			g_tracker_test_measure_count++;
	}
*/
}


void TrackerError()
{
	g_CalcERR = OFF;
}

/* ========================================= BLOK KOLEJNOSCI WYKONYWANYCH OBLICZEN ========================================= */

void Calculations()
{
	/*
	 * Obliczenia wykonywane cyklicznie
	 */

	int year = g_Calc_Year;
	int month = g_Calc_month;
	int day = g_Calc_day;

	DayOfYear(year, month, day); // dziala	raz dziennie
	ExuationOfTime(); // dziala			raz dziennie
	TimeZone();
	SunDeclination(); // dziala			raz dziennie
	SunriseSunset_3(); // dziala		raz dziennie
	SunPosition(); //	dziala			co krotki interwal czasowy zalezny od SunriseSunset_3

	//CalcTriggTableClean();

}


/* ========================================= BLOK HARMONOGRAMOW POMIAROW, OBLICZEN I KROKOW TRACKERA ========================================= */

void Calc_Scheduler_And_Flags_Check()
{
	CalcCalendar(g_Calc_Year, g_Calc_month, g_Calc_day); // kalendarz oparty na systick
	//RTC_Time_to_g_variables(); // dla kalendarza opartego o RTC

	MeasureCount(); // funkcja sprawdza czy nastal czas pomiaru

	TrackerStepCount(); // funkcja sprawdza czy nastal czas przestawienia trackera

	//g_CalcTRIGG = CHANGE

	if (g_Time_s == g_tracker_stop_time)
	{
		CalcNewDay();
	}

	g_CalcRDY = OFF;

	if (g_CalcERR == ON)
		TrackerError();
}


void CalcTriggTableIn(uint32_t i, int32_t in)
{
	/* Funkcja jedynie uzupelnia wartosci w tabeli przechowujacej informacje o wykreytych zmianach konkretnych parametrow potrzebnych do obliczen
	 *
	 * g_CalcTriggTable {date, time, pos, dir}
	 *
	*/

	g_CalcTriggTable [i] = in;


	//g_CalcTriggTableIndex++;

}


void CalcTriggTableCheck() //OK
{
	/* Funkcja sprawdza zawartosc tablicy g_CalcTriggTable. Ta tablica zawiera informacje odnosnie modyfikowanych parametrow. Po wykonaniu potrzebnych obliczen nastepuje zerowanie.
	 *
	 * g_CalcTriggTable {date, time, pos, dir}
	 *
	 */

	uint32_t n, i = 0;
	uint32_t date, time, pos, dir = 0;

	for (i = 0; i <= TRIGG_VEC-1; i++)
	{

		n = g_CalcTriggTable [i];

		/*
		if (n == 1)
			g_CalcTRIGG = CHANGE;
		*/

		if (i == 0 && n == ON) // date
			date = ON;

		if (i == 1 && n == ON) // time
			time = ON;

		if (i == 2 && n == ON) // pos
			pos = ON;

		if (i == 3 && n == ON) // dir
			dir = ON;

	}

	if (date == ON && time == ON && pos == ON && (dir == ON || dir == OFF))
	{
		if (dir == OFF)
			USART_Tx("\nDomyslnie parametr dir = 0\n\r");

		if (dir == ON)
			USART_Tx("\nPozycjonowanie z uwzglednieniem parametru dir\n\r");;

		CalcTriggTableClean();

		Calculations();

		CleanStepsTables();
		CalcSteps();

		USART_Tx("\nUstawiono wszystkie wymagane parametry.\n\r");
		USART_Tx("Wlaczono symulacje sledzenia Slonca.\n\r");

		g_CalcRDY = ON; // flaga wlaczajaca automat
	}

}


void CalcTriggTableClean()
{
	uint32_t i;
	for (i = 0; i <= TRIGG_VEC-1; i++)
		g_CalcTriggTable [i] = 0;

	g_CalcTRIGG = OFF;
}


void SetCalcTime(uint32_t hour, uint32_t minute, uint32_t second)
{
	//CalcTriggTableIn(1, CHANGE);

	g_Time_s = hour*3600 + minute*60 + second;

}


void SetCalcCalendar(uint32_t Year, uint32_t month, uint32_t day)
{
	//CalcTriggTableIn(0, CHANGE);

	g_Calc_Year = Year;
	g_Calc_month = month;
	g_Calc_day = day;

}

void CalcSteps()
{
	// Funkcja wywoluje obliczenia krokow pomiarow i krokow trackera

	MeasureInterval();
	TrackerStep();
}


void CleanStepsTables()
{
	// Funkcja czysci tablice krokow pomiarow i trackera

	MeasureIntervalTableClean();
	TrackerCountTableClean();
}


void CalcCalendar(uint32_t Year, uint32_t month, uint32_t day)
{
	// Funkcja pelniaca role kalendarza opartego na g_Time_s czyli systick


	if (g_Time_s == 86400) // zakonczenie doby
	{
		g_Time_s = 0;
		g_Calc_day++;
		g_day_of_year++;
//		g_Measure_day++; // dzien pomiarowy
		CalculationOutputTableClean();

	}

// Zerowanie dni roku po osiagnieciu kolejnego dnia po ostatnim dniu roku
	if (g_day_of_year == 365 && g_CalcLeapYear == OFF)
	{
		g_Calc_day = g_Calc_month = g_day_of_year = 1;
		g_Calc_Year++;
	}

	else if (g_day_of_year == 366 && g_CalcLeapYear == ON)
	{
		g_Calc_day = g_Calc_month = g_day_of_year = 1;
		g_Calc_Year++;
	}

	// Automatyczna korekcja czasu <= rozbudowac funkcje o strefy czasu
	int32_t n;

	if(g_CalcLeapYear == OFF)
		n = 0;
	else if (g_CalcLeapYear == ON)
		n = 1;

	if (g_day_of_year >= 79 && g_day_of_year <= 264)
		g_CalcCalSumerTime = 1;
	else if (g_day_of_year >= 265 && g_day_of_year <=78)
		g_CalcCalSumerTime = 0;


	if ((g_day_of_year == 79 + n) && g_Time_s == 10800)// rownonoc wiosenna / zegar przestawia sie z 3:00 na 4:00
	{
		g_Time_s = g_Time_s + 3600;
		TimeZoneSwitch();
	}

	if (g_day_of_year == 171 + n)// przesilenie letnie
		;

	if ((g_day_of_year == 265 + n) && g_Time_s == 10800)// rownonoc zimowa / zegar przestawia sie z 3:00 na 2:00
	{
		g_Time_s = g_Time_s - 3600;
		TimeZoneSwitch();
	}

	if (g_day_of_year == 355 + n)// przesilenie zimowe
		;
}


void TimeZone()
{
	/*
	 *	Funkcja obliczajaca strefe czasowa na podstawie lokalizacji.
	 *	g_longitude przyjmuje wartosci ujemne dla E i wartosci dodatnie dla W (ustawione w parserze)
	 */

	//g_CalcCalSumerTime;
	//g_longitude;

	float roundf_time_zone;

	roundf_time_zone = roundf((g_longitude / 15) * (-1));

	if (g_CalcCalSumerTime == 1)
		roundf_time_zone++;

	g_zone_meridian = roundf_time_zone * 15;
}


void TimeZoneSwitch()
{
	/*
	 * Funkcja zmiany strefy czasowej powiazana z kalendarzem
	 */

	if (g_CalcCalSumerTime == 1 && g_CalcTriggTable[1] == 0) // ulega zmianie jezeli wymusza to obliczany kalendarz
		{
			if (g_longitude > 0 && g_longitude < 0)
				g_TimeZone =+ 1;
			else if (g_longitude == 0)
				g_TimeZone = 0;

		}
	else if (g_CalcCalSumerTime == 0 && g_CalcTriggTable[1] == 0)
	{
			if (g_longitude > 0 && g_longitude < 0)
				g_TimeZone =- 1;
			else if (g_longitude == 0)
				g_TimeZone = 0;
	}
}


void CalcNewDay()
{
	// Funcja wymusza obliczenie krokow dla nastepnego dnia

	g_Calc_day++;

	Calculations();

	TrackerCountTableClean();
	TrackerStep();

	g_Calc_day--;
}


void MeasureInterval() // OK
{
	// Funkcja oblicza o ktorych godzinach ma nastapic pomiar i wartosci maja byc wpisane do tablicy pomiarowej <= wystarczy wykonac 1x dziennie lub gdy zmieniono godziny pomiarowe

	int32_t last_measure;
	int32_t measure_step_stop = ON;
	g_Measure_Step_Value = 0;
/*
	g_Measure_Step_Value = LAST_MEASURE - FIRST_MEASURE; // aktualnie jest to 14h czyli okres pomiarowy dzienny rowny 50400s
	g_Measure_Step_Value = g_Measure_Step_Value / MEASURE_INTERVAL; // ilosc pomiarow w ciagu dnia z pewnym interwalem czasowym

	for (i = 0; i <= g_Measure_Step_Value; i++)
	{
		if(i > 0 && i <= g_Measure_Step_Value)
			g_MeasureIntervalTable [i] = g_MeasureIntervalTable [i-1] + MEASURE_INTERVAL;

		else if (i == 0)
			g_MeasureIntervalTable [i] = FIRST_MEASURE;
	}
*/

	while (measure_step_stop == ON)
	{

		if (g_Measure_Step_Value==0)
		{
			g_MeasureIntervalTable [g_Measure_Step_Value] = FIRST_MEASURE;
			g_Measure_Step_Value = 1;
		}

		if (g_MeasureIntervalTable [g_Measure_Step_Value] < LAST_MEASURE && g_Measure_Step_Value > 0 && g_Measure_Step_Value <= MEASURE_MAX_STEPS-2) // 3-ci warunek poprawny??
		{
			g_MeasureIntervalTable [g_Measure_Step_Value] = g_MeasureIntervalTable [g_Measure_Step_Value-1] + MEASURE_INTERVAL;
			last_measure = g_MeasureIntervalTable[g_Measure_Step_Value] + MEASURE_INTERVAL;
			g_Measure_Step_Value++;
		}

		if (last_measure >= LAST_MEASURE || g_Measure_Step_Value == MEASURE_MAX_STEPS-1)
		{
			g_MeasureIntervalTable [g_Measure_Step_Value] = LAST_MEASURE;
			measure_step_stop = OFF;
		}
	}
}


void MeasureCount() // OK
{
	// Funkcja sprawdza czy nastal ustalony czas do wpisu pomiarow do tablicy pomiarowej

	//int32_t time = g_Time_s; // kopia wartosci aktualnego czasu by podczas obliczen nie ulegl zmianie

	int32_t time;

	if (g_TrackerTEST == ON)
		time = g_Test_time_simul;
	else
		time = g_Time_s;

//	time = g_Time_s;


	if (time == g_MeasureIntervalTable [g_Measure_Step_Count])
	{
			ADC1ToMeasureTable(); // wywoluje funkcje uzupelniajaca tabele pomiarowa

			g_Measure_Step_Count++;
	}

	if (g_Measure_Step_Count == MEASURE_MAX_STEPS || g_Measure_Step_Count == g_Measure_Step_Value)
		g_Measure_Step_Count = 0;

}


void MeasureIntervalTableClean()
{
	// Zerowanie tablicy interwalow pomiarow

	uint32_t i;

	for (i = 0; i <= MEASURE_MAX_STEPS; i++) // zerowanie tablicy interwalow pomiarowych
		g_MeasureIntervalTable [i] = 0;
}


void TrackerStep() // OK
{
	// Funkcja okresla ilosc krokow trackera w ciagu dnia
	// Funkcja uwzgledniajaca liczbe godzin slonecznych poszczegolnych dni w ciagu roku
	// Wzor ze strony:
	// http://www.zielonaenergia.eco.pl/index.php?option=com_content&view=article&id=264:jak-obliczy-uzysk-energii-z-ognniw-fotowoltaicznych&catid=46:soce&Itemid=204

	uint32_t last_step;
	uint32_t tracker_step_stop = ON; // zmienna potrzebna do petli while

	//float g_tracker_start_time, g_tracker_stop_time;
	float sunny_hours_in_a_day;

	float time_middle = 43200 + g_delta_time; // trzeba zaokraglicz funkcja round()

	float declination = g_sun_declination; // * DEG_TO_RAD; // nie ma potrzeby bo ten parametr jest w radianach
	float latitude = g_latitude * DEG_TO_RAD;

	// ---------- Obliczanie ilosci dni slonecznych w ciagu dnia ----------
	float tg_declination = -tanf(declination);
	float tg_latitude = tanf(latitude);

	sunny_hours_in_a_day = (acosf(tg_declination * tg_latitude)) / (7.5 * DEG_TO_RAD); // [h]
	sunny_hours_in_a_day *= 3600; // [s]
	sunny_hours_in_a_day = sunny_hours_in_a_day / 2; // zaokraglona polowa parametru parametru
	//---------------------------------------------------------------------

	g_tracker_start_time = time_middle - sunny_hours_in_a_day;
	g_tracker_stop_time = time_middle + sunny_hours_in_a_day;

	//Obliczanie czasu pracy aktywnej
	g_tracker_start_time = (int)round(g_tracker_start_time);
	g_tracker_stop_time = (int)round(g_tracker_stop_time);


	while (tracker_step_stop == ON)
	{
		if (g_Tracker_Step_Value == 0) // warunek poczatkowy
		{
			g_TrackerTimeStepsTable [g_Tracker_Step_Value] = g_tracker_start_time;
			g_Tracker_Step_Value = 1;
		}

		if (g_Tracker_Step_Value > 0 && (g_TrackerTimeStepsTable [g_Tracker_Step_Value] < g_tracker_stop_time)) // nie jest spelniony drugi warunek
		{
			g_TrackerTimeStepsTable [g_Tracker_Step_Value] = g_TrackerTimeStepsTable [g_Tracker_Step_Value-1] + TRACKER_STEP_INTERVAL;
			last_step = g_TrackerTimeStepsTable [g_Tracker_Step_Value] + TRACKER_STEP_INTERVAL;
			g_Tracker_Step_Value++;
		}

		if ( last_step >= g_tracker_stop_time && (g_Tracker_Step_Value <= TRACKER_MAX_TIME_STEPS) ) // warunek koncowy
		{
			g_TrackerTimeStepsTable [g_Tracker_Step_Value] = g_tracker_stop_time;

			if ( (last_step > g_tracker_stop_time) && ((last_step - g_tracker_stop_time) > TRACKER_STEP_INTERVAL) ) // warunek tak naprawde opcjonalny
			{
				g_Tracker_Step_Value++;
				g_TrackerTimeStepsTable [g_Tracker_Step_Value] = last_step;
			}

			tracker_step_stop = OFF;
		}

		if  (g_Tracker_Step_Value > TRACKER_MAX_TIME_STEPS) // na wypadek bledu
		{
			TrackerError();
			tracker_step_stop = OFF;
		}

	}
}


void TrackerStepCount() //
{
	// Funkcja sprawdza czy nastal czas przestawienia trackera

	//uint32_t time = g_Time_s;

	float Sun_H_previous_value = g_Sun_H;
	float Sun_V_previous_value = g_Sun_V;

	int32_t time;
	int32_t tracker_step_count;
	//int32_t tracker_step_count = g_Tracker_Step_Count;

	if (g_TrackerTEST == ON)
	{
		time = g_Test_time_simul;
		tracker_step_count = g_tracker_test_step_count;
	}
	else
	{
		time = g_Time_s;
		tracker_step_count = g_Tracker_Step_Count;
	}
	//time = g_Time_s;


	if (time == g_TrackerTimeStepsTable [tracker_step_count])// || tracker_step_count <= g_Tracker_Step_Value)
	{
		SunPosition(); // obliczenia pozycji Slonca

		if (Sun_H_previous_value != g_Sun_H && Sun_V_previous_value != g_Sun_V)
			MoveServos();
		else if (Sun_H_previous_value > g_Sun_H) // niekoniecznie potrzebne
			TrackerError();

		//tracker_step_count++;

		if (g_TrackerTEST == ON)
			g_tracker_test_step_count++; // = tracker_step_count;
		else
			g_Tracker_Step_Count++; // = tracker_step_count;
	}

	if ((tracker_step_count == TRACKER_MAX_TIME_STEPS) || (tracker_step_count == g_Tracker_Step_Value))
	{
		if (g_TrackerTEST == ON)
			g_tracker_test_step_count = 0;
		else
			g_Tracker_Step_Count = 0;
	}

	if (tracker_step_count > g_Tracker_Step_Value)
		TrackerError();
// Teoretycznie mozna stworzyc warunek by tracker po zakonczeniu pracy wracal do pewnej wartosci poczatkowej (np. obliczonej na nastepny dzien)

}


void TrackerCountTableClean()
{
	uint32_t i, year;

	g_day_of_year;
	//g_Calc_Year;

	year = g_Calc_Year;

	g_Tracker_Step_Value = 0;

	for (i = 0; i <= TRACKER_MAX_TIME_STEPS; i++) // zerowanie tablicy krokow H trackera
	{
		if (i == 100 || i == 150 || i == 200 || i == 250 || i == 300)
			year = g_Calc_Year;
		g_TrackerTimeStepsTable [i] = 0;
	}

	g_Calc_Year = year;
}


/* ========================================= BLOK OBLICZEN ZWIAZANYCH Z POZYCJA SLONCA NAD HORYZONTEM ========================================= */

void DayOfYear(int32_t year, int32_t month, int32_t day) // <= dziala poprawnie
{
	/*	Funkcja oblicza wg daty ktory jest dzien roku	*/

	/*
	 	1. Jeœli rok jest podzielny przez 4, przejdŸ do kroku 2. W przeciwnym razie przejdŸ do kroku 5.
    	2. Jeœli rok jest daje siê równo podzieliæ przez 100, przejdŸ do kroku 3.
    		W przeciwnym razie przejdŸ do kroku 4.
    	3. Jeœli rok jest podzielny przez 400, przejdŸ do kroku 4.
    		W przeciwnym razie przejdŸ do kroku 5.
    	4. Rok jest to rok przestêpny (ma 366 dni).
    	5. Rok nie jest to rok przestêpny (ma 365 dni).
	 */

	int32_t doF = 28;
	int32_t set = 0;
	int32_t i;

	int32_t DIM [12] = {31,doF,31,30,31,30,31,31,30,31,30,31};

	int day_of_year = 0;

	//g_CalcLeapYear = 0; // zerowanie flagi w razie gdy wczesniej byl rok przestepny

	//g_day_of_year = 0;

	//	Obliczanie czy rok jest przystepny <== musi spelnic jeden z tych warunkow
	if (year % 4 == 0 && year % 100 != 0)
		set++;

	if (year % 400 == 0)
		set++;


	if (set >= 1) // ile luty ma miec dni
	{
		DIM [1] = doF = 29;
		g_CalcLeapYear = ON;
	}

	else
	{
		DIM [1] = doF = 28;
		g_CalcLeapYear = OFF;
	}

	if (month == 1)
		day_of_year = day;

	else  if (month >= 2 && month <= 12)
		for (i=0; i <= month-2; i++)
		{
			day_of_year = day_of_year + DIM[i];

			if (i == month - 2)
				day_of_year = day_of_year + day;
		}

	g_day_of_year = day_of_year;

	//return g_day_of_year;
}


void ExuationOfTime()
{
	/*	Algorytm oblicza rownanie czasu czyli blad wyznaczania czasu dla poszczegolnych
	 * dni w danym roku kalendarzowym
	 */

	float B = 0;

	if(g_CalcLeapYear == OFF)
		B = (g_day_of_year - 1) * 0.98630136; // 360/365 = 0.98630136

	else if (g_CalcLeapYear == ON)
		B = (g_day_of_year - 1) * 0.983606557377; // 360/366 = 0.983606557377;

	B = B * DEG_TO_RAD; // 356.065582 v 357,0410959
	//B = 6.231542681;

	g_delta_time = 229.2*(0.000075+0.001868*cosf(B)-0.032077*sinf(B)-0.014615*cosf(2*B)-0.04089*sinf(2*B)); // liczone w minutach

	//g_delta_time = g_delta_time * 60; // wynik rownania czasu w sekundach

}


void SunDeclination()
{
	/*	Funkcja oblicza deklinacje Slonca dla danego dnia roku
	 *
	 * 	Deklinacja Slonca okresla polozenie Slonca wzgledem rownika (sinus miedzy zwrotnikami)
	 *
	 * 	Funkcja zwraca odpowiedz w stopniach!
	 *
	 */

	g_sun_declination = 0;

	float days;

	if (g_CalcLeapYear == OFF)
		days = 365;
	else if (g_CalcLeapYear == ON)
		days = 366;

	float par = 360*((284+g_day_of_year)/days);
	par = par * DEG_TO_RAD;
	float sinpar = sinf(par);

	g_sun_declination = 23.45 * sinpar; // stopnie

	g_sun_declination =  g_sun_declination * DEG_TO_RAD; // wynik w radianach

/*
	if (g_CalcLeapYear == 0) // rok normalny
		g_sun_declination = 0.409279709592670 * sinf((360 * ((284 + g_day_of_year)/365)) * DEG_TO_RAD); // 23.45 * DEG_TO_RAD

	else if(g_CalcLeapYear == 1) // rok przestepny
		g_sun_declination = 0.409279709592670 * sinf((360 * ((284 + g_day_of_year)/366)) * DEG_TO_RAD); // 23.45 * DEG_TO_RAD
*/

}

/*
void SunSunriseSunset_1()
{
	//	Funkcja wylicza wschod i zachod Slonca dla danego dnia roku dla konkretnego poloz. geogr.
	//	1. http://williams.best.vwh.net/sunrise_sunset_algorithm.htm
	//	2. http://cybermoon.pl/wiedza/algorithms/wschody_slonca.html
	// 	3. "Wyklad z podstaw astronomii" wykl Iwona Wtyrzyszczak

	// 1. Obliczanie ktory dzien roku

	int32_t n1, n2, n3, n;
	int32_t dzien, miesiac, rok, zenit, latitude, longitude;

	n1 = 275 * miesiac / 9;
	n2 = (miesiac + 9) / 12;
	n3 = 1 + ((rok - 4 * (rok /4) + 2) / 3);
	n = n1- (n2 * n3) + dzien - 30;

	// 2. Konwersja szer geogr do wartosci godzinowych
	// tutaj trzeba poprawic poniewaz dla poszczeg opcji powinny byc petle if
	int32_t lngHour, t;

	lngHour = longitude / 15;

	t = n + ((6 - lngHour)/24);

	t = n + ((18 - lngHour)/24);

	// 3. Obliczenie sredniej slonecznej anomalii

	int32_t m;
	m = (0.9856 * t) - 3.289;

	// 4. Obliczenie polozenia Slonca

	int32_t l; // l musi byc w zakresie {0;360)
	l = m + (1.916 * sin(m)) + (0.02 * sin(2 * m)) + 282.634;

	// 5. Obliczenie rektascencji Slonca

	int32_t ra; // ra musi byc w zakresie {0;360}
	ra = atan(0.91764 * tan(l));

	// 6.

	int32_t lquad, rquad;
	lquad = (l/90)*90;
	rquad = (ra/90)*90;
	ra = ra + (lquad-rquad);

	// 7. Konwersja rektascencji Slonca do wartosci godzinowych
	ra = ra / 15;

	int32_t sindec, cosdec;
	sindec = 0.39782 * sin(l);
	cosdec = cos(asin(sindec));

	float cosinush = (cos(zenit) - (sindec * sin(latitude))) / (cosdec * cos(latitude));
	if (cosinush > 1); // slonce nigdy nie wschodzi w tym miejscu w okreslonym czasie
	if (cosinush < -1); // slonce nigdy nie zachodzi w tym miejscu w okreslonym czasie



}
*/

void SunriseSunset_3() // <= dziala
{
	/*	Funkcja wylicza wschod i zachod Slonca dla danego dnia roku dla konkretnego poloz. geogr.	*/
	//	1. http://williams.best.vwh.net/sunrise_sunset_algorithm.htm
	//	2. http://cybermoon.pl/wiedza/algorithms/wschody_slonca.html
	// 	3. "Wyklad z podstaw astronomii" wykl Iwona Wtyrzyszczak


	float tw = 0, cos_tw = 0;

	// Zmienne potrzebne do obliczenia modulow
	float latitude = g_latitude;
	float sun_declination = g_sun_declination;


	// Wartosci testowe dla Poznania na dzien 21 grudnia
	//g_latitude = 52.0000;
	//g_sun_declination = -23.4333; // -23st26min = -23.43333333



//MODULY LICZB DO OBLICZEN

	if(g_sun_declination >= 0)
		sun_declination = g_sun_declination;

	else if (g_sun_declination < 0)
		sun_declination = g_sun_declination * (-1);

	if (g_latitude >= 0)
		latitude = g_latitude;

	else if (g_latitude < 0)
		latitude = g_latitude * (-1);

// Zamiana stopni na radiany

	//sun_declination = sun_declination * DEG_TO_RAD;
	latitude = latitude * DEG_TO_RAD;

// OBLICZENIA WG ALGORYTMU 3
	cos_tw = tanf(sun_declination) * tanf(latitude);
	tw = acosf(cos_tw) * RAD_TO_DEG; // + ponownie zamienione na stopnie
	//tw = tw * 0.0665941102; // (24/360) = 0.06666666667 * korekcja bledu (na przykladzie Poznania)
	tw *= 240; // przeliczenie na sec

	// Obliczenia sa wg czasu prawdziwego, a nie sredniego. (czas prawdziwy nie uplywa tak samo na calej dlugosci - zalezne od ksztaltu bryly)
	g_sunset = tw + 43200;// 12; // prawdziwy czas sloneczny zachodu
	g_sunrise = 86400 - g_sunset; //24 - g_sunset; // prawdziwy czas sloneczny wschodu

	// Wyniki po uwzglednieniu poprawki wynikajacej z rownania czasu <= aktualnie wyrazone w [h]
	g_sunset = (g_sunset + g_delta_time);// / 3600;
	g_sunrise = (g_sunrise + g_delta_time);// / 3600;
}


void SunPosition()
{
	/*	Algorytm oblicza pozycje Slonca na horyzoncie w danym dniu o danych wpspolrzednych geograficznych	*/
	//http://en.wikipedia.org/wiki/Position_of_the_Sun

	//float Sun_Elevation = 0; // alfa
	//float Sun_Azimuth = 0; // gamma

	float HSA = 0;	// godzinowy kat sloneczny, oznaczany jako mala omega
	float Solar_Time = 0;// = 43200; // BRAKUJE TEGO CZASU DO OBLICZEN!!!

	float cosHSA = 0; // cosinus godzinowego kata slonecznego
	float cosSD = 0; // cosinus deklinacji slonca
	float sinSD = 0; // sinus deklinacji slonca
	float cosSE = 0; // cosinus elewacji slonca
	float cosLAT = 0; // cosinus szerokosci geograficznej
	float sinLAT = 0; // sinus szerokosci geograficznej
	float sinHSA = 0; // sinus godzinowego kata slonecznego

	float tgtg = 0; //tg deklinacji dzielony przez tg szerokosci geograficznej (latitude)

	float sun_declination = g_sun_declination; // deklinacja w radianach
	float latitude = g_latitude * DEG_TO_RAD; // szerokosc geo (N do S) w radianach
	float longitude; // dlugosc geo (E do W) w stopniach

	if (g_longitude < 0)
		longitude = g_longitude * (-1); // modul dlugosci E do W
	else
		longitude = g_longitude;

	if (g_TrackerTEST == ON)
		Solar_Time = ((float)g_Test_time_simul /60) + g_delta_time + 4*(g_zone_meridian - longitude); // delta_time wyrazona w [min]
	if (g_TrackerTEST == OFF)
		Solar_Time = ((float)g_Time_s /60) + g_delta_time + 4*(g_zone_meridian - longitude); // <======== OPROGRAMOWAC CZAS STREFOWY dla poludnika strefowego g_zone_meridian bo bez tego jest zle

	//Solar_Time = (float)g_Time_s / 3600; // czas sloneczny z sek na min


	tgtg = tanf(sun_declination) / tanf(latitude);

	HSA = (15 * (Solar_Time/60 - 12)); //* DEG_TO_RAD; // 12:00 czyli poludnie <= korekcja bledu we wzorze; Solar_Time ma byc w godzinach, a nie minutach
	//HSA = (Solar_Time/60 - 12) * DEG_TO_RAD;

	if (HSA < -180)
		HSA = -180 * DEG_TO_RAD; // zabezpieczenie przed wyjsciem poza zakres -180 >= HSA >= 180
	if (HSA > 180)
		HSA = 180 * DEG_TO_RAD; // zabezpieczenie przed wyjsciem poza zakres -180 >= HSA >= 180
	if (HSA >= -180 && HSA <= 180)
		HSA *= DEG_TO_RAD; // OBLICZA JAKIES GLUPOTY!!!!!!!!!!!!!

	cosHSA = cosf(HSA);
	sinHSA = sinf(HSA);

	cosSD = cosf(sun_declination);
	sinSD = sinf(sun_declination);
	cosLAT = cosf(latitude);
	sinLAT = sinf(latitude);

	g_Sun_V = asinf(cosLAT * cosSD * cosHSA + sinLAT * sinSD); // elewacja

	cosSE = cosf(g_Sun_V);

/*
 * Blad we wzorach w ksiazce!
 *
	if (cosHSA >= tgtg)
		g_Sun_H = asinf((cosSD * sinHSA)/cosSE); // horyzont
	else if (cosHSA < tgtg)
		g_Sun_H = asinf((cosSD * sinHSA)/cosSE) * (-1);

*/

	//g_Sun_H = asinf((-1)*(cosSD * sinHSA)/cosSE);

	g_Sun_H = HSA;

/*
	if (g_Sun_H < 0)
		g_Sun_H = PI - g_Sun_H;
*/

	g_Sun_V = g_Sun_V * RAD_TO_DEG;	// zamiana z radianow na stopnie
	g_Sun_H = g_Sun_H * RAD_TO_DEG;	// zamiana z radianow na stopnie


	//KONTROLA OBLICZEN
	uint32_t step_count;

	if (g_TrackerTEST == ON)
		step_count = g_tracker_test_step_count;
	else
		step_count = g_Tracker_Step_Count;

	g_SolarTimeVal[step_count] = Solar_Time /60;
	g_HSAVal[step_count] = HSA * RAD_TO_DEG;
}
//	http://vesta.astro.amu.edu.pl/Staff/Iwona/geograf.pdf
