/******************************************************************************
* Project Name		: Bootloadable Blinking LED
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4245AXI-483
* Software Used		: PSoC Creator 3.0
* Compiler    		: ARMGCC 4.7.3, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-049-42xx PSoC 4 Prototyping Kit 
*
********************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/******************************************************************************
*                           THEORY OF OPERATION
* This is a blinking LED project. A PWM component drives the pin to blink the 
* LED at regular intervals. This project contains a bootloadable component so 
* that it can be bootloaded into PSoC 4 which has a bootloader already programmed 
* into it.
* Default UART Port Configuration for bootloading the PSoC 4 in CY8CKIT-049-42xx
* Baud Rate : 115200 bps
* Data Bits : 8
* Stop Bits : 1
* Parity    : None
******************************************************************************/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include <device.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#include <effects.h>

#define	DEFAULT_TIMEOUT	( 50 )

// switch direction of leds
//#define REVERSE_DIRECTION ( 1 )


/***************************************
 * Global variables
 ***************************************/

extern const uint32 StripLights_CLUT[ ];
extern uint32  StripLights_ledArray[StripLights_ROWS][StripLights_COLUMNS];

/***************************************
 * Functions
 ***************************************/

void echo_uart(void);
void run_server(void);

/**
 * @brief Helper function to call routines one at time in standalone mode
 *
 * @param ch select which routine
 *
 * @return none
 */

void Select( unsigned char ch )
{
			switch (ch ) {
				case 'a':
					Icicle(6,9 , 6);
					break;
				case 'b':
					Sparkler( 100,1,8 ,0);
					break;
				
				case 'c':
					Sparkler(100,1,8 ,1);
					break;
				
				case 'd':
					StripLights_DisplayClear( StripLights_LTGREEN );
					break;
				case 'e':
					StripLights_DisplayClear( StripLights_WHITE );
					break;
				case 'f':
					StripLights_DisplayClear( StripLights_BLUE );
					break;
				case 'g':
					SingleLEDPingPong(1 ,6, getColor(rand()%StripLights_COLOR_WHEEL_SIZE));
					break;
				case 'h':
					Tween1();
					break;
				case 'i':
					ColorFader(1, getColor(rand()%StripLights_COLOR_WHEEL_SIZE));
					break;
				case 'j':
					CandyCane(5, StripLights_RED, StripLights_WHITE );
					break;
				case 'k':
					Snake(5);
					break;
				case 'l':
					ColorWheel(10);
					break;
				case 'm':
					CandyCane(5, getColor(rand()%StripLights_COLOR_WHEEL_SIZE), StripLights_WHITE );
					break;
					
				case 'n':
					CandyCaneSmooth(5, (led_color)getColor(rand()%StripLights_COLOR_WHEEL_SIZE), 
									(led_color)getColor(rand()%StripLights_COLOR_WHEEL_SIZE) );
					break;

				case '0':
					CySoftwareReset();
					break;
			}
}

/**
 * @brief Init striplights and uarts, call server or echo
 *
 * @return never returns
 */

int main()
{

	// striplights component init
    StripLights_Start();
	
	/* Start UART component and clear the TX,RX buffers */
	UART_Start();
	UART_SpiUartClearRxBuffer();

	// Switch on ESP8266's PD function
	CH_PD_SetDriveMode(CH_PD_DM_STRONG ) ;
	CH_PD_Write(1);

	// start UART for esp wifi 
	uWIFI_Start();
	uWIFI_SpiUartClearRxBuffer();
    
	CyGlobalIntEnable;  /* Un/comment this line to dis/enable global interrupts. */

	// LED output
	P1_6_SetDriveMode(P1_6_DM_STRONG ) ;
	
	// LED on.
	P1_6_Write(1);
	
	//cycle each of the primaries on all LED's, DisplayClear sets and pushes to LED's, MemoryClear just clears the buffer.
	StripLights_DisplayClear( StripLights_RED_MASK );	CyDelay( 500 ); 
	StripLights_DisplayClear( StripLights_GREEN_MASK );	CyDelay( 500 ); 
	StripLights_DisplayClear( StripLights_BLUE_MASK );	CyDelay( 500 ); 

	// Set to off
	StripLights_DisplayClear( 0 );

	// uncomment for echo back, useful to setup ESP8266 as well
	//echo_uart();
	
	run_server();
	
	
	//reboot on return
    CySoftwareReset();
    
    return 0;
}
/*
	AT+CIFSR
	AT+CIPMUX=1
	AT+CIPSERVER=1,40002
	AT+CIPSTO=9000
	AT+CIPMUX=0
*/

