/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUTHENTICATION_H
#define __AUTHENTICATION_H
/* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* Authentication table - to add a NFC user, enter his app UniqueID in " ", --*/
static const char *Authtable[] = {
		"6ee82303f53b9495",				//Sony Xperia SP
		"72c70317c3f659e6",				//Huawei p8
		"d32ef503725485f2",				//Galaxy S6e
		"sgh905f"
};

/* User Program Configuration ------------------------------------------------*/
uint16_t NFCID = 0x140;
uint16_t DOORID = 0x141;
uint8_t stoper = 4;

#endif /* __MAIN_H */
