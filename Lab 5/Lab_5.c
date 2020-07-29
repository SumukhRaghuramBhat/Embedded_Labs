/**************************************************************************************************************************************************************************************************************
 *      ECGR 4101/5101, Fall 2018: Lab 5 - Using an LCD and an Accelerometer
    Avadhut Naik (801045233)
 *
 *      Utilizing TI's Educational Booster Pack MKII with TIVA TM4C123GH6PM, the LCD of the former displays a ball of 3 x 3 pixels upon startup. Depending on the
 *  tilt of the board determined by the on-board accelerometer of the MKII board, the ball will move around the LCD Screen of the board when the board is tilted by more
 *  than 10 degrees along the x and / or y axis. While the (on board) accelerometer is an analog device, the values provided by it along the X and Y axes are converted into
 *  digital values by the on-board ADC (ADC0, with pins PD0 and PD1 configured as inputs for X and Y axis respectively) of the TM4C board. The direction of the ball
 *  movement is determined by these very ADC Values. When the the board is upright or tilted by more than 10 degrees along  either of the axes, the ball travels towards
 *  the lower side of the screen. When the board is tilted along both X and Y axes, the direction of travel for the ball is determined by the tilt of the board along both axes.
 *  On board being made flat again, the movement of the ball ceases and it remains in the very position. Additionally, a delay has been introduced to enforce the lab
 *  requirement of moving the ball by not more than a pixel every 50 milliseconds.
 *
 *      Certain hardware modifications (hardware connections using jumper wires) have been undertaken as a prerequisite for implementation of the Lab, the details of which
 *  are as follows:
 *  Resistors R9 and R10 upon the TM4C board have been removed (de-soldered). Also, additional connections were made using jumper wires.
 *  -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *                                                                                              Hardware Connections
 *  -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *                                                                               |-----------------------------------------------------------|
 *                                                                               |      Boosterpack Pin     |       Tiva C Pin               |
 *                                                                               |-----------------------------------------------------------|
 *                                                                               |  LCD SPI CLK (J1 7)      |        PA2                     |
 *                                                                               |  LCD RST (J2 17)         |        PA7                     |
 *                                                                               |  LCD MOSI (J2 15)        |        PA5                     |
 *                                                                               |  LCD SPI CS (J2 13)      |        PA3                     |
 *                                                                               |  LCD RS   (J4 31)        |         PA6                    |
 *                                                                               |--------------------------|--------------------------------|
 *
 *          ISSUES FACED IN IMPLEMENTATION
 *  1)  Arranging all the libraries and File Paths for 'building' the code was the first major challenge faced.
 *  2)  Though eventually resolved, getting ADC to convert Analog values on both channel (AIN 6 and AIN 7) into digital simultaneously was the next hurdle we resolved
 *  through the use of Sequencer 1, collecting accelrometer's analog values along X and Y axes through pins PD0 (AIN7) and PD1 (AIN6) respectively in an alternate fashion.
 *  3)  With ADC issue resolved, getting the LCD of the MKII board to work was the next, perhaps the most major, hurdle we overcame through additional jumper wire
 *  connections as shown in the table above.
 *  4)  With the LCD now working and ball with a diameter of 3 pixels now visible on the LCD Screen, determining its direction and initiating its movement along the X and Y
 *  axes depending upon the tilt of the board was the next hurdle. To overcome the same, we implemented five versions of the code, noting down ADC values (for both axes)
 *  upon the board being horizontal and on tilting it by more than 10 degrees along the X and Y Axes.
 *  5) Finally introducing the delay component to sate the Lab requirement was the last hurdle, again resolved through extensive experimentation.
 *
 *          COMPONENTS UTILIZED:
 *  TIVA TM4CGH6PM LaunchPad evaluation kit, Educational Booster pack MKII, Connecting Wires. Code Composer Studio
 *
 *          REFERENCES
 *  1)  Code examples from Dr. Valvano's web page:
 *  http://edx-org-utaustinx.s3.amazonaws.com/UT601x/ValvanoWareTM4C123.zip
 *  2)  Embedded System Design using TM4C LaunchPad Development Kit.
 *  3)  User's Guide: BOOSTXL-EDUMKII Educational BoosterPack Plug-in Module Mark II
 *  4)  The board (MKII) accelerometer:
 *  http://kionixfs.kionix.com/en/datasheet/KXTC9-2050%20Specifications%20Rev%202.pdf
 *  5)  TI support forum:
 *  https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/453698
 *  6)  Crystalfontz (Link Provided by Dr. Conrad):
 *  https://forum.crystalfontz.com/showthread.php/7394-Connecting-the-small-CFAF128128B-0145T-TFT-to-an-Arduino-Uno-or-SparkFun-RedBoard
 *  7)  Crystalfontz (Link for LCD Display)
 *  https://www.crystalfontz.com/product/cfaf128128b0145t-graphic-tft-128x128
 **************************************************************************************************************************************************************************************************************
 */

