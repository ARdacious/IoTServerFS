#ifndef __I2C_SCANNER_H__
#define __I2C_SCANNER_H__

/*
 * Function that tries different pin combinations for I2C bus configuration
 */
void scanAllI2C();

/*
 * Function that scans for I2C devices on current configured bus
 */
void scanI2C();

#endif
