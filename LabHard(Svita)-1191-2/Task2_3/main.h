#include "stm32f0xx.h"
#include <stdio.h>
typedef int bool; // Определение своего типа данных 
#define true 1 // объявление константы в виде макроса
#define false 0 // объявление константы в виде макроса
#define DELETE 0x10
#define MAX_MESSAGE_LENGTH 50
#define NUMBER_TO_ASCII 48
void initUSART1(void);
int convert_dec_to(int decimal, int to, char *buffer);
void notification(uint8_t *message);
void show_char(char ch);  
