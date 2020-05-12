/**
 *  @brief
 *      Board Support Package.
 *
 *		Forth TRUE is -1, C TRUE is 1.
 *      LEDs and switches.
 *      D0 to D15 digital port pins.
 *      No timeout (osWaitForever) for mutex ->
 *        could be problematic in real world applications.
 *  @file
 *      bsp.c
 *  @author
 *      Peter Schmid, peter@spyr.ch
 *  @date
 *      2020-03-26
 *  @remark
 *      Language: C, STM32CubeIDE GCC
 *  @copyright
 *      Peter Schmid, Switzerland
 *
 *      This project Mecrsip-Cube is free software: you can redistribute it
 *      and/or modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation, either version 3 of
 *      the License, or (at your option) any later version.
 *
 *      Mecrsip-Cube is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with Mecrsip-Cube. If not, see http://www.gnu.org/licenses/.
 */

// System include files
// ********************
#include "cmsis_os.h"
#include <stdio.h>

// Application include files
// *************************
#include "app_common.h"
#include "main.h"
#include "bsp.h"


// Private function prototypes
// ***************************

// Global Variables
// ****************

// Hardware resources
// ******************

// RTOS resources
// **************

static osMutexId_t DigitalPort_MutexID;
static const osMutexAttr_t DigitalPort_MutexAttr = {
		NULL,				// no name required
		osMutexPrioInherit,	// attr_bits
		NULL,				// memory for control block
		0U					// size for control block
};

static osMutexId_t Adc_MutexID;
static const osMutexAttr_t Adc_MutexAttr = {
		NULL,				// no name required
		osMutexPrioInherit,	// attr_bits
		NULL,				// memory for control block
		0U					// size for control block
};

static osSemaphoreId_t Adc_SemaphoreID;


// Private Variables
// *****************
ADC_ChannelConfTypeDef sConfig = {0};
extern ADC_HandleTypeDef hadc1;


// Public Functions
// ****************

/**
 *  @brief
 *      Initializes the BSP.
 *  @return
 *      None
 */
void BSP_init(void) {
	DigitalPort_MutexID = osMutexNew(&DigitalPort_MutexAttr);
	if (DigitalPort_MutexID == NULL) {
		Error_Handler();
	}
	Adc_MutexID = osMutexNew(&Adc_MutexAttr);
	if (Adc_MutexID == NULL) {
		Error_Handler();
	}
	Adc_SemaphoreID = osSemaphoreNew(1, 0, NULL);
	if (Adc_SemaphoreID == NULL) {
		Error_Handler();
	}

	// Configure Regular Channel
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

}


// only for testing purposes
void BSP_blinkThread(void *argument) {
/*
	: blink-thread  ( -- )
	  begin
	    led1@ 0= led1!   \ toggle blue LED
	    200 osDelay drop
	    switch1?
	  until
	  0 led1!
	;
*/
	while (! BSP_getSwitch1()) {
		BSP_setLED1(!BSP_getLED1());
		osDelay(200);
	}
	BSP_setLED1(FALSE);
	osThreadExit();
}


/**
 *  @brief
 *	    Sets the LED1 (blue).
 *
 *	@param[in]
 *      state    FALSE for dark LED, TRUE for bright LED.
 *  @return
 *      none
 *
 */
void BSP_setLED1(int state) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (LL_GetPackageType() == LL_UTILS_PACKAGETYPE_QFN48) {
		// QFN48 Package -> Dongle
		if (state) {
			HAL_GPIO_WritePin(LD1_DONGLE_GPIO_Port, LD1_DONGLE_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(LD1_DONGLE_GPIO_Port, LD1_DONGLE_Pin, GPIO_PIN_RESET);
		}
	} else {
		// Nucleo Board
		if (state) {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		}
	}

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *      Gets the LED1 (blue) state
 *
 *  @return
 *      FALSE for dark LED, TRUE for bright LED.
 */
int BSP_getLED1(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (LL_GetPackageType() == LL_UTILS_PACKAGETYPE_QFN48) {
		// QFN48 Package -> Dongle
		if (HAL_GPIO_ReadPin(LD1_DONGLE_GPIO_Port, LD1_DONGLE_Pin) == GPIO_PIN_SET) {
			return_value = -1;
		} else {
			return_value = FALSE;
		}
	} else {
		// Nucleo Board
		if (HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin) == GPIO_PIN_SET) {
			return_value = -1;
		} else {
			return_value = FALSE;
		}
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}


/**
 *  @brief
 *	    Sets the LED2 (green).
 *
 *	@param[in]
 *      state    FALSE for dark LED, TRUE for bright LED.
 *  @return
 *      none
 *
 */
void BSP_setLED2(int state) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (state) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	}

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *      Gets the LED2 (green) state
 *
 *  @return
 *      FALSE for dark LED, TRUE for bright LED.
 */
