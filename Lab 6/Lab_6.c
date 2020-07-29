/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/******************************************************************************************************************************************
 * ECGR 4101/5101, Fall 2018: Lab 6 - Internet-of-Things (IoT) and WiFi Version 2.0
    Avadhut Naik (801045233)

 *      Utilizing TI's SimpleLink Wi-Fi CC3220SF board with Internet-on-a-Chip and Wi-Fi capabilities, the digital temperature sensor (I2C compatible) of the board is utilized
 *  to provide temperature values. Extracting the degree celsius values from the same with the aid of the digital Temperature Sensor's datasheet, the same is converted into a
 *  string and concatenated at the end of the request URI to the be sent to the HTTP server through the wireless access point "Embedded_Lab_EXT". Upon receiving
 *  message from the server of the entry for temperature being successfully created (which was a lab requirement), the same can be view through the show URI though any
 *  browser of a Laptop or desktop connected to the same access point. While the I2C Communication protocol has been utilized for extracting temperature values from the
 *  sensor (as a part of lab requirement), the converted centigrade temperature values are sent to the server through the HTTP Get method, utilizing the wireless capabilities
 *  of the board. Upon any error occurring in either connecting with the wireless access point or the HTTP Server or in the event of the HTTP request being not processed by
 *  the server to provide the expected response, appropriate error messages shall be displayed over the console.
 *  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *      The following set of specifications were derived from the Lab requirements:
 *  1.  Start with how GET/POST works in the HTTP protocol and specifically how to pass a parameter with a GET.
 *  2.  After connecting to the embedded lab WIFI, it is time to learn about the I2C protocol. First, understand how its addressing works and tries to read the integrated
 *  temperature sensor.
 *  3. Having grasped the I2C Protocol and its addressing techniques, to read temperature values from the temperature sensor, and having noted the default slave address of
 * Temperature sensor (in hex) to be 0x41, an I2C transaction was next initiated to read Temperature values from the sensor.
 * 4.   With the Temperature values obtained and the I2C connection closed, the same were converted into centigrades through the aid of the datasheet of the Temperature
 * sensor, additionally employing sign extension for 2's complement negative values.
 * 5.   With the temperature obtained in centigrades in an integer variable, the same was converted to into a string and concatenated at the end of the request URI string to be
 * sent to the server over Wi-Fi.
 * 6.   Being already familiar with the HTTP Get method and techniques to pass parameters with the same, HTTP client is created next and its header set as per the protocol
 * specifications. Another integer variable is utilized to store the status code and appropriate message in case of failure.
 * 7.   Attempting to establish connection comes next in the aftermath of which HTTP request is sent with the URI containing the Temperature value for server to store.
 * 8. Another integer variable is utilized in combination with a string to store and print the status code and message received respectively from the server along with another
 * integer variable to count the number of bytes received which is achieved through the assistance of a boolean flag.
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *      Additionally, the following steps were undertaken to make CC3220SF read Temperature Sensor values:
 *  The Inter Integrated Circuit (I2C) supports bidirectional data transfer in half duplex mode through a synchronous bus and 2 channels viz. SDA and SCL. With the master
 *  (the processor, in this case) initiating the start condition over the falling edge of SDA line while SCL is high, it then sends the address of device it wishes to communicate
 *  with over the SDA followed by the Read /write bit. While 0 indicates that the master will write, 1 indicates that the master will read from the device. Initiating
 *  communication in the aftermath of receiving ACK from the slave device which is done by the slave holding the data line low, the master will read data from / write data to
 *  the slave device on completion of which it sends the stop condition by making the SDA line high which SCL is held high. With the digital temperature sensor of the
 *  CC3220SF board being I2C Compatible, following steps were undertaken to read temperature values from it.
 *  1.  Referring to the User's guide of the CC3220SF board, default slave address of the Temperature sensor was obtained (0x41).
 *  2.  Defining the default address along with the read bit for the temperature value from the sensor, the necessary parameters for I2C communication were set through
 *  the predefined I2CParams which included initialization through the 'init' method, setting the  bit rate at 400 KHz and initiating the start condition for I2C communication
 *  with the address of the Temperature Sensor through the I2C_open () method. Setting the slave address to the address of temperature sensor, specifying the type of
 *  operation to be performed (i,e, read) and setting the number of bytes to be read from the temperature sensor, the temperature values are obtained from the sensor.
 *  3.  With the aid of the datasheet of the temperature Sensor, the values obtained are converted into centigrades with sign extension, additionally, undertaken for 2's
 *  complement negative values.
 *  4. Finally, to avoid decimal precision of the temperature values obtained, the latter is divided by 32 in accordance with the datasheet of the sensor so as to obtain an integer
 *  value for temperature.
 *  ****************************************************************************************************************************************
 */

