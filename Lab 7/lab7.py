'''
    ECGR 4101/5101, Fall 2018: Lab 7 - Internet-of-Things (IoT) and WiFi Version 1.0
    Avadhut Naik (801045233)
    
    Utilizing TI’s CC3220SF board, the same has been configured as an HTTP server and connected to the Embedded Lab Wi-Fi (Embedded_Lab_EXT). With the board also obtaining accelerometer
values along the X and Y axes through I2C Communication, the very values are accessed wirelessly through the python program below through HTTP GET method (API’s of which are defined
in the program ‘link_local_task.c’). With values obtained being encoded in the bytes format, the same is decoded into strings which then been split, the values cast into integer to be
assigned to variables x_new and y_new for X and Y axes respectively. With the variables being assigned to the patch.center () method, they facilitate the movement of ball over the screen
which is set at coordinates (0, 0) upon the program being executed, in accordance with the direction of tilt of the board. With the HTTP Get Request being sent to the board through the
program continuously, which is displayed over the TERA Term through Serial communication, tilting the board in any direction will result in movement of ball over the screen in accordance
with the accelerometer values along the X and Y axes.
********************************************************************************************************************************************************************************************

The following set of specifications were derived from lab requirements:
1.  Start with how GET/POST works in the HTTP protocol and specifically how to pass a parameter with a GET.
2.  After connecting to the embedded lab WIFI, it is time to learn about the I2C protocol. First, understand how its addressing works and tries to read the integrated temperature sensor.
3.  Having grasped I2C protocol and its addressing techniques and having noted the default save address of Accelerometer (in hex) to be 0x18, the ‘out_of_box_CC3220SF_LAUNCHXL_tirtos_ccs’
was next imported from projects and complied. Launching Tera Term application to view UART output data (mainly IP of the board), the program was run with the Baud rate set to 115200, Data
Bits set to 8, a single stop bit and no parity.
4.  Connecting laptop to the board Wi-Fi, browse to IP address of the board 10.123.45.1 from the browser to add profile for the Wi-Fi of Embedded Lab as follows:
    SSID: Embedded_Lab_Ext
    Security Type: WPA/WPA2
    Security Key: embedded
    Profile Priority: 15
5.  Restarting the program through the CCS, the board was now connected to the Embedded Lab Wi-Fi and connfugured as an HTTP Server through the program ‘out_of_box_CC3220SF_LAUNCHXL_tirtos_ccs’
with IP of 10.0.0.61, responding with accelerometer data along the X and Y axes for the following URL’s
    {X-Axis}: [http://<BOARD_IP>/sensor?axisx]
    {Y-Axis}: ["http://<BOARD_IP>/sensor?axisy]
6.  An I2C Transaction is also undertaken by the board to acquire the accelerometer values along X and Y axes with I2C0 and a communication speed of 400 KHz. I2C address for X and Y axes
is 0x2 and 0x4 respectively.
7.  With the data obtained from the board being encoded as a bytes object, the same will be decoded into a string which is then split to obtain the value and cast into an integer to be
assigned to variables x_new and y_new for X and Y axes respectively.
8.  With the variables being parsed to the parse.center () method, the accelerometer values obtained from the board wirelessly through HTTP GET method API will be used to determine and
undertake movement of ball which is initially centred at coordinates (0, 0) upon execution, in accordance with the tilt of the board as the latter will change accelerometer values which
in turn will cause ball movement over the screen.
9.  Additionally, the HTTP Get request received and responded to by the board for X and Y axes accelerometer data can be viewed through the UART over the Tera term terminal.
********************************************************************************************************************************************************************************************


The following steps were undertaken to make CC3220SF read Temperature Sensor values:
The Inter Integrated Circuit (I2C) supports bidirectional data transfer in half duplex mode through a synchronous bus and 2 channels viz. SDA and SCL. With the master (the processor, in
this case) initiating the start condition over the falling edge of SDA line while SCL is high, it then sends the address of device it wishes to communicate with over the SDA followed by
the Read /write bit. While 0 indicates that the master will write, 1 indicates that the master will read from the device. Initiating communication in the aftermath of receiving ACK from
the slave device which is done by the slave holding the data line low, the master will read data from / write data to the slave device on completion of which it sends the stop condition
by making the SDA line high which SCL is held high. With the digital temperature sensor of the CC3220SF board being I2C Compatible, following steps were undertaken to read temperature
values from it.
1.	Referring to the User's guide of the CC3220SF board, default slave address of the Temperature sensor was obtained (0x41).
2.	Initially, opening up the TMP006Driver, the manufacturer and version of the board is obtained upon verification of which i.e. upon verifying that board is Texas Instruments CC3220SF, the configuration register value will be obtained.
3.	Obtaining the Sensor voltage register value in the aftermath (i.e. at address 0x0), the ambient temperature register value will be acquired from the address 0x1 through the read from I2C API (to get the required number of bytes)
4.	Applying Format Conversion to value obtained, the same is then converted into Farenheits and assigned to variable ‘temperatureVal’ if the value has been successfully acquired.
5.	With the variable holding the current temperature value, the same is appended and sent with the payload as response to the HTTP GET request received while displaying the request
reieved and values sent over thte Tera Term Termainal through UART.

For obtaining the accelerometer values for the X and Y axes, the following steps are undertaken:
1.	Referring to the User's guide of the CC3220SF board, default slave address of the Accelerometer was obtained (0x18).
2.	Initially setting the register to be written to, the read from I2C API is invoked to acquire the required number of bytes from the slave address (i.e. 0x18).
3.	Initializing the BMA222 accelerometer, in the aftermath the chip ID number will be obtained through the BMA222Open () function followed by the accelerometer data readings through pointers *psAccX and *psAccY for X and Y axes which point towards the raw AccX value and raw AccY value stores respectively. 
4.	Reading the LSB and MSB blocks through BlockRead () function from the store for both axes, the board will periodically check for new values X and Y axes, in the event of which the
new values will be acquired to replace the old acquired values which shall assigned to variables xval and yval after range conversion.
5.	With the variables having the accelerometer values along X and Y axes, the variable values (either X or Y axis value)  are appended and sent with the payload as response to the
received HTTP GET request from the below python code while displaying request received and values sent over the Tera Term Terminal through UART.
******************************************************************************************************************************************************************************************

The following steps were followed to initiate ball movement over the screen in accordance with tilt of the board from values obtained from accelerometer:
1.	Import the “out_of_box_CC3220SF_LAUNCHXL_tritos_ccs”. Compile the code and run it to the CC3220SF board.
2.	Creating a new thread (named mainthread) within the main_tirtos.c through pthread_create () function of the POSIX library and initializing the I2C, UART and HTTP Server through it,
the same will be scheduled by the scheduler.
3.	Further, creating a sub-thread for Wi-Fi connection and read Sensor values through I2C protocol communication (link_Local_task.c). The same also defined the API’s for HTTP GET
method utilized in the python code below and responds to requests receieved through it (through sensor values appended to payload).
4.	Setting the I2C address for X and Y axes to be 0x2 and 0x4 respectiveluy to communicate with the accelerometer and WAIT for HTTP request.
5.	Upon running the program over the CC3220SF board, the board wshall initialy start in theh Access point Mode. Connecting laptop to the same and browsing to its IP 10.123.45.1
through the browser, add the Embedded_Lab_EXT WiFi in the profiles tab. Upon resetting the program through CCS, the board will now be connected to the Embedded_Lab_EXT Wi-Fi, the
confirmation of which will be provided through UART on the Tera Term Terminal which shall display the Access Point the board is connected to along with its Acquired IP (10.0.0.61) among
other details.
6.	Generate the URL for HTTP GET method within the python code as http://<BOARD_IP>/sensor?axisx to acquire X-axis accelerometer data and http://<BOARD_IP>/sensor?axisy to acquire
Y-axis accelerometer data.
7.	With the data obtained from the board being encoded as a bytes object, the same will be decoded into a string which is then split to obtain the value and cast into an integer to
be assigned to variables x_new and y_new for X and Y axes respectively.
8.	With the variables being parsed to the parse.center () method, the accelerometer values obtained from the board wirelessly through HTTP GET method API will be used to determine and
undertake movement of ball which is initially centred at coordinates (0, 0) upon execution, in accordance with the tilt of the board as the latter will change accelerometer values which in turn will cause ball movement over the screen.
9.	Additionally, the HTTP Get request received and responded to by the board for X and Y axes accelerometer data can be viewed through the UART over the Tera term terminal.
********************************************************************************************************************************************************************************************
'''

