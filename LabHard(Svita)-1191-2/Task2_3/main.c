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

int main(void)
{
	InitUSART1();																																				//Инициализация USART1 лаборатоного стенда
	uint8_t text[10] = {0x61,0x76,0x72,0x2D,0x35,0x3E,0x20,0x20, 0x20, 0x20};						//Массив содержащий сообщение "avr-5>    "	
	while(1){
		//Сброс массива text
		text[6] = 0x20;																																		//6 - элемент массив заменяется на пробел
		text[7] = 0x20;																																		//7 - элемент массив заменяется на пробел
		text[8] = 0x20;																																		//8 - элемент массив заменяется на пробел
		text[9] = 0x20;																																		//9 - элемент массив заменяется на пробел
		
		//Получение данных из USART
		while ((USART1->ISR & USART_ISR_RXNE) == 0) {}																		//Чтение регистра состояния ISR и анализ флага RXNE (выставляется в 1, когда новый пакет данных получен приёмником Rx и скопирован в RDR)
		uint8_t d = (uint8_t)USART1->RDR;																									//Копирование данных из USART (регистр RDR) в програмную переменную.
		// Чтение регистра RDR приводит к сбросу флага RXNE = 0
		
		//Отпрвака данных в USART (для очистки консоли)
//			for (uint8_t i=0; i<1; i++){																										//Повтор действия 10 раз (количество выведенных элементов в консоли
//		while ((USART1->ISR & USART_ISR_TXE) == 0) {}																			//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
//			USART1->TDR = 0X7F;																															//Передача пустого символа в USART(регистр TDR)
////		//Запись в регистр TDR приводит к сбросу флага TXE = 0
//}
				while ((USART1->ISR & USART_ISR_TXE) == 0) {}																			//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
			USART1->TDR = d;
		//Перевод числа в систему с основанием 12
		for (uint16_t i = 9 ; i>6; i--){       
			if(d<12) {																																			
				text[i] = d+48;																															
				break;																																		
			}
			uint8_t temp=0;
			temp = d%12;
			if(temp==10){
			
			    text[i] = 0x41; 
			}
			else if(temp==11){
			   text[i] = 0x42;
			}
			else if(temp==12){
				text[i]= 0x43;
			}
			else{
			   text[i]=temp + 48;
			}
			d = d/12;
		} 																																				//в число записывается целая часть от деления
		
		
		//Вывод сообщения с кодом надатой клавиши
//		for (uint8_t i=0; i<10; i++){
//			while ( (USART1->ISR & USART_ISR_TXE) == 0) {}																	//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
//			USART1->TDR = text[i];																													//Передача данных из програмной переменной в USART(регистр TDR)
//			//Запись в регистр TDR приводит к сбросу флага TXE = 0
//		}
		
	}
	
}

//Функция инициализации USART лабораторного комплекса STM_01
void InitUSART1(){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;																								//Включение тактирования USART1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;																									//Включение тактирования порта А
	
	//Настройка линий порта А: РА9(ТХ_1) - выход передатчика; PA10(RX_1) - вход приёмника
	GPIOA->MODER |= 0x00280000;																													//Перевести линии РА9 и РА10 в режим альтернативной функции
	GPIOA->AFR[1] |= 0x00000110;																												//Включить на линиях РА9 и РА10 альтернативную функцию AF1
		
	//Настройка линии передатчика Тх (РА9)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;																									//Сбросить 9 бит GPIOA->OTYPER - переключение в режим push-pull для линии РА9 (активный выход) 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;																									//Отключение подтягивающих резисторов для линии РА9 
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;																						//Установка высокой скорости синхронизации линии РА9
	
	//Настройка линии приемника Rx (РА10)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;																								//Сброс режима подтягивающих резисторов для линии РА10
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;																								//Включение подтягивающего резистора pull-up на входной линии РА10 (вход приемника Rx)
	
	//Конфигурирование USART
	USART1->CR1 &= ~USART_CR1_UE;																												//Запрещение работы модуля USART1 для изменения параметров его конфигурации
	USART1->BRR= 69;																																		/*Настройка делителя частоты, тактирующего USART и задающего скорость приема и передачи данных на уровне 9600 бит/с: 
																																												Частота тактирующего генератора = 8 МГц 
																																												Скорость обмена по USART - 38400 бит/с; коэффициент деления - 8000000 / 38400 - 208; Округленное значение - 208*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;																																	/*Разрешить работу приемника и передатчика USART. Остальные биты этого регистра сброшены, что обеспечивает: 
																																												количество бит данных в пакете 8;
																																												контроль четности - отключен; 
																																												прерывания по любым флагам USART - запрещены;
																																												состояние USART - отключен*/
	USART1->CR2 = 0x1000;																																		//Количество стоповых бит - 1
	USART1->CR3 = 0;																																		//DMA1 - отключен
	USART1->CR1 |= USART_CR1_UE;																												//По завершении конфигурирования USART разрешить его работу (биту UE регистра CR1 присвоить 1)
}

/*---------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе STM_01;
**		2. На компьютере запустите любую программу для мониторинга последовательных портов ПК (Terminal, PuTTY или др.) и подключитесь к соответствующему COM-порту на скорости 9600 бит/с;
**		3. С помощью программы мониторинга отправьте на лабораторный стенд произвольный символ, обратно вернётся сообщение содержащее номер нажатой клавиши в системе исчсления с основанием 5
**-------------------------------------------------------------------------------------------------------------*/
