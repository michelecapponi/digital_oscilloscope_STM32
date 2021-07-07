#include "reception.h"

void received_commands(struct_datiR *reception_parameters, int *trigger_type, int *trigger_level, int *sampling_time, uint8_t data_received)
{
	// Controllo ad uno ad uno i caratteri ricevuti ed attivo i flag di comando corrispondente
	// Una volta ricevuto il tipo di comando, si procede al salvataggio dei caratteri esadecimali in un array
	// Infine, ricevuto lo stop bit, i dati ricevuti vengono convertiti ed associati al corrispondente comando (trigger type, trigger level o sampling time)
	// Viene anche fatto un confronto tra il valore ricevuto del nuovo sampling period e l'ultimo salvato, così da capire se le impostazioni del timer debbano essere modificate
    if(data_received=='*' && reception_parameters->flag_asterisco==0)
    {
        reception_parameters->flag_asterisco=1;
        reception_parameters->flag_trigger=0;
        reception_parameters->flag_trigger_type=0;
        reception_parameters->flag_trigger_level=0;
        reception_parameters->flag_sampling=0;
        reception_parameters->flag_sampling_time=0;
        reception_parameters->returned_value=0;
        reception_parameters->check_error=0;
        reception_parameters->index_hexadecimal=0;
        reception_parameters->flag_change_period=0;
        reception_parameters->flag_cancelletto=0;
    }
    else if(data_received=='T' && reception_parameters->flag_asterisco==1 && reception_parameters->flag_trigger==0)
    {
        reception_parameters->flag_trigger=1;
    }
    else if(data_received=='T' && reception_parameters->flag_trigger==1 && reception_parameters->flag_trigger_type==0)
    {
        reception_parameters->flag_trigger_type=1;
    }
    else if(data_received=='L' && reception_parameters->flag_trigger==1 && reception_parameters->flag_trigger_level==0)
    {
        reception_parameters->flag_trigger_level=1;
    }
    else if(data_received=='S' && reception_parameters->flag_asterisco==1 && reception_parameters->flag_sampling==0)
    {
        reception_parameters->flag_sampling=1;
    }
    else if(data_received=='P' && reception_parameters->flag_sampling==1 && reception_parameters->flag_sampling_time==0)
    {
        reception_parameters->flag_sampling_time=1;
    }
    else if(data_received=='#' && (((reception_parameters->flag_trigger_type==1 || reception_parameters->flag_trigger_level==1) && reception_parameters->index_hexadecimal==2) || (reception_parameters->flag_sampling_time==1 && reception_parameters->index_hexadecimal==8)) && reception_parameters->check_error==0)
    {
        reception_parameters->returned_value=hex_to_decimal_conversion(reception_parameters);
        if(reception_parameters->flag_trigger_type==1)
        {
            *trigger_type=reception_parameters->returned_value;
        }
        else if(reception_parameters->flag_trigger_level==1)
        {
             *trigger_level=reception_parameters->returned_value;
        }
        else if(reception_parameters->flag_sampling_time==1)
        {
            if(*sampling_time!=reception_parameters->returned_value)
            {
                reception_parameters->flag_change_period=1;
            }
             *sampling_time=reception_parameters->returned_value;
        }
        reception_parameters->flag_asterisco=0;
        reception_parameters->flag_cancelletto=1;
    }
    else
    {
        reception_parameters->data_hex[reception_parameters->index_hexadecimal]=data_received;
        reception_parameters->index_hexadecimal++;
        if(reception_parameters->flag_trigger_type==1 || reception_parameters->flag_trigger_level==1)
        {
            if(reception_parameters->index_hexadecimal>2)
            {
                reception_parameters->check_error=1;
            }
        }
        else if(reception_parameters->flag_sampling_time==1)
        {
            if(reception_parameters->index_hexadecimal>8)
            {
                reception_parameters->check_error=1;
            }
        }
        else
        {
            reception_parameters->check_error=1;
        }
    }
}


int hex_to_decimal_conversion(struct_datiR *reception_parameters)
{
    int data_decimal=0;
    int data_temp=0;
    for(int i=0; i<reception_parameters->index_hexadecimal; i++)
    {
        switch (reception_parameters->data_hex[i])
        {
        case '0':
            data_temp=0;
            break;
        case '1':
            data_temp=1;
            break;
        case '2':
            data_temp=2;
            break;
        case '3':
            data_temp=3;
            break;
        case '4':
            data_temp=4;
            break;
        case '5':
            data_temp=5;
            break;
        case '6':
            data_temp=6;
            break;
        case '7':
            data_temp=7;
            break;
        case '8':
            data_temp=8;
            break;
        case '9':
            data_temp=9;
            break;
        case 'A':
            data_temp=10;
            break;
        case 'B':
            data_temp=11;
            break;
        case 'C':
            data_temp=12;
            break;
        case 'D':
            data_temp=13;
            break;
        case 'E':
            data_temp=14;
            break;
        case 'F':
            data_temp=15;
            break;
        case 'a':
            data_temp=10;
            break;
        case 'b':
            data_temp=11;
            break;
        case 'c':
            data_temp=12;
            break;
        case 'd':
            data_temp=13;
            break;
        case 'e':
            data_temp=14;
            break;
        case 'f':
            data_temp=15;
            break;
        default:
        	// Se entro qui vuol dire che ho ricevuto un carattere non valido
            reception_parameters->check_error=1;
            return -1;
            break;
        }
        data_decimal=data_decimal+data_temp*pow(16, (reception_parameters->index_hexadecimal-(i+1)));
    }
    return data_decimal;
}

void init_function(struct_datiR *reception_parameters)
{
	reception_parameters->flag_asterisco=0;
	reception_parameters->flag_sampling=0;
	reception_parameters->flag_sampling_time=0;
	reception_parameters->flag_trigger=0;
	reception_parameters->flag_trigger_level=0;
	reception_parameters->flag_trigger_type=0;
	reception_parameters->flag_change_period=0;
	reception_parameters->index_hexadecimal=0;
	reception_parameters->flag_cancelletto=0;

	for (int i=0; i<8; i++)
	{
		reception_parameters->data_hex[MAX_SIZE]='0';
	}

	reception_parameters->check_error=0;
}
