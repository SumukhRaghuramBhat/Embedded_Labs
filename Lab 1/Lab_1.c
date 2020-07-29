/*
     ECGR 4101/5101, Fall 2018: Lab 1
     Avadhut Naik (801045233)

     Using TI MSP 430, LED 1 (RED LED) blinks at 0.25 Hz, 50% duty cycle until Switch S2 is pressed. LED 2 is OFF.
     LED 2 (GREEN LED) blinks in its aftermath at 0.25 Hz, 50% duty cycle until Switch S2 is released while LED 1 turns OFF.
     LED 1 resumes and continues to blink on switch being relased as before while LED 2 stops blinking.
 */

#include<msp430.h>

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    P1DIR |= 0x01;              // Set P1.0 to output Direction
    P1DIR |= 0x61;              //Set P1.6 to Output Direction
    P1REN |= 0x08;              //Enable P1.3 pull-up Resistor Network
    P1OUT |= 0X08;

    while(1)
    {
        volatile unsigned long a = 125000;
        if ((P1IN & BIT3))          // If button is open (P1.3 HIGH)
        {
            P1OUT = P1OUT & ~BIT6; //Turn OFF LED 2 (P1.6)
            P1OUT ^= 0x01;         //Toggle LED 1 (P1.0) ON
            while (a>0)            // Introduce Delay
                {
                    a--;
                }
        }
        else
        {
            P1OUT = P1OUT & ~BIT0;  // Turn OFF LED 1 (P1.0)
            P1OUT ^= BIT6;          //Toggle LED 2 (P1.6) ON
            while (a>0)             //Introduce Delay
            {
                a--;
            }
        }
    }
}
