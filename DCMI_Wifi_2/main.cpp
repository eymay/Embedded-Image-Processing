#include "mbed.h"
#include "mainx.h"
#include "ov7670.h"

#define THREAD_STACK 2048

#define FRAME_BYTES 12672
#define PROCESSED_FRAME_BYTES 15000

Mutex  frameBuffer_access, processedBuffer_access; 
Semaphore processedBuffer_resources(0, PROCESSED_FRAME_BYTES);
int processedFrameIndex, sendByteIndex;
bool newImage = false;

uint8_t frameBuffer[15000] = { 0 };
uint8_t processedBuffer[PROCESSED_FRAME_BYTES] = { 0 };

void Capture(){
    while(1){
        frameBuffer_access.lock();
        OV7670_CaptureSnapshot((uint32_t) frameBuffer, FRAME_BYTES);
        newImage = true;
        frameBuffer_access.unlock();

    }
}
void Processor(){
    while (1) {
        if(processedFrameIndex == 0 && newImage == true){
            frameBuffer_access.lock();
            newImage = false;
        }
        if(processedFrameIndex >= FRAME_BYTES){
            processedFrameIndex = 0;
            frameBuffer_access.unlock();
        }
        processedBuffer_access.lock();
        //Algorithm increasing index
        processedBuffer_resources.release();
        processedBuffer_access.unlock();
    }
}

void Sender(){
    while (1) {
        processedBuffer_resources.acquire();
        processedBuffer_access.lock();
        //Sending function
        processedBuffer_access.unlock();
    }
}

// main() runs in its own thread in the OS
int main()
{
    int k;
    k = mainx();
    Thread *producer = new Thread(osPriorityNormal, THREAD_STACK, nullptr, "producer");
    producer->start(Capture);
    Thread *processor = new Thread(osPriorityNormal, THREAD_STACK, nullptr, "processor");
    processor->start(Processor);
    Thread *consumer = new Thread(osPriorityNormal, THREAD_STACK, nullptr, "consumer");
    consumer->start(Sender);
    thread_sleep_for(10);
    while (true);
}

