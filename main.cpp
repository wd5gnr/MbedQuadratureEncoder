/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "USBSerial.h"

#include "Encoder.h"

#define TEST_LATENCY 0



// Blinking rate in milliseconds
std::chrono::milliseconds rate=500ms;






Thread encthread;    // thread for encoder
Thread stdio_clear;  // thread to watch for USB terminal connection

    // Note: init to false requires connect to enumerate
    // If true, you must connect to the board to start
USBSerial usbSerial(false);



// If you want to use printf etc with USB Serial...
// HOWEVER. USBSerial must be connected prior to elaboration or no go
namespace mbed
{
    FileHandle *mbed_override_console(int fd)
    {
        return &usbSerial;
    }

}


// This thread watches for a USB terminal connection and resets stdio to work with it
void stdio_reset()
{
    int conn,last=0;
    while (1)
    {
    if ((conn=usbSerial.connected()) && !last)  // note NOT conn==usbSerial....
    {
       clearerr(stdout);
       clearerr(stderr);
       clearerr(stdin);
    }
    last=conn;   // remember connection state
    ThisThread::sleep_for(500ms);  // check again later
    }
}

DigitalOut led(LED1);


#if TEST_LATENCY==0 
Encoder encoder(PA_3,PA_4);  // our encoder

// The encoder thread
void do_enc(void)
{
    int evalue=0;
    encoder.setAccumulate(true);  // let the class keep track of counts; note it can roll over, though
    while (1)
    {
        evalue=encoder.read();
        printf("Encoder: %d\r\n",evalue);
         ThisThread::sleep_for(750ms);
    }
}
#else

InterruptIn testin(PA_3);

void ledon(void)
{
    led=true;
}

void ledoff(void)
{
    led=false;
}


void do_enc(void)
{
    testin.rise(callback(ledon));
    testin.fall(callback(ledoff));
    while (1) ThisThread::sleep_for(1s);
}

#endif

// Main thread
int main()
{
    usbSerial.connect();  // connect USB port
    // Initialise the digital pin LED1 as an output
    

// If you redirect stdin/stdout etc to USBSerial, you have a problem
// When there is no serial port open at run time. The stream gets an error and
// then nothing will happen until you clear it.
// So the trick is to occasionally clear errors on stdout/stdin/stderr

    printf("Here we go!\r\n");  // Get started
// start the threads
    stdio_clear.start(stdio_reset);  
    encthread.start(do_enc);
// let us know we are running
    while (true) {
#if TEST_LATENCY==0
        led = !led;   // flip LED if output is true
#endif        
        ThisThread::sleep_for(rate);  // sleepy time
    }
}