/******************************************************************************************************************************************
 *                                                                                                             Header Files
 ******************************************************************************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/******************************************************************************************************************************************
 *                                                                                              Driver Header Files
 ******************************************************************************************************************************************
 */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>


/******************************************************************************************************************************************
 *                                                                                              Board Header Files
 ******************************************************************************************************************************************
 */
#include "Board.h"

/******************************************************************************************************************************************
 *                                                                                              BSD Support Files
 ******************************************************************************************************************************************
 */
#include "string.h"
#include <ti/display/Display.h>
#include <ti/net/http/httpclient.h>
#include "semaphore.h"

/******************************************************************************************************************************************
 *                                                                               Define Macros for Temperature Sensor Address and Result Register
 ******************************************************************************************************************************************
 */
#define TMP006_ADDR 0x41
#define TMP006_DIE_TEMP 0x0001  /* Die Temp Result Register */

/******************************************************************************************************************************************
 *                                                                                     Define Macros for Conditional Compilation
 ******************************************************************************************************************************************
 */
#ifndef Board_TMP_ADDR
#define Board_TMP_ADDR       TMP006_ADDR
#endif

/******************************************************************************************************************************************
 *                                           Define Macros for Server IP Address, HTTP Client name and Size of buffer for data receieved from HTTP Server
 ******************************************************************************************************************************************
 */
#define HOSTNAME              "10.0.0.31"
#define USER_AGENT            "HTTPClient (ARM; TI-RTOS)"
#define HTTP_MIN_RECV         (256)

/******************************************************************************************************************************************
 *                                                                                      Externally defined variables and methods
 ******************************************************************************************************************************************
 */
extern Display_Handle display;
extern sem_t ipEventSyncObj;
extern void printError(char *errString, int code);

/*
 * Function Name : httpTask
 * Return Type: void* (void pointer i.e. the pointer has no return type and can be type-casted into any type)
 * Input Parameters: one (void* pvParameters)
 * Description: With the function initialing declaring the variables to utilized within, it called the Display, GPIO and I2C initialization methods in the aftermath, Configuring the
 * GPIO LED pin, it later turns on the User LED over the CC3220SF board. Creating I2C transaction for usage is what follows next in which I2C parameters are initialized
 * through the 'I2C_Params_init ()' method, in the aftermath of which the bitRate for transaction is set to 400KHz and the I2C is opened for communication through the
 * 'I2C_open()' method which takes the address of the device to communicate with (0x41) as one of the two parameters. Setting the 'tBuff [1]' array to one 1 which
 * indicates that the data is to be read from the sensor, it sets the slave address as the address of temperature sensor (0x41). Further stating that the a single byte i.e 'tBuff'
 * variable is to be written while 2 bytes of data is to be read from the Sensor (from 'rBuff[2]'), it reads data from the digital temperature sensor and stores the same in the
 * integer variable 'temperature' by converting the same into centigrades. The conversion has been undertaken through assistance of the Datasheet of the Temperature Sensor.
 * In the event of the MSB of the digital data being 1 which indicates a 2's complement negative integer, sign extension has been undertaken in the aftermath. To ensure a
 * proper integer value of temperature is displayed the result of the former computations is divided by 32 in the aftermath of which the I2C is closed. Converting the integer
 * variable 'temperature' into a string through the 'sprintf ()' function, the resultant string 'temp []' is then concatenated with the 'REQUEST_URI[]' string to generate the URI
 * with the current temperature value to be sent to the server.
 *      Creating the HTTP Client, in the aftermath through the 'HTTPClient_create ()' function, the integer variable 'statusCode' is utilized to indicate if any errors occur in
 * the same and to print appropriate message. Setting the HTTP header as per the protocol standards through the 'HTTPClient_setHeader ()' function, another variable
 * 'returnvalue' is utilized to indicate if any errors occur in the same. Establishing connection through 'HTTPClient_connect ()' is what follows. Variable 'returnvalue' will
 * again indicate errors, if any, to display message appropriately. With connection being established with the server an HTTP request is made in the aftermath with the URI
 * generated earlier through the 'HTTPClient_sendRequest ()' method. With 'returnvalue' again being utilized for the status code received from the server the same is printed
 * over the screen. Reading and printing the server response is what has been undertaken in its aftermath through the 'receiveddata[HTTP_MIN_RECV]' string and the
 * boolean flag 'additionalDataFlag'. To calculate the number of bytes received from the server another variable 'length' is utilized which shall be used to print the received
 * data from the server in bytes over the screen. Finally, the method disconnects and disposes the created HTTP Client.
 */
