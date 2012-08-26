/*************************************************** 
This is a library for our Adafruit 16-channel PWM & Servo driver

Pick one up today in the adafruit shop!
------> http://www.adafruit.com/products/815

These displays use I2C to communicate, 2 pins are required to  
interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
****************************************************/

#include <Adafruit_PWMServoDriver.h>

#define USE_MOTATE 1

#if USE_MOTATE==1
	#include <MotateTWI.h>
#else
// #include <Wire.h>
#endif

#if USE_MOTATE==1
using namespace Motate::TWI;
#endif

Adafruit_PWMServoDriver::Adafruit_PWMServoDriver(uint8_t addr) {
	_i2caddr = addr;
}

boolean Adafruit_PWMServoDriver::begin(void) {
#if !USE_MOTATE==1
	Wire.begin();
#endif
	reset();
	return true;
}


void Adafruit_PWMServoDriver::reset(void) {
#if USE_MOTATE==1
	Master.open(_i2caddr, Writing);
	Master.write(PCA9685_MODE1);
	Master.write(PCA9685_AUTOINCREMENT, true);
	Master.close();
#else
	write8(PCA9685_MODE1, 0x0);
#endif
}

void Adafruit_PWMServoDriver::setPWMFreq(float freq) {
	//Serial.print("Attempting to set freq ");
	//Serial.println(freq);

	float prescaleval = 25000000;
	prescaleval /= 4096;
	prescaleval /= freq;
	prescaleval -= 1;
	Serial.print("Estimated pre-scale: "); Serial.println(prescaleval);
	uint8_t prescale = floor(prescaleval + 0.5);
	Serial.print("Final pre-scale: "); Serial.println(prescale);  


#if USE_MOTATE==1
	Master.open(_i2caddr, Writing);
	Master.write((uint8_t)PCA9685_MODE1, /*last = */true);
	Master.open(_i2caddr, Reading);
	Master.setExpecting(1);
	do {} while (!Master.available());
	uint8_t oldmode = Master.read();
	
	uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep
	
	Master.open(_i2caddr, Writing);
	Master.write(PCA9685_MODE1);
	Master.write(newmode, true);
	
	Master.open(_i2caddr, Writing);
	Master.write(PCA9685_PRESCALE);
	Master.write(prescale, true);

	Master.open(_i2caddr, Writing);
	Master.write(PCA9685_MODE1);
	Master.write(oldmode, true);
	
	Master.sync();
	delay(5);
	
	Master.open(_i2caddr, Writing);
	Master.write(PCA9685_MODE1);
	Master.write(oldmode | 0x80, true);
	
	Master.close();

#else
	uint8_t oldmode = read8(PCA9685_MODE1);
	uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep

	write8(PCA9685_MODE1, newmode); // go to sleep
	write8(PCA9685_PRESCALE, prescale); // set the prescaler
	write8(PCA9685_MODE1, oldmode);
	delay(5);
	write8(PCA9685_MODE1, oldmode | 0x80);
	
#endif
	//  Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void Adafruit_PWMServoDriver::setPWM(uint8_t num, uint16_t on, uint16_t off) {
#if USE_MOTATE==1
	// uint8_t set_command[] = {LED0_ON_L+4*num, (uint8_t)on, (uint8_t)(on>>8), (uint8_t)off, (uint8_t)(off>>8)};
	
	Master.open(_i2caddr, Writing);
	// Master.write(set_command);
	Master.write(LED0_ON_L+4*num);
	Master.write((uint8_t)on);
	Master.write((uint8_t)(on>>8));
	Master.write((uint8_t)off);
	Master.write((uint8_t)(off>>8), true);
	Master.close();
#else
	write8(LED0_ON_L+4*num, on);
	write8(LED0_ON_H+4*num, on >> 8);
	write8(LED0_OFF_L+4*num, off);
	write8(LED0_OFF_H+4*num, off >> 8);
#endif
}


#if USE_MOTATE==1
// uint8_t Adafruit_PWMServoDriver::read8(uint8_t addr) {
// 	Master.open(_i2caddr, Writing);
// 	Master.write(addr, /*last = */true);
// 	//do {} while (!Master.write(addr));
// 	// Master.flush();
// 	// Master.close();
// 
// 	Master.open(_i2caddr, Reading);
// 	// Master.setExpecting(1);
// 	// do { ; } while (!Master.available());
// 	char c;
// 	Master.read(&c, 1);
// 	Master.close();
// 
// 	return (uint8_t)c;
// }

// void Adafruit_PWMServoDriver::write8(uint8_t addr, uint8_t d) {
// 	Master.open(_i2caddr, Writing);
// 	Master.write(addr);
// 	Master.write(d, /*last = */true);
// 	Master.close();
// }
// 
// 
// 
// 
// 
// 
// 



#else
uint8_t Adafruit_PWMServoDriver::read8(uint8_t addr) {
	Wire.beginTransmission(_i2caddr);
	Wire.write(addr);
	Wire.endTransmission();

	Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)1);
	return Wire.read();
}

void Adafruit_PWMServoDriver::write8(uint8_t addr, uint8_t d) {
	Wire.beginTransmission(_i2caddr);
	Wire.write(addr);
	Wire.write(d);
	Wire.endTransmission();
}
#endif