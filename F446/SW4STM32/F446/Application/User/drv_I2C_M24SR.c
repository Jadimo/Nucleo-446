/**
  ******************************************************************************
  * @file    drv_I2C_M24SR.c
  * @author  MMY Application Team
  * @version V1.2.0
  * @date    20-october-2014
  * @brief   This file provides a set of functions needed to manage the I2C of
	*				   the M24SR device.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "drv_I2C_M24SR.h"

/** @addtogroup M24SR_Driver
  * @{
  */

/** @defgroup M24SR_I2C
 * 	@{
 *  @brief  This file includes the I2C driver used by M24SR family to communicate with the MCU.  
 */


/** @defgroup M24SR_I2C_Private_Functions
 *  @{
 */
extern I2C_HandleTypeDef hi2c1;

uint8_t uSynchroMode = M24SR_WAITINGTIME_POLLING;
volatile uint8_t	GPO_Low = 0;

#if ((defined USE_STM32L0XX_NUCLEO) ||(defined USE_STM32F0XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO)|| (defined USE_STM32L4XX_NUCLEO)) 
#else
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart);
#endif

#ifdef USE_STM32F4XX_NUCLEO
#define I2C_STATE_NONE            ((uint32_t)(HAL_I2C_MODE_NONE))                                                        /*!< Default Value >*/
#define I2C_NO_OPTION_FRAME       ((uint32_t)0xFFFF0000U) /*!< XferOptions default value */
#endif
/**
  * @}
  */


/** @defgroup M24SR_I2C_Public_Functions
  * @{
  */

/**
  * @brief  This function initializes the M24SR_I2C interface
	* @retval None  
  */
__weak void M24SR_I2CInit ( void )
{

}

/**
  * @brief  This function initializes the M24SR_I2C interface
	* @retval None  
  */
__weak void	M24SR_GPOInit(void)
{

}

/**
  * @brief  this functions configure I2C synchronization mode
	* @param  mode : interruption or polling
  * @retval None
  */
void M24SR_SetI2CSynchroMode( uc8 mode )
{
#if defined (I2C_GPO_SYNCHRO_ALLOWED) || defined (I2C_GPO_INTERRUPT_ALLOWED)
	uSynchroMode = mode;
#else
	if(mode == M24SR_WAITINGTIME_GPO || mode == M24SR_INTERRUPT_GPO)
		uSynchroMode = M24SR_WAITINGTIME_POLLING;
	else
		uSynchroMode = mode;
#endif /*  I2C_GPO_SYNCHRO_ALLOWED */
}

/**
  * @brief  This functions polls the I2C interface
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_PollI2C ( void )
{
	HAL_StatusTypeDef status;
	uint32_t tickstart = 0;
	uint32_t currenttick = 0;
	
  /* Get tick */
	M24SR_GetTick(&tickstart);
	
	/* Wait until M24SR is ready or timeout occurs */
  do
  {
		status = HAL_I2C_IsDeviceReady(&hi2c1, M24SR_ADDR, M24SR_I2C_POLLING, 1);
		M24SR_GetTick(&currenttick);
	} while( ( (currenttick - tickstart) < M24SR_I2C_TIMEOUT) && (status != HAL_OK) );
	
	if (status == HAL_OK)
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;


}


