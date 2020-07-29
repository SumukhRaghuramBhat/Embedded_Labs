/*
 *  ECGR 4101/5101, Fall 2018: Lab 2
    Avadhut Naik (801045233)

         Utilizing TIMSP430 , the LED bar, consisting of 10 LED's, on the perfboard is controlled by the Potentiometer and Photo-Resistor.
    With both providing analog voltages as outputs, the same is utilized  by the 10 bit ADC of the TIMSP430 board to generate their
    digital representation, the ranges of which are represented by LED's over the LED bar. While the potentiometer is connected to the
    Analog Input Pin 0 (A0) of the board, the Photo_Resistor is connected to the Analog Input Pin 1 (A1) of the board. With the LED bar
    initially representing Potentiometer voltages, the input switches to the Photo_resistor whence the switch S2 is pressed.
 */

#include <msp430.h> 

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop Watchdog timer
    unsigned int ADC_values = 0;
    unsigned int count = 2;
    P1DIR |= 0x30;                      //Set P1.4 and P1.5 to Output Direction
    P1REN |= 0x08;                      //Enable P1.3 pull-up Resistor Network
    P1OUT |= 0X08;
    P2SEL = ~(BIT6 + BIT7);             //Enable pins 2.6 and 2.7 (pins 19 and 18 respectively) to be used as GPIO
    P2SEL = 0x00;
    P2DIR |= 0xFF;                      //Set Port 2 to Output Direction

    while (1)
    {
        if ((~P1IN & BIT3))
        {
            count++;
        }
        if (P1IN & BIT3 && count%2 == 0)
        {
            ADC10CTL0 = SREF_0 + ADC10SHT_1 + ADC10ON + MSC; //Configure and Setup ADC Control Register
            ADC10CTL1 = INCH_0;          //Select Input (PA.0)
            ADC10AE0 |= 0x01;            //Set / Select A.0 (Pin 2) Option for ADC Input
            ADC10CTL0 |= ENC + ADC10SC;     //Initiate Sampling and Conversion
            ADC_values = ADC10MEM;          //Store ADC Values into variables to control LED's
            P1OUT &= ~0x30;
            P2OUT &= ~0xFF;
            if (ADC_values < 50)
            {
                P1OUT &= ~0x30;
                P2OUT &= ~0xFF;
            } else if (ADC_values > 80 && ADC_values < 150)
            {
                P1OUT &= ~0x20;
                P2OUT &= ~0x02;
                P2OUT |= 0x01;
            }else if (ADC_values > 160 && ADC_values < 230)
            {
                P2OUT &= ~0x01;
                P2OUT &= ~0x04;
                P2OUT |= 0x02;
            }else if (ADC_values > 240 && ADC_values < 310)
            {
                P2OUT &= ~0x02;
                P2OUT &= ~0x08;
                P2OUT |= 0x04;
            }else if (ADC_values > 320 && ADC_values < 390)
            {
                P2OUT &= ~0x04;
                P2OUT &= ~0x10;
                P2OUT |= 0x08;
            }else if (ADC_values > 400 && ADC_values < 470)
            {
                P2OUT &= ~0x08;
                P2OUT &= ~0x20;
                P2OUT |= 0x10;
            }else if (ADC_values > 475 && ADC_values < 550)
            {
                P2OUT &= ~0x10;
                P2OUT &= ~0x40;
                P2OUT |= 0x20;
            }else if (ADC_values > 560 && ADC_values < 650)
            {
                P2OUT &= ~0x20;
                P2OUT &= ~0x80;
                P2OUT |= 0x40;
            }else if (ADC_values > 660 && ADC_values < 810)
            {
                P2OUT &= ~0x40;
                P1OUT &= ~0x10;
                P2OUT |= 0x80;
            }else if (ADC_values > 820 && ADC_values < 970)
            {
                P2OUT &= ~0x80;
                P1OUT &= ~0x20;
                P1OUT |= 0x10;
            }else if (ADC_values > 970 && ADC_values <= 1024)
            {
                P1OUT &= ~0x10;
                P1OUT |= 0x20;
            }
            else
            {
                P1OUT &= ~0x30;
                P2OUT &= ~0xFF;
            }
        }
        else if (P1IN & BIT3 && count % 2 ==1)
        {
            ADC10CTL0 = SREF_0 + ADC10SHT_1 + ADC10ON + MSC;    //Configure and Setup ADC Control Register
            ADC10CTL1 = INCH_1;                                 //Select Input (PA.1)
            ADC10AE0 |= 0x02;                                   //Set / Select A.1 (Pin 3) Option for ADC Input
            ADC10CTL0 |= ENC + ADC10SC;                         //Initiate Sampling and Conversion
            ADC_values = ADC10MEM;                              //Store ADC Values into variables to control LED's
            P1OUT &= ~0x30;
            P2OUT &= ~0xFF;
            if (ADC_values < 50)
            {
                P1OUT &= ~0x30;
                P2OUT &= ~0xFF;
            } else if (ADC_values > 70 && ADC_values < 140)
            {
                P1OUT &= ~0x20;
                P2OUT &= ~0x02;
                P2OUT |= 0x01;
            }else if (ADC_values > 150 && ADC_values < 220)
            {
                P2OUT &= ~0x01;
                P2OUT &= ~0x04;
                P2OUT |= 0x02;
            }else if (ADC_values > 230 && ADC_values < 300)
            {
                P2OUT &= ~0x02;
                P2OUT &= ~0x08;
                P2OUT |= 0x04;
            }else if (ADC_values > 310 && ADC_values < 380)
            {
                P2OUT &= ~0x04;
                P2OUT &= ~0x10;
                P2OUT |= 0x08;
            }else if (ADC_values > 390 && ADC_values < 460)
            {
                P2OUT &= ~0x08;
                P2OUT &= ~0x20;
                P2OUT |= 0x10;
            }else if (ADC_values > 470 && ADC_values < 540)
            {
                P2OUT &= ~0x10;
                P2OUT &= ~0x40;
                P2OUT |= 0x20;
            }else if (ADC_values > 550 && ADC_values < 620)
            {
                P2OUT &= ~0x20;
                P2OUT &= ~0x80;
                P2OUT |= 0x40;
            }else if (ADC_values > 630 && ADC_values < 700)
            {
                P2OUT &= ~0x40;
                P1OUT &= ~0x10;
                P2OUT |= 0x80;
            }else if (ADC_values > 710 && ADC_values < 780)
            {
                P2OUT &= ~0x80;
                P1OUT &= ~0x20;
                P1OUT |= 0x10;
            }else if (ADC_values >790 && ADC_values <= 1024)
            {
                P1OUT &= ~0x10;
                P1OUT |= 0x20;
            }
            else
            {
                P1OUT &= ~0x30;
                P2OUT &= ~0xFF;
            }
        }
    }
    return 0;
}
