/**
 * Arduino library to control Dps310
 *
 * "Dps310" represents Infineon's high-sensetive pressure and temperature sensor. 
 * It measures in ranges of 300 - 1200 hPa and -40 and 85 °C. 
 * The sensor can be connected via SPI or I2C. 
 * It is able to perform single measurements
 * or to perform continuous measurements of temperature and pressure at the same time, 
 * and stores the results in a FIFO to reduce bus communication. 
 *
 * Have a look at the datasheet for more information. 
 */

#ifndef DPSCLASS_H_INCLUDED
#define DPSCLASS_H_INCLUDED

#define NUM_OF_COMMON_REGMASKS 16

#include <SPI.h>
#include <Wire.h>
#include "util/dps_config.h"

using namespace dps;

class DpsClass
{
  public:
	//constructor
	DpsClass(void);
	//destructor
	~DpsClass(void);

	/**
	 * I2C begin function for Dps310 with standard address
	 */
	void begin(TwoWire &bus);
	/**
	 * Standard I2C begin function
	 *
	 * &bus: 			I2CBus which connects MC to Dps310
	 * slaveAddress: 	Address of the Dps310 (0x77 or 0x76)
	 */
	void begin(TwoWire &bus, uint8_t slaveAddress);
	/**
	 * SPI begin function for Dps310 with 4-wire SPI
	 */
	void begin(SPIClass &bus, int32_t chipSelect);
	/**
	 * Standard SPI begin function
	 *
	 * &bus: 			SPI bus which connects MC to Dps310
	 * chipSelect: 		Number of the CS line for the Dps310
	 * threeWire: 		1 if Dps310 is connected with 3-wire SPI
	 * 					0 if Dps310 is connected with 4-wire SPI (standard)
	 */
	void begin(SPIClass &bus, int32_t chipSelect, uint8_t threeWire);
	/**
	 * End function for Dps310
	 * Sets the sensor to idle mode
	 */
	void end(void);

	/**
	 * returns the Product ID of the connected Dps310 sensor
	 */
	uint8_t getProductId(void);
	/**
	 * returns the Revision ID of the connected Dps310 sensor
	 */
	uint8_t getRevisionId(void);

	/**
	 * Sets the Dps310 to standby mode
	 *
	 * returns:		0 on success
	 * 				-2 if object initialization failed
	 * 				-1 on other fail
	 */
	int16_t standby(void);

	/**
	 * performs one temperature measurement and writes result to the given address
	 *
	 * &result:		reference to a 32-Bit signed Integer value where the result will be written
	 * 				It will not be written if result==NULL
	 * returns: 	status code
	 */
	int16_t measureTempOnce(float &result);
	/**
	 * performs one temperature measurement and writes result to the given address
	 * the desired precision can be set with oversamplingRate
	 *
	 * &result:				reference to a 32-Bit signed Integer where the result will be written
	 * 						It will not be written if result==NULL
	 * oversamplingRate: 	a value from 0 to 7 that decides about the precision
	 * 						of the measurement
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements and combine the results
	 * returns: 			status code
	 */
	int16_t measureTempOnce(float &result, uint8_t oversamplingRate);
	/**
	 * starts a single temperature measurement
	 *
	 * returns: 	status code
	 */
	int16_t startMeasureTempOnce(void);
	/**
	 * starts a single temperature measurement
	 * The desired precision can be set with oversamplingRate
	 *
	 * oversamplingRate: 	a value from 0 to 7 that decides about the precision
	 * 						of the measurement
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements and combine the results
	 * returns: 			status code
	 */
	int16_t startMeasureTempOnce(uint8_t oversamplingRate);
	/**
	 * performs one pressure measurement and writes result to the given address
	 *
	 * &result:		reference to a 32-Bit signed Integer value where the result will be written
	 * 				It will not be written if result==NULL
	 * returns: 	status code
	 */
	int16_t measurePressureOnce(float &result);
	/**
	 * performs one pressure measurement and writes result to the given address
	 * the desired precision can be set with oversamplingRate
	 *
	 * &result:				reference to a 32-Bit signed Integer where the result will be written
	 * 						It will not be written if result==NULL
	 * oversamplingRate: 	a value from 0 to 7 that decides about the precision
	 * 						of the measurement
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements and combine the results
	 * returns: 			status code
	 */
	int16_t measurePressureOnce(float &result, uint8_t oversamplingRate);

