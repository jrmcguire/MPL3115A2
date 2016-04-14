/* 
 * File:   example.c
 * Author: Jason McGuire (j.mcguire.2015@ieee.org)
 * Description: A library for a MPL3115A2 Barometric Pressure Sensor using I2C
 * See example.c for implementation of the library
 * Sample Product: https://www.adafruit.com/products/1893
 * Created on October 13, 2015, 9:08 AM
 
License Information:
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
 
 
 */
 
#include "MPL3115A2.h"

// Available Functions:
unsigned char read8bits(char); // read an 8 bit register 
void write8bits(char, char); // write a byte to an 8 bit register
unsigned int read16bits(char); // read two bytes from two registers 
void I2Cstart(); // Start signal for I2C 
void I2Cstop(); // Stop signal for I2C
void I2Cack(); // Acknowledgement signal for I2C
void I2Cnack(); // No-Acknowledgement signal for I2C
void I2Crestart(); // Restart signal for I2C
unsigned char I2Cread(); // Read signal for I2C
void I2Cwrite(char); // Write a byte for I2C
void I2Cwait(); // Wait for flags to clear 
float readPressure(); // Get a Pressure Reading in kPa


void I2Cstart(){
    SEN = 1; // Initiate start condition on SDA/SCL. Cleared by hardware
    while(SEN); // wait for it to be cleared 
}

void I2Cstop(){
    PEN = 1; // Initiate stop condition on SDA/SCL. Cleared by hardware 
    while(PEN); // wait for it to be cleared 
    
}

void I2Cack(){
    ACKDT = 0; // Acknowledge 
    ACKEN = 1; // Initiate acknowledge condition on SDA/SCL
    while(ACKEN); // wait for it to be cleared 
}

void I2Cnack(){
    ACKDT = 1; // Not Acknowledge
    ACKEN = 1; // Initiate acknowledge condition of SDA/SCL
    while(ACKEN); // wait for it to be cleared 
    
}

void I2Crestart(){
    RSEN = 1; // Initiate restart condition on SDA/SCL.
    while(RSEN); // wait for it to be cleared 
    
}

unsigned char I2Cread(){
    RCEN = 1; // enables receive mode for I2C
    while(!BF); // wait until SSPBUF is full
    I2Cwait(); 
    return SSPBUF; // get the data 
}

void I2Cwrite(char data){
    SSPBUF = data; // put data in the buffer
    while(BF); //  wait until data leaves the buffer 
    I2Cwait();
} 

void I2Cwait(){
    while ((SSPCON2 & 0x1F ) || (SSPSTAT & 0x04 )); 
}


float readPressure(){
    
    // write 0x3 to 0x26: 
    I2Cstart(); // start 
    I2Cwrite(MLP_WR_ADDR); // slave address write
    while(ACKSTAT); // wait for ack
    I2Cwrite(0x26); // send reg address to write 
    while(ACKSTAT); // wait for ack
    I2Cwrite(0x03); // send data to write 
    while(ACKSTAT); // wait for ack
    I2Cstop(); // stop 
    
    // read 3 bytes 
    unsigned char lsb;
    unsigned char csb;
    unsigned char msb;
    unsigned int lowerbytes;
    unsigned long int data; 
    I2Cstart(); // start 
    I2Cwrite(MLP_WR_ADDR);// slave address write
    while(ACKSTAT); // wait for ack
    I2Cwrite(0x01); // send reg address to read 
    while(ACKSTAT);// wait for ack 
    I2Crestart(); // start* = this start is either a restart or a stop followed by a start 
    I2Cwrite(MLP_RD_ADDR); // slave address + read 
    while(ACKSTAT); // wait for ack
    msb = I2Cread(); // data is received 
    I2Cack(); // send ack 
    csb = I2Cread(); // data is received
    I2Cack(); // send ack
    lsb = I2Cread(); // data is received 
    I2Cnack(); // send nack 
    I2Cstop(); // stop 
    
    // process data 
    data = msb;
    data = data << 16;
    lowerbytes = csb;
    lowerbytes = lowerbytes << 8;
    lowerbytes = lowerbytes | lsb;
    data = data | lowerbytes;
    float pressure = (float)data/64;
    return pressure/1000;
}