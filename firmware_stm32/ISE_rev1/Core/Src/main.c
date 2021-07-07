/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "transmission.h"
#include "reception.h"
#include "peripheral_configuration.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// Variabili Globali

// Buffer circolare per il canale 0
uint8_t circular_buffer0[MAX_SIZE_BUFFER];

// Buffer circolare per il canale 1
uint8_t circular_buffer1[MAX_SIZE_BUFFER];

// Variabile per il singolo byte ricevuto da seriale
uint8_t data_received;

// Indice per il controllo del canale appena convertito
int channel_index=0;

// Indice per la posizione del prossimo campione nel buffer 0
int sample_index0=0;

// Indice per la posizione del prossimo campione nel buffer 1
int sample_index1=0;

// Variabile per il trigger level (inizializzazione al valore di default)
int trigger_level=128;

// Variabile per il trigger type (inizializzazione al valore di default = AUTO)
int trigger_type=0;

// Variabile per il sampling time (inizializzazione al valore di default = 1 ms)
int sampling_time=1000000;

/*VARIABILI INTERRUPT ADC*/

// Indice che comunica quando iniziare la trasmissione (quando sono stati presi 255 campioni dopo aver trovato il trigger level)
int cntdown_transmission_trigger_level=256;

// Indice che comunica quando iniziare la trasmissione (quando sono stati presi 512 campioni (caso in cui non viene superato il trigger level e siamo in AUTO))
int cntdown_transmission_hz_sweep=256;

// Variabile di appoggio per l'identificazione della singola trasmissione (SINGLE MODE)
int one_shot_transmission=0;

// Flag per capire se il dato è stato trasmesso correttamente
int correctlySentData=0;

// Contatore per l'horizontal sweep time
int hz_sweep_counter=0;

// Variabile per controllare se ho superato il trigger level partendo da un valore inferiore ad esso (inizializzata a 256 per far si che all'accensione non sia superato automaticamente il trigger level per un valore campionato superiore)
int previous_value=256;

// Flag per il controllo della condizione di trigger dovuta al superamento dell'horizontal sweep time
int flag_hz_sweep=0;

// Flag per il controllo della condizione di trigger dovuta al superamento del trigger level
int flag_trigger_level=0;

// Acknowledge per avvisare il main loop del riempimento del buffer in seguito al superamento del trigger level
int ack_trigger_level=0;

// Acknowledge per avvisare il main loop del riempimento del buffer in seguito al raggiungimento dell'horizontal sweep
int ack_hz_sweep=0;


