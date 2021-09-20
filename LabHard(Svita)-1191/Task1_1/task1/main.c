#include "main.h"

static _Bool statePB0=0; /*Текущее состояние на выводе PB.0: 0 – светодиод
 погашен; 1 - светодиод горит*/
static uint32_t half_period=125; //Полупериод мигания светодиода

int main(){
__disable_irq(); //Глобальное запрещение прерываний
InitPortB(); //Инициализация порта B
InitTimer6(); //Инициализация таймера TIM6
NVIC->ISER[0] |= 0x20000; /*Разрешение в NVIC прерывания от модуля TIM6 (17 линия)*/
__enable_irq(); //Глобальное разрешение прерываний
TIM6->CR1 |=0x1; //Разрешение работы таймера
	while(1);
}

void InitPortB(void){
/*Включение тактирования порта B: RCC_AHBENR_GPIOBEN=0x00040000*/
RCC->AHBENR|=RCC_AHBENR_GPIOBEN;
/*Переключение линий 0 и 8 порта B в режим "Output":
GPIO_MODER_MODER0_0=0x1; GPIO_MODER_MODER8_0=0x10000*/
GPIOB->MODER|=GPIO_MODER_MODER0_0 | GPIO_MODER_MODER8_0;
//Переключение линий 12(SW4) и 13(SW3) порта B в режим "Input"
GPIOB->MODER&=~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13);
/*Разрешение работы светодиодов на стенде STM_01 с помощью установки
логической "1" на выводе PB.8*/
GPIOB->BSRR=0x100;
}
 
void InitTimer6(void){
RCC->APB1ENR=RCC_APB1ENR_TIM6EN; /*Включение тактирования модуля TIM6*/
TIM6->PSC=7999; //Установка коэффициента предделителя равного 8000
TIM6->ARR=half_period;
TIM6->DIER|=0x1;
}

void TIM6_DAC_IRQHandler(void){
TIM6->ARR = (uint32_t)half_period<<(((GPIOB->IDR)&0x3000)>>12);
statePB0 = !statePB0;
GPIOB->BRR = 0x1;
GPIOB->BSRR = statePB0;
TIM6->SR=0;
}