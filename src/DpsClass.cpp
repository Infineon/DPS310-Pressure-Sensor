#include "DpsClass.h"

const int32_t DpsClass::scaling_facts[DPS__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

//////// 		Constructor, Destructor, begin, end			////////

DpsClass::DpsClass(void)
{
	//assume that initialization has failed before it has been done
	m_initFail = 1U;
}

DpsClass::~DpsClass(void)
{
	end();
}

void DpsClass::begin(TwoWire &bus)
{
	begin(bus, DPS310__STD_SLAVE_ADDRESS);
}

void DpsClass::begin(TwoWire &bus, uint8_t slaveAddress)
{
	//this flag will show if the initialization was successful
	m_initFail = 0U;

	//Set I2C bus connection
	m_SpiI2c = 1U;
	m_i2cbus = &bus;
	m_slaveAddress = slaveAddress;

	// Init bus
	m_i2cbus->begin();

	delay(50); //startup time of Dps310

	init();
}

void DpsClass::begin(SPIClass &bus, int32_t chipSelect)
{
	begin(bus, chipSelect, 0U);
}

void DpsClass::begin(SPIClass &bus, int32_t chipSelect, uint8_t threeWire)
{
	//this flag will show if the initialization was successful
	m_initFail = 0U;

	//Set SPI bus connection
	m_SpiI2c = 0U;
	m_spibus = &bus;
	m_chipSelect = chipSelect;

	// Init bus
	m_spibus->begin();
	m_spibus->setDataMode(SPI_MODE3);

	pinMode(m_chipSelect, OUTPUT);
	digitalWrite(m_chipSelect, HIGH);

	delay(50); //startup time of Dps310

	//switch to 3-wire mode if necessary
	//do not use writeByteBitfield or check option to set SPI mode!
	//Reading is not possible until SPI-mode is valid
	if (threeWire)
	{
		m_threeWire = 1U;
		if (writeByte(DPS310__REG_ADR_SPI3W, DPS310__REG_CONTENT_SPI3W))
		{
			m_initFail = 1U;
			return;
		}
	}

	init();
}

void DpsClass::end(void)
{
	standby();
}

////////		Declaration of other public functions starts here			////////

uint8_t DpsClass::getProductId(void)
{
	return m_productID;
}

uint8_t DpsClass::getRevisionId(void)
{
	return m_revisionID;
}

int16_t DpsClass::measureTempOnce(float &result)
{
	return measureTempOnce(result, m_tempOsr);
}

int16_t DpsClass::measureTempOnce(float &result, uint8_t oversamplingRate)
{
	//Start measurement
	int16_t ret = startMeasureTempOnce(oversamplingRate);
	if (ret != DPS__SUCCEEDED)
	{
		return ret;
	}

	//wait until measurement is finished
	delay(calcBusyTime(0U, m_tempOsr) / DPS310__BUSYTIME_SCALING);
	delay(DPS310__BUSYTIME_FAILSAFE);

	ret = getSingleResult(result);
	if (ret != DPS__SUCCEEDED)
	{
		standby();
	}
	return ret;
}

int16_t DpsClass::startMeasureTempOnce(void)
{
	return startMeasureTempOnce(m_tempOsr);
}

int16_t DpsClass::startMeasureTempOnce(uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS__FAIL_TOOBUSY;
	}

	if (oversamplingRate != m_tempOsr)
	{
		//configuration of oversampling rate
		if (configTemp(0U, oversamplingRate) != DPS__SUCCEEDED)
		{
			return DPS__FAIL_UNKNOWN;
		}
	}

	//set device to temperature measuring mode
	return setOpMode(CMD_TEMP);
}

int16_t DpsClass::measurePressureOnce(float &result)
{
	return measurePressureOnce(result, m_prsOsr);
}

int16_t DpsClass::measurePressureOnce(float &result, uint8_t oversamplingRate)
{
	//start the measurement
	int16_t ret = startMeasurePressureOnce(oversamplingRate);
	if (ret != DPS__SUCCEEDED)
	{
		return ret;
	}

	//wait until measurement is finished
	delay(calcBusyTime(0U, m_prsOsr) / DPS310__BUSYTIME_SCALING);
	delay(DPS310__BUSYTIME_FAILSAFE);

	ret = getSingleResult(result);
	if (ret != DPS__SUCCEEDED)
	{
		standby();
	}
	return ret;
}

int16_t DpsClass::startMeasurePressureOnce(void)
{
	return startMeasurePressureOnce(m_prsOsr);
}

int16_t DpsClass::startMeasurePressureOnce(uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS__FAIL_TOOBUSY;
	}
	//configuration of oversampling rate, lowest measure rate to avoid conflicts
	if (oversamplingRate != m_prsOsr)
	{
		if (configPressure(0U, oversamplingRate))
		{
			return DPS__FAIL_UNKNOWN;
		}
	}
	//set device to pressure measuring mode
	return setOpMode(CMD_PRS);
}