/**************************************************************************************************************************************************************************************************************
 *                                                                                              Header Files
 **************************************************************************************************************************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "PLL.h"
#include "ST7735.h"
#include "tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"

/**************************************************************************************************************************************************************************************************************
 *                                                                                              Global Variables
 **************************************************************************************************************************************************************************************************************
 */
//Array for storing data collected from FIFO of ADC's//
volatile uint32_t ui32ADC0Value[4];

//Variable for storing accelerometer data, converted into digital format, along X-Axis //
volatile uint32_t ui32ADC_values_x;

//Variable for storing accelerometer data, converted into digital format, along Y-Axis //
volatile uint32_t ui32ADC_values_y;

//Variables for positioning the ball along X and Y axes//
volatile double x_cordinates, y_cordinates;

//Variables to determine movement of ball in accordance with the tilt of the board//
volatile float move_x, move_y;

//Ball with a diameter of 3 pixels//
const uint16_t ball [] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                           0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 0xFFFF,
                           0xFFFF, 0x0000, 0x0000, 0x0000, 0xFFFF,
                           0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 0xFFFF,
                           0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

//Function to introduce delay in movement of the ball//
void delayreq (short x);

/**************************************************************************************************************************************************************************************************************
 * Function Name: main ()
 * Input Parameters: None.
 * Return Type void
 * Methods Called: delayreq (short x);
 * Description: With the function name being main, this function initializes the system. Setting the system clock to 80 MHz, the method then enables the ADC0 of the TIVA
 * board, further configuring the latter to use Sequencer 1 with highest Priority with Processor triggering the Sequence. With analog output of the MKII Accelerometer
 * available on pins PD0 and PD1 of the TIVA board for X and Y axes respectively, the method further sets up Port D pins 0 (AIN7) and 1 (AIN6) to be utilized as analog
 * input pins for the 12 bit ADC of the TIVA TM4C board. For the analog values from accelerometer along X and Y axes to be converted into digital simultaneously, the
 * Sequencer 1 switches input from Channel 7 (PD0) to Channel 6 (PD1) for every other sequence. Enabling the ADC Sequencer, the method then sets the LCD Screen
 * of the MKII board to white followed by positioning the ball (3 x 3 pixel) at its center through double variables 'x_cordinates' and 'y_cordinates' and displaying the same
 * through ST7735_DrawBitmap (int16_t x, int16_t y, const uint16_t *image, int16_t w, int16_t h) method. Entering the while (1) loop in the aftermath, for moving the ball
 * according to the tilt of the board, the method clears the ADC interrupt Status Flag and triggers the ADC Conversion, the output of which is stored into an array. With the
 * values of samples 0 and 2 representing the ADC values along the X direction while values of samples 1 and 3 representing the ADC values along the Y direction, the
 * method sums the values into variables  'ui32ADC_values_x' and  'ui32ADC_values_y' for X and Y axes respectively. With extensive experimentation having been
 * undertaken to determine ADC values (for X and Y axes) in all possible tilt positions of the board (i.e. in various angles along both axis), the same were found to be range of
 *  2700 to 5400. Utilizing these very values in the in steps of 125 in the nested if-else loops in the method, the angle of tilt along both axes will been determined. The float
 *  variables 'move_x' and 'move_y' have been assigned values accordingly (i.e. depending upon ADC Values in X and Y axes respectively which in turn depend upon the tilt
 *  of the board). With lab requirement being to not show movement of ball up to 10 degrees of tilt of the board along either axes, the method then goes on to check if the tilt
 *  is less than 10 degrees through the ADC values for X and Y axes. The ADC values (along both axes) for tilt of up to 10 degrees was found to be between 3700 and 4400
 *  in both directions. Consequently, with the 'x_cordinates and "y_cordinates' being unchanged while ADC values remain in the range, the ball shall not move. Neither will it
 *  move when board on being tilted more than 10 degrees is made flat / tilt reduced to less than 10 degrees again as the method will again enter the loop and cease ball
 *  movement.
 *  On tilt being more than 10 degrees along X and Y axes, the ball shall move in the direction and angle of tilt within the four quadrants (through the nested if-else loops)
 *  through addition / subtraction of the appropriate values (depending on ADC values) of 'move_x' and 'move_y' to variables 'x_cordinates' and 'y_cordinates' respectively,
 *  the image of which shall be displayed over the LCD through ST7735_DrawBitmap (int16_t x, int16_t y, const uint16_t *image, int16_t w, int16_t h) method. With ADC
 *  Values for board tilt of up to 10 degrees being from 3700 to 4400, in the event of the board being tilted only along one axis (either X or Y), the ADC values along that axis
 *  will either increase to be more than 4400 or decrease to be less than 3700. The ADC value of the other axis in the meanwhile will remain fixed (at around 4050 i.e. 1.65 V).
 *  The ball in such scenario, will move towards the lower part of the screen (in parallel to the very axis it has been tilted in) through addition of 1 to its x or y coordinates
 *  instead of variables move_x or move_y. Finally, again as a lab requirement, the delayreq (short x) method will be called each time the ball moves a pixel and its image is
 *  displayed over the LCD so as to introduce appropriate delay.
 **************************************************************************************************************************************************************************************************************
 */
