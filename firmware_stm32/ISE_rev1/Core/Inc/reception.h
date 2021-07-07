#ifndef _RECEPTION_H
#define _RECEPTION_H

#include "transmission.h"
#include <math.h>

#define MAX_SIZE 8


// Struttura contenente le variabili utili per la ricezione
typedef struct{
	// Flag per la rilevazione dello start bit
	int flag_asterisco;
	// Flag per la rilevazione della T (Trigger type o Trigger level)
	int flag_trigger;
	// Flag per la rilevaione della S (Sampling period
	int flag_sampling;
	// Flag per la rilevazione della seconda T (trigger Type)
	int flag_trigger_type;
	// Flag per la rilevazione della L (trigger Level)
	int flag_trigger_level;
	// Flag per la rilevazione della P (sampling Period)
	int flag_sampling_time;
	// Array che contiene i caratteri esadecimali che rappresentano il valore del comando e da convertire in decimale
	char data_hex[MAX_SIZE];
	// Variabile per la rilevazione degli errori
    int check_error;
    // Flag utilizzato per capire se è avvenuta una variazione nel periodo di campionamento
	int flag_change_period;
	// Valore ritornato dalla funzione di conversione
	int returned_value;
	// Indice dell'array esadecimale
	int index_hexadecimal;

	int flag_cancelletto;
}struct_datiR;

// Funzione per la rilevazione del comando ricevuto e relativi dati
void received_commands(struct_datiR *reception_parameters, int *trigger_type, int *trigger_level, int *sampling_time, uint8_t data_received);

// Funzione per la conversione esadecimale-decimale
int hex_to_decimal_conversion(struct_datiR *reception_parameters);

// Funzione di inizializzazione della struttura dati
void init_function(struct_datiR *reception_parameters);

#endif // _RECEPTION_H
