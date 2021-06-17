/*----------------------------------------------------------------------------------------------------------------------------------------
**Проект: "Interrupt-NVIC".
**Назначение программы: псевдопараллельное выполнение двух задач:
**																									* мигание светодиодом с аппаратным контролем частоты переключения;
**																									* определение ASCII-кодов клавиш, нажимаемых на удаленном терминале;
**Разработчик: Свита Артем Николаевич - 1191б
**Цель: создание программы, использующей внутренние прерывания микроконтроллера STM32F072RBT 
**Решаемые задачи:
**		1. Реализация функции-обработчика внутреннего прерывания микроконтроллера STM32F072RBT (USART);
**		2. Конфигурирование NVIC;
**		3. Настройка модуля USART на генерацию сигнала прерывания при возникновении заданных событий.
**----------------------------------------------------------------------------------------------------------------------------------------*/

#include "main.h"

static uint32_t sum = 4, d = 2, n = 1, curA = 4;						//Переменные хранящие: сумму членов арифмет. прогрессии, разность, количество итераций, текущее значение
static uint8_t flag;																				//Флаг текущего сотояния функции debug
static uint8_t flagF = 0;																		//Флаг нажатия на F5

int main()
{
	__disable_irq();																					//Глобальное запрещение прерываний
	InitUSART1();																							//Инициализация модуля USART1
	NVIC->ISER[0] |= 0x08000000; 															//Разрешение в NVIC прерывания от модуля USART1 	
	__enable_irq();																						//Глобальное разрешение прерываний 

	// Выполнение арифметической прогрессии
	while(1){
		if(n == 16){																						//По проишествии 15 итераций
			//Возврат переменных прогрессии к начальным значениям
			sum = 4;															
			d = 2;
			n = 1;
			curA = 4;
		}
		debug();																								//Вызов функции debug
		curA = curA + d;																				//Добавление к текущему значению разности арифметической прогрессии
		sum += curA;																						//Вычисление суммы всех членов прогрессии
		n += 1;																									//Увелить на 1 переменную отображающую количество итераций
	}
}

//Функция инициализации USART лабораторного комплекса
void InitUSART1(){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;													//Включение тактирования USART1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;														//Включение тактирования порта А
	
	//Настройка линий порта А: РА9(ТХ_1) - выход передатчика; PA10(RX_1) - вход приёмника
	GPIOA->MODER |= 0x00280000;																		//Перевести линии РА9 и РА10 в режим альтернативной функции
	GPIOA->AFR[1] |= 0x00000110;																	//Включить на линиях РА9 и РА10 альтернативную функцию AF1
	
	//Настройка линии передатчика Тх (РА9)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;														//Сбросить 9 бит GPIOA->OTYPER - переключение в режим push-pull для линии РА9 (активный выход) 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;														//Отключение подтягивающих резисторов для линии РА9 
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;											//Установка высокой скорости синхронизации линии РА9
	
	//Настройка линии приемника Rx (РА10)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;													//Сброс режима подтягивающих резисторов для линии РА10
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;													//Включение подтягивающего резистора pull-up на входной линии РА10 (вход приемника Rx)
	
	//Конфигурирование USART
	USART1->CR1 &= ~USART_CR1_UE;																	//Запрещение работы модуля USART1 для изменения параметров его конфигурации
	USART1->BRR=69;																								/*Настройка делителя частоты, тактирующего USART и задающего скорость приема и передачи данных на уровне 115200 бит/с: 
																																	Частота тактирующего генератора = 8 МГц 
																																	Скорость обмена по USART - 115200 бит/с; коэффициент деления - 8000000 / 115200 - 69,4444(4); Округленное значение - 69*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;										/*Разрешить работу приемника и передатчика USART. Остальные биты этого регистра сброшены, что обеспечивает: 
																																	количество бит данных в пакете 8;
																																	контроль четности - отключен; 
																																	прерывания по любым флагам USART - запрещены;
																																	состояние USART - отключен*/
	USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE; 						/*Разрешение (в модуле USART1) на выдачу сигнала прерывания при возникновении событий:
																																	прием кадра в буферный регистр; завершение передачи кадра */
	USART1->CR2 = 0;																							//Количество стоповых бит - 1
	USART1->CR3 = 0;																							//DMA1 - отключен
	USART1->CR1 |= USART_CR1_UE;																	//По завершении конфигурирования USART разрешить его работу (биту UE регистра CR1 присвоить 1)
}

