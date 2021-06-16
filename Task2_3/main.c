#include "main.h"
int main(void)
{
	uint16_t i=6;
	InitUSART1();
	while(1)
	{
		uint16_t message[9]={0x74,0x69,0x76,0x2d,0x37,0x3e,0,0,0};
		while ((USART1->ISR & USART_ISR_RXNE) == 0) { }
		uint8_t d = (uint8_t)USART1->RDR;
		for (int8_t s=0;s<9;s++)
		{				
			while ((USART1->ISR & USART_ISR_TXE) == 0) { }
			USART1->TDR = 0x7f;			
		}
		for(uint16_t k=0;d!=0;k++)
		{
			message[8-k]=(d%7)+48;
			d=d/7;
		}
		i=6;
		for (int32_t s=0;s<9;s++)
		{				
			if(s==i&&s==48)
			{
				i++;
				continue;
			}
			while ((USART1->ISR & USART_ISR_TXE) == 0) { }
			USART1->TDR = message[s];			
		}		
	}
}


void InitUSART1(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= 0x00280000;
	GPIOA->AFR[1] |= 0x00000110;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;
	USART1->CR1 &= ~USART_CR1_UE;
	USART1->BRR=17;
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
	USART1->CR2 = 0x3000;
	USART1->CR3 = 0;
	USART1->CR1 |= USART_CR1_UE;
}
