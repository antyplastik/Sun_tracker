/*
 * servo.h
 *
 *  Created on: Jan 10, 2016
 *      Author: Kamil
 */

#ifndef SERVO_H_
#define SERVO_H_

/*
 * I/O kanalow PWM:
 * - PC6	TIM_CH1	(Servo V)
 * - PC7	TIM_CH2 (Servo H)
 */

#define ServoPort		GPIOC
#define ServoPortRCC	RCC_AHBPeriph_GPIOC
#define Servo_H_Out		GPIO_Pin_7
#define Servo_H_AF		GPIO_PinSource7
#define Servo_V_Out		GPIO_Pin_6
#define Servo_V_AF		GPIO_PinSource6
#define Servo_TIM		TIM8

/*
#define LEDPWMPort 			GPIOE
#define LEDPWMPortRCC		RCC_AHBPeriph_GPIOE
#define LED_Warning_H_PWM 	GPIO_Pin_15
#define LED_Warning_V_PWM 	GPIO_Pin_9
*/

// ------- funkcje sterujace praca serwomechanizmow
void Servo_Ctrl(int32_t servo_H,int32_t servo_V); // funkcja z konwersja stopni na szerokosc impulsu
void Servo_V_PWM(int32_t servo_V); // ustawia konkretna wartosc pulse, ktora jest porownywana z wartoscia licznika
void Servo_H_PWM(int32_t servo_H); // ustawia konkretna wartosc pulse, ktora jest porownywana z wartoscia licznika
// -----------------------------------------------


/*
 * Prototypy
 */

void Servo_Init(); // inicjacja serw - funkcja zbiorcza
void Servo_IO_Init(void); // ustawienia GPIO z AF
void PWM_Servo_Timer_Init(int32_t pwmprescaler, int32_t pwmperiod);// ustawienia licznika z kanalami OC

//void LED_Warning_Init(void);
//void LED_Warning_H_Init(void);
//void LED_Warning_V_Init(void);
//void PWM_LED_Timer_Init(void);

void ChangeStateWarningLED();

// ------ Handler'y przerwan
//void TIM8_IRQHandler(void);

#endif /* SERVO_H_ */