int BSP_getLED2(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET) {
		return_value = -1;
	} else {
		return_value = FALSE;
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}

/**
 *  @brief
 *	    Sets the LED3 (red).
 *
 *	@param[in]
 *      state    FALSE for dark LED, TRUE for bright LED.
 *  @return
 *      none
 *
 */
void BSP_setLED3(int state) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (state) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	}

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *      Gets the LED3 (red) state
 *
 *  @return
 *      FALSE for dark LED, TRUE for bright LED.
 */
int BSP_getLED3(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) == GPIO_PIN_SET) {
		return_value = -1;
	} else {
		return_value = FALSE;
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}


/**
 *  @brief
 *      Gets the switch1 state
 *
 *      No debouncing
 *  @return
 *      FALSE for open switch, TRUE for closed (pressed) switch.
 */
int BSP_getSwitch1(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (LL_GetPackageType() == LL_UTILS_PACKAGETYPE_QFN48) {
		// QFN48 Package -> Dongle
		if (HAL_GPIO_ReadPin(B1_DONGLE_GPIO_Port, B1_DONGLE_Pin) == GPIO_PIN_RESET) {
			return_value = -1;
		} else {
			return_value = FALSE;
		}
	} else {
		// Nucleo Board
		if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			return_value = -1;
		} else {
			return_value = FALSE;
		}
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}

/**
 *  @brief
 *      Gets the switch2 state
 *
 *      No debouncing
 *  @return
 *      FALSE for open switch, TRUE for closed (pressed) switch.
 */
int BSP_getSwitch2(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (LL_GetPackageType() == LL_UTILS_PACKAGETYPE_QFN48) {
		// QFN48 Package -> Dongle has no switch2
		return_value = FALSE;
	} else {
		// Nucleo Board
		if (HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin) == GPIO_PIN_RESET) {
			return_value = -1;
		} else {
			return_value = FALSE;
		}
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}

/**
 *  @brief
 *      Gets the switch3 state
 *
 *      No debouncing
 *  @return
 *      FALSE for open switch, TRUE for closed (pressed) switch.
 */
int BSP_getSwitch3(void) {
	int return_value;

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	if (LL_GetPackageType() == LL_UTILS_PACKAGETYPE_QFN48) {
		// QFN48 Package -> Dongle has no switch3
		return_value = FALSE;
	} else {
		// Nucleo Board
		if (HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin) == GPIO_PIN_RESET) {
			return_value =  -1;
		} else {
			return_value = FALSE;
		}
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}


// digital port pins D0 to D15 (Arduino numbering)
// ***********************************************

typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
} PortPin_t;

static const PortPin_t PortPin_a[16] = {
		{ D0_GPIO_Port, D0_Pin } ,
		{ D1_GPIO_Port, D1_Pin } ,
		{ D2_GPIO_Port, D2_Pin } ,
		{ D3_GPIO_Port, D3_Pin } ,
		{ D4_GPIO_Port, D4_Pin } ,
		{ D5_GPIO_Port, D5_Pin } ,
		{ D6_GPIO_Port, D6_Pin } ,
		{ D7_GPIO_Port, D7_Pin } ,
		{ D8_GPIO_Port, D9_Pin } ,
		{ D9_GPIO_Port, D9_Pin } ,
		{ D10_GPIO_Port, D10_Pin } ,
		{ D11_GPIO_Port, D11_Pin } ,
		{ D12_GPIO_Port, D12_Pin } ,
		{ D13_GPIO_Port, D13_Pin } ,
		{ D14_GPIO_Port, D14_Pin } ,
		{ D15_GPIO_Port, D15_Pin }
};

/**
 *  @brief
 *	    Sets the digital output port pins (D0 .. D15).
 *
 *	@param[in]
 *      state    lower word sets the pins.
 *  @return
 *      none
 *
 */
void BSP_setDigitalPort(int state) {
	uint8_t i;
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	for (i=0; i<16; i++) {
		HAL_GPIO_WritePin(PortPin_a[i].port, PortPin_a[i].pin, state & 0x01);
		state = state >> 1;
	}

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *	    Gets the digital output port pins (D0 .. D15).
 *
 *  @return
 *      state port pins
 *
 */
int BSP_getDigitalPort(void) {
	int i;
	int return_value = 0;
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	for (i=15; i>-1; i--) {
		return_value = return_value << 1;
		return_value |= HAL_GPIO_ReadPin(PortPin_a[i].port, PortPin_a[i].pin);
	}

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}


/**
 *  @brief
 *	    Sets the digital output port pin (D0 .. D15).
 *
 *	@param[in]
 *      pin_number    0 to 15.
 *	@param[in]
 *      state         0/1
 *  @return
 *      none
 *
 */
void BSP_setDigitalPin(int pin_number, int state) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	HAL_GPIO_WritePin(PortPin_a[pin_number].port, PortPin_a[pin_number].pin, state);

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *	    Gets the digital input port pin (D0 .. D15).
 *
 *	@param[in]
 *      pin_number    0 to 15.
 *  @return
 *      state         0/1
 *
 */
int BSP_getDigitalPin(int pin_number) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	int return_value = HAL_GPIO_ReadPin(PortPin_a[pin_number].port, PortPin_a[pin_number].pin);

	osMutexRelease(DigitalPort_MutexID);
	return return_value;
}