import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation
import requests
import time


fig = plt.figure()
fig.set_dpi(100)
fig.set_size_inches(7, 6.5)

ax = plt.axes(xlim=(-65, 65), ylim=(-65, 65))
ax.set_aspect('equal')
ax.grid(True, which='both')
patch = plt.Circle((0, 0), 1, fc='r')

BOARD_IP = "10.0.0.61"


def init():
    patch.center = (0, 0)
    ax.add_patch(patch)
    return patch,

def animate(i):
    x, y = patch.center
    patch.center = (x, y) # If an error happens, it will keep the previous location
    try:
    '''Generate URL for HTTP GET method to request accelrometer data along X-axis from the board'''
        resp = requests.get("http://"+BOARD_IP+"/sensor?axisx")
    '''With the data obtained being a bytes object, decode the same into a string and split the latter as undertake casting into an integer to assign it to the integer variable'''
        x_new= int(resp.content.decode('utf-8')[6:])

    '''Generate URL for HTTP GET method to request accelrometer data along Y-axis from the board'''
        resp = requests.get("http://"+BOARD_IP+"/sensor?axisy")
    '''With the data obtained being a bytes object, decode the same into a string and split the latter as undertake casting into an integer to assign it to the integer variable'''  
        y_new= int(resp.content.decode('utf-8')[6:])

    '''sleep for the duration and parse the acquired accelrometer values through the integer variables to the parse.center method to determine and undertake ball movement on screen'''
        time.sleep(0.25)
        patch.center = (x_new, y_new)
    except ConnectionError:
        pass
    return patch,

anim = animation.FuncAnimation(fig, animate, 
                               init_func=init, 
                               frames=360, 
                               interval=62,
                               blit=True)

plt.show()