//Функция-обработчик прерывания от модуля USART1 
void USART1_IRQHandler(void)
{	
	uint16_t pack;																								//Переменная хранящая принятый пакет
	//Событие готовности принятых данных к чтению 
	if (USART1->ISR & USART_ISR_RXNE) { 													//Если в регистре состояний USART1 установлен флаг "RXNE", то
		pack=USART1->RDR; 																					//Чтение принятого битового пакета из буферного регистра приемника USART1 
		
		//Обработка на основе принятого пакета
		switch ( pack ) {
			case 0x69:																									//При нажатии на клавишу "i"
			flag = 2;																										//Присваивание флагу функции debug значения 2
			flagF = 0;																									//Сброс флага нажатия на F5
			break;																											//Выход из обработчика
		case 0x65:																										//При нажатии на клавишу "e"
			flag = 3;																										//Присваивание флагу функции debug значения 3
		  flagF = 0;																									//Сброс флага нажатия на F5
			break;																											//Выход из обработчика
		case 0x73:																										//При нажатии на клавишу "s"
			flag = 4;																										//Присваивание флагу функции debug значения 4
		  flagF = 0;																									//Сброс флага нажатия на F5
			break;																											//Выход из обработчика
		
		// Начало обработки нажатия на F5
		case 27:																											//При вводе первого кода нажатия на F5
			flagF = 1;																									//Увеличение флага нажатия на F5 на единицу
			break;																											//Выход из обработчика
		case 91:																											//При вводе второго кода нажатия на F5
			if (flagF == 1){																						//Проверка ввода предыдущего кода 
				flagF = 2;																								//Увеличение флага нажатия на F5 на единицу
			}
			break;																											//Выход из обработчика
		case 49:																											//При вводе третьего кода нажатия на F5
			if (flagF == 2){																						//Проверка ввода предыдущего кода 
				flagF = 3;																								//Увеличение флага нажатия на F5 на единицу
			}
			break;																											//Выход из обработчика
		case 53:																											//При вводе четвёртого кода нажатия на F5
			if (flagF == 3){																						//Проверка ввода предыдущего кода 
				flagF = 4;																								//Увеличение флага нажатия на F5 на единицу
			}
			break;																											//Выход из обработчика
		case 126:																											//При вводе пятого кода нажатия на F5
			if (flagF == 4){																						//Проверка ввода предыдущего кода 
				flagF = 0;																								//Сброс флага нажатия на F5
				flag = 1;																									//Присваивание флагу функции debug значения 1
			}
			break;																											//Выход из обработчика
		// Конец обработки нажатия на F5

		default:																											//В случае нажатия на какую либо другую кнопку
			flagF = 0;																									//Сброс флага нажатия на F5
			break;																											//Выход из обработчика
		}	
	}
}

//debug - функция приостановки выполнения программы и вывода текущих значений переменных
void debug(){
	while(1){
		if(flag == 1){																							//Если флаг равен 1
			flag = 0;																									//Обнуление флага
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
				USART1->TDR = 0x0D;																			//Переход на новую строку
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
				USART1->TDR = 0x0A;																			//Возврат каретки
				break;																										//Окончание работы функции debug
		}
		if(flag == 2){																							//Если флаг равен 2
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x6E;																				//вывод символа 'n'
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x3D;																				//вывод символа '='				
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
				output(n);																							//Вызов функции output для вывода значения n
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0D;																				//Переход на новую строку		
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0A;																				//Возврат каретки
			flag = 0;																									//Обнуление флага				
		}
		if(flag == 3){																							//Если флаг равен 3
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x41;																				//вывод символа 'A'							
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x6E;																				//вывод символа 'n'					
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x3D;																				//вывод символа '='									
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			output(curA);																							//Вызов функции output для вывода значения curA
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0D;																				//Переход на новую строку	
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0A;																				//Возврат каретки
			flag = 0;																									//Обнуление флага				
		}
		if(flag == 4){																							//Если флаг равен 4
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x73;																				//вывод символа 's'							
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x3D;																				//вывод символа '='							
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			output(sum);																							//Вызов функции output для вывода значения sum
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0D;																				//Переход на новую строку
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = 0x0A;																				//Возврат каретки
			flag = 0;																									//Обнуление флага
		}
	}
}

//output функция для вывода числа
//number - число которое нужно вывести
void output (uint32_t number){
	uint8_t notZero = 0;																					//Флаг не равенства нулю
	uint8_t d100;																									//Сотни числа
	d100=(uint8_t)(number/100);																		//Присваивание d100 сотен полученного числа
	if (d100 != 0){																								//Если у числа есть сотни
		notZero = 1;																								//Присваивание флагу notZero единицы
		number -= d100*100;																					//Вычитание из числа сотен
		while ((USART1->ISR & USART_ISR_TXE) == 0) {} 							//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
		USART1->TDR = d100 + 48;																		//Вывод сотен
	}
	if (notZero == 1 ||  number/10 != 0){													//Если у числа были сотни или у числа есть десятки
		while ((USART1->ISR & USART_ISR_TXE) == 0) {} 							//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			USART1->TDR = (uint8_t)(number/10 + 48);									//Вывод десятков числа
	}
	while ((USART1->ISR & USART_ISR_TXE) == 0) {} 								//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
	USART1->TDR = (uint8_t)(number%10 + 48);											//Вывод единиц числа
	notZero = 0;																									//Сброс флага notZero
}

/*----------------------------------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе ЗТМ_01;
**		2. В управлении частотой мигания светодиода используйте микропереключатели SW3(старший разряд) и SW4(младший разряд). Примерная частота мигания: 00-4 Гц; 01-2 Гц; 10-1 Гц; 11 - 0.5 Гц;
**		3. На компьютере запустите приложение PuTTY и подключитесь к соответствующему COM-порту на скорости 115200 бит/с;
** 		4. При активном окне терминала нажмите различные кнопки клавиатуры и отследите ASCII-код каждой клавиши (управляющим клавишам должны соответствовать последовательности ASCII-кодов);
**----------------------------------------------------------------------------------------------------------------------------------------*/
