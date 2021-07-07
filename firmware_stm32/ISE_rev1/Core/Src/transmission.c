#include "transmission.h"
#include <stdio.h>
#include <string.h>


void buffer_transmission(UART_HandleTypeDef *huart2, uint8_t *circular_buffer0, uint8_t *circular_buffer1, int *correctlySentData)
{
	// Creo un buffer in cui verranno inseriti i campioni come da specifiche
	uint8_t transmission_buffer[MAX_SIZE_BUFFER*4+2];

	char hexadecimal_value[3]={'\0'};

	// Inserisco lo start bit
	transmission_buffer[0]='*';

	int j=1;

	// Inserisco i campioni nel formato aabb (aa = campione primo canale, bb = campione secondo canale)
	for(int i=0; i<(MAX_SIZE_BUFFER); i++)
	{
		sprintf(hexadecimal_value, "%.2X", circular_buffer0[i]);
		transmission_buffer[j]=(uint8_t)hexadecimal_value[0];
		transmission_buffer[j+1]=(uint8_t)hexadecimal_value[1];
		sprintf(hexadecimal_value, "%.2X", circular_buffer1[i]);
		transmission_buffer[j+2]=(uint8_t)hexadecimal_value[0];
		transmission_buffer[j+3]=(uint8_t)hexadecimal_value[1];
		j=j+4;
	}

	// Inserisco lo stop bit
	transmission_buffer[MAX_SIZE_BUFFER*4+1]='#';

	// Trasmetto il buffer
	HAL_UART_Transmit_IT(huart2, transmission_buffer, MAX_SIZE_BUFFER*4+2);

	// Attendo che la trasmissione sia completata
	while (*correctlySentData == 0){}


}
