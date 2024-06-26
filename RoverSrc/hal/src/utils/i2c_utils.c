#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "utils/gpio_utils.h"
#include "utils/i2c_utils.h"

int initI2cBus(char *bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    if (i2cFileDesc < 0)
    {
        printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
        perror("Error is:");
        exit(-1);
    }

    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0)
    {
        perror("Unable to set I2C device to slave address.");
        exit(-1);
    }
    return i2cFileDesc;
}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2)
    {
        perror("Unable to write i2c register");
        exit(-1);
    }
}

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr))
    {
        perror("Unable to write i2c register.");
        exit(-1);
    }

    // Now read the value and return it
    char value = 0;
    res = read(i2cFileDesc, &value, sizeof(value));
    if (res != sizeof(value))
    {
        perror("Unable to read i2c register");
        exit(-1);
    }
    return value;
}

void readI2cBlockData(int i2cFileDesc, unsigned char regAddr, unsigned char *data, int length)
{
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr))
    {
        perror("Unable to write i2c register.");
        exit(-1);
    }

    // Now read the value and return it
    res = read(i2cFileDesc, data, length);
    if (res != length)
    {
        perror("Unable to read i2c register");
        exit(-1);
    }
}

void configurePinI2c(const char *pin)
{
    configurePin(pin, GPIO_I2C_MODE);
}