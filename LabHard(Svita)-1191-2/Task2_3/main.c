/*---------------------------------------------------------------------------------------------------------------
**Проект: "Сервер для определения ASCII-кодов клавиатуры".
**Назначение программы: принять по USART пакеты данных от ПК, вернуть сообщение содержащее код клавиши в заданной системе исчесления
**Разработчик: Свита Артем Николаевич - 1191б
**Цель: создать сервер для определения ASCII-кодов клавиатуры с представлениемем их в заданной системе счисления
**Решаемые задачи:
**				1. Конфигурирование линий GPIO на выполнение альтернативных функций;
**				2. Конфигурирование USART;
**				3. Получение данных о нажатой клавише
**				4. Перевод кода клавиши в систему исчесления с основанием 12
**				5. Вывод сообщения содержащего этот код
**-------------------------------------------------------------------------------------------------------------*/

#include "main.h"	//Заголовчоный файл с описанием подключаемых библиотечных модулей

#include "main.h"

int main(void) {
	initUSART1();
	char result[MAX_MESSAGE_LENGTH];
	char message[MAX_MESSAGE_LENGTH];
	char *result_message = message;
	while(1) {
		char *buffer = result;
		while ((USART1->ISR & USART_ISR_TXE) == 0) {}
		while ((USART1->ISR & USART_ISR_RXNE) == 0) {}
		uint16_t d = USART1->RDR;
		int result_length = convert_dec_to(d, 12, buffer);
		for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
			show_char(DELETE);
		}
		char values[result_length];
		int w = 0;
		for (int i = result_length - 1; i >= 0; i--) {
			values[w] = buffer[i];
			w++;
		}
		sprintf(message, "san - 12 > %s\r", values);
		notification(result_message);
	}
}

void initUSART1(void) {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= 0x280000;
	GPIOA->AFR[1] |= 0x110;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;
	USART1->CR1 &= ~USART_CR1_UE;
	USART1->BRR = 69;
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	USART1->CR1 |= USART_CR1_UE;
}

void notification(uint8_t* message) {
	while(*message != 0)
  {
		show_char(*message);
		message++;
	}
	
}
	
void show_char(char ch) {
	while ((USART1->ISR & USART_ISR_TXE) == 0) {}
  USART1->TDR = ch;
}

int convert_dec_to(int decimal, int to, char *buffer) {
	int i = 0;
	uint8_t chars[] = {'A', 'B', 'C', 'D'};
	
	while(true) {
			if (decimal < to) {
				if (decimal >= 10) {
				buffer[i] = chars[decimal - 10];
			}
			else {
				buffer[i] = (char)decimal + NUMBER_TO_ASCII;
			}
			i++;
			break;
		}
		char convert = (char)(decimal % to);
	  if (convert >= 10) {
			buffer[i] = chars[convert - 10];
	  }
	  else {
			buffer[i] = (char)convert + NUMBER_TO_ASCII;
	  }
		decimal /= to;
		i++;
	}
	return i;
}

/*---------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе STM_01;
**		2. На компьютере запустите любую программу для мониторинга последовательных портов ПК (Terminal, PuTTY или др.) и подключитесь к соответствующему COM-порту на скорости 9600 бит/с;
**		3. С помощью программы мониторинга отправьте на лабораторный стенд произвольный символ, обратно вернётся сообщение содержащее номер нажатой клавиши в системе исчсления с основанием 5
**-------------------------------------------------------------------------------------------------------------*/