void* httpTask(void* pvParameters)
{
    //Variable to store temperature value in Celsius//
    int temperature;
    //String to store temperature value obtained from sensor//
    char temp [] = "";
    //Variables and instances for initiating and completing an I2C Transaction//
    uint8_t tBuff[1];
    uint8_t rBuff[2];
    I2C_Handle i2c;
    I2C_Params i2cParams;
    I2C_Transaction i2cTransaction;
    bool additionalDataFlag = false;
    //String to store data received from server//
    char receiveddata[HTTP_MIN_RECV];
    //Variable to store value returned by the server//
    int16_t returnvalue = 0;
    //Variable to calculate the length of data received from server//
    int16_t length = 0;

    //Call init functions of drivers//
    Display_init();
    GPIO_init();
    I2C_init();

    // Configure the LED pin //
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    // Turn on user LED //
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    // Create I2C for usage by initializing the parameters, setting the bit rate to 400KHz and opening the I2C for a transaction //
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C_TMP, &i2cParams);

    // Point to the T ambient register by providing its address and read its 2 bytes by setting the write count to 1 and read count to 2//
    tBuff[0] = TMP006_DIE_TEMP;
    i2cTransaction.slaveAddress = Board_TMP_ADDR;
    i2cTransaction.writeBuf = tBuff;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rBuff;
    i2cTransaction.readCount = 2;

    //Initiate I2C transaction//
    if (I2C_transfer(i2c, &i2cTransaction)) {

        //Extract degrees C from the received data in accordance with TMP102 datasheet //
        temperature = (rBuff[0] << 6) | (rBuff[1] >> 2);

        //If the MSB is set '1', then we have a 2's complement negative value which needs to be sign extended//
        if (rBuff[0] & 0x80)
        {
            temperature |= 0xF000;
        }
    }

    //For simplicity, divide the temperature value by 32 to get rid of the decimal precision inn accordance with TI's TMP006 datasheet//
    temperature /= 32;

    Display_printf(display, 0, 0, "Sample: %d (C)\n", temperature);

    // Deinitialized I2C //
    I2C_close(i2c);

    //Convert the temepereture value into integer variable into a string//
    sprintf (temp, "%d", temperature);
    Display_printf(display, 0, 0, "Sample: %s (C)\n", temp);

    //Declare the Request URI String//
   char REQUEST_URI [] = "/lab6/?Action=Save&Student_ID=801045233&Student_ID=801043306&Tempr=";

   //Concatenate the received temperature value string with the URI String to send temperature to the server//
    strcat (REQUEST_URI, temp);
    Display_printf(display, 0, 0, "Sample: %s (C)\n", REQUEST_URI);

    Display_printf(display, 0, 0, "Sending a HTTP GET request to '%s'\n", HOSTNAME);

    //Create a HTTP CLient//
    HTTPClient_Handle httpClientHandle;
    int16_t statusCode;
    httpClientHandle = HTTPClient_create(&statusCode,0);
    if(statusCode < 0)
    {
        printError("httpTask: creation of http client handle failed", statusCode);
    }

    //Set HTTP Header as the protocol specifications//
    returnvalue = HTTPClient_setHeader(httpClientHandle, HTTPClient_HFIELD_REQ_USER_AGENT, USER_AGENT,strlen(USER_AGENT), HTTPClient_HFIELD_PERSISTENT);
    if(returnvalue < 0)
    {
        printError("httpTask: setting request header failed", returnvalue);
    }

    //Establish Connection with the HTTP Server//
    returnvalue = HTTPClient_connect(httpClientHandle,HOSTNAME,0,0);
    if(returnvalue < 0)
    {
        printError("httpTask: connect failed", returnvalue);
    }
    //Send HTTP Request containing the temperature value in the URI to the HTTP Server//
    returnvalue = HTTPClient_sendRequest(httpClientHandle,HTTP_METHOD_GET,REQUEST_URI, NULL, 0, 0);
    if(returnvalue < 0)
    {
        printError("httpTask: send failed", returnvalue);
    }

    if(returnvalue != HTTP_SC_OK)
    {
        printError("httpTask: cannot get status", returnvalue);
    }

    Display_printf(display, 0, 0, "HTTP Response Status Code: %d\n", returnvalue);

    length = 0;
    do
    {
        //Store and print the response received from the HTTP Server//
        returnvalue = HTTPClient_readResponseBody(httpClientHandle, receiveddata, sizeof(receiveddata), &additionalDataFlag);
        if(returnvalue < 0)
        {
            printError("httpTask: response body processing failed", returnvalue);
        }
        Display_printf(display, 0, 0, "%.*s \r\n",returnvalue, receiveddata);
        //Calculate the size of data received from the server//
        length += returnvalue;
    }
    while(additionalDataFlag);
    //Display the received data size in bytes from the HTTP Server//
    Display_printf(display, 0, 0, "Received %d bytes of payload\n", length);

    //Disconnect the HTTP Client//
    returnvalue = HTTPClient_disconnect(httpClientHandle);
    if(returnvalue < 0)
    {
        printError("httpTask: disconnect failed", returnvalue);
    }
    //Dispose the created HTTP Client//
    HTTPClient_destroy(httpClientHandle);
    return(0);
}