	/**
	 * starts a single pressure measurement
	 *
	 * returns: 	status code
	 */
	int16_t startMeasurePressureOnce(void);
	/**
	 * starts a single pressure measurement
	 * The desired precision can be set with oversamplingRate
	 *
	 * oversamplingRate: 	a value from 0 to 7 that decides about the precision
	 * 						of the measurement
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements and combine the results
	 * returns: 			status code
	 */
	int16_t startMeasurePressureOnce(uint8_t oversamplingRate);
	/**
	 * gets the result a single temperature or pressure measurement in °C or Pa
	 *
	 * &result:		reference to a 32-Bit signed Integer value where the result will be written
	 * returns: 	status code
	 */
	int16_t getSingleResult(float &result);

	/**
	 * starts a continuous temperature measurement
	 * The desired precision can be set with oversamplingRate
	 * The desired number of measurements per second can be set with measureRate
	 *
	 * measureRate: 		a value from 0 to 7 that decides about
	 * 						the number of measurements per second
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements per second
	 * oversamplingRate: 	a value from 0 to 7 that decides about
	 * 						the precision of the measurements
	 * 						If this value equals m, the DPS310 will perform
	 * 						2^m internal measurements and combine the results
	 * 						to one more exact measurement
	 * returns: 			status code
	 * 	NOTE: 				If measure rate is n and oversampling rate is m,
	 * 						the DPS310 performs 2^(n+m) internal measurements per second.
	 * 						The DPS310 cannot operate with high precision and high speed
	 * 						at the same time.
	 * 						Consult the datasheet for more information.
	 */
	int16_t startMeasureTempCont(uint8_t measureRate, uint8_t oversamplingRate);

	/**
	 * starts a continuous temperature measurement
	 * The desired precision can be set with oversamplingRate
	 * The desired number of measurements per second can be set with measureRate
	 *
	 * measureRate: 		a value from 0 to 7 that decides about
	 * 						the number of measurements per second
	 * 						If this value equals n, the DPS310 will perform
	 * 						2^n measurements per second
	 * oversamplingRate: 	a value from 0 to 7 that decides about the precision
	 * 						of the measurements
	 * 						If this value equals m, the DPS310 will perform
	 * 						2^m internal measurements
	 * 						and combine the results to one more exact measurement
	 * returns: 			status code
	 * 	NOTE: 				If measure rate is n and oversampling rate is m,
	 * 						the DPS310 performs 2^(n+m) internal measurements per second.
	 * 						The DPS310 cannot operate with high precision and high speed
	 * 						at the same time.
	 * 						Consult the datasheet for more information.
	 */
	int16_t startMeasurePressureCont(uint8_t measureRate, uint8_t oversamplingRate);

	/**
	 * starts a continuous temperature and pressure measurement
	 * The desired precision can be set with tempOsr and prsOsr
	 * The desired number of measurements per second can be set with tempMr and prsMr
	 *
	 * tempMr				measure rate for temperature
	 * tempOsr				oversampling rate for temperature
	 * prsMr				measure rate for pressure
	 * prsOsr				oversampling rate for pressure
	 * returns: 			status code
	 * 	NOTE: 				High precision and speed for both temperature and pressure
	 * 						can not be reached at the same time.
	 * 						Estimated time for temperature and pressure measurement
	 * 						is the sum of both values.
	 * 						This sum must not be more than 1 second.
	 * 						Consult the datasheet for more information.
	 */
	int16_t startMeasureBothCont(uint8_t tempMr, uint8_t tempOsr, uint8_t prsMr, uint8_t prsOsr);

	/**
	 * Gets the interrupt status flag of the FIFO
	 *
	 * Returns: 	1 if the FIFO is full and caused an interrupt
	 * 				0 if the FIFO is not full or FIFO interrupt is disabled
	 * 				-1 on fail
	 */
	int16_t getIntStatusFifoFull(void);
	/**
	 * Gets the interrupt status flag that indicates a finished temperature measurement
	 *
	 * Returns: 	1 if a finished temperature measurement caused an interrupt
	 * 				0 if there is no finished temperature measurement
	 * 					or interrupts are disabled
	 * 				-1 on fail
	 */
	int16_t getIntStatusTempReady(void);
	/**
	 * Gets the interrupt status flag that indicates a finished pressure measurement
	 *
	 * Returns: 	1 if a finished pressure measurement caused an interrupt
	 * 				0 if there is no finished pressure measurement
	 * 					or interrupts are disabled
	 * 				-1 on fail
	 */
	int16_t getIntStatusPrsReady(void);

	/**
	 * Function to fix a hardware problem on some devices
	 * You have this problem if you measure a temperature which is too high (e.g. 60°C when temperature is around 20°C)
	 * Call correctTemp() directly after begin() to fix this issue
	 */
	int16_t correctTemp(void);

  protected:
	//scaling factor table
	static const int32_t scaling_facts[DPS__NUM_OF_SCAL_FACTS];

	Mode m_opMode;

	//flags
	uint8_t m_initFail;
	uint8_t m_productID;
	uint8_t m_revisionID;

