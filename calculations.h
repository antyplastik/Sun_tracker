/*
 * obliczenia.h
 *
 *  Created on: Mar 8, 2015
 *      Author: Kamil
 */

#ifndef CALCULATIONS_H_
#define CALCULATIONS_H_

#define RAD_TO_DEG	57.2957795131
#define DEG_TO_RAD	0.01745329252 // pi/180
#define PI			3.1415926535897932

#define ON		1 // zmiana w wyniku jakiegos licznika
#define OFF		0 // zmiana w wyniku jakiegos licznika
#define CHANGE	2 // zmiana w wyniku wymuszenia z zewnatrz

#define CALC_LIMIT				2048 // starcza na miesiac
#define MEASURE_INTERVAL		900 // staly interwal pomiarow
#define TRACKER_STEP_INTERVAL	240 // staly interwal trackera co 4 min czyli 1st obrotu ziemskiego
#define RTC_call				600 // wzywanie RTC i kalibracja co 10 min

#define	TRIGG_VEC				4	// ilosc potrzebnych parametrow do wyzwolenia obliczen w przypadku wprowadzenia parametrow recznie (konsola)

#define TRACKER_TEST_TIME_MULTI	100 //

#define TRACKER_MAX_TIME_STEPS			300 // wartosc wystarczajaca na 20h z krokiem co 4min
//#define TRACKER_MAX_STEPS_H			96
//define TRACKER_MAX_STEPS_V			64
#define MEASURE_MAX_STEPS			96	// tablica przechowuje wartosci g_Time_s dla poszczegolnych pomiarow (max liczba pomiarow w ciagu dnia)

#define MEASURE_TABLE_COL		9 // ilosc kolumn w tabeli pomiarowej

#define FIRST_MEASURE	21600 // pierwszy pomiar o 6:00
#define LAST_MEASURE		72000 // ostatni pomiar o 20:00

void CalcTriggTableCheck();
void CalcTriggTableIn(uint32_t pos, int32_t in);

void Calculations_Init();
void CalculationsInput();
void CalculationsOutput();
void CalculationOutputTableClean();
void MoveServos();

void ADC1ToMeasureTable(); // funkcja pobiera dane z tabeli ADC1 (wyp. przez DMA) i wpisuje do tabeli pomiarowej dnia
void MeasureTableClean();
void MeasureToUSART(); // funkcja wysyla pomierzone i obliczone dane w formie tabeli przez konsole

void CalcTest();
void TrackerTest(uint32_t arg);
void TrackerTestScheduler(); // funkcja testera wykonujaca krok co 1s zgodnie z systick
void TrackerError();

void Calculations();

void Calc_Scheduler_And_Flags_Check();
void CalcTriggTableIn(uint32_t pos, int32_t in);
void CalcTriggTableCheck();
void CalcTriggTableClean();

void SetCalcTime(uint32_t hour, uint32_t minute, uint32_t second);
void SetCalcCalendar(uint32_t Year, uint32_t month, uint32_t day);

void TimeZone();
void TimeZoneSwitch();

void CalcCalendar(uint32_t Year, uint32_t month, uint32_t day);
void CalcNewDay(); // funkcja obliczen dla nowego dnia

void CalcSteps(); // Funkcja wywoluje obliczenia krokow pomiarow i trackera

void MeasureInterval();
void MeasureCount(); // godziny pomiarow sa zdefiniowane na sztywno wiec nie
void MeasureIntervalTableClean();

void TrackerStep(); // oblicza kroki jakie ma wykonac tracker w ciagu dnia
void TrackerStepCount(); // pilnuje by tracker wykonal krok o wskazanej godzinie
void TrackerCountTableClean();

void CleanStepsTables();

//------------------------------------ BLOK OBLICZEN --------------------------------------------
void DayOfYear(int32_t year, int32_t month, int32_t day);
void ExuationOfTime();
void SunDeclination();
//void SunriseSunset_1();
//void SunriseSunset_2();
void SunriseSunset_3();
void SunPosition();
//-----------------------------------------------------------------------------------------------

#endif /* OBLICZENIA_H_ */
