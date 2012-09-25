/*
 *  button.cpp
 *  Timelapse+
 *
 *  Created by Elijah Parker
 *  Copyright 2012 Timelapse+
 *  Licensed under GPLv3
 *
 */
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "button.h"
#include "clock.h"
#include "hardware.h"
// The followinging are interrupt-driven keypad reading functions
//  which includes DEBOUNCE ON/OFF mechanism, and continuous pressing detection

extern Clock clock;

unsigned char PROGMEM button_pins[]={4,2,4,5,7,6};

#define B_PORT PORTD
#define B_DDR DDRD
#define B_PIN PIND
#define FB_PORT PORTE
#define FB_DDR DDRE
#define FB_PIN PINE


Button::Button()
{
   // setup interrupt-driven keypad arrays  
   // reset button arrays
   char p;
   for(char i=0; i<NUM_KEYS; i++){
     button_count[i]=0;
     button_status[i]=0;
     button_flag[i]=0;
     p = pgm_read_byte(&button_pins[i]);
     if(i < 2)
     {
       clrBit(p, FB_DDR);
       setBit(p, FB_PORT);
     }
     else
     {
       clrBit(p, B_DDR);
       setBit(p, B_PORT);
     }
   }
}

volatile void Button::poll(){
  char i, p;
  
  for(i=0; i<NUM_KEYS; i++)
  {
    p = pgm_read_byte(&button_pins[i]);
    if(i < 2) p = (getBit(p, FB_PIN) == LOW); else p = (getBit(p, B_PIN) == LOW);
    if(p)  // key is pressed 
    { 
      if(button_count[i]<DEBOUNCE_MAX)
      {
        button_count[i]++;
        if(button_count[i]>DEBOUNCE_ON)
        {
          if(button_status[i] == 0)
          {
            button_flag[i] = 1;
            button_status[i] = 1; //button debounced to 'pressed' status
            clock.awake(); // keep from sleeping since a button was pressed
          }
		  
        }
      }
	
    }
    else // no button pressed
    {
      if (button_count[i] >0)
      {  
      	button_flag[i] = 0;	
	     	button_count[i]--;
        if(button_count[i]<DEBOUNCE_OFF){
          button_status[i]=0;   //button debounced to 'released' status
        }
      }
    }
    
  }
}

// returnes key pressed and removes it from the buffer //
char Button::get()
{
  char key, i;
  if(clock.slept()) flushBuffer();
  for(i=0; i<NUM_KEYS; i++)
  {
    if(button_flag[i] !=0)
    {
      button_flag[i]=0;  // reset button flag
      key=++i;
      break;
    }
    else
    {
      key = 0;
    }
  }
  return key;
}

// returns key pressed and does not remove it from the buffer //
char Button::pressed()
{
  char key, i;
  if(clock.slept()) flushBuffer();
  for(i=0; i<NUM_KEYS; i++)
  {
    if(button_flag[i] !=0)
    {
      key=++i;
      break;
    }
    else
    {
      key = 0;
    }
  }
  return key;
}

char Button::waitfor(char key)
{
  while(get() != key) wdt_reset();
  return key;
}

void Button::flushBuffer()
{
  char i;
  for(i=0; i<NUM_KEYS; i++)
  {
     button_count[i]=0;
     button_status[i]=0;
     button_flag[i]=0;
  }
}

