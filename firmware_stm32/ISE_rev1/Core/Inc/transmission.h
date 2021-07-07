#ifndef _TRANSMISSION_H
#define _TRANSMISSION_H
#include "main.h"
#define MAX_SIZE_BUFFER 512


// Funzione per la trasmissione dei campioni dei due canali
void buffer_transmission(UART_HandleTypeDef *huart2, uint8_t *circular_buffer0, uint8_t *circular_buffer1, int *correctlySentData);

#endif // _TRANSMISSION_H