#define MAX_BUFFER		( 500 )

char rx_buffer[ MAX_BUFFER ];

/**
 * @brief Send a string to uarts, possibly wait for a reply with timeout
 *
 * @param text descriptive text for command, or NULL
 * @param command command to send to ESP UART
 * @param wait string to expect on return, or NULL if none
 * @param timeout number of times to call response ( not actual timer, just counter) only used if wait parameter specified
 *
 * @return 0 on  timeout, 1 on ok or no timeout
 */


uint8_t send_command( const char *const text, const char *const command, const char *const wait, uint16_t timeout) 
{
	int length;

	// if text passed in, send to host
	if( text ){
		UART_UartPutString( text );
	}

	// if command passed in, send to host and wifi uarts
	
	if( command) { 
		UART_UartPutString( command);
		uWIFI_UartPutString( command);
	}
	
	
	// if wait for reply text, supplied, scan buffer for command
	if ( wait ) {
	
		while( 1 ) {
			
			// attempt to get a string (needs timeout)
			length = at_getstr(rx_buffer,MAX_BUFFER,50);
			
			timeout -- ;
			
			// return 0 on timed out
			if( timeout == 0 ) {
				UART_UartPutString("\nTimed out waiting for correct reply\n");
				return 0;
			}
			// got a return
			if( length ) {
				
				if ( 1 ) {
					
					UART_UartPutString( "buff = [\n");
					UART_UartPutString( rx_buffer);		
					UART_UartPutString( "]\n");
				}
				
				// look for string
				if(strstr( rx_buffer, wait ) != NULL ) {
					UART_UartPutString("\nfound - [");
					UART_UartPutString( rx_buffer ) ;
					// found
					break;
				}
			} else {
				
				if ( 0 ) {
					UART_UartPutString("\nEmpty buffer\n");
				}
			}
		}
	}
	
	while( uWIFI_SpiUartGetRxBufferSize() ) {
		
		UART_UartPutChar( uWIFI_UartGetChar() );	
	}
	
	
	while( uWIFI_SpiUartGetRxBufferSize() ) {
		
		UART_UartPutChar( uWIFI_UartGetChar() );
	}
	
	return 1;
	
}

/**
 * @brief Sets length LEDs to colour from
 *
 * @param from colour to use 
 * @param length number of LEDS to set
 *
 * @return none
 */
void StripLights_SetXToColour ( uint32 from, unsigned short length ) 
{
	int x;
	
	length = MIN(StripLights_MAX_X,length);
	
	 for(x = StripLights_MIN_X; x <= length ; x++) {
		StripLights_Pixel(x,0,from);
	}
	
	StripLights_Trigger(1);
	
}
/**
 * @brief Does all the work of getting data, simple parse and sending to LED's
 *
 * @return none
 */