// analog port pins A0 to A5 (Arduino numbering)
// *********************************************
static const uint32_t AnalogPortPin_a[6] = {
		ADC_CHANNEL_1, // A0 PC0
		ADC_CHANNEL_2, // A1 PC1
		ADC_CHANNEL_6, // A2 PA1
		ADC_CHANNEL_5, // A3 PA0
		ADC_CHANNEL_4, // A4 PC3
		ADC_CHANNEL_3   // A5 PC2
};

/**
 *  @brief
 *	    Gets the analog input port pin (A0 .. A5) ADC value.
 *
 *	@param[in]
 *      pin_number    0 to 5.
 *  @return
 *      12 bit ADC value *
 */
int BSP_getAnalogPin(int pin_number) {
	int return_value;
	HAL_StatusTypeDef status;

	// only one thread is allowed to use the ADC
	osMutexAcquire(Adc_MutexID, osWaitForever);

	sConfig.Channel = AnalogPortPin_a[pin_number];
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	status = HAL_ADC_Start_IT(&hadc1);
	if (status != HAL_OK) {
		Error_Handler();
	}
	// blocked till ADC conversion is finished
	status = osSemaphoreAcquire(Adc_SemaphoreID, osWaitForever);

	return_value = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop_IT(&hadc1);

	osMutexRelease(Adc_MutexID);
	return return_value;
}


// digital port pin mode
// *********************
typedef struct {
	uint32_t mode;
	uint32_t pull;
} PortPinMode_t;

static const PortPinMode_t DigitalPortPinMode_a[6] = {
				{ GPIO_MODE_INPUT,     GPIO_NOPULL } ,   // 0 in
				{ GPIO_MODE_INPUT,     GPIO_PULLUP } ,   // 1 pullup
				{ GPIO_MODE_INPUT,     GPIO_PULLDOWN } , // 2 pulldow
				{ GPIO_MODE_OUTPUT_PP, GPIO_NOPULL } ,   // 3 pushpull
				{ GPIO_MODE_OUTPUT_OD, GPIO_NOPULL } ,   // 4 opendrain
				{ GPIO_MODE_AF_PP,     GPIO_NOPULL } ,   // 5 pwm
		};
/**
 *  @brief
 *	    Sets the digital port pin mode (D0 .. D15).
 *
 *      0 in, 1 in pullup, 2 in pulldown, 3 out pushpull, 4 out open drain, 5 out pwm.
 *	@param[in]
 *      pin_number    0 to 15.
 *	@param[in]
 *      mode          0 to 5
 *  @return
 *      none
 *
 */
void BSP_setDigitalPinMode(int pin_number, int mode) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

    GPIO_InitStruct.Pin = PortPin_a[pin_number].pin;
    GPIO_InitStruct.Mode = DigitalPortPinMode_a[mode].mode;
    GPIO_InitStruct.Pull = DigitalPortPinMode_a[mode].pull;
    HAL_GPIO_Init(PortPin_a[pin_number].port, &GPIO_InitStruct);

	osMutexRelease(DigitalPort_MutexID);
}


/**
 *  @brief
 *	    Sets the digital output port pin (D3=3, D5=5, D6=6, D9=9, D10=10, D11=11) to a PWM value (0..1000).
 *
 *	@param[in]
 *      pin_number    D3=3, D5=5, D6=6, D9=9, D10=10, D11=11
 *	@param[in]
 *      value         0 to 1000
 *  @return
 *      none
 *
 */
void BSP_setPwmPin(int pin_number, int value) {
	// only one thread is allowed to use the digital port
	osMutexAcquire(DigitalPort_MutexID, osWaitForever);

	// D3 TIM1CH3


	osMutexRelease(DigitalPort_MutexID);
}


// Private Functions
// *****************

// Callbacks
// *********

/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hadc);

	osSemaphoreRelease(Adc_SemaphoreID);
}


/**
  * @brief  ADC error callback in non-blocking mode
  *         (ADC conversion with interruption or transfer by DMA).
  * @note   In case of error due to overrun when using ADC with DMA transfer
  *         (HAL ADC handle parameter "ErrorCode" to state "HAL_ADC_ERROR_OVR"):
  *         - Reinitialize the DMA using function "HAL_ADC_Stop_DMA()".
  *         - If needed, restart a new ADC conversion using function
  *           "HAL_ADC_Start_DMA()"
  *           (this function is also clearing overrun flag)
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hadc);

	Error_Handler();
	osSemaphoreRelease(Adc_SemaphoreID);
}