/**
  * @brief  This functions implements I2C token release sequence used by M24SR
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
  * @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
#if ((defined USE_STM32L0XX_NUCLEO) ||(defined USE_STM32F0XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO)|| (defined USE_STM32L4XX_NUCLEO)) 
int8_t M24SR_I2CTokenRelease( void )
{
  uint32_t tmpreg = 0;
  uint32_t Timeout = 1;
  uint32_t tickstart;
  
    /* Process Locked */
  __HAL_LOCK(&hi2c1);
  
  hi2c1.State = HAL_I2C_STATE_BUSY_TX;
  hi2c1.ErrorCode   = HAL_I2C_ERROR_NONE;
  
  /* Get the CR2 register value */
  tmpreg = hi2c1.Instance->CR2;
  
  /* clear tmpreg specific bits */
  tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
  
  /* update tmpreg */
  tmpreg |= (uint32_t)(((uint32_t)M24SR_ADDR & I2C_CR2_SADD) | \
            (uint32_t)I2C_SOFTEND_MODE | (uint32_t)I2C_GENERATE_START_WRITE);
  /* update CR2 register */
  hi2c1.Instance->CR2 = tmpreg;    
  
  M24SR_WaitMs(40);
 
  hi2c1.Instance->CR2 |= I2C_CR2_STOP;
  
  tickstart = HAL_GetTick();
  /* Wait until STOPF flag is set */
  while(__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_STOPF) == RESET)
  {

    /* Check for the Timeout */
    if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
    {
      hi2c1.ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
      hi2c1.State = HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(&hi2c1);

      return M24SR_ERROR_I2CTIMEOUT;
    }
  }
    
  /* Clear STOP Flag */
  __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_STOPF);

    /* Clear Configuration Register 2 */
  I2C_RESET_CR2(&hi2c1);

  hi2c1.State = HAL_I2C_STATE_READY; 	  

  /* Process Unlocked */
  __HAL_UNLOCK(&hi2c1);

  return M24SR_STATUS_SUCCESS;
}
#else 
int8_t M24SR_I2CTokenRelease ( void )
{ 
if(hi2c1.State == HAL_I2C_STATE_READY)
{  
  uint32_t Timeout = 1;
  uint32_t tickstart = HAL_GetTick();    
 
    /* Process Locked */
  __HAL_LOCK(&hi2c1);

#if (defined USE_STM32F4XX_NUCLEO || defined USE_STM32F1XX_NUCLEO)
    /* Disable Pos */
    hi2c1.Instance->CR1 &= ~I2C_CR1_POS;     
    hi2c1.State     = HAL_I2C_STATE_BUSY_TX;
 #else
  hi2c1.State = HAL_I2C_STATE_MASTER_BUSY_TX;  
#endif  

  hi2c1.Mode      = HAL_I2C_MODE_MASTER;
  hi2c1.ErrorCode = HAL_I2C_ERROR_NONE;    
  hi2c1.pBuffPtr    = NULL;
  hi2c1.XferCount   = 0;  
  hi2c1.XferSize   = 0;  
  
#ifdef USE_STM32F4XX_NUCLEO  
  hi2c1.XferOptions = I2C_NO_OPTION_FRAME;    
#endif
  
  /* Generate Start */
  hi2c1.Instance->CR1 |= I2C_CR1_START;

  /* Wait until SB flag is set */
  if(I2C_WaitOnFlagUntilTimeout(&hi2c1, I2C_FLAG_SB, RESET, Timeout, tickstart) != HAL_OK)
  {
    return HAL_TIMEOUT;
  }
  
  hi2c1.Instance->DR = I2C_7BIT_ADD_WRITE(M24SR_ADDR);
    
  M24SR_WaitMs(40);
  
  /* Clear ADDR flag */
  __HAL_I2C_CLEAR_ADDRFLAG(&hi2c1);    

  /* Generate Stop */
  SET_BIT(hi2c1.Instance->CR1,I2C_CR1_STOP);  

  hi2c1.ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c1.State = HAL_I2C_STATE_READY; 	  
  hi2c1.Mode = HAL_I2C_MODE_NONE;

  /* Process Unlocked */
  __HAL_UNLOCK(&hi2c1);
}
    return HAL_OK;
}

/**
  * @brief  This function handles I2C Communication Timeout.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for I2C module
  * @param  Flag specifies the I2C flag to check.
  * @param  Status The new Flag status (SET or RESET).
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  /* Wait until flag is set */
  while((__HAL_I2C_GET_FLAG(hi2c, Flag) ? SET : RESET) == Status) 
  {
    /* Check for the Timeout */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U)||((HAL_GetTick() - Tickstart ) > Timeout))
      {
        #ifdef USE_STM32F4XX_NUCLEO
          hi2c->PreviousState = I2C_STATE_NONE;
        #endif  
        hi2c->State= HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);
        
        return HAL_TIMEOUT;
      }
    }
  }
  
  return HAL_OK;
}
#endif  // ((defined USE_STM32L0XX_NUCLEO) ||(defined USE_STM32F0XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO)|| (defined USE_STM32L4XX_NUCLEO)) 

