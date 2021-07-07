#include "peripheral_configuration.h"
#include <stdio.h>
#include <stdint.h>


void change_configuration(int sampling_time, TIM_HandleTypeDef *htim3, struct_datiR *reception_parameters, int *ack_hz_sweep, int *ack_trigger_level, int *sample_index0, int*sample_index1)
{
	if(reception_parameters->flag_change_period==1)
	{
		uint16_t period = sampling_time/TIMER_RESOLUTION_NS;

		// Comando per settare il valore del registro di AUTORELOAD del timer
		__HAL_TIM_SET_AUTORELOAD(htim3, period);
	}
	// Se è cambiato il perido di campionamento, devo svuotare i buffer e riempirli da capo
	*sample_index0=0;
	*sample_index1=0;

	// Azzero anche le variabili dell' ack
	*ack_hz_sweep=0;
	*ack_trigger_level=0;

}
