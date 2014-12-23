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
#include <stdio.h>
#include <stdlib.h>
#include <device.h>

#include <effects.h>

// Some colour routines , not used 
// http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both 

led_color hsv_to_rgb(hsv_color hsv)
{
    led_color rgb;
    unsigned char region, p, q, t;
    uint16_t h, s, v, remainder;

	//grey
    if (hsv.h.s == 0)
    {
        rgb.c.r = hsv.h.v;
        rgb.c.g = hsv.h.v;
        rgb.c.b = hsv.h.v;
		
        return rgb;
    }

    // converting to 16 bit to prevent overflow 
    h = hsv.h.h;
    s = hsv.h.s;
    v = hsv.h.v;

    region = h / 43;
    remainder = (h - (region * 43)) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.c.r = v;
            rgb.c.g = t;
            rgb.c.b = p;
            break;
        case 1:
            rgb.c.r = q;
            rgb.c.g = v;
            rgb.c.b = p;
            break;
        case 2:
            rgb.c.r = p;
            rgb.c.g = v;
            rgb.c.b = t;
            break;
        case 3:
            rgb.c.r = p;
            rgb.c.g = q;
            rgb.c.b = v;
            break;
        case 4:
            rgb.c.r = t;
            rgb.c.g = p;
            rgb.c.b = v;
            break;
        default:
            rgb.c.r = v;
            rgb.c.g = p;
            rgb.c.b = q;
            break;
    }

    return rgb;
}

hsv_color rgb_to_hsv(led_color rgb)
{
    hsv_color hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.c.r < rgb.c.g ? (rgb.c.r < rgb.c.b ? rgb.c.r : rgb.c.b) : (rgb.c.g < rgb.c.b ? rgb.c.g : rgb.c.b);
    rgbMax = rgb.c.r > rgb.c.g ? (rgb.c.r > rgb.c.b ? rgb.c.r : rgb.c.b) : (rgb.c.g > rgb.c.b ? rgb.c.g : rgb.c.b);

    hsv.h.v = rgbMax;
    if (hsv.h.v == 0)
    {
        hsv.h.h = 0;
        hsv.h.s = 0;
        return hsv;
    }

    hsv.h.s = 255 * ((long)(rgbMax - rgbMin)) / hsv.h.v;
    if (hsv.h.s == 0)
    {
        hsv.h.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.c.r)
        hsv.h.h = 0 + 43 * (rgb.c.g - rgb.c.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.c.g)
        hsv.h.h = 85 + 43 * (rgb.c.b - rgb.c.r) / (rgbMax - rgbMin);
    else
        hsv.h.h = 171 + 43 * (rgb.c.r - rgb.c.g) / (rgbMax - rgbMin);

    return hsv;
}

uint8 TweenU8toU8(uint8 source, uint8 target,int amount)
{
    float percent,temp;
	
	percent = (float)amount/100.0f;
    
	temp = (source + (target - source) * percent ) ;
	
	return temp;

}

uint32 TweenC1toC2(led_color c1, led_color c2,int amount)
{
    led_color result;
    float percent ,r,g,b;
    
	if ( amount < 0 ) amount = 0;
	if (amount > 100 ) amount = 100 ;
	
    percent = (float)amount/100.0f;
    
    /// mix two colours, simple linear interpolate , won't work well for all colour transitions
    r = (c1.c.r + ( c2.c.r - c1.c.r ) * percent ) ;
    g = (c1.c.g + ( c2.c.g - c1.c.g ) * percent ) ;
    b = (c1.c.b + ( c2.c.b - c1.c.b ) * percent ) ;
	
	result.c.r = r;
	result.c.g = g;
	result.c.b = b;
	
	//debugging
    if ( 0 ) {
		char buffer[1024];
		sprintf(buffer,"%d) %02x %02x %02x  %02x %02x %02x  = %02x %02x %02x\r\n\n",amount,
												c1.c.r, c1.c.g, c1.c.b, 
												c2.c.r, c2.c.g, c2.c.b, 
												result.c.r,result.c.g,result.c.b);
		UART_UartPutString( buffer );
	}      
	
    return result.rgb;
}

int TweenC1toC2Range( uint16_t count, uint16_t x, uint32 source, uint32 target)
{
    int i,tx=0;
    led_color result,c1,c2;
    float percent ,r,g,b;
    c1.rgb = source;
    c2.rgb = target;
    
    // i * diff = 100% for steps
    
    float diff = 1.0f / count ;
    
    for ( i = 0 ; i < count ; i++ ) {
        
        percent = (float)i* diff;;
        
        /// mix two colours, simple linear interpolate , won't work well for all colour transitions
        r = (c1.c.r + ( c2.c.r - c1.c.r ) * percent ) ;
        g = (c1.c.g + ( c2.c.g - c1.c.g ) * percent ) ;
        b = (c1.c.b + ( c2.c.b - c1.c.b ) * percent ) ;

		
		result.c.r = r;
		result.c.g = g;
		result.c.b = b;
		
        // set color at pixel
		tx =x +i;
		tx%= StripLights_COLUMNS;
       StripLights_Pixel( tx,0,result.rgb);
    
       BOOT_CHECK();
       
    }
    return tx;
}

uint32 AddColor( led_color c1, led_color c2 )
{	
	return TweenC1toC2(c1,c2,50);	
}