int16_t DpsClass::startMeasureTempCont(uint8_t measureRate, uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(measureRate, oversamplingRate) >= DPS310__MAX_BUSYTIME)
	{
		return DPS__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configTemp(measureRate, oversamplingRate))
	{
		return DPS__FAIL_UNKNOWN;
	}

	if (enableFIFO())
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (DpsClass::setOpMode(CONT_TMP))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t DpsClass::startMeasurePressureCont(uint8_t measureRate, uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(measureRate, oversamplingRate) >= DPS310__MAX_BUSYTIME)
	{
		return DPS__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configPressure(measureRate, oversamplingRate))
		return DPS__FAIL_UNKNOWN;
	//enable result FIFO
	if (enableFIFO())
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (DpsClass::setOpMode(CONT_PRS))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t DpsClass::startMeasureBothCont(uint8_t tempMr,
									   uint8_t tempOsr,
									   uint8_t prsMr,
									   uint8_t prsOsr)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(tempMr, tempOsr) + calcBusyTime(prsMr, prsOsr) >= DPS310__MAX_BUSYTIME)
	{
		return DPS__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configTemp(tempMr, tempOsr))
	{
		return DPS__FAIL_UNKNOWN;
	}
	//update precision and measuring rate
	if (configPressure(prsMr, prsOsr))
		return DPS__FAIL_UNKNOWN;
	//enable result FIFO
	if (enableFIFO())
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (setOpMode(CONT_BOTH))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t DpsClass::standby(void)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//set device to idling mode
	int16_t ret = setOpMode(IDLE);
	if (ret != DPS__SUCCEEDED)
	{
		return ret;
	}
	ret = disableFIFO();
	return ret;
}

//////// 	Declaration of private functions starts here	////////

uint16_t DpsClass::calcBusyTime(uint16_t mr, uint16_t osr)
{
	// TODO: check range
	//formula from datasheet (optimized)
	return ((uint32_t)20U << mr) + ((uint32_t)16U << (osr + mr));
}

int16_t DpsClass::readByte(uint8_t regAddress)
{
	//delegate to specialized function if Dps310 is connected via SPI
	if (m_SpiI2c == 0)
	{
		return readByteSPI(regAddress);
	}

	m_i2cbus->beginTransmission(m_slaveAddress);
	m_i2cbus->write(regAddress);
	m_i2cbus->endTransmission(0);
	//request 1 byte from slave
	if (m_i2cbus->requestFrom(m_slaveAddress, 1U, 1U) > 0)
	{
		return m_i2cbus->read(); //return this byte on success
	}
	else
	{
		return DPS__FAIL_UNKNOWN; //if 0 bytes were read successfully
	}
}

int16_t DpsClass::readByteSPI(uint8_t regAddress)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS__FAIL_UNKNOWN;
	}
	//mask regAddress
	regAddress &= ~DPS310__SPI_RW_MASK;
	//reserve and initialize bus
	m_spibus->beginTransaction(SPISettings(DPS310__SPI_MAX_FREQ,
										   MSBFIRST,
										   SPI_MODE3));
	//enable ChipSelect for Dps310
	digitalWrite(m_chipSelect, LOW);
	//send address with read command to Dps310
	m_spibus->transfer(regAddress | DPS310__SPI_READ_CMD);
	//receive register content from Dps310
	uint8_t ret = m_spibus->transfer(0xFF); //send a dummy byte while receiving
	//disable ChipSelect for Dps310
	digitalWrite(m_chipSelect, HIGH);
	//close current SPI transaction
	m_spibus->endTransaction();
	//return received data
	return ret;
}

int16_t DpsClass::readBlockSPI(RegBlock_t regBlock, uint8_t *buffer)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS__FAIL_UNKNOWN;
	}
	//do not read if there is no buffer
	if (buffer == NULL)
	{
		return 0; //0 bytes were read successfully
	}
	//mask regAddress
	regBlock.regAddress &= ~DPS310__SPI_RW_MASK;
	//reserve and initialize bus
	m_spibus->beginTransaction(SPISettings(DPS310__SPI_MAX_FREQ,
										   MSBFIRST,
										   SPI_MODE3));
	//enable ChipSelect for Dps310
	digitalWrite(m_chipSelect, LOW);
	//send address with read command to Dps310
	m_spibus->transfer(regBlock.regAddress | DPS310__SPI_READ_CMD);

	//receive register contents from Dps310
	for (uint8_t count = 0; count < regBlock.length; count++)
	{
		buffer[count] = m_spibus->transfer(0xFF); //send a dummy byte while receiving
	}

	//disable ChipSelect for Dps310
	digitalWrite(m_chipSelect, HIGH);
	//close current SPI transaction
	m_spibus->endTransaction();
	//return received data
	return regBlock.length;
}