void run_server(void)
{
	uint8_t *buf_ptr;
	
	uint8_t ret = 0;
	
	//Set to black
	StripLights_DisplayClear(0);
	
	// LED off
	P1_6_Write(0);

	// this caused all sorts of issues, so i removed it
#if 0
	while( ret == 0 ) {
		
		ret = send_command("resetting\r\n", "AT+RST\r\n\n","ready",5000); //.com on later firmware, ready on others
	}
	P1_6_Write(0);
#endif
	

	// Simple progress meter
	StripLights_SetXToColour( getColor(1) ,1  );	
		
	// returns "no change" , 1 CONNECT TO AP, 2 BE AN AP, 3 BOTH
	send_command("cwmode=3\r\n", "AT+CWMODE=1\r\n",NULL,DEFAULT_TIMEOUT);

	// Simple progress meter, stage 2
	StripLights_SetXToColour( getColor(1) ,5  );		
	
	do {
		// LED On
		P1_6_Write(1);
		
		// Not really used, can be used to see if already connected
		send_command("get ip\r\n","AT+CIFSR\r\n",NULL,0);
		CyDelay(400);
		
		// wireless AP settings, first param is ap name, second password
		ret =send_command("connecting\r\n","AT+CWJAP=\"monkeysee\",\"monkeydo\"\r\n","OK",1000);

		// LED Off		
		P1_6_Write(0);

	}while( ret == 0 );

	// progress meter, stage 3
	StripLights_SetXToColour( getColor(1) ,10  );	


	do {
		CyDelay(400);
		ret= send_command("check connection\r\n","AT+CWJAP?\r\n","OK",DEFAULT_TIMEOUT); 
	} while( ret == 0 );

	// progress meter, stage 4
	StripLights_SetXToColour( getColor(1) ,15  );	
	
	//GET LOCAL IP ADDRESS
	do {
		CyDelay(400);
		ret= send_command("get ip\r\n","AT+CIFSR\r\n",NULL,0); 
	} while( ret == 0 );

	// progress meter, stage 5
	StripLights_SetXToColour( getColor(1) ,20  );	
	
	//START UP MULTI-IP CONNECTION
	// 	0 Single IP connection
	// 	1 Multi IP connection
	do {
		CyDelay(400);
		ret= send_command("multip\r\n","AT+CIPMUX=1\r\n","OK",DEFAULT_TIMEOUT);
	} while( ret == 0 );

	// progress meter, stage 6
	StripLights_SetXToColour( getColor(1) ,25  );	

	do {
		CyDelay(400);
		ret= send_command("cipserver\r\n","AT+CIPSERVER=1,40002\r\n","OK",DEFAULT_TIMEOUT);
	} while( ret == 0 );
	
	// progress meter, stage 7
	StripLights_SetXToColour( getColor(1) ,30  );	
	
	// switch into UDP listen/receive mode, all data passed in will be of +IDT,0,length:data format

	do {
		CyDelay(400);
		ret= send_command("cipsto\r\n","AT+CIPSTO=9000\r\n","OK",DEFAULT_TIMEOUT);
	} while( ret == 0 );

	// progress meter, stage 8
	StripLights_SetXToColour( getColor(1) ,45  );	

	do {
		CyDelay(400);
		ret= send_command("cipmux\r\n","AT+CIPMUX=0\r\n","OK",DEFAULT_TIMEOUT);
	} while( ret == 0 );

		
	// progress meter, stage 9
	StripLights_SetXToColour( getColor(1) ,50  );	

		
	// setup done, tell host (if connected)
	UART_UartPutString("\nSetup and ready!\n");
	
	// progress meter, stage 10, done
	StripLights_SetXToColour( getColor(2) ,StripLights_MAX_X );	

	CyDelay(200);

	// all off
	StripLights_DisplayClear(0);

	while(1) { 
		int i ;
		uint8_t ch;
		
		// if switch is help, run into bootloader , mostly for dev
		BOOT_CHECK();

		//led off
		P1_6_Write(0);

		// wait for data from ESP UART
		while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
		
		
		// fetch one byte of data
		ch = uWIFI_UartGetChar();

		
		// find start of +IPD,0,450:
		if( ch == '+' ) {

			//wait, this could be set to < 4 instead and then can drop the other checks.
			while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
			
			ch = uWIFI_UartGetChar();
			
			if( ch == 'I' ) {
				
				while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
				ch = uWIFI_UartGetChar();			
			
				if( ch == 'P' ) {	
					
					while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
					ch = uWIFI_UartGetChar();
					
					if( ch == 'D' ) {	
						
						while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
						ch = uWIFI_UartGetChar();

						//UART_UartPutString("Found +IPD\n");
						
						// illformatted
						if( ch != ',' )  {
							UART_UartPutString("Unexpected char #1\n");
							break;
						}
						
//led on
						P1_6_Write(1);
						
						// scan for end of descriptive
						// 10 will be enough 0,450:
						i = 10 ;
						do {
							while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
							ch = uWIFI_UartGetChar();
							i--;
							if( i ==0 ) {
								UART_UartPutString("couldn't find : marker\n");
								break;
							}								
						} while( ch != ':' );
						
						//UART_UartPutString("Found Start of data block\n");

						// point to start or end of LED buffer
#if defined(REVERSE_DIRECTION)	
						buf_ptr = (uint8_t*)&StripLights_ledArray[0][StripLights_MAX_X-1];
						for( i = 0 ; i <= StripLights_MAX_X ; i++  ) {
#else
						buf_ptr = (uint8_t*)&StripLights_ledArray[0][0];
						for( i = 0 ; i <= StripLights_MAX_X ; i++  ) {
#endif

// fill in rx_buffer from ESP UART
						//gbr
	
							while ( uWIFI_SpiUartGetRxBufferSize() < 3 );
	
							// 0 = green
							// 1 = red
							// 2 = blue
							
							buf_ptr[1] = uWIFI_UartGetChar();
							buf_ptr[0] = uWIFI_UartGetChar();	
							buf_ptr[2] = uWIFI_UartGetChar();
							
#if defined(REVERSE_DIRECTION)
							buf_ptr -= sizeof(uint32_t);
#else
							buf_ptr += sizeof(uint32_t);
#endif
						}


						//end of buffer
						while ( uWIFI_SpiUartGetRxBufferSize() == 0 );
						ch = uWIFI_UartGetChar();
						
						// check this char for sanity if wanted
						/*
						UART_UartPutChar( ch) ;
						UART_UartPutString(" - Buffer filled\n");
						UART_UartPutString( rx_buffer );
						UART_UartPutString("END\n");
						*/

						//send to LED strip
		   				while( StripLights_Ready() == 0);
						StripLights_Trigger(1);
						
						//CyDelay(4);
						BOOT_CHECK();		
					}
				}
			}
		}
	}
}
	
/**
 * @brief Just echo across UARTs
 *
 *
 * @return none
 */
void echo_uart(void)
{
	while(1) {
		
		int ret;
		
		BOOT_CHECK();
	
		ret = 0 ;
		
		while( ret == 0 ) {
		
			ret = send_command("resetting\r\n", "AT+RST\r\n\n","ready",5000); //.com on later firmware, ready on others
		}

			
		// echo from usb uart to wifi uart
		if( UART_SpiUartGetRxBufferSize() ) {
			
			uWIFI_UartPutChar( UART_UartGetChar() );
		}
		
		
		//echo from wifi uart to usb uart
		if( uWIFI_SpiUartGetRxBufferSize() ) {
			
			UART_UartPutChar( uWIFI_UartGetChar() );
		}
	}
}

// various simple effects  (not used, sending PC does work) could be offline mode

void ColorFader( int count , uint32 color) 
{
	while(count--){
		FadeToColor( 0,StripLights_COLUMNS, color, 50,1 );
	}
}

void Tween1( void )
{
	hsv_color tween;
	static led_color src;
	static hsv_color result ;
	
	src.c.r = rand()%255;
    src.c.g = rand()%255;
    src.c.b = rand()%255;

	tween = rgb_to_hsv((led_color)getColor(rand()%StripLights_COLOR_WHEEL_SIZE));
	
	result.hsv = TweenerHSV(
		0,
		StripLights_COLUMNS,
		result.hsv,
		tween.hsv,
		10
		,1);
	
	// Tweener( 100,src.rgb );

	src.c.r += 5-(rand()%10);
    src.c.g += 5-(rand()%10);
    src.c.b += 5-(rand()%10);

	result.hsv = TweenerHSV(
		StripLights_COLUMNS,
		StripLights_COLUMNS,
		result.hsv,
		tween.hsv,
		10
		,-1
	);
		
}

void CandyCane ( uint16_t count , uint32 c1, uint32 c2 )
{
    int i,x;
    uint8_t flip =0;
	
    // Candy cane
	
	// loop effect for this many times
    for( i=0; i < count ; i++ )
    {   
		
        // all strip, for every other led
        for(x = StripLights_MIN_X; x <= StripLights_MAX_X; x+=2)
        {
			// if flipped. draw c1,c2 otherwise c2,c1
            if( flip ) {
                StripLights_Pixel(x, 0, c1);
                StripLights_Pixel(x+1, 0, c2);
            } else {
                StripLights_Pixel(x+1, 0, c1);
                StripLights_Pixel(x, 0, c2);
            }
        }

		// toggle flip
        flip = 1 - flip;
        
		// wait and trigger
        while( StripLights_Ready() == 0);
		StripLights_Trigger(1);
		
		// delay between transitions 
       	CyDelay( 100 );        
		
        BOOT_CHECK();
    }   
}

/*
 * FadeLED - Tween one LED to a specified colour
 *
 */
void FadeLED( uint16 i, uint32 target, int percentage)
{
		led_color trgb;

		trgb.rgb = StripLights_GetPixel(i,0);
		
		trgb.rgb = TweenC1toC2( trgb, (led_color)target, percentage ) ;

		StripLights_Pixel( i, 0, trgb.rgb );
}
		
void CandyCaneSmooth ( uint16_t count , led_color c1, led_color c2 )
{
    int i,x,percentage;
    uint8_t flip =0;
	uint32 t1,t2;
	
    // Candy cane
	if (0 ) {
		char buffer[256];
		sprintf(buffer,"c1 = %02x %02x %02x\n",c1.c.r,c1.c.g,c1.c.b);
		UART_UartPutString( buffer );
		sprintf(buffer,"c2 = %02x %02x %02x\n",c2.c.r,c2.c.g,c2.c.b);
		UART_UartPutString( buffer );
	}
	
			
	// loop effect for this many times
    for( i=0; i < count ; i++ )
    {   
		
		for( percentage = 0 ; percentage <= 100 ; percentage+=5 ) { 
			
			//  calculate target colours
			t1 = TweenC1toC2( c1, c2, percentage ) ;
			t2 = TweenC1toC2( c2, c1, percentage ) ;
						
	        // all strip, for every other led
	        for(x = StripLights_MIN_X; x <= StripLights_MAX_X; x+=2)
	        {
				// if flipped. draw c1,c2 otherwise c2,c1
	            if( flip ) {
	                StripLights_Pixel(x, 0, t1);
	                StripLights_Pixel(x+1, 0, t2);
	            } else {
	                StripLights_Pixel(x, 0, t2);
	                StripLights_Pixel(x+1, 0, t1);
	            }
	        }

			// toggle flip
	        flip = 1 - flip;
	        
			// wait and trigger
	        while( StripLights_Ready() == 0);
			StripLights_Trigger(1);
			
			
			// delay between transitions 
	       	CyDelay( 120 );        
			
	        BOOT_CHECK();
		}
    }   
}
void SingleLEDPingPong( uint16_t count , uint8 fade_amount, uint32 color) 
{
	int i,x;
	
    for( i=0; i < count ; i++ )
    {   
    	for(x = StripLights_MIN_X; x <= StripLights_MAX_X; x++)
        {
			if(fade_amount ) {
				// Fade  strip	
				FadeStrip( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
			} else { 
				StripLights_MemClear(0);
			}
		
            
            StripLights_Pixel(x, 0, color);
            
            while( StripLights_Ready() == 0);
	    	StripLights_Trigger(1);
            CyDelay( 5 );        
            BOOT_CHECK();
        }
        
        for(x = StripLights_MIN_X; x <= StripLights_MAX_X; x++)
        {
			if(fade_amount ) {
				// Fade  strip	
				FadeStrip( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
			} else { 
				StripLights_MemClear(0);
			}
			
            StripLights_Pixel(StripLights_MAX_X-x, 0, color);
            
            while( StripLights_Ready() == 0);
	    	StripLights_Trigger(1);
			
            CyDelay( 5 );        
			
            BOOT_CHECK();
        }
    }
}

// snake tail chaser
void Snake( uint16_t count )
{
	int i,x;        
    uint32 startColor;
     
	count = count ;
	
    startColor = StripLights_RED;

    for(x = StripLights_MIN_X+1; x <= StripLights_MAX_X; x++)
    {            
        
        if( x & 6)
	        for(i = StripLights_MIN_X; i <= StripLights_MAX_X; i++)
	        {            
	            uint32_t colour = StripLights_GetPixel(i, 0);
	            StripLights_Pixel(i, 0, colour/2);
	        }        
        
        StripLights_Pixel(x, 0, startColor);
        
        while( StripLights_Ready() == 0);
    	StripLights_Trigger(1);
        CyDelay( 15 );        

        if( x % 10 == 5 ) startColor+=0x010101;
        
        BOOT_CHECK();
    }
}

void Twinkle( uint16_t count ) 
{
        int i,x;
        led_color col;
        uint32 startColor;
        startColor = StripLights_WHITE;
            
        
        for(x = 0; x <= count; x++)
        {            

			col.c.r = rand();
			col.c.g = rand();
			col.c.b = rand();

			startColor = col.rgb;
                       
			StripLights_Pixel(rand()%StripLights_MAX_X, 0, startColor);
			
            for(i = StripLights_MIN_X; i <= StripLights_MAX_X; i++)
            {            
                col.rgb = StripLights_GetPixel(i, 0);
                
                if ( col.c.r > 0 ) col.c.r -= col.c.r/2; 
                if ( col.c.g > 0 ) col.c.g -= col.c.g/2; 
                if ( col.c.b > 0 ) col.c.b -= col.c.b/2; 
                
                StripLights_Pixel(i, 0, col.rgb );
            }        
      
            while( StripLights_Ready() == 0);
	    	StripLights_Trigger(1);
            CyDelay( 15 );        
            
            BOOT_CHECK();
        }
        
}

void FadeStrip(  uint16 start, int16 length ,int percentage )
{
	led_color trgb;
	int i;
	
    for(i = start; i <= start+length; i++) {
    
		// get pixel
		trgb.rgb = StripLights_GetPixel(i,0);
		
		trgb.rgb = TweenC1toC2( trgb,(led_color) StripLights_BLACK, percentage ) ;

		StripLights_Pixel( i, 0, trgb.rgb );
	}
}

//shortcut for generate a sparkle
inline uint8 calculate_sparkle( register uint8 i ) 
{
	register uint8 rnd;

	// every so often return very dark, to make it sparkle
	if ( rand() % 20 == 10 ) {
		return 3;
	}
	
	// pick value from hole range
	rnd = ( rand() % 255 );

	// scale down by level
	rnd /= ( i + 1 ) ;
	
	// scale down again if near end
	if ( i > 4 ) rnd /= 2;
	if ( i > 6 ) rnd /= 2;
	
	return rnd;
}

void Icicle (uint8 redraw, uint8 length, int fade_amount )           
 {
    int x,j,i;
	led_color temp;

	// for entire length of strip, plus engough to move it off the display)
	for(x = StripLights_MIN_X; x <= StripLights_MAX_X + ( length * 2 ); x++)
	{
		
		if(fade_amount ) {
			// Fade  strip	
			FadeStrip( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
		} else { 
			StripLights_MemClear(0);
		}
		
		
		// draw in same place 8 times
		for ( j = 0 ; j < redraw ;j++ ){
			
			// length of icicle
			for(i=0; i < length; i++)
			{
				// caculate a randow twink based on current position in length
				temp.c.r =
				temp.c.g =
				temp.c.b = calculate_sparkle( i  );

				// draw a pixel at x+i
			    StripLights_Pixel(x+i, 0, temp.rgb );
				
				CyDelay( 1 );  
		 	}    
			    
		    // strip ready?
			while( StripLights_Ready() == 0);
			
			//push current data to led strip
		    StripLights_Trigger(1);
		    CyDelay( 3 );        
		}
		
		// check if firmware load requested
	    if( Boot_P0_7_Read ( ) == 0 )   CySoftwareReset();
	}
}



void Sparkler ( uint16 runtime, int fade_amount , int num_sparkles ,char white ) 
{
    int x,j;
	led_color temp;

	// length of time to run
	for(x = 0; x <= runtime ; x++)
	{
		if(fade_amount ) {
			// Fade  strip	
			FadeStrip( StripLights_MIN_X, StripLights_MAX_X , fade_amount );
		} else { 
			StripLights_MemClear(0);
		}
		 
		
		// draw in same place 8 times
		for ( j = 0 ; j < num_sparkles ;j++ ){
						
			temp.c.r = calculate_sparkle( j );
			
			if (white ) { 
				temp.c.g = temp.c.b = temp.c.r;
			} else {
				temp.c.g = calculate_sparkle( j );
				temp.c.b = calculate_sparkle( j );
			}
				
			// draw a pixel 
			StripLights_Pixel(rand()%StripLights_MAX_X, 0, temp.rgb );
		}
		
	    // strip ready?
		while( StripLights_Ready() == 0);
		
		//push current data to led strip
	    StripLights_Trigger(1);
	    CyDelay( 3 );     
	}
	
	if( fade_amount ) {
		// fade at end
		for(x = 0; x <= 200 ; x++)
		{
			// Fade  strip	
			FadeStrip( StripLights_MIN_X, StripLights_MAX_X , fade_amount );

			// strip ready?
			while( StripLights_Ready() == 0);

			//push current data to led strip
			StripLights_Trigger(1);
			CyDelay( 3 );     
		}
	}		
}

void ColorWheel( uint16_t count )
{
	static int i =0xAAA ,x;
	static uint32 color;
	static uint32 startColor ;   

    
	if (i >= count ) {
		i = 0;
	}
	
	if ( i == 0xaaa ) {
		i = 0;
		color = 0;
		startColor = 0;
	}
	
    for( ; i < count ; i++ )
    {   
        color = startColor;
        for(x = StripLights_MIN_X; x <= StripLights_MAX_X; x++)
        {
            StripLights_Pixel(x, 0, getColor( color ));

        	 color++;
            
			if(color >= StripLights_COLOR_WHEEL_SIZE) color = 0;
       	}
		
		startColor++;
    
		if(startColor >= StripLights_COLOR_WHEEL_SIZE) startColor = 0;
    
       while( StripLights_Ready() == 0);
    
		StripLights_Trigger(1);
	
       CyDelay( 50 );

        BOOT_CHECK();
    }
}

void Tweener( uint16_t count, uint32 from) 
{
    int i,newx;
    led_color src, target;

	src.rgb  = from;

	newx = 0;
    
    for( i = 0 ; i < count; i ++ )
    {
       //StripLights_MemClear( 0 );
            
        target.c.r += 5-(rand()%10);
        target.c.g += 5-(rand()%10);
        target.c.b += 5-(rand()%10);

        newx = TweenC1toC2Range(StripLights_COLUMNS/2,newx,src.rgb,target.rgb);
		newx = TweenC1toC2Range(StripLights_COLUMNS/2,newx,target.rgb,src.rgb);
        
       while( StripLights_Ready() == 0); 
   	   StripLights_Trigger(1); CyDelay( 50 );
    
    }
	
}


void FadeToColor( uint16_t startx, uint16_t count, uint32 target, uint32 delay, int direction)
{
    int j,i;
	int offset,oldoffset;

	led_color frgb,trgb;
    hsv_color  src,target_hsv,result;

	frgb.rgb = target;

	src = rgb_to_hsv( frgb ) ;

	offset = startx;

	for ( j = 0 ; j < 100 ; j ++ ) {
		
		oldoffset = offset;		
		
	    for( i = 0 ; i < count; i ++ ){
			
			// get colour of current LED at offset
			trgb.rgb = StripLights_GetPixel(offset,0);

			// convert current led color to hsv
			target_hsv = rgb_to_hsv( trgb );
			
			result = target_hsv;
			// tween, what we want to  what it is at percentage i
			result.h.h = TweenU8toU8( target_hsv.h.h, src.h.h, j );
			
			// update pixel
	      	StripLights_PixelHSV(offset, 0, result );
			
			// handle travel direction of pixel
			offset += direction;
			if (offset < (int)StripLights_MIN_X ) offset = startx ;
			if (offset > startx+count ) offset = StripLights_MIN_X ;
	      	
		}
		
		// check bootloader mode
        BOOT_CHECK();    
		
		// if wants a delay, update led strip and delay
		if(delay) {
   			while( StripLights_Ready() == 0);

			StripLights_Trigger(1);
			CyDelay( delay );
		}
			
		offset = oldoffset;		

	}	
}

uint32 TweenerHSV( uint16_t startx, uint16_t count, uint32 from,uint32 to,uint32 delay,int direction) 
{
    int i;
	int offset;
	led_color frgb,trgb;
    hsv_color  src, target,result;

	trgb.rgb = to;
	frgb.rgb = from;
	
	src = rgb_to_hsv( frgb ) ;
	target = rgb_to_hsv( trgb );

	result = src;
	
	offset = startx;
	
    for( i = 1 ; i < count; i ++ )
    {
        result.h.h = TweenU8toU8(src.h.h, target.h.h, i);
        
      	StripLights_PixelHSV(offset, 0, result );
		
		offset += direction;
		
		if (offset < (int)StripLights_MIN_X ) offset = startx ;
		if (offset > startx+count ) offset = StripLights_MIN_X ;
      	
        BOOT_CHECK();    
		
		if(delay) {
   			while( StripLights_Ready() == 0);

			StripLights_Trigger(1);
			CyDelay( delay );
		}
		
    }
		    
   while( StripLights_Ready() == 0);

	StripLights_Trigger(1);

   CyDelay( 5 );


	return result.hsv;
}

uint32 TweenerALLHSV( uint16_t count, uint32 from,uint32 to,uint32 delay) 
{
    int i;
	
	led_color frgb,trgb;
    hsv_color  src, target,result;

	trgb.rgb = to;
	frgb.rgb = from;
	
	src = rgb_to_hsv( frgb ) ;
	target = rgb_to_hsv( trgb );

	result = src;
	
    for( i = 1 ; i < count; i ++ )
    {
        result.h.h = TweenU8toU8(src.h.h, target.h.h, i);
        
      	StripLights_DisplayClearHSV( result );
		
		if(delay){
			CyDelay( delay );
		}
		
        BOOT_CHECK();    
    }
	return result.hsv;
}

// quick helper function for testing hsv/rgb.
void StripLights_PixelHSV(int32 x,int32 y,hsv_color hsv )
{
	led_color rgb;
	
	rgb = hsv_to_rgb( hsv ) ;
	
	StripLights_Pixel( x,y,rgb.rgb);
}
// quick helper function for testing hsv/rgb.
void StripLights_DisplayClearHSV(hsv_color hsv )
{
	led_color rgb;
	
	rgb = hsv_to_rgb( hsv ) ;
	
	StripLights_DisplayClear( rgb.rgb);
}
