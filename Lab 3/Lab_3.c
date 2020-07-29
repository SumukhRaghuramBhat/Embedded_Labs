/*
    ECGR 4101/5101, Fall 2018: Lab 3
    Avadhut Naik (801045233)

        Utilizing TI's TIVA TM4C, the LED bar, consisting of 10 LED's, on the perfboard is controlled by the Potentiometer and the
    Photo-Resistor. With both providing analog voltages as outputs, the same is utilized  by the 12 bit ADC's of the TIVA TM4C
    board to generate their digital representation, the ranges of which are represented by LED's over the LED bar. While the
    potentiometer is connected to the Analog Input Pin 11 (AIN11) (PB5) of the board, the Photo-Resistor is connected to the
    first pin on Port B i.e. PB0. With PB0 not being an analog input pin, the first pin of Port D i.e. PD0 i.e. Analog Input
    Pin 7 (AIN7) is utilized as a Input pin for Photo-Resistor. The pins PB0 and PD0 are connected by means of a jumper wire.
    With the LED bar initially representing Potentiometer voltages, the input switches to the Photo-Resistor whence the switch SW2
    is pressed. While the potentiometer utilizes ADC 0 of the TM4C board with Sequencer SS1, the Photo-Resistor utilizes ADC 1 of
    the board with Sequencer SS1. Additionally, with the switch SW2 connected to the first pin of Port F i.e. PF0 and the latter
    being locked (by default) due to it being an NMI Input, the same has been unlocked for utilizing switch SW2 to toggle inputs to
    the LED bar.
*/

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"

int main (void)
{
    //Array for storing data collected from FIFO of ADC's//
    volatile uint32_t ui32ADC0Value[4];

    volatile uint32_t ui32ADC_values;
    volatile unsigned int count = 2;

    //Set up the System Clock to run at 40 MHz.//
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    //Enable System Clock for peripherals (Port A, Port B, Port E) and configure the GPIO pins of the ports as outputs //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2|GPIO_PIN_4);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_5);

    //Utilize Direct Register Programming to unlock the GPIOLOCK for Pin 0 of Port F i.e. PF0 //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;

    //Set Pin 0 of Port F  i.e. PF0 as Input //
    GPIODirModeSet (GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);

    //Configure the internal Pull-up Resistors of Pin PF0 to utilize switch SW2//
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    while (1)
    {
        if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0) == 0x00) // if Switch SW2 is pressed //
        {
            GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0x00);
            GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2|GPIO_PIN_4, 0x00);
            GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0|GPIO_PIN_5, 0x00);
            count += 1;
        }
        if (count % 2 == 0)
        {
            //Enable the ADC 0 peripheral //
            SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

            //Configure the ADC Sequencer to utilize Sequencer 1 with highest Priority with Processor triggering the Sequence //
            ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

            //Enable GPIO for ADC 0 module. Set PB5 i.e. AIN11 to be analog input pin for ADC 0 //
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
            GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

            //Configure the Sequencer 1 of ADC 0 module to collect samples from Channel 11 i.e. AIN 11 //
            ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH11);
            ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH11);
            ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH11);

            //Set the Interrupt flag when the last sample is done and inform ADC logic of following being the last sample on Sequencer 1//
            ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH11|ADC_CTL_IE|ADC_CTL_END);

            //Enable ADC Sequencer 1 //
            ADCSequenceEnable(ADC0_BASE, 1);

            //Clear the ADC interrupt Status Flag //
            ADCIntClear(ADC0_BASE, 1);

            //Trigger ADC Conversion //
            ADCProcessorTrigger(ADC0_BASE, 1);
            while(!ADCIntStatus(ADC0_BASE, 1, false))
            {
            }

            //Store the data from Sequencer Output FIFO into the array //
            ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

            ui32ADC_values = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3]);

            if (ui32ADC_values <= 196)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2|GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5, 0x00);
            } else if (ui32ADC_values > 196 && ui32ADC_values < 546)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5, 0xFF);

            }else if (ui32ADC_values > 586 && ui32ADC_values < 936)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0xFF);

            }else if (ui32ADC_values > 976 && ui32ADC_values < 1326)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0xFF);

            }else if (ui32ADC_values > 1366 && ui32ADC_values < 1716)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0xFF);
            }else if (ui32ADC_values > 1756 && ui32ADC_values < 2106)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0xFF);

            }else if (ui32ADC_values > 2146 && ui32ADC_values < 2496)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0xFF);

            }else if (ui32ADC_values > 2536 && ui32ADC_values < 2886)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0xFF);

            }else if (ui32ADC_values > 2926 && ui32ADC_values < 3276)
            {
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0xFF);

            }else if (ui32ADC_values > 3316 && ui32ADC_values < 3666)
            {
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0xFF);

            }else if (ui32ADC_values > 3706)
            {
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4, 0xFF);
            }
        }
        else if (count % 2 == 1)
        {
            //Enable the ADC 1 peripheral //
            SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

            //Configure the ADC Sequencer to utilize Sequencer 1 with highest Priority with Processor triggering the Sequence //
            ADCSequenceConfigure(ADC1_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

            //Enable GPIO for ADC 1 module. Set PD0 i.e. AIN7 to be analog input pin for ADC 1 //
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);

            //Configure the Sequencer 1 of ADC 1 module to collect samples from Channel 7 i.e. AIN 7 //
            ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ADC_CTL_CH7);
            ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ADC_CTL_CH7);
            ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ADC_CTL_CH7);

            //Set the Interrupt flag when the last sample is done and inform ADC logic of following being the last sample on Sequencer 1//
            ADCSequenceStepConfigure(ADC1_BASE,1,3,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);

            //Enable ADC Sequencer 1 //
            ADCSequenceEnable(ADC1_BASE, 1);

            //Clear the ADC interrupt Status Flag //
            ADCIntClear(ADC1_BASE, 1);

            //Trigger ADC Conversion //
            ADCProcessorTrigger(ADC1_BASE, 1);
            while(!ADCIntStatus(ADC1_BASE, 1, false))
            {
            }

            //Store the data from Sequencer Output FIFO into the array //
            ADCSequenceDataGet(ADC1_BASE, 1, ui32ADC0Value);

            ui32ADC_values = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3]);

            if (ui32ADC_values <= 196)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2|GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5, 0x00);
            } else if (ui32ADC_values > 196 && ui32ADC_values < 546)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5, 0xFF);

            }else if (ui32ADC_values > 586 && ui32ADC_values < 936)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0xFF);

            }else if (ui32ADC_values > 976 && ui32ADC_values < 1326)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0xFF);

            }else if (ui32ADC_values > 1366 && ui32ADC_values < 1716)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0xFF);
            }else if (ui32ADC_values > 1756 && ui32ADC_values < 2106)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0xFF);

            }else if (ui32ADC_values > 2146 && ui32ADC_values < 2496)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0xFF);

            }else if (ui32ADC_values > 2536 && ui32ADC_values < 2886)
            {
                GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0xFF);

            }else if (ui32ADC_values > 2926 && ui32ADC_values < 3276)
            {
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0xFF);

            }else if (ui32ADC_values > 3316 && ui32ADC_values < 3666)
            {
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, 0x00);
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0xFF);

            }else if (ui32ADC_values > 3706)
            {
                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0x00);
                GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4, 0xFF);
            }
        }
    }
}
