#include "stm32f0xx.h" //Подключение биб-ки с моделью stm32f0xx
void InitPortB(void); //Декларация функции инициализации порта B
void InitTimer6(void); // Декларация ф-ции инициализации таймера
void TIM6_DAC_IRQHandler(void); /*Декларация функции обработки
 прерывания от таймера TIM6*/