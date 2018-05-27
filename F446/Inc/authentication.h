/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUTHENTICATION_H
#define __AUTHENTICATION_H
/* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* Authentication table - to add a NFC user, enter his app UniqueID in " ", --*/
static const char *Authtable[] = {
		"6ee82303f53b9495",				//Sony Xperia SP
		"test0721",
		"smgs9plus",
		"sgh905f"
};

/* User Program Configuration ------------------------------------------------*/
uint16_t NFCID = 0x140;
uint16_t DOORID = 0x141;
uint8_t stoper = 5;

#endif /* __MAIN_H */
