#ifndef _PERIPHERAL_CONFIGURATION_H
#define _PERIPHERAL_CONFIGURATION_H
#include "main.h"

#include "reception.h"

// In base alle impostazioni del prescaler del timer, per riportare l'unita' di misura in ns, si deve dividere il tempo ricevuto per 2000
#define TIMER_RESOLUTION_NS 2000

// Funzione per la modifica del valore del registro di AUTORELOAD del timer
void change_configuration(int sampling_time, TIM_HandleTypeDef *htim3, struct_datiR *reception_parameters, int *ack_hz_sweep, int *ack_trigger_level, int *sample_index0, int *sample_index1);

#endif // _PERIPHERAL_CONFIGURATION_H