// Struttura dati utilizzata per la ricezione
struct_datiR reception_parameters;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // Comando per far partire l'ADC in modalita' interrupt
  HAL_ADC_Start_IT(&hadc1);
  // Comando per far partire il timer 3 per la generazione del trigger per l'ADC
  HAL_TIM_Base_Start(&htim3); //faccio partire il timer

  // Comando per l'impostazione della variabile di ricezione per la trasmissione seriale
  HAL_UART_Receive_IT(&huart2, &data_received, 1);

  // Funzione di inizializzazione per la struttura precedentemente dichiarata
  init_function(&reception_parameters);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      //Switch utilizzato per la trasmissione
      switch (trigger_type)
      {

      /*MODALITA AUTO*/
      case 0:
    	  // Controllo se è arrivato l'ack dovuto al trigger level dall'ADC
          if(ack_trigger_level==1)
              // Trasmetto se il countdown del trigger level è arrivato a '0'
          {

              /*DISABILITO IL TIMER DURANTE LA TRASMISSIONE*/
              HAL_TIM_Base_Stop(&htim3);

              /*EFFETTUO LA TRASMISSIONE DEL BUFFER*/
              buffer_transmission(&huart2, circular_buffer0, circular_buffer1, &correctlySentData); //effettuo la trasmissione del buffer

              // Se vengo dalla modalità single, resetto il flag per la singola trasmissione
              one_shot_transmission=0;

              /*RIPRISTINO ACK TRIGGER LEVEL*/
              ack_trigger_level=0;

              sample_index0=0;
              sample_index1=0;

              // Reinizializzo il flag per la corretta trasmissione
              if(correctlySentData == 1)
              {
                  correctlySentData = 0;
              }

              /*FACCIO RIPARTIRE IL TIMER*/
              HAL_TIM_Base_Start(&htim3);
          }
          // Controllo se è arrivato l'ack dovuto al horizontal sweep dall'ADC
          else if(ack_hz_sweep==1)

          {
              /*DISABILITO IL TIMER DURANTE LA TRASMISSIONE*/
              HAL_TIM_Base_Stop(&htim3);

              /*EFFETTUO LA TRASMISSIONE DEL BUFFER*/
              buffer_transmission(&huart2, circular_buffer0, circular_buffer1, &correctlySentData); //effettuo la trasmissione del buffer

              // Se vengo dalla modalità single, resetto il flag per la singola trasmissione
              one_shot_transmission=0;

              /*RIPRISTINO ACK HORZIONTAL SWEEP*/
              ack_hz_sweep=0;

              sample_index0=0;
              sample_index1=0;

              // Reinizializzo il flag per la corretta trasmissione
              if(correctlySentData == 1)
              {
                  correctlySentData = 0;
              }

              /*FACCIO RIPARTIRE IL TIMER*/
              HAL_TIM_Base_Start(&htim3);
          }
          break;


      /*MODALITA NORMAL*/
      case 1:
    	  // Potendo arrivare qui dalla modalità AUTO, resettiamo l'eventuale ack derivato dall' horizontal sweep
          ack_hz_sweep=0;

          // Controllo se ho ricevuto un ack del trigger level dell'ADC
          if(ack_trigger_level==1)
          {
              /*DISABILITO IL TIMER DURANTE LA TRASMISSIONE*/
              HAL_TIM_Base_Stop(&htim3);

              /*EFFETTUO LA TRASMISSIONE DEL BUFFER*/
              buffer_transmission(&huart2, circular_buffer0, circular_buffer1, &correctlySentData); //effettuo la trasmissione del buffer

              // Se vengo dalla modalità single, resetto il flag per la singola trasmissione
              one_shot_transmission=0;

              /*RIPRISTINO ACK TRIGGER LEVEL*/
              ack_trigger_level=0;

              sample_index0=0;
              sample_index1=0;

              // Reinizializzo il flag per la corretta trasmissione
              if(correctlySentData == 1)
              {
                  correctlySentData = 0;
              }

              /*FACCIO RIPARTIRE IL TIMER*/
              HAL_TIM_Base_Start(&htim3);
          }
          break;


      case 2:
    	  // Potendo arrivare qui dalla modalità AUTO, resettiamo l'eventuale ack derivato dall' horizontal sweep
          ack_hz_sweep=0;

          // Controllo se è arrivato l'ack del trigger level e non ho ancora mai trasmesso il buffer
          if(ack_trigger_level==1 && one_shot_transmission==0)
          {
              /*DISABILITO IL TIMER DURANTE LA TRASMISSIONE*/
              HAL_TIM_Base_Stop(&htim3); //disabilito il timer per la trasmissione

              /*EFFETTUO LA TRASMISSIONE DEL BUFFER*/
              buffer_transmission(&huart2, circular_buffer0, circular_buffer1, &correctlySentData); //effettuo la trasmissione del buffer

              // Essendo in modalita' SINGLE, attivo questo flag per la singola trasmissione
              one_shot_transmission=1;

              /*RIPRISTINO ACK TRIGGER LEVEL*/
              ack_trigger_level=0;

              sample_index0=0;
              sample_index1=0;

              // Reinizializzo il flag per la corretta trasmissione
              if(correctlySentData == 1)
              {
                  correctlySentData = 0;
              }

              /*FACCIO RIPARTIRE IL TIMER*/
              HAL_TIM_Base_Start(&htim3);
          }
          break;
      case 3:

          // Se vengo dalla modalità single, resetto il flag per la singola trasmissione
          one_shot_transmission=0;

          break;
      default:
          break;
      }


      // Se ho riscontrato un errore, faccio togglar il led interno della scheda
      if(reception_parameters.check_error==1)
      {
          HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
          reception_parameters.check_error=0;
          reception_parameters.flag_asterisco=0;
          reception_parameters.flag_trigger=0;
          reception_parameters.flag_trigger_type=0;
          reception_parameters.flag_trigger_level=0;
          reception_parameters.flag_sampling=0;
          reception_parameters.flag_sampling_time=0;
          reception_parameters.returned_value=0;
          reception_parameters.index_hexadecimal=0;
          reception_parameters.flag_change_period=0;
      }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 168;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 500;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef * hadc)
{

	uint8_t current_value=0;

	if(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC)){ //controllo se l'interrupt è scaturito da una conversione


		/*OPERAZIONI SUL CANALE 0*/
		if(channel_index==0) //controllo se l'interrupt è avvenuto in seguito alla conversione del primo canale
		{
			current_value=HAL_ADC_GetValue(&hadc1); //memorizzo il valore convertito in una variabile



			/*CONTROLLO LA DIMENSIONE DEL BUFFER PER SAPERE SE DEVO SHIFTARE I CAMPIONI*/
			if(sample_index0==MAX_SIZE_BUFFER)//se il buffer è pieno effettuo lo shift di due posizioni di tutti i campioni precedenti
			{
				sample_index0=MAX_SIZE_BUFFER-1; //se si arriva a 1024, si ritorna a 1022
				for(int i=1; i<MAX_SIZE_BUFFER; i++) //shift del buffer circolare in base alla risoluzione della conversione esadecimale
				{
					circular_buffer0[i-1]=circular_buffer0[i];
				}
			}



			/*INSERISCO IL NUOVO CAMPIONE NEL BUFFER DEL CANALE 0*/
			circular_buffer0[sample_index0]=current_value;


			/*AGGIORNAMENTO DELLE VARIABILI NECESSARIO PER LE SUCCESSIVE ROUTINE DI INTERRUPT DELL'ADC*/
			sample_index0=sample_index0+1; //aggiorno l'indice del buffer
			channel_index=1; //aggiorno il canale: al prossimo ingresso nella routine di interrupt per l'EOC, ci sarà la conversione del canale 1



			/*VALUTARE SE INSERIRE UNA CONDIZIONE CHE BLOCCA LE SUCCESSIVE ISTRUZIONI NEL CASO IN CUI L'ACK NON SIA STATO RESETTATO*/
			if(ack_hz_sweep==0 && ack_trigger_level==0)
			{
				/*VALUTAZIONI DI TUTTE LE POSSIBILI CONDIZIONI DI TRIGGER*/
				if (flag_trigger_level==0) //se ho già superato il trigger level e sto facendo il countdown non devo entrare in questo if
				{
					if (current_value>=trigger_level && previous_value<trigger_level && sample_index0>=256)//se il valore precedente era minore del tr_level e il valore corrente è maggiore del tr_level è innescata la condizione di trigger
					{
						flag_trigger_level=1;
					}else if(flag_hz_sweep==0) //se non è ancora attiva la condizione di trigger dovuta al superamento del trigger level e non era precedentemente attiva la condizione di trigger dell'horizontal sweep devo verificare se ora ho oltrepassato la dinamica orizzontale
					{
						if(hz_sweep_counter>=256)
						{
							flag_hz_sweep=1;
						}
					}
				}
				hz_sweep_counter++; //incremento il valore dell'orizzontal sweep counter (la variabile mi serve per capire se ho oltrepassato la dinamica orizzontale)
				previous_value=current_value; //aggiorno la variabile previous value con il valore corrente letto (spostata)
			}



		/*OPERAZIONI SUL CANALE 1*/
		}else if(channel_index==1)
		{

			current_value=HAL_ADC_GetValue(&hadc1); //memorizzo il valore convertito in una variabile


			/*CONTROLLO LA DIMENSIONE DEL BUFFER PER SAPERE SE DEVO SHIFTARE I CAMPIONI*/
			if(sample_index1==MAX_SIZE_BUFFER)
				{
					sample_index1=MAX_SIZE_BUFFER-1; //se si arriva a 1024, si ritorna a 1022
					for(int i=1; i<MAX_SIZE_BUFFER; i++) //shift del buffer circolare in base alla risoluzione della conversione esadecimale
					{
						circular_buffer1[i-1]=circular_buffer1[i];
					}
				}

			/*INSERISCO IL NUOVO CAMPIONE NEL BUFFER DEL CANALE 1*/
			circular_buffer1[sample_index1]=current_value;

			/*AGGIORNAMENTO DELLE VARIABILI NECESSARIO PER LE SUCCESSIVE ROUTINE DI INTERRUPT DELL'ADC*/
			sample_index1=sample_index1+1;
			channel_index=0; //al prossimo ingresso nella routine di interrupt per l'EOC, ci sarà la conversione del canale 0 (modificato)


			/*VALUTARE SE INSERIRE UNA CONDIZIONE CHE BLOCCA LE SUCCESSIVE ISTRUZIONI NEL CASO IN CUI L'ACK NON SIA STATO RESETTATO*/
			if(ack_hz_sweep==0 && ack_trigger_level==0)
			{
				/*CONTROLLO SE IL CANALE 0 HA ATTIVO IL FLAG DI INTERRUPT E DECREMENTO EVENUTALMENTE I VARI COUNTDOWN*/
				if (flag_trigger_level==1) //controllo se era attivo il flag del trigger level perchè in caso devo decrementare il suo countdown e se sono arrivato a 0 setto l'ack per il main loop
				{
					cntdown_transmission_trigger_level--;
					if (cntdown_transmission_trigger_level==0)
					{
						ack_trigger_level=1; //setto un ack per il main loop in modo da avvisare del riempimento del buffer in seguito al superamento del trigger level
						trigger_configuration_reset(); // ripristino al valore inziale tutti i flag e i countdown
					}
				}else if(flag_hz_sweep==1)
				{
					cntdown_transmission_hz_sweep--;
					if (cntdown_transmission_hz_sweep==0)
					{
						ack_hz_sweep=1;
						trigger_configuration_reset();
					}
				}
			}

		}
	}
}


/*DEFINISCO LA FUNZIONE CHE CONSENTE DI RIPRSTINARE I VALORI DELLE VARIABILI DEL TRIGGER*/
void trigger_configuration_reset()
{
	flag_hz_sweep=0;
	flag_trigger_level=0;
	cntdown_transmission_trigger_level=256;
	cntdown_transmission_hz_sweep=256;
	hz_sweep_counter=0;
}






void HAL_UART_TxCpltCallback (UART_HandleTypeDef *huart)
{
correctlySentData = 1;
}

void HAL_UART_RxCpltCallback (UART_HandleTypeDef *huart)
{
	// Funzione per la ricezione
	received_commands(&reception_parameters, &trigger_type, &trigger_level, &sampling_time, data_received);

	if(reception_parameters.flag_cancelletto==1){
		// Se ho cambiato qualche configurazione, devo ricominciare a prendere i campioni da capo
		change_configuration(sampling_time, &htim3, &reception_parameters, &ack_hz_sweep, &ack_trigger_level, &sample_index0, &sample_index1);
		reception_parameters.flag_cancelletto=0;
	}

	// Dopo aver ricevuto un carattere, riattivo la ricezione per i prossimi
	HAL_UART_Receive_IT(&huart2, &data_received, 1);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
