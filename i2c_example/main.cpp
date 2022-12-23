#include "mbed.h"

// main() runs in its own thread in the OS

I2C i2c(I2C_SDA, I2C_SCL);

const int addr7bit = 0x21;      // 7 bit I2C address
const int addr8bit = 0x21 << 1; // 8bit I2C address, 0x90

int main()
{

    char cmd[2];
    while (1) {
        cmd[0] = 0x01;
        cmd[1] = 0x00;
        char readReg[1];
        char data[2];
        readReg[0] = 0x01;
        i2c.write(addr8bit, readReg, 1);

        ThisThread::sleep_for(500);

        //cmd[0] = 0x00;
        //i2c.write(addr8bit, cmd, 1);
        i2c.read(addr8bit, data, 2);

        //float tmp = (float((cmd[0] << 8) | cmd[1]) / 256.0);
        printf("%s\n", data);
    }

}

