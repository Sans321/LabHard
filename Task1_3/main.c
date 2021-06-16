#include "main.h"//потключение библеотек

int main(void)//основная функция
{
	uint32_t byf;//переменная для чтения
	int32_t i=0;//счотчик
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;//
	GPIOB->OSPEEDR|=0X5555; //все порты на скорость в 10 мгц - 0b0101010101010101
	GPIOB->MODER|=GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 |//переключаем в режим "Oytpyt" линии порта Б с 0 по 7 и 9
								GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 |
								GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER9_0;
	GPIOB->MODER&=~( GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER11 | GPIO_MODER_MODER10);
	GPIOB->ODR|=0x200;//
	unsigned reg[3] = {0x23F, 0x206, 0x25B};//массив в каждый элемент которого код числа для 7 сигмент
	unsigned namber[4] = {0, 0, 0, 0};//массив чисел каторые надо вывести на 7 сиг
	while(1)//перевод и вывод
	{
		namber[0] = 0;//обнуление выводимого числа
		namber[1] = 0;//
		namber[2] = 0;//
		namber[3] = 0;//
		i=0;//обнуление счотчика
		byf=(((GPIOB->IDR)&0x1000)>>12)+(((GPIOB->IDR)&0x2000)>>12)+4+(((GPIOB->IDR)&0x4000)>>11)+(((GPIOB->IDR)&0x8000)>>11);//получение вводимого числа
		while(byf!=0)//перевод
		{
			namber[i]=byf%3;//делем число на систем счисления и остаток записываем
			byf=byf/3;//целую освавляем
			i=i+1;//смещение счотчика
		}
		for (int32_t s=3;s>=0;s--)//вывод
		{
				if(s<i)//отсичение незначимых нулей
				{
					GPIOB->ODR=reg[namber[s]];//вывод числа
					if(s==0)//добовление точки
					{
						GPIOB->ODR|=0x280;//добавление точки в конец числа
					}
					delay(500000);//ожидание 
					GPIOB->ODR=0xffff0000;//выключение дисплея
					delay(50000);//ожидание
				}
		}
	}
}
void delay(uint32_t count)//метод ожидания
{
	for (uint32_t i=0;i<count;i++);
}
