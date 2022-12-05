/*!
	@file   drv_spi.c
	@brief  <brief description here>
	@t.odo	-
	---------------------------------------------------------------------------

	MIT License
	Copyright (c) 2021 Ioannis Deligiannis, Federico Carnevale

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/

#include "drv_spi.h"

#ifdef DRV_SPI_ENABLED
/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static spi_t *spi_interfaces[6] = {NULL};
static uint32_t spi_interfaces_cnt = 0;

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

i_status spi_initialize(spi_t* instance)
{
	if(instance == NULL)
		return I_ERROR;

	if((instance->dt_rx == NULL)||(instance->dt_tx == NULL))
		return I_ERROR;

	if(HAL_SPI_GetState(instance->handler)!=HAL_SPI_STATE_READY)
	{
		instance->mx_init(); // only if not initialized
	}
	else
	{
		HAL_SPI_DeInit(instance->handler);
		instance->mx_init();
	}

	instance->init_sts = 1;
	instance->is_busy = 0;
	instance->pack_received = 0;
	instance->error = 0;

	HAL_SPIEx_FlushRxFifo(instance->handler);

	for(register int i=0;i<spi_interfaces_cnt;i++)
		if(spi_interfaces[i] == instance)
			return I_OK;
	spi_interfaces[spi_interfaces_cnt] = instance;
	spi_interfaces_cnt++;

	if(instance->type == SPI_MASTER)
		HAL_GPIO_WritePin(instance->nss_gpio_port, instance->nss_gpio_pin, SET);
	return I_OK;
}

i_status spi_deinitialize(spi_t* instance)
{
	if(instance == NULL)
		return I_ERROR;

	instance->init_sts = 0;
	instance->is_busy = 0;
	instance->pack_received = 0;
	HAL_SPI_DeInit(instance->handler);
	return I_OK;
}

i_status spi_set_msg(spi_t* instance,uint8_t *msg,uint8_t len,uint8_t start_pos)
{
	if(instance == NULL)
		return I_ERROR;

	if(instance->dt_sz < (len+start_pos))
		return I_INVALID;

	__disable_irq();
	memmove(&instance->dt_tx[start_pos],msg,len);
	__DSB();
	__enable_irq();

	return I_OK;
}

i_status spi_send_msg( spi_t* instance)
{
	if(instance == NULL)
		return I_ERROR;

	if(instance->init_sts == 0)
		return I_ERROR;

	instance->spi_timeout = HAL_GetTick();
	while((instance->is_busy != 0) && iTIMEOUT( HAL_GetTick(),instance->spi_timeout,100)==0 ) __NOP();

	if(instance->is_busy==1)
		return I_ERROR;
	instance->is_busy = 1;
	instance->dt_tx[0] = '<';
	instance->dt_tx[1] = '<';
	instance->dt_tx[instance->dt_sz-2] = '>';
	instance->dt_tx[instance->dt_sz-1] = '>';
	if(instance->type == SPI_MASTER)
		HAL_GPIO_WritePin(instance->nss_gpio_port, instance->nss_gpio_pin, RESET);
	for(int i=0;i<50;i++)__NOP();
	HAL_StatusTypeDef temp = HAL_SPI_TransmitReceive_DMA(instance->handler, instance->dt_tx,instance->dt_rx,instance->dt_sz );

	return temp == HAL_OK ? I_OK : temp == HAL_ERROR ? I_ERROR :I_DEBUG_01;
}

i_status spi_start(spi_t* instance)
{
	if(instance == NULL)
		return I_ERROR;

	if(instance->type == SPI_SLAVE)
	{
		instance->dt_tx[0] = '<';
		instance->dt_tx[1] = '<';
		instance->dt_tx[instance->dt_sz-2] = '>';
		instance->dt_tx[instance->dt_sz-1] = '>';
		return HAL_SPI_TransmitReceive_DMA(instance->handler, instance->dt_tx,instance->dt_rx,instance->dt_sz) == HAL_OK ? I_OK : I_ERROR;
	}
	return I_INVALID;
}

i_status spi_stop(spi_t* instance)
{
	if(instance == NULL)
		return I_ERROR;

	HAL_SPI_Abort(instance->handler);
	HAL_SPI_DMAStop(instance->handler);
	HAL_SPIEx_FlushRxFifo(instance->handler);

	if(instance->type == SPI_MASTER)
		HAL_GPIO_WritePin(instance->nss_gpio_port, instance->nss_gpio_pin,GPIO_PIN_SET);

	for(int i=0;i<50;i++)
	__NOP();
	HAL_SPI_FlushRxFifo(instance->handler);
	HAL_SPI_DeInit(instance->handler);

	return I_OK;
}

void spi_set(spi_t* instance, uint32_t position,uint8_t *value, uint32_t val_sz)
{
	__disable_irq();
	memmove(&instance->dt_tx[position],value,val_sz);
	__DSB();
	__enable_irq();
}

i_status spi_get(spi_t* instance,uint32_t position,uint8_t *value, uint32_t val_sz)
{
	__disable_irq();
	memmove(value,&instance->dt_tx[position],val_sz);
	__DSB();
	__enable_irq();
	return I_OK;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	while(hspi->Instance->SR&0x0040)
	{
		__HAL_SPI_CLEAR_OVRFLAG(hspi);
	}

	while(hspi->Instance->SR&0x0010)
	{
		__HAL_SPI_CLEAR_CRCERRFLAG(hspi);
	}

	static spi_t* current_spi;

	for(register int i=0;i<spi_interfaces_cnt;i++)
		if(spi_interfaces[i]->handler == hspi)
			current_spi = spi_interfaces[i];

	if(current_spi == NULL) return;
	current_spi->error = 1;
}

/******************************************************************************
* Definition  | Callback Functions
******************************************************************************/

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	static spi_t* current_spi;

	for(register int i=0;i<spi_interfaces_cnt;i++)
		if(spi_interfaces[i]->handler == hspi)
			current_spi = spi_interfaces[i];

	if(current_spi == NULL) return;

	current_spi->is_busy = 0;

	if(current_spi->type== SPI_MASTER)
		HAL_GPIO_WritePin(current_spi->nss_gpio_port, current_spi->nss_gpio_pin, GPIO_PIN_SET);

	current_spi->pack_received = 1;

	#if __has_include("FreeRTOS.h")
	current_spi->spi_last_reception = xTaskGetTickCount();
	#else
	current_spi->spi_last_reception = HAL_GetTick();
	#endif

	if(current_spi->error == 1 || current_spi->dt_rx[0] != '<' || current_spi->dt_rx[1] != '<' || current_spi->dt_rx[current_spi->dt_sz-2] != '>' || current_spi->dt_rx[current_spi->dt_sz-1] != '>')
	{
		HAL_SPI_DMAStop(current_spi->handler);
		HAL_SPI_DeInit(current_spi->handler);
		spi_initialize(current_spi);
		spi_start(current_spi);
	}
}

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
