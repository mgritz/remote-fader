#ifndef _I2C_H_
#define _I2C_H_

void i2cSetup(int);
void i2cWrite(int, int);
void i2cSend(unsigned char);

#define I2C_SUCCESS 	0
#define I2C_NO_ANSWER 	1
unsigned char i2cRequest(unsigned char);

#endif /* inlcude guard */
