/****************************************************************************/
//  File    : main.c
//---------------------------------------------------------------------------
//   Date      | Author | Version |  Modification 
//-------------+--------+---------+------------------------------------------
// 9 Nov 2021 |  WHI   |   1.1   |  Creation
/****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"

#define SAMPLES_IN_BUFFER 6

int Avg_ADC;

uint16_t P_ADV_TEMPE_TABLE[] = {
	963,  1001, 1040, 1079, 1120,
	1161, 1202, 1244, 1287, 1330,
	1374, 1418, 1462, 1507, 1552
};

uint16_t M_ADV_TEMPE_TABLE[] = {
	963, 926, 889, 854, 819,
	785, 752, 720, 688, 658,
	629, 600, 572, 546, 520
};


float GetTempeFromADC(int nADC)
{	
	int nToI = sizeof(P_ADV_TEMPE_TABLE) / sizeof(P_ADV_TEMPE_TABLE[0]);
	
	if (nADC > P_ADV_TEMPE_TABLE[nToI - 1])
		return (float)(nToI - 1);
	
	else if (nADC < M_ADV_TEMPE_TABLE[nToI - 1])
		return (float) -(nToI - 1);
	
	else if (nADC <= M_ADV_TEMPE_TABLE[0])
	{
		for(int i = 0; i < nToI; i++)
		{
			if(nADC == M_ADV_TEMPE_TABLE[i])
				return (float) - i;
			else if(nADC > M_ADV_TEMPE_TABLE[i])
			{
				float mTemp = (float)i - 1;	
				mTemp += (float)(nADC - M_ADV_TEMPE_TABLE[i - 1]) / (float)(M_ADV_TEMPE_TABLE[i] - M_ADV_TEMPE_TABLE[i - 1]);
				return (float) - mTemp;
			}
		}	
	}
	
	else
	{
		for(int i = 1; i < nToI; i++)
		{
			if(nADC == P_ADV_TEMPE_TABLE[i])
				return i;
			else if(nADC < P_ADV_TEMPE_TABLE[i])
			{
				float pTemp = (float)i - 1;	
				pTemp += (float)(nADC - P_ADV_TEMPE_TABLE[i - 1]) / (float)(P_ADV_TEMPE_TABLE[i] - P_ADV_TEMPE_TABLE[i - 1]);
				return pTemp;
			}			
		}
	}
	
	return 0.0F;
	
}
	

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if(p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		ret_code_t err_code;
	
		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);

		int sum = 0;
		int max = p_event->data.done.p_buffer[0];
		int	min = p_event->data.done.p_buffer[0];

		for(int i = 0; i < SAMPLES_IN_BUFFER; i++)
		{
			if (p_event->data.done.p_buffer[i] > max)
				max = p_event->data.done.p_buffer[i];
			
			if (p_event->data.done.p_buffer[i] < min)
				min = p_event->data.done.p_buffer[i];
			
			sum += p_event->data.done.p_buffer[i];
		}

		sum -= max;
		sum -= min;

		Avg_ADC = sum / (SAMPLES_IN_BUFFER-2);
	}

}

void saadc_init(void)
{
	ret_code_t err_code;
	nrf_saadc_channel_config_t channel_config =
	NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN5);
	
	channel_config.reference = NRF_SAADC_REFERENCE_VDD4; // VDD/4
	channel_config.gain = NRF_SAADC_GAIN1_4; // Gaub factor 1/4
	channel_config.acq_time = NRF_SAADC_ACQTIME_20US; // ADC Sampling time

	err_code = nrf_drv_saadc_init(NULL, saadc_callback);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_channel_init(0, &channel_config);
	APP_ERROR_CHECK(err_code);

}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
	NRF_LOG_DEFAULT_BACKENDS_INIT();
	saadc_init();
	NRF_LOG_INFO("Application started.");

	while (1)
	{
	
	}
}


/** @} */