int16_t DpsClass::writeByte(uint8_t regAddress, uint8_t data)
{
	return writeByte(regAddress, data, 0U);
}

int16_t DpsClass::writeByte(uint8_t regAddress, uint8_t data, uint8_t check)
{
	//delegate to specialized function if Dps310 is connected via SPI
	if (m_SpiI2c == 0)
	{
		return writeByteSpi(regAddress, data, check);
	}
	m_i2cbus->beginTransmission(m_slaveAddress);
	m_i2cbus->write(regAddress);		  //Write Register number to buffer
	m_i2cbus->write(data);				  //Write data to buffer
	if (m_i2cbus->endTransmission() != 0) //Send buffer content to slave
	{
		return DPS__FAIL_UNKNOWN;
	}
	else
	{
		if (check == 0)
			return 0;					  //no checking
		if (readByte(regAddress) == data) //check if desired by calling function
		{
			return DPS__SUCCEEDED;
		}
		else
		{
			return DPS__FAIL_UNKNOWN;
		}
	}
}

int16_t DpsClass::writeByteSpi(uint8_t regAddress, uint8_t data, uint8_t check)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS__FAIL_UNKNOWN;
	}
	//mask regAddress
	regAddress &= ~DPS310__SPI_RW_MASK;
	//reserve and initialize bus
	m_spibus->beginTransaction(SPISettings(DPS310__SPI_MAX_FREQ,
										   MSBFIRST,
										   SPI_MODE3));
	//enable ChipSelect for Dps310
	digitalWrite(m_chipSelect, LOW);
	//send address with read command to Dps310
	m_spibus->transfer(regAddress | DPS310__SPI_WRITE_CMD);

	//write register content from Dps310
	m_spibus->transfer(data);

	//disable ChipSelect for Dps310
	digitalWrite(m_chipSelect, HIGH);
	//close current SPI transaction
	m_spibus->endTransaction();

	//check if necessary
	if (check == 0)
	{
		//no checking necessary
		return DPS__SUCCEEDED;
	}
	//checking necessary
	if (readByte(regAddress) == data)
	{
		//check passed
		return DPS__SUCCEEDED;
	}
	else
	{
		//check failed
		return DPS__FAIL_UNKNOWN;
	}
}

int16_t DpsClass::writeByteBitfield(uint8_t data, RegMask_t regMask)
{
	return writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

int16_t DpsClass::writeByteBitfield(uint8_t data,
									uint8_t regAddress,
									uint8_t mask,
									uint8_t shift,
									uint8_t check)
{
	int16_t old = readByte(regAddress);
	if (old < 0)
	{
		//fail while reading
		return old;
	}
	return writeByte(regAddress, ((uint8_t)old & ~mask) | ((data << shift) & mask), check);
}

int16_t DpsClass::readByteBitfield(RegMask_t regMask)
{
	int16_t ret = readByte(regMask.regAddress);
	if (ret < 0)
	{
		return ret;
	}
	return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}

int16_t DpsClass::readBlock(RegBlock_t regBlock, uint8_t *buffer)
{
	// TODO: add length check

	//delegate to specialized function if Dps310 is connected via SPI
	if (m_SpiI2c == 0)
	{
		return readBlockSPI(regBlock, buffer);
	}
	//do not read if there is no buffer
	if (buffer == NULL)
	{
		return 0; //0 bytes read successfully
	}

	m_i2cbus->beginTransmission(m_slaveAddress);
	m_i2cbus->write(regBlock.regAddress);
	m_i2cbus->endTransmission(0);
	//request length bytes from slave
	int16_t ret = m_i2cbus->requestFrom(m_slaveAddress, regBlock.length, 1U);
	//read all received bytes to buffer
	for (int16_t count = 0; count < ret; count++)
	{
		buffer[count] = m_i2cbus->read();
	}
	return ret;
}

void DpsClass::getTwosComplement(int32_t *raw, uint8_t length)
{
	if (*raw & ((uint32_t)1 << (length - 1)))
	{
		*raw -= (uint32_t)1 << length;
	}
}

void DpsClass::getRawResult(int32_t *raw, RegBlock_t reg)
{
	uint8_t buffer[3] = {0};
	int16_t i = readBlock(reg, buffer);
	*raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];

	getTwosComplement(raw, 24);
}

void DpsClass::getRawResult(int32_t *raw, RegBlock_t reg, bool *isPrs)
{
	uint8_t buffer[3] = {0};
	int16_t i = readBlock(reg, buffer);
	*raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];

	getTwosComplement(raw, 24);
	*isPrs = buffer[2] & 0x01;
}