	//settings
	uint8_t m_tempMr;
	uint8_t m_tempOsr;
	uint8_t m_prsMr;
	uint8_t m_prsOsr;
	uint8_t m_tempSensor;

	// compensation coefficients for both dps310 and dps422
	int32_t m_c00;
	int32_t m_c10;
	int32_t m_c01;
	int32_t m_c11;
	int32_t m_c20;
	int32_t m_c21;
	int32_t m_c30;
	//last measured scaled temperature
	//(necessary for pressure compensation)
	float m_lastTempScal;

	//bus specific
	uint8_t m_SpiI2c; //0=SPI, 1=I2C
					  //used for I2C
	TwoWire *m_i2cbus;
	uint8_t m_slaveAddress;
	//used for SPI
	SPIClass *m_spibus;
	int32_t m_chipSelect;
	uint8_t m_threeWire;

	/**
	 * Initializes the sensor.
	 * This function has to be called from begin()
	 * and requires a valid bus initialization.
	 */
	virtual void init(void) = 0;
	/**
	 * reads the compensation coefficients from the DPS310
	 * this is called once from init(), which is called from begin()
	 *
	 * returns: 	0 on success, -1 on fail
	 */
	virtual int16_t readcoeffs(void) = 0;

	/**
	 * Sets the Operation Mode of the Dps310
	 *
	 * opMode: 			the new OpMode that has to be set
	 * return: 			0 on success, -1 on fail
	 *
	 * NOTE!
	 * You cannot set background to 1 without setting temperature and pressure
	 * You cannot set both temperature and pressure when background mode is disabled
	 */
	virtual int16_t setOpMode(uint8_t opMode);
	/**
	 * Configures temperature measurement
	 *
	 * tempMr: 	the new measure rate for temperature
	 * 				This can be a value from 0U to 7U.
	 * 				Actual measure rate will be 2^tempMr,
	 * 				so this will be a value from 1 to 128.
	 * tempOsr: 	the new oversampling rate for temperature
	 * 				This can be a value from 0U to 7U.
	 * 				Actual measure rate will be 2^tempOsr,
	 * 				so this will be a value from 1 to 128.
	 * returns: 	0 normally or -1 on fail
	 */
	virtual int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
	/**
	 * Configures pressure measurement
	 *
	 * prsMr: 		the new measure rate for pressure
	 * 				This can be a value from 0U to 7U.
	 * 				Actual measure rate will be 2^prs_mr,
	 * 				so this will be a value from 1 to 128.
	 * prsOs: 	the new oversampling rate for temperature
	 * 				This can be a value from 0U to 7U.
	 * 				Actual measure rate will be 2^prsOsr,
	 * 				so this will be a value from 1 to 128.
	 * returns: 	0 normally or -1 on fail
	 */
	virtual int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);

	virtual int16_t flushFIFO() = 0;

	virtual float calcTemp(int32_t raw) = 0;

	virtual float calcPressure(int32_t raw) = 0;

	int16_t enableFIFO();

	int16_t disableFIFO();
	/**
	 * calculates the time that the DPS310 needs for 2^mr measurements
	 * with an oversampling rate of 2^osr (see table "pressure measurement time (ms) versus oversampling rate")
	 *
	 * mr: 		Measure rate for temperature or pressure
	 * osr: 	Oversampling rate for temperature or pressure
	 * returns: time that the DPS310 needs for this measurement
	 * 			a value of 10000 equals 1 second
	 * 	NOTE! 	The measurement time for temperature and pressure
	 * 			in sum must not be more than 1 second!
	 * 			Timing behavior of pressure and temperature sensors
	 * 			can be considered as equal.
	 */
	uint16_t calcBusyTime(uint16_t temp_rate, uint16_t temp_osr);

	/**
	 * reads the next raw value from the Dps310 FIFO
	 *
	 * value: 	address where the value will be written
	 * returns:	-1 on fail
	 * 			0 if result is a temperature raw value
	 * 			1 if result is a pressure raw value
	 */
	int16_t getFIFOvalue(int32_t *value, RegBlock_t reg);

	/**
	 * Gets the results from continuous measurements and writes them to given arrays
	 *
	 * *tempBuffer: 	The start address of the buffer where the temperature results
	 * 					are written
	 * 					If this is NULL, no temperature results will be written out
	 * &tempCount:		This has to be a reference to a number which contains
	 * 					the size of the buffer for temperature results.
	 * 					When the function ends, it will contain
	 * 					the number of bytes written to the buffer
	 * *prsBuffer: 		The start address of the buffer where the pressure results
	 * 					are written
	 * 					If this is NULL, no pressure results will be written out
	 * &prsCount:		This has to be a reference to a number which contains
	 * 					the size of the buffer for pressure results.
	 * 					When the function ends, it will contain
	 * 					the number of bytes written to the buffer
	 * returns:			status code
	 */
	int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount, RegMask_t reg);

	/**
	 * reads a byte from dps310
	 *
	 * regAdress: 	Address that has to be read
	 * returns: 	register content or -1 on fail
	 */
	int16_t readByte(uint8_t regAddress);
	/**
	 * reads a byte from dps310 via SPI
	 * this function is automatically called by readByte
	 * if Dps310 is connected via SPI
	 *
	 * regAdress: 	Address that has to be read
	 * returns: 	register content or -1 on fail
	 */
	int16_t readByteSPI(uint8_t regAddress);
	/**
	 * reads a block from dps310
	 *
	 * regAdress: 	Address that has to be read
	 * length: 		Length of data block
	 * buffer: 	Buffer where data will be stored
	 * returns: 	number of bytes that have been read successfully
	 * 				NOTE! This is not always equal to length
	 * 					  due to rx-Buffer overflow etc.
	 */
	int16_t readBlock(RegBlock_t regBlock, uint8_t *buffer);
	/**
	 * reads a block from dps310 via SPI
	 *
	 * regAdress: 	Address that has to be read
	 * length: 		Length of data block
	 * readbuffer: 	Buffer where data will be stored
	 * returns: 	number of bytes that have been read successfully
	 * 				NOTE! This is not always equal to length
	 * 					  due to rx-Buffer overflow etc.
	 */
	int16_t readBlockSPI(RegBlock_t regBlock, uint8_t *readbuffer);
	/**
	 * writes a given byte to a given register of dps310 without checking
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * data:		Byte that will be written to the register
	 * return:		0 if byte was written successfully
	 * 				or -1 on fail
	 */
	int16_t writeByte(uint8_t regAddress, uint8_t data);
	/**
	 * writes a given byte to a given register of dps310
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * data:		Byte that will be written to the register
	 * check: 		If this is true, register content will be read after writing
	 * 				to check if update was successful
	 * return:		0 if byte was written successfully
	 * 				or -1 on fail
	 */
	int16_t writeByte(uint8_t regAddress, uint8_t data, uint8_t check);
	/**
	 * writes a given byte to a given register of dps310 via SPI
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * data:		Byte that will be written to the register
	 * check: 		If this is true, register content will be read after writing
	 * 				to check if update was successful
	 * return:		0 if byte was written successfully
	 * 				or -1 on fail
	 */
	int16_t writeByteSpi(uint8_t regAddress, uint8_t data, uint8_t check);
	/**
	 * updates some given bits of a given register of dps310 without checking
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * data:		BitValues that will be written to the register
	 * shift:		Amount of bits the data byte is shifted (left) before being masked
	 * mask: 		Masks the bits of the register that have to be updated
	 * 				Bits with value 1 are updated
	 * 				Bits with value 0 are not changed
	 * return:		0 if byte was written successfully
	 * 				or -1 on fail
	 */
	int16_t writeByteBitfield(uint8_t data, RegMask_t regMask);
	/**
	 * updates some given bits of a given register of dps310
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * data:		BitValues that will be written to the register
	 * shift:		Amount of bits the data byte is shifted (left) before being masked
	 * mask: 		Masks the bits of the register that have to be updated
	 * 				Bits with value 1 are updated
	 * 				Bits with value 0 are not changed
	 * check: 		enables/disables check after writing
	 * 				0 disables check
	 * 				if check fails, -1 will be returned
	 * return:		0 if byte was written successfully
	 * 				or -1 on fail
	 */
	int16_t writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check);
	/**
	 * reads some given bits of a given register of dps310
	 *
	 * regAdress: 	Address of the register that has to be updated
	 * mask: 		Masks the bits of the register that have to be updated
	 * 				Bits masked with value 1 are read
	 * 				Bits masked with value 0 are set 0
	 * shift:		Amount of bits the data byte is shifted (right) after being masked
	 * return:		read and processed bits
	 * 				or -1 on fail
	 */
	int16_t readByteBitfield(RegMask_t regMask);

	//this construction recognizes non-32-bit negative numbers
	//and converts them to 32-bit negative numbers with 2's complement
	void getTwosComplement(int32_t *raw, uint8_t length);

	int16_t getRawResult(int32_t *raw, RegBlock_t reg);

	/**
	 * @brief Get the Raw Result object
	 * 
	 * @param raw 
	 * @param reg 
	 * @param isPrs 1 if the result is a pressure measurement, 0 is it is a temperature measurement. For DPS310 & DPS422
	 */
	int16_t getRawResult(int32_t *raw, RegBlock_t reg, bool *isPrs);
};

#endif //DPSCLASS_H_INCLUDED