int main (void)
{
    // Set the system clock to 80MHz//
    PLL_Init(Bus80MHz);
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    ST7735_InitR(INITR_REDTAB);

    //Enable the ADC 0 peripheral //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    //Configure the ADC Sequencer to utilize Sequencer 1 with highest Priority with Processor triggering the Sequence //
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    //Enable GPIO for ADC 0 module. Set Port D pins 0 and 1 i.e. PD0 and PD1 as inputs for ADC0.//
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
    //Configure the Sequencer 1 of ADC 0 module to collect samples from Channel 7 and Channel 6 i.e. AIN 7 (for X-axis) and AIN 6 (for Y-Axis)//
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH6);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7);
    //Set the Interrupt flag when the last sample is done and inform ADC logic of following being the last sample on Sequencer 1//
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3,ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);
    //Enable ADC Sequencer 1 //
    ADCSequenceEnable(ADC0_BASE, 1);

    //Set the LCD Screen to White//
    ST7735_FillScreen (0xFFFF);
    //Position the ball initially at the Center of the Screen. The variables represent X and Y position of the ball. //
    x_cordinates = 61;
    y_cordinates = 70;
    //Display Image of the ball on LCD//
    ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);

    while (1)
    {
        //Clear the ADC interrupt Status Flag //
        ADCIntClear(ADC0_BASE, 1);
        //Trigger ADC Conversion //
        ADCProcessorTrigger(ADC0_BASE, 1);
        while(!ADCIntStatus(ADC0_BASE, 1, false))
        {
        }
        //Store the data from Sequencer Output FIFO into the array //
        ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

        //Add Samples 0 and Sample 2 from  Channel 7 i.e. AIN 7 (PD0) into the variable as it represents digital representation of the accelerometer output along X-Axis//
        ui32ADC_values_x = (ui32ADC0Value[0] + ui32ADC0Value[2]);
        //Add Samples 1 and Sample 3 from  Channel 6 i.e. AIN 6 (PD1) into the variable as it represents digital representation of the accelerometer output along Y-Axis//
        ui32ADC_values_y = (ui32ADC0Value[1]+ ui32ADC0Value[3]);

        /*
         * Determine angle (depending on tilt) for movement of ball along X-Axis. The ADC values utilized as thresholds have been obtained after repeated experimentation to
         * be between 2700 and 4400
         */
        if (ui32ADC_values_x > 4400 || ui32ADC_values_x <=3700)
        {
            move_x = 0.1;
        }
        if (ui32ADC_values_x >= 4525 || ui32ADC_values_x <=3575)
        {
            move_x = 0;
            move_x = 0.2;
        }
        if (ui32ADC_values_x >= 4650 || ui32ADC_values_x <=3450)
        {
            move_x = 0;
            move_x = 0.3;
        }
        if (ui32ADC_values_x >= 4775 || ui32ADC_values_x <=3325)
        {
            move_x = 0;
            move_x = 0.4;
        }
        if (ui32ADC_values_x >= 4900 || ui32ADC_values_x <=3200)
        {
            move_x = 0;
            move_x = 0.5;
        }
        if (ui32ADC_values_x >= 5025 || ui32ADC_values_x <=3075)
        {
            move_x = 0;
            move_x = 0.6;
        }
        if (ui32ADC_values_x >= 5150 || ui32ADC_values_x <=2950)
        {
            move_x = 0;
            move_x = 0.7;
        }
        if (ui32ADC_values_x >= 5275 || ui32ADC_values_x <=2825)
        {
            move_x = 0;
            move_x = 0.8;
        }
        if (ui32ADC_values_x >= 5400 || ui32ADC_values_x <=2700)
        {
            move_x = 0;
            move_x = 0.9;
        }

        /*
         * Determine angle (depending on tilt) for movement of ball along Y-Axis. The ADC values utilized as thresholds have been obtained after repeated experimentation to
         * be between 2700 and 4400
         */
        if (ui32ADC_values_y >= 4400 || ui32ADC_values_y <= 3700)
        {
            move_y = 0;
            move_y = 0.9;
        }
        if (ui32ADC_values_y >= 4525 || ui32ADC_values_y <= 3575)
        {
            move_y = 0;
            move_y = 0.8;
        }
        if (ui32ADC_values_y >= 4650 || ui32ADC_values_y <= 3450)
        {
            move_y = 0;
            move_y = 0.7;
        }
        if (ui32ADC_values_y >= 4775 || ui32ADC_values_y <= 3325)
        {
            move_y = 0;
            move_y = 0.6;
        }
        if (ui32ADC_values_y >= 4900 || ui32ADC_values_y <= 3200)
        {
            move_y = 0;
            move_y = 0.5;
        }
        if (ui32ADC_values_y >= 5025 || ui32ADC_values_y <= 3075)
        {
            move_y = 0;
            move_y = 0.4;
        }
        if (ui32ADC_values_y >= 5150 || ui32ADC_values_y <= 2950)
        {
            move_y = 0;
            move_y = 0.3;
        }
        if (ui32ADC_values_y >= 5275 || ui32ADC_values_y <= 2825)
        {
            move_y = 0;
            move_y = 0.2;
        }
        if (ui32ADC_values_y >= 5400 || ui32ADC_values_y <= 2700)
        {
            move_y = 0;
            move_y = 0.1;
        }

        //Cease movement of ball as ADC Values correspond to accelerometer output (along X and Y axes) when tilt of the board along either axes is 10 degrees or less//
        if ((3700 < ui32ADC_values_x <= 4400) && (3700 < ui32ADC_values_y <= 4400))
        {
            x_cordinates = x_cordinates;
            y_cordinates = y_cordinates;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            //Introduce Delay of 50 milliseconds//
            delayreq (5);
        }

        //Determine Direction and initiate movement of ball along X and / or Y Axis//
        if ((ui32ADC_values_x > 4400) && (ui32ADC_values_y > 4400)) //Move the ball in Fourth Quadrant//
        {
            x_cordinates = x_cordinates + move_x;
            y_cordinates = y_cordinates - move_y;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x < 3700) && (ui32ADC_values_y > 4400))  //Move the ball in Third Quadrant //
        {
            x_cordinates = x_cordinates - move_x;
            y_cordinates = y_cordinates - move_y;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x < 3700) && (ui32ADC_values_y < 3700))  //Move the ball in Second Quadrant //
        {
            x_cordinates = x_cordinates - move_x;
            y_cordinates = y_cordinates + move_y;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x > 4400) && (ui32ADC_values_y < 3700))  //Move the ball in First Quadrant //
        {
            x_cordinates = x_cordinates + move_x;
            y_cordinates = y_cordinates + move_y;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x > 4400) && (ui32ADC_values_y < 4400))  // Move the ball horizontally along / parallel to the X-Axis //
        {
            x_cordinates = x_cordinates + 1;
            y_cordinates = y_cordinates;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x < 3700) && (ui32ADC_values_y < 4400))  // Move the ball horizontally along / parallel to the X-Axis //
        {
            x_cordinates = x_cordinates - 1;
            y_cordinates = y_cordinates;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        }else if ((ui32ADC_values_x < 4400) && (ui32ADC_values_y > 4400))   // Move the ball vertically along / parallel to the Y-Axis //
        {
            x_cordinates = x_cordinates;
            y_cordinates = y_cordinates - 1;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        } else if ((ui32ADC_values_x < 4400) && (ui32ADC_values_y < 3700))  // Move the ball vertically along / parallel to the Y-Axis //
        {
            x_cordinates = x_cordinates;
            y_cordinates = y_cordinates + 1;
            ST7735_DrawBitmap (x_cordinates, y_cordinates, ball, 5, 5);
            delayreq (5);
        }
    }
}

/**************************************************************************************************************************************************************************************************************
 * Function Name: delayreq (short x)
 * Input Parameters: One (short x).
 * Return Type: void
 * Methods Called in: main ()
 * Description: With lab requirement being introduction of delay to enforce that the movement of ball remains smooth while not changing more than one pixel every 50
 * milliseconds, this method enforces the same through variable 'uint32_t timedelay' and the function input parameter 'short x'. Providing a delay of 10 milliseconds when
 * value of x is set to 1(through decrementing both variables), the same can be taken to 50 milliseconds by passing 5 as an input parameter to the function i.e assigning the
 * variable 'short x' the value of 5 whenever the function is called within the main loop. With the function being called with parameter '5' every time the ball moves in any
 * direction and is displayed, lab requirement of introducing delay and moving the ball by a pixel every 50 milliseconds is implemented.
 **************************************************************************************************************************************************************************************************************
 */

void delayreq (short x)
{
    volatile uint32_t timedelay;
    while (x > 0)
    {
        timedelay = (400000 * 2) / 100;
        while (timedelay > 0)
        {
            timedelay --;
        }
        x --;
    }
}
