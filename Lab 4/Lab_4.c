/*
    ECGR 4101/5101, Fall 2018: Lab 4
   Avadhut Naik (801045233)

        Utilizing a couple of TI's TIVA TM4C, one will be configured as a UART Receiver while the other will be the Transmitter. With both utilizing the same program,
    the selection of boards as transmitter and receiver shall be undertaken through pin 7 of Port A i.e. PA7, which has been connected to the VCC and set as input.
    While the board with 0V as input on pin PA7 shall be the receiver, the other shall be transmitter which shall transmit upon switch SW2 being pressed, in accordance
    with lab requirements, the data byte 0xAA to the receiver through UART enabled and configured on pin 1 of Port B i.e. PB1. The receiver board, upon reception of
    the data byte, shall blink and toggle between its on-board red, blue and green LED's in the very sequence. To make LED blink and sequence perceivable, sufficient
    delay has been introduced. Additionally, a small delay has also been introduced to tackle the deboucing effect of the switch SW2 on receiver side. With LED blink
    ceasing upon end of delay triggered upon successful reception of the data byte, the same can be initiated again upon pressing switch SW2 of transmitter board which
    shall again initiate transmission of data byte 0xAA to the receiver through UART.
 */

#include<stdint.h>
#include<stdbool.h>
#include <math.h>
#include <time.h>
#include"inc/hw_memmap.h"
#include"inc/hw_ints.h"
#include"inc/hw_types.h"
#include "inc/hw_gpio.h"
#include"driverlib/gpio.h"
#include"driverlib/pin_map.h"
#include"driverlib/sysctl.h"
#include"driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#define GPIO_PB1_U1TX 0x00010401    // UART PIN ADDRESS FOR UART TX
#define GPIO_PB0_U1RX 0x00010001    // UART PIN ADDRESS FOR UART RX

int main (void)
{
    //Variable for selecting board type (transmitter or receiver)//
    volatile unsigned int function_select = 0;
    //Variable to data received through UART//
    volatile unsigned int received_value = 0;
    uint32_t ui32Status;
    volatile short count = 0;

    //Set System Clock at 40 MHz//
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //Enable UART 1 Peripheral//
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // CONFIGURE UART, BAUD RATE 9600, DATA BITS 8, STOP BIT 1, PARITY NONE//
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTEnable(UART1_BASE);

    //Enable GPIO Port B for UART Transmission and Reception//
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_1);   //Pin 1 of Port B i.e. PB1 is configured as UART Transmitter//

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0);   //Pin 0 of Port B i.e. PB0 is configured as UART Receiver//

    //Enable System Clock for peripherals for Port F and configure the GPIO pins 1, 2 and 3 i.e. PF1, PF2, PF3 of the ports as outputs to make LED's blink//
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3);

    //Utilize Direct Register Programming to unlock the GPIOLOCK for Pin 0 of Port F i.e. PF0 //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;

    //Set Pin 0 of Port F  i.e. PF0 as Input //
    GPIODirModeSet (GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);
    //Configure the internal Pull-up Resistors of Pin PF0 to utilize switch SW2//
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //Enable System Clock for peripheral Port A and configure the GPIO pin 7 i.e. PA7 of the port A as input for selecting transmitter and receiver boards //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIODirModeSet (GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
    function_select = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7);

    while (1)
    {
        if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0) == 0x00)
        {
            count ++;
        }
        ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status
        UARTIntClear(UART1_BASE, ui32Status);

        if (function_select != 0)
        {
            if (count % 2 == 1)
            {
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0xFF);
                //Enable UART 1 Peripheral//
                SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
                SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
                // CONFIGURE UART, BAUD RATE 9600, DATA BITS 8, STOP BIT 1, PARITY NONE//
                UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
                UARTEnable(UART1_BASE);
                //Enable GPIO Port B for UART Transmission//
                GPIOPinConfigure(GPIO_PB1_U1TX);
                GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_1);   //Pin 1 of Port B i.e. PB1 is configured as UART Transmitter//
                //Send the data byte 0xAA to receiver board//
                UARTCharPut(UART1_BASE, 0xAA);

                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0x00);
                count ++;
            }
            else if (count > 0 && count%2 == 0)
            {
                UARTDisable(UART1_BASE);
            }
        }
        else
        {
            //Delay Variable//
            volatile unsigned long i = 100000;
            //Store the values received over UART into the variable//
            received_value = UARTCharGet(UART1_BASE);
            //Delay Introduced to handle Debouncing effect//
            while (i > 0)
            {
                i--;
            }
            i = 1000000;
            if (received_value == 0xAA)
            {
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0x00);
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0xFF);
                while (i > 0)
                {
                    i --;
                }
                i = 1000000;
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0x00);
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0xFF);
                while (i > 0)
                {
                    i --;
                }
                i = 1000000;
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0xFF);
                while (i > 0)
                {
                    i --;
                }
                i = 1000000;
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0x00);
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0x00);
                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0x00);
            }
        }
    }
    return 0;
}
