/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <project.h>
#include <stdlib.h>
#include <device.h>

#include <effects.h>

/* Byte to Byte time out in ms for detecting end of block data from host*/
#define BYTE2BYTE_TIME_OUT 2

#define BTLDR_SIZEOF_READ_BUFFER 0xff
#define BTLDR_SIZEOF_WRITE_BUFFER 0xff
#define END_OF_PACKET 0x17

/* UART receive data count variable  */
volatile uint16 receivedDataCount=0;
    
/*******************************************************************************
* Function Name: CyBtldrCommRead
********************************************************************************
*
* Summary:
*  Receives the command. 
*
* Parameters:  
*  pData:    A pointer to the area to store the block of data received
*             from the device.
*  size:     Maximum size of the read buffer
*  count:    Pointer to an unsigned short variable to write the number
*             of bytes actually read.
*  timeOut:  Number of units to wait before returning because of a timeOut.
*            Timeout is measured in 10s of ms.
*
* Return: 
*  cystatus: This function will return CYRET_SUCCESS if at least one byte is received
*			 successfully within the timeout interval. If no data is received this 
*			 function will return CYRET_EMPTY.
*
* Theory: 
*  'receivedDataCount' is updated with number of bytes received in the UART RX 
*  interrupt routine. This variable is used to check whether some data is received 
*  within the timeout period specified in *.cydwr. If data is received before the timeout, 
*  the control will remain in another loop waiting for more data until no data is 
*  received for a BYTE2BYTE_TIME_OUT(2 ms) interval.
*
*  Note: Increase the BYTE2BYTE_TIME_OUT to 10 ms for baud rates less than 9600.  
* 
*  BYTE2BYTE_TIME_OUT is used for detecting timeout marking end of block data from host. 
*  This has to be set to a value which is greater than the expected maximum delay 
*  between two bytes during a block/packet transmission from the host. 
*  You have to account for the delay in hardware converters while calculating this value,
*  if you are using any USB-UART bridges.   
*******************************************************************************/
cystatus CyBtldrCommRead(uint8 * pData, uint16 Size, uint16 * Count, uint8 TimeOut)
{
    uint16 cntr,dataIndexCntr;
    uint16 tempCount,oldDataCount;
	
    cystatus status = CYRET_EMPTY;

    /* Check whether data is received within the timeout period. 
	*  Timeout period is in units of 10ms.
	*  If at least one byte is received within the timeout interval, wait for more data */
	for (cntr = 0; cntr < TimeOut*10; cntr++)
    {
	    receivedDataCount = UART_SpiUartGetRxBufferSize();
		
		/* If at least one byte is received within the timeout interval enter the next loop
		* waiting for more data reception */
		if(receivedDataCount!=0) 
	   	{
			/* Wait for more data until 2 ms byte to byte time out interval receivedDataCount 
			* variable is updated in on each data reception. If no data is received during the 
			* last 2 ms (BYTE2BYTE_TIME_OUT) then it is considered as end of transmitted data 
			* block (packet) from the host and the program execution will break from the data 
			* awaiting loop with status=CYRET_SUCCESS */
			do{
			   	oldDataCount = receivedDataCount;
				CyDelay(BYTE2BYTE_TIME_OUT);
				receivedDataCount = UART_SpiUartGetRxBufferSize();			    
			}while(receivedDataCount > oldDataCount);
			status = CYRET_SUCCESS;	
			break;
		}
		/* If no data is received, give a delay of 1ms and check again until the Timeout specified in .cydwr. */
		else 
		{
			CyDelay(1);
		}
    }
	
	/* Initialize the data read indexes and Count value*/
	*Count = 0;
	dataIndexCntr = 0;
	
	/* If receivedDataCount>0 , move the received data to the pData buffer */
	while(receivedDataCount > 0)
	{
		tempCount=receivedDataCount;
		*Count  =(*Count ) + tempCount;
		
		/* Check if buffer overflow will occur before moving the data */
		if(*Count < Size)
		{
			for (cntr = 0;((cntr < tempCount) ); cntr++)
			{
				/* Read the data and move it to the pData buffer */
				pData[dataIndexCntr++] = UART_SpiUartReadRxData();   
			}
			/* Disable the interrupts before updating the receivedDataCount and 
			*  re-enable the interrupts after updating */
			CyGlobalIntDisable;
			
			/* subtract the read data count from received data count */
			receivedDataCount=receivedDataCount-tempCount;
			
			CyGlobalIntEnable;
			
			/* Check if the last data received is End of packet(0x17) 
			*  If not wait for additional 5ms */
			if(pData[dataIndexCntr-1]!= END_OF_PACKET)
			    CyDelay(5);
	 	}	
		
		/* If there is no space to move data, break from the loop */
		else
		{
			*Count=(*Count)-tempCount;
			UART_SpiUartClearRxBuffer();
			status = CYRET_EMPTY;
			break;
		}
	}
	return status;
}
/**
 * @brief Gets a string from the ESP UART
 *
 * @param buffer read data into here
 * @param length length of buffer
 * @param timeout countdown timer to fail after  (Approx 5 * timeout)
 *
 * @return length of string
 */
uint16_t at_getstr( char *const buffer ,uint16_t length,uint16_t timeout)
{
	int it = 0;
	
	if( buffer == NULL || length == 0  )
		return 0;
	
	memset(buffer,0,length);
	
	do{
		// fetch from uWIFI		
		while ( uWIFI_SpiUartGetRxBufferSize() == 0 ) {
			
			timeout--;
			
			if( timeout == 0 )  {
				
				//debugging
				if( 0 ) {
					UART_UartPutString("at_getstr timedout\n");
				}
				
				return 0;
			}
			
			CyDelay( 5 );
		}
			
			buffer[it]=uWIFI_UartGetChar();
			
			it++;
			
			// max
			if( it == length ){
				return it;
			}
			
	}while (buffer[it-1] != '\n');
	
	return it;
}

uint8_t at_parse(const char* data, const char* data_expected)
{
	
	int it=0;
	int cont=0;
	int cont2=0;
	int ok;
	
	while ( data[it] != '\0' )
	{
		it++;
	}

	while (cont < it)	
	{
		if (data[cont] == data_expected[cont2])
		{
			
			cont2=0;
			
			while (data_expected[cont2] != '\0')
			{
				if (data[cont+cont2] == data_expected[cont2])
				{    
					cont2++;
					ok=1; 
				}
				else
				{
					ok=0; 
					break;
				}
			}  
			
			if( ok ) {
				return 1;
			} 
		}
		cont++;
		cont2=0;
	}	
	return 0;
}
/* 
sample packet
+IPD,0,450:jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj
OK
*/

/* [] END OF FILE */
