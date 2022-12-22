#pragma once 
#include "mbed.h"
#include "ov7670.h"
#include "MODSERIAL.h"
 
 
 
MODSERIAL pc(USBTX,USBRX);
Timer t;
bool new_send = false;
 
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
 
 
//Camera
OV7670 camera
(
    D14,D15,            // SDA,SCL(I2C / SCCB)
    p23,NC,p25,         // VSYNC,HREF,WEN(FIFO)  
    Port0,0x07878000,   // PortIn data        p18(P0.26),p17(P0.25),p16(P0.24),p15(P0.23),p11(P0.18),p12(P0.17),p14(P0.16),p13(P0.15)
    p26,p29,p30         // RRST,OE,RCLK
); 
 
//RESET
extern "C" void mbed_reset();
 
//Serial
char word[25];
int t1 = 0; 
int t2 = 0;
int t3 = 0;
 
//
void parse_cmd();
void CameraSnap();
void CameraGrab();