/******************************************************************************************************************************************
 *                                                                                                main_tirtos.c
 ******************************************************************************************************************************************
 */
/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************************************************************************************
 *      Initially including the important the RTOS and TI-ROTS Header files through #define, the program, mainly initializes the board and spawns thread necessary for
 *Connecting the board to the wireless Access Point and then execute the HTTP Get Command through which Temperature Sensor values (converted into Celsius) are sent
 *to the server to receive response which is stored and displayed through a string with an integer variable to indicate the size of data received as response. In other words,
 *to the programs, spawns the thread necessary for execution of programs below and above.
 *******************************************************************************************************************************************
 */

/******************************************************************************************************************************************
 *                                                                                              Header Files
 ******************************************************************************************************************************************
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

/******************************************************************************************************************************************
 *                                                                                             Reference Externally defined methods
 ******************************************************************************************************************************************
 */
extern void * mainThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    4096

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t thread;
    pthread_attr_t pAttrs;
    struct sched_param priParam;
    int retc;
    int detachState;

    /* Call board init functions */
    Board_initGeneral();

    /* Set priority and stack size attributes */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 1;

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&pAttrs, detachState);
    if(retc != 0)
    {
        /* pthread_attr_setdetachstate() failed */
        while(1)
        {
            ;
        }
    }

    pthread_attr_setschedparam(&pAttrs, &priParam);

    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE);
    if(retc != 0)
    {
        /* pthread_attr_setstacksize() failed */
        while(1)
        {
            ;
        }
    }

    retc = pthread_create(&thread, &pAttrs, mainThread, NULL);
    if(retc != 0)
    {
        /* pthread_create() failed */
        while(1)
        {
            ;
        }
    }

    BIOS_start();

    return (0);
}

