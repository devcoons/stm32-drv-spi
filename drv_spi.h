/*!
	@file   drv_spi.h
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

#ifndef DRIVERS_INC_DRV_SPI_H_
#define DRIVERS_INC_DRV_SPI_H_

/******************************************************************************
* Includes
******************************************************************************/

#include <inttypes.h>
#include <limits.h>
#include <string.h>

#if __has_include("FreeRTOS.h")
#include "FreeRTOS.h"
#endif

#if __has_include("task.h")
#include "task.h"
#endif

#if __has_include("cmsis_os.h")
#include "cmsis_os.h"
#endif

#if __has_include("spi.h")
	#include "spi.h"
	#define DRV_SPI_ENABLED
#endif

#ifdef DRV_SPI_ENABLED
/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

#ifndef iTIMEOUT
#define iTIMEOUT(s,c,t)  (s < c ?  s - (UINT_MAX - c)   : s - c) < t ? 0 : 1
#endif

#if !defined(ENUM_I_STATUS)
#define ENUM_I_STATUS
typedef enum
{
	I_OK 			= 0x00,
	I_INVALID 		= 0x01,
	I_EXISTS 		= 0x02,
	I_NOTEXISTS 		= 0x03,
	I_FAILED 		= 0x04,
	I_EXPIRED 		= 0x05,
	I_UNKNOWN 		= 0x06,
	I_INPROGRESS 		= 0x07,
	I_IDLE			= 0x08,
	I_FULL			= 0x09,
	I_EMPTY			= 0x0A,
	I_YES			= 0x0B,
	I_NO			= 0x0C,
	I_SKIP			= 0x0D,
	I_LOCKED 		= 0x0E,
	I_INACTIVE 		= 0x0F,
	I_ACTIVE 		= 0x10,
	I_READY		 	= 0x11,
	I_WAIT 			= 0x12,
	I_OVERFLOW 		= 0x13,
	I_CONTINUE 		= 0x14,
	I_STOPPED 		= 0x15,
	I_WARNING 		= 0x16,
	I_SLEEP 		= 0x17,
	I_DEEPSLEEP 		= 0x18,
	I_STANDBY 		= 0x19,
	I_GRANTED 		= 0x1A,
	I_DENIED 		= 0x1B,
	I_DEBUG_01 		= 0xE0,
	I_DEBUG_02 		= 0xE1,
	I_DEBUG_03 		= 0xE2,
	I_DEBUG_04 		= 0xE3,
	I_DEBUG_05 		= 0xE4,
	I_DEBUG_06 		= 0xE5,
	I_DEBUG_07 		= 0xE6,
	I_DEBUG_08 		= 0xE7,
	I_DEBUG_09 		= 0xE8,
	I_DEBUG_10 		= 0xE9,
	I_DEBUG_11 		= 0xEA,
	I_DEBUG_12 		= 0xEB,
	I_DEBUG_13 		= 0xEC,
	I_DEBUG_14 		= 0xED,
	I_DEBUG_15 		= 0xEE,
	I_DEBUG_16 		= 0xEF,
	I_MEMALIGNED		= 0xFC,
	I_MEMUNALIGNED		= 0xFD,
	I_NOTIMPLEMENTED 	= 0xFE,
	I_ERROR 		= 0xFF
}i_status;
#endif

typedef enum
{
	SPI_MASTER = 0x00,
	SPI_SLAVE = 0x01
}DRV_SPI_TYPE;

typedef struct
{
	DRV_SPI_TYPE type;
	SPI_HandleTypeDef * handler;
	void(*mx_init)();		
	uint8_t *dt_tx;
	uint8_t *dt_rx;
	uint32_t dt_sz;
	uint8_t init_sts;
	uint8_t is_busy;
	uint8_t error;
	uint8_t pack_received;
	uint32_t spi_last_reception;
	uint32_t spi_timeout;
	GPIO_TypeDef *nss_gpio_port;//HW information - Port Chip select
	uint16_t nss_gpio_pin; //HW information - Pin Chip select
}spi_t;

/******************************************************************************
* Declaration | Public Functions
******************************************************************************/

i_status spi_initialize(spi_t* instance);
i_status spi_deinitialize(spi_t* instance);
i_status spi_start(spi_t* instance);
i_status spi_stop(spi_t* instance);
i_status spi_set_msg(spi_t* instance,uint8_t *msg,uint8_t len,uint8_t start_pos);
i_status spi_send_msg(spi_t* instance);
i_status spi_get(spi_t* instance, uint32_t position, uint8_t *value, uint32_t value_sz);
void spi_set(spi_t* instance, uint32_t position, uint8_t *value, uint32_t value_sz);

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
#endif