/**
  * @brief  This functions sends the command buffer 
	* @param  NbByte : Number of byte to send
  * @param  pBuffer : pointer to the buffer to send to the M24SR
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_SendI2Ccommand ( uint8_t NbByte , uint8_t *pBuffer )
{
	uint16_t status ;
	
	// Add a check to detect an issue
	errchk(M24SR_PollI2C ( ));
	
	if( HAL_I2C_Master_Transmit(&hi2c1, M24SR_ADDR, (uint8_t*)pBuffer,NbByte, 1) == HAL_OK)	
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;
	
Error :
  return M24SR_ERROR_I2CTIMEOUT;
}

/**
  * @brief  This functions reads a response of the M24SR device
	* @param  NbByte : Number of byte to read (shall be >= 5)
  * @param  pBuffer : Pointer on the buffer to retrieve M24SR response
  * @retval M24SR_STATUS_SUCCESS : The function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_ReceiveI2Cresponse ( uint8_t NbByte , uint8_t *pBuffer )
{
	uint16_t status ;
	
	// Before calling this function M24SR must be ready, here is a check to detect an issue
	errchk(M24SR_PollI2C ( ));
	
	if( HAL_I2C_Master_Receive(&hi2c1, M24SR_ADDR, (uint8_t*)pBuffer,NbByte, 1) == HAL_OK)
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;
	
Error :
  return M24SR_ERROR_I2CTIMEOUT;	
}



/**
  * @brief  This functions returns M24SR_STATUS_SUCCESS when a response is ready
  * @retval M24SR_STATUS_SUCCESS : a response of the M24LR is ready
	* @retval M24SR_ERROR_DEFAULT : the response of the M24LR is not ready
  */
int8_t M24SR_IsAnswerReady ( void )
{
	uint16_t status ;
  uint32_t retry = 0xFFFFF;
	uint8_t stable = 0;
	GPIO_PinState PinState;

  switch (uSynchroMode)
  {
  case M24SR_WAITINGTIME_POLLING :
    errchk(M24SR_PollI2C ( ));
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_WAITINGTIME_TIMEOUT :
    // M24SR FWI=5 => (256*16/fc)*2^5=9.6ms but M24SR ask for extended time to program up to 246Bytes.
    // can be improved by 
    M24SR_WaitMs(80);	
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_WAITINGTIME_GPO :
    /* mbd does not support interrupt for the moment with nucleo board */
    do
    {
			M24SR_GPO_ReadPin(&PinState);
      if( PinState == GPIO_PIN_RESET)
      {
        stable ++;						
      }
      retry --;						
    }
    while(stable <5 && retry>0);
    if(!retry)
      goto Error;				
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_INTERRUPT_GPO :
    /* Check if the GPIO is not already low before calling this function */
		M24SR_GPO_ReadPin(&PinState);
    if(PinState == GPIO_PIN_SET)
    {
      while (GPO_Low == 0);
    }
    GPO_Low = 0;
    return M24SR_STATUS_SUCCESS;
    
  default : 
    return M24SR_ERROR_DEFAULT;
  }
  
Error :
  return M24SR_ERROR_DEFAULT;
}

/**
  * @brief  This function enable or disable RF communication
	* @param	OnOffChoice: GPO configuration to set
  */
void M24SR_RFConfig_Hard( uc8 OnOffChoice)
{
	/* Disable RF */
	if ( OnOffChoice != 0 )
	{	
		M24SR_RFDIS_WritePin(GPIO_PIN_RESET);
	}
	else
	{	
		M24SR_RFDIS_WritePin(GPIO_PIN_SET);
	}
}

/**
  * @brief  This function retrieve current tick
  * @param	ptickstart: pointer on a variable to store current tick value
  */
__weak void M24SR_GetTick( uint32_t *ptickstart )
{
	
}

/**
  * @brief  This function wait the time given in param (in milisecond)
	* @param	time_ms: time value in milisecond
  */
__weak void M24SR_WaitMs(uint32_t time_ms)
{
	
}

/**
  * @brief  This function read the state of the M24SR GPO
	* @param	none
  * @retval GPIO_PinState : state of the M24SR GPO
  */
__weak void M24SR_GPO_ReadPin( GPIO_PinState * pPinState)
{

}

/**
  * @brief  This function set the state of the M24SR RF disable pin
	* @param	PinState: put RF disable pin of M24SR in PinState (1 or 0)
  */
__weak void M24SR_RFDIS_WritePin( GPIO_PinState PinState)
{
	
}



/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