/*
 *  ======== dummyOutput ========
 *  Dummy SysMin output function needed for benchmarks and size comparison of FreeRTOS and TI-RTOS solutions.
 */
void dummyOutput(void)
{
}

/******************************************************************************************************************************************
 *                                                                                                        platform.c
 ******************************************************************************************************************************************
 */
/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*******************************************************************************************************************************************
 *      With program enabling the the CC3220SF board to establish a connection with wireless Access Point while taking the Access Point Name, the associated security and
 *password as input parameters along with the Application Name, it shall also print the Application Banner as has been defined in the method over the Console. Additionally,
 *it shall also print error messages if any errors occur in establishing connection with the Wireless Access Point over the console to which it establishes a serial communication
 *line.
 *******************************************************************************************************************************************
 */

/******************************************************************************************************************************************
 *                                                                                              Header and Driver Header Files
 ******************************************************************************************************************************************
 */
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/slnetifwifi.h>
#include <ti/display/Display.h>
#include <ti/drivers/SPI.h>

#include "Board.h"
#include "pthread.h"
#include "semaphore.h"

/******************************************************************************************************************************************
 *                                                                              Define Important Parameters to be Utilized in the Application
 ******************************************************************************************************************************************
 */
#define APPLICATION_NAME                      "HTTP GET"
#define DEVICE_ERROR                          ("Device error, please refer \"DEVICE ERRORS CODES\" section in errors.h")
#define WLAN_ERROR                            ("WLAN error, please refer \"WLAN ERRORS CODES\" section in errors.h")
#define SL_STOP_TIMEOUT (200)
#define SPAWN_TASK_PRIORITY (9)
#define SPAWN_STACK_SIZE (4096)
#define TASK_STACK_SIZE (2048)
#define SLNET_IF_WIFI_PRIO (5)
#define SLNET_IF_WIFI_NAME "CC3220SF"

// AP SSID//
#define SSID_NAME "Embedded_Lab_EXT"

//Security type could be SL_WLAN_SEC_TYPE_WPA_WPA2//
#define SECURITY_TYPE SL_WLAN_SEC_TYPE_WPA_WPA2

//Password of the secured AP//
#define SECURITY_KEY "embedded"

pthread_t httpThread = (pthread_t)NULL;
pthread_t spawn_thread = (pthread_t)NULL;

int32_t mode;
Display_Handle display;

/******************************************************************************************************************************************
 *                                                                                          Reference the eternally defined methods
 ******************************************************************************************************************************************
 */
extern void* httpTask(void* pvParameters);

/*
 *Function Name : printError
 * Return Type: void
 * Input Parameters: two (char *errString, int code)
 * Description: In the event of an error occurring in the application, the method is used to print out the appropriate error code along with the appropriate Error Message.
 */
void printError(char *errString, int code)
{
    Display_printf(display, 0, 0, "Error! code = %d, Description = %s\n", code, errString);
    while(1)
    {
        ;
    }
}

/*
 *Function Name : SimpleLinkNetAppEventHandler
 * Return Type: void
 * Input Parameters: one (SlNetAppEvent_t *pNetAppEvent)  which serves as a pointer to Netapp event data.
 * Description: This handler gets called whenever a Netapp event is reported by the host driver / NWP. As an addendum to the logic implemented, the same ca be varied for
 * any event so desired.This handler is used by 'network_terminal' application to show case the following scenarios:
    1. Handling IPv4 / IPv6 IP address acquisition.
    2. Handling IPv4 / IPv6 IP address Dropping.
 */
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    int32_t             status = 0;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;

    if(pNetAppEvent == NULL)
    {
        return;
    }

    switch(pNetAppEvent->Id)
    {
    case SL_NETAPP_EVENT_IPV4_ACQUIRED:
    case SL_NETAPP_EVENT_IPV6_ACQUIRED:

        //Initialize SlNetSock layer with CC3220SF interface//
        SlNetIf_init(0);
        SlNetIf_add(SLNETIF_ID_1, SLNET_IF_WIFI_NAME,
                   (const SlNetIf_Config_t *)&SlNetIfConfigWifi,
                    SLNET_IF_WIFI_PRIO);

        SlNetSock_init(0);
        SlNetUtil_init(0);
        if(mode != ROLE_AP)
        {
            //Display message for IPv4 / IPv6 acquired aling with acquired Gateway address//
            Display_printf(display, 0, 0,"[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                        "Gateway=%d.%d.%d.%d\n\r",
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,3),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,2),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,1),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,0),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,3),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,2),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,1),
                        SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,0));

            pthread_attr_init(&pAttrs);
            priParam.sched_priority = 1;
            status = pthread_attr_setschedparam(&pAttrs, &priParam);
            status |= pthread_attr_setstacksize(&pAttrs, TASK_STACK_SIZE);

            status = pthread_create(&httpThread, &pAttrs, httpTask, NULL);
            if(status)
            {
                printError("Task create failed", status);
            }
        }
        break;
    default:
        break;
    }
}

/*
 *Function Name : SimpleLinkFatalErrorEventHandler
 * Return Type: void
 * Input Parameters: one (SlDeviceFatal_t *slFatalErrorEvent)  which serves as a pointer to Fatal Error Event.
 * Description:  This handler gets called whenever a socket event is reported by the NWP / Host driver. After this routine is called, the user's  application must restart the
 * device in order to recover.
 */

void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
{
    /* Unused in this application */
}

/*
 *Function Name : SimpleLinkNetAppRequestMemFreeEventHandler
 * Return Type: void
 * Input Parameters: one (uint8_t *buffer)  which serves as a pointer to a NetApp request response
 * Description: This handler gets called whenever the NWP is done handling with This handler gets called whenever the NWP is done handling with dynamic memory with
 * these requests.
 */
void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer)
{
    /* Unused in this application */
}

/*
 *Function Name : SimpleLinkNetAppRequestEventHandler
 * Return Type: void
 * Input Parameters: two (SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse)
 * While the former servers as a pointer for NetApp request Structure, the latter servers as a pointer to NetApp Request Response
 * Description:  This handler gets called whenever a NetApp event is reported by the NWP / Host driver.
 */
void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse)
{
    /* Unused in this application */
}

/*
 *Function Name : SimpleLinkNetAppRequestEventHandler
 * Return Type: void
 * Input Parameters: two (SlNetAppHttpServerEvent_t *pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse)
 * While the first parameter serves as a pointer to the http event data the latter serves as a pointer to the http response.
 * Description:  This handler gets called whenever a HTTP event is reported   by the NWP internal HTTP server.
 */
void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse)
{
    /* Unused in this application */
}

/*
 * Function Name : SimpleLinkWlanEventHandler
 * Return Type: void
 * Input Parameters: one (SlWlanEvent_t *pWlanEvent) which serves as a pointer to Wlan event
 * Description: This handler gets called whenever a WLAN event is reported by the host driver / NWP. This handler is used by 'network_terminal' application to show case
 * the following scenarios:
    1. Handling connection / Disconnection.
    2. Handling Addition of station / removal.
    3. RX filter match handler.
    4. P2P connection establishment.
 */
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    /* Unused in this application */
}

/*
 *Function Name : SimpleLinkGeneralEventHandler
 * Return Type: void
 * Input Parameters: one (SlDeviceEvent_t *pDevEvent) which serves as a pointer to a device error event.
 * Description:  This handler gets called whenever a general error is reported  by the NWP / Host driver. Since these errors are not fatal,  application can handle them.
 */
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    /* Unused in this application */
}

/*
 *Function Name : SimpleLinkSockEventHandler
 * Return Type: void
 * Input Parameters: one (SlSockEvent_t *pSock) which serves as apointer to a socket event data
 * Description: This handler gets called whenever a socket event is reported by the NWP / Host driver.
 */
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    /* Unused in this application */
}

/*
 *Function Name : Connect
 * Return Type: void
 * Input Parameters: none
 * Description: This method is used to establish connection of the CC3220SF board with the wireless Access Point which in this case the Access Point with the name
 * "Embedded_Lab_EXT". Upon failing to establish a connection an apprrpriate error messahe shall be printed over the console by the message.
 */
void Connect(void)
{
    SlWlanSecParams_t secParams = {0};
    int16_t ret = 0;
    secParams.Key = (signed char*)SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;
    Display_printf(display, 0, 0, "Connecting to : %s.\r\n",SSID_NAME);
    ret = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    if(ret)
    {
        printError("Connection failed", ret);
    }
}

/*
 *Function Name : DisplayBanner
 * Return Type: void
 * Input Parameters: one (char * AppName) which points to the String representing the name of the application implemented.
 * Description:  This method displays application startup display on UART.
 */
static void DisplayBanner(char * AppName)
{
    Display_printf(display, 0, 0, "\n\n\n\r");
    Display_printf(display, 0, 0, "\t\t *************************""************************\n\r");
    Display_printf(display, 0, 0, "\t\t            %s Application       \n\r", AppName);
    Display_printf(display, 0, 0, "\t\t **************************""***********************\n\r");
    Display_printf(display, 0, 0, "\n\n\n\r");
}

/*
 * Function Name : mainThread
 * Return Type: void
 * Input Parameters: one (void* pvParameters)
 * Description: With the methods declaring the essential variables and structures at the start along with initializations for serial communications, it then attempts to establish
 * UART connection with the display for messages in the application to be displayed. Upon successfully establishing connection, it starts the SImpleLink Host (i.e CC3220SF)
 * and turns on the Wi-Fi Network Processor (NWP) to so as to initialize the device. Attempting to set NWP mode as STA in the aftermath, it displays an error message on
 * failure. Upon success, the NWP restarts for changes too take effect. If the NWP mode on restartong the device is still not set to STA, an appropriate error message from
 * DEVICE_ERROR will be displayed over the screen. If thhe changes have successfully taken place, the deivce will attempt to connect to the wireless access point.
 */

void mainThread(void *pvParameters)
{
    int32_t status = 0;
    pthread_attr_t pAttrs_spawn;
    struct sched_param priParam;

    SPI_init();
    Display_init();
    display = Display_open(Display_Type_UART, NULL);
    if(display == NULL)
    {
        /* Failed to open display driver */
        while(1)
        {
            ;
        }
    }
    /* Print Application name */
    DisplayBanner(APPLICATION_NAME);

    /* Start the SimpleLink Host */
    pthread_attr_init(&pAttrs_spawn);
    priParam.sched_priority = SPAWN_TASK_PRIORITY;
    status = pthread_attr_setschedparam(&pAttrs_spawn, &priParam);
    status |= pthread_attr_setstacksize(&pAttrs_spawn, SPAWN_STACK_SIZE);

    status = pthread_create(&spawn_thread, &pAttrs_spawn, sl_Task, NULL);
    if(status)
    {
        printError("Task create failed", status);
    }

    /* Turn NWP on - initialize the device*/
    mode = sl_Start(0, 0, 0);
    if (mode < 0)
    {
        Display_printf(display, 0, 0,"\n\r[line:%d, error code:%d] %s\n\r", __LINE__, mode, DEVICE_ERROR);
    }

    if(mode != ROLE_STA)
    {
        /* Set NWP role as STA */
        mode = sl_WlanSetMode(ROLE_STA);
        if (mode < 0)
        {
            Display_printf(display, 0, 0,"\n\r[line:%d, error code:%d] %s\n\r", __LINE__, mode, WLAN_ERROR);
        }

        /* For changes to take affect, we restart the NWP */
        status = sl_Stop(SL_STOP_TIMEOUT);
        if (status < 0)
        {
            Display_printf(display, 0, 0,"\n\r[line:%d, error code:%d] %s\n\r", __LINE__, status, DEVICE_ERROR);
        }

        mode = sl_Start(0, 0, 0);
        if (mode < 0)
        {
            Display_printf(display, 0, 0,"\n\r[line:%d, error code:%d] %s\n\r", __LINE__, mode, DEVICE_ERROR);
        }
    }

    if(mode != ROLE_STA)
    {
        printError("Failed to configure device to it's default state", mode);
    }
    Connect();
}



