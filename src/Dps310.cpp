#include "Dps310.h"

const int32_t Dps310::scaling_facts[DPS310__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

//////// 		Constructor, Destructor, begin, end			////////

Dps310::Dps310(void)
{
	//assume that initialization has failed before it has been done
	m_initFail = 1U;

	dpsRegister = Dps310Register();
}

Dps310::~Dps310(void)
{
	end();
}

void Dps310::begin(TwoWire &bus)
{
	begin(bus, DPS310__STD_SLAVE_ADDRESS);
}

void Dps310::begin(TwoWire &bus, uint8_t slaveAddress)
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

void Dps310::begin(SPIClass &bus, int32_t chipSelect)
{
	begin(bus, chipSelect, 0U);
}

void Dps310::begin(SPIClass &bus, int32_t chipSelect, uint8_t threeWire)
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

void Dps310::end(void)
{
	standby();
}

////////		Declaration of other public functions starts here			////////

uint8_t Dps310::getProductId(void)
{
	return m_productID;
}

uint8_t Dps310::getRevisionId(void)
{
	return m_revisionID;
}

int16_t Dps310::standby(void)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//set device to idling mode
	int16_t ret = setOpMode(IDLE);
	if (ret != DPS310__SUCCEEDED)
	{
		return ret;
	}
	//flush the FIFO
	ret = writeByteBitfield(1U, dpsRegister.registers[FIFO_FL]);
	if (ret < 0)
	{
		return ret;
	}
	//disable the FIFO
	ret = writeByteBitfield(0U, dpsRegister.registers[FIFO_EN]);
	return ret;
}

int16_t Dps310::measureTempOnce(int32_t &result)
{
	return measureTempOnce(result, m_tempOsr);
}

int16_t Dps310::measureTempOnce(int32_t &result, uint8_t oversamplingRate)
{
	//Start measurement
	int16_t ret = startMeasureTempOnce(oversamplingRate);
	if (ret != DPS310__SUCCEEDED)
	{
		return ret;
	}

	//wait until measurement is finished
	delay(calcBusyTime(0U, m_tempOsr) / DPS310__BUSYTIME_SCALING);
	delay(DPS310__BUSYTIME_FAILSAFE);

	ret = getSingleResult(result);
	if (ret != DPS310__SUCCEEDED)
	{
		standby();
	}
	return ret;
}

int16_t Dps310::startMeasureTempOnce(void)
{
	return startMeasureTempOnce(m_tempOsr);
}

int16_t Dps310::startMeasureTempOnce(uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS310__FAIL_TOOBUSY;
	}

	if (oversamplingRate != m_tempOsr)
	{
		//configuration of oversampling rate
		if (configTemp(0U, oversamplingRate) != DPS310__SUCCEEDED)
		{
			return DPS310__FAIL_UNKNOWN;
		}
	}

	//set device to temperature measuring mode
	return setOpMode(0U, 1U, 0U);
}

int16_t Dps310::measurePressureOnce(int32_t &result)
{
	return measurePressureOnce(result, m_prsOsr);
}

int16_t Dps310::measurePressureOnce(int32_t &result, uint8_t oversamplingRate)
{
	//start the measurement
	int16_t ret = startMeasurePressureOnce(oversamplingRate);
	if (ret != DPS310__SUCCEEDED)
	{
		return ret;
	}

	//wait until measurement is finished
	delay(calcBusyTime(0U, m_prsOsr) / DPS310__BUSYTIME_SCALING);
	delay(DPS310__BUSYTIME_FAILSAFE);

	ret = getSingleResult(result);
	if (ret != DPS310__SUCCEEDED)
	{
		standby();
	}
	return ret;
}

int16_t Dps310::startMeasurePressureOnce(void)
{
	return startMeasurePressureOnce(m_prsOsr);
}

int16_t Dps310::startMeasurePressureOnce(uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS310__FAIL_TOOBUSY;
	}
	//configuration of oversampling rate, lowest measure rate to avoid conflicts
	if (oversamplingRate != m_prsOsr)
	{
		if (configPressure(0U, oversamplingRate))
		{
			return DPS310__FAIL_UNKNOWN;
		}
	}
	//set device to pressure measuring mode
	return setOpMode(0U, 0U, 1U);
}

int16_t Dps310::getSingleResult(int32_t &result)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}

	//read finished bit for current opMode
	int16_t rdy;
	switch (m_opMode)
	{
	case CMD_TEMP: //temperature
		rdy = readByteBitfield(dpsRegister.registers[TEMP_RDY]);
		break;
	case CMD_PRS: //pressure
		rdy = readByteBitfield(dpsRegister.registers[PRS_RDY]);
		break;
	default: //DPS310 not in command mode
		return DPS310__FAIL_TOOBUSY;
	}

	//read new measurement result
	switch (rdy)
	{
	case DPS310__FAIL_UNKNOWN: //could not read ready flag
		return DPS310__FAIL_UNKNOWN;
	case 0: //ready flag not set, measurement still in progress
		return DPS310__FAIL_UNFINISHED;
	case 1: //measurement ready, expected case
		Dps310::Mode oldMode = m_opMode;
		m_opMode = IDLE; //opcode was automatically reseted by DPS310
		switch (oldMode)
		{
		case CMD_TEMP:					 //temperature
			return getTemp(&result);	 //get and calculate the temperature value
		case CMD_PRS:					 //pressure
			return getPressure(&result); //get and calculate the pressure value
		default:
			return DPS310__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS310__FAIL_UNKNOWN;
}

int16_t Dps310::startMeasureTempCont(uint8_t measureRate, uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS310__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(measureRate, oversamplingRate) >= DPS310__MAX_BUSYTIME)
	{
		return DPS310__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configTemp(measureRate, oversamplingRate))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	//enable result FIFO
	if (writeByteBitfield(1U, dpsRegister.registers[FIFO_EN]))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (setOpMode(1U, 1U, 0U))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	return DPS310__SUCCEEDED;
}

int16_t Dps310::startMeasurePressureCont(uint8_t measureRate, uint8_t oversamplingRate)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS310__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(measureRate, oversamplingRate) >= DPS310__MAX_BUSYTIME)
	{
		return DPS310__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configPressure(measureRate, oversamplingRate))
		return DPS310__FAIL_UNKNOWN;
	//enable result FIFO
	if (writeByteBitfield(1U, dpsRegister.registers[FIFO_EN]))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (setOpMode(1U, 0U, 1U))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	return DPS310__SUCCEEDED;
}

int16_t Dps310::startMeasureBothCont(uint8_t tempMr,
									 uint8_t tempOsr,
									 uint8_t prsMr,
									 uint8_t prsOsr)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in idling mode
	if (m_opMode != IDLE)
	{
		return DPS310__FAIL_TOOBUSY;
	}
	//abort if speed and precision are too high
	if (calcBusyTime(tempMr, tempOsr) + calcBusyTime(prsMr, prsOsr) >= DPS310__MAX_BUSYTIME)
	{
		return DPS310__FAIL_UNFINISHED;
	}
	//update precision and measuring rate
	if (configTemp(tempMr, tempOsr))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	//update precision and measuring rate
	if (configPressure(prsMr, prsOsr))
		return DPS310__FAIL_UNKNOWN;
	//enable result FIFO
	if (writeByteBitfield(1U, dpsRegister.registers[FIFO_EN]))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (setOpMode(1U, 1U, 1U))
	{
		return DPS310__FAIL_UNKNOWN;
	}
	return DPS310__SUCCEEDED;
}

int16_t Dps310::getContResults(int32_t *tempBuffer,
							   uint8_t &tempCount,
							   int32_t *prsBuffer,
							   uint8_t &prsCount)
{
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	//abort if device is not in background mode
	if (!(m_opMode & INVAL_OP_CONT_NONE))
	{
		return DPS310__FAIL_TOOBUSY;
	}

	//prepare parameters for buffer length and count
	uint8_t tempLen = tempCount;
	uint8_t prsLen = prsCount;
	tempCount = 0U;
	prsCount = 0U;

	//while FIFO is not empty
	while (readByteBitfield(dpsRegister.registers[FIFO_EMPTY]) == 0)
	{
		int32_t result;
		//read next result from FIFO
		int16_t type = getFIFOvalue(&result);
		switch (type)
		{
		case 0: //temperature
			//calculate compensated pressure value
			result = calcTemp(result);
			//if buffer exists and is not full
			//write result to buffer and increase temperature result counter
			if (tempBuffer != NULL)
			{
				if (tempCount < tempLen)
				{
					tempBuffer[tempCount++] = result;
				}
			}
			break;
		case 1: //pressure
			//calculate compensated pressure value
			result = calcPressure(result);
			//if buffer exists and is not full
			//write result to buffer and increase pressure result counter
			if (prsBuffer != NULL)
			{
				if (prsCount < prsLen)
				{
					prsBuffer[prsCount++] = result;
				}
			}
			break;
		case -1:   //read failed
			break; //continue while loop
				   //if connection failed permanently,
				   //while condition will become false
				   //if read failed only once, loop will try again
		}
	}
	return DPS310__SUCCEEDED;
}

int16_t Dps310::setInterruptPolarity(uint8_t polarity)
{
	//Interrupts are not supported with 4 Wire SPI
	if (!m_SpiI2c & !m_threeWire)
	{
		return DPS310__FAIL_UNKNOWN;
	}
	return writeByteBitfield(polarity, dpsRegister.registers[INT_HL]);
}

int16_t Dps310::setInterruptSources(bool fifoFull, bool tempReady, bool prsReady)
{
	//Interrupts are not supported with 4 Wire SPI
	if (!m_SpiI2c & !m_threeWire)
	{
		return DPS310__FAIL_UNKNOWN;
	}

	writeByteBitfield(fifoFull, dpsRegister.registers[INT_EN_FIFO]);
	writeByteBitfield(tempReady, dpsRegister.registers[INT_EN_TEMP]);
	writeByteBitfield(prsReady, dpsRegister.registers[INT_EN_PRS]);
}

int16_t Dps310::getIntStatusFifoFull(void)
{
	return readByteBitfield(dpsRegister.registers[INT_FLAG_FIFO]);
}

int16_t Dps310::getIntStatusTempReady(void)
{
	return readByteBitfield(dpsRegister.registers[INT_FLAG_TEMP]);
}

int16_t Dps310::getIntStatusPrsReady(void)
{
	return readByteBitfield(dpsRegister.registers[INT_FLAG_PRS]);
}

int16_t Dps310::correctTemp(void)
{
	if (m_initFail)
	{
		return DPS310__FAIL_INIT_FAILED;
	}
	writeByte(0x0E, 0xA5);
	writeByte(0x0F, 0x96);
	writeByte(0x62, 0x02);
	writeByte(0x0E, 0x00);
	writeByte(0x0F, 0x00);

	//perform a first temperature measurement (again)
	//the most recent temperature will be saved internally
	//and used for compensation when calculating pressure
	int32_t trash;
	measureTempOnce(trash);

	return DPS310__SUCCEEDED;
}

//////// 	Declaration of private functions starts here	////////

void Dps310::init(void)
{
	int16_t prodId = readByteBitfield(dpsRegister.registers[PROD_ID]);
	if (prodId != DPS310__PROD_ID)
	{
		//Connected device is not a Dps310
		m_initFail = 1U;
		return;
	}
	m_productID = prodId;

	int16_t revId = readByteBitfield(dpsRegister.registers[REV_ID]);
	if (revId < 0)
	{
		m_initFail = 1U;
		return;
	}
	m_revisionID = revId;

	//find out which temperature sensor is calibrated with coefficients...
	int16_t sensor = readByteBitfield(dpsRegister.registers[TEMP_SENSORREC]);
	if (sensor < 0)
	{
		m_initFail = 1U;
		return;
	}

	//...and use this sensor for temperature measurement
	m_tempSensor = sensor;
	if (writeByteBitfield((uint8_t)sensor, dpsRegister.registers[TEMP_SENSOR]) < 0)
	{
		m_initFail = 1U;
		return;
	}

	//read coefficients
	if (readcoeffs() < 0)
	{
		m_initFail = 1U;
		return;
	}

	//set to standby for further configuration
	standby();

	//set measurement precision and rate to standard values;
	configTemp(DPS310__TEMP_STD_MR, DPS310__TEMP_STD_OSR);
	configPressure(DPS310__PRS_STD_MR, DPS310__PRS_STD_OSR);

	//perform a first temperature measurement
	//the most recent temperature will be saved internally
	//and used for compensation when calculating pressure
	int32_t trash;
	measureTempOnce(trash);

	//make sure the DPS310 is in standby after initialization
	standby();

	// Fix IC with a fuse bit problem, which lead to a wrong temperature
	// Should not affect ICs without this problem
	correctTemp();
}

int16_t Dps310::readcoeffs(void)
{
	// TODO: remove magic number
	uint8_t buffer[18];
	//read COEF registers to buffer
	int16_t ret = readBlock(dpsRegister.registerBlocks[COEF],
							buffer);

	//compose coefficients from buffer content
	m_c0Half = ((uint32_t)buffer[0] << 4) | (((uint32_t)buffer[1] >> 4) & 0x0F);
	//this construction recognizes non-32-bit negative numbers
	//and converts them to 32-bit negative numbers with 2's complement
	if (m_c0Half & ((uint32_t)1 << 11))
	{
		m_c0Half -= (uint32_t)1 << 12;
	}
	//c0 is only used as c0*0.5, so c0_half is calculated immediately
	m_c0Half = m_c0Half / 2U;

	//now do the same thing for all other coefficients
	m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
	if (m_c1 & ((uint32_t)1 << 11))
	{
		m_c1 -= (uint32_t)1 << 12;
	}

	m_c00 = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (((uint32_t)buffer[5] >> 4) & 0x0F);
	if (m_c00 & ((uint32_t)1 << 19))
	{
		m_c00 -= (uint32_t)1 << 20;
	}

	m_c10 = (((uint32_t)buffer[5] & 0x0F) << 16) | ((uint32_t)buffer[6] << 8) | (uint32_t)buffer[7];
	if (m_c10 & ((uint32_t)1 << 19))
	{
		m_c10 -= (uint32_t)1 << 20;
	}

	m_c01 = ((uint32_t)buffer[8] << 8) | (uint32_t)buffer[9];
	if (m_c01 & ((uint32_t)1 << 15))
	{
		m_c01 -= (uint32_t)1 << 16;
	}

	m_c11 = ((uint32_t)buffer[10] << 8) | (uint32_t)buffer[11];
	if (m_c11 & ((uint32_t)1 << 15))
	{
		m_c11 -= (uint32_t)1 << 16;
	}

	m_c20 = ((uint32_t)buffer[12] << 8) | (uint32_t)buffer[13];
	if (m_c20 & ((uint32_t)1 << 15))
	{
		m_c20 -= (uint32_t)1 << 16;
	}

	m_c21 = ((uint32_t)buffer[14] << 8) | (uint32_t)buffer[15];
	if (m_c21 & ((uint32_t)1 << 15))
	{
		m_c21 -= (uint32_t)1 << 16;
	}

	m_c30 = ((uint32_t)buffer[16] << 8) | (uint32_t)buffer[17];
	if (m_c30 & ((uint32_t)1 << 15))
	{
		m_c30 -= (uint32_t)1 << 16;
	}

	return DPS310__SUCCEEDED;
}

int16_t Dps310::setOpMode(bool background, bool temperature, bool pressure)
{
	// TODO: change uint8_t to bool
	uint8_t opMode = (background & DPS310__LSB) << 2U | (temperature & DPS310__LSB) << 1U | (pressure & DPS310__LSB);
	return setOpMode(opMode);
}

int16_t Dps310::setOpMode(uint8_t opMode)
{
	//Filter invalid OpModes
	if (opMode == INVAL_OP_CMD_BOTH || opMode == INVAL_OP_CONT_NONE)
	{
		return DPS310__FAIL_UNKNOWN;
	}

	if (writeByteBitfield(opMode, dpsRegister.registers[OPMODE]) == -1)
	{
		return DPS310__FAIL_UNKNOWN;
	}
	m_opMode = (Dps310::Mode)opMode;
	return DPS310__SUCCEEDED;
}

int16_t Dps310::configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	// TODO: check range

	int16_t ret = writeByteBitfield(tempMr, dpsRegister.registers[TEMP_MR]);
	ret = writeByteBitfield(tempOsr, dpsRegister.registers[TEMP_OSR]);

	//abort immediately on fail
	if (ret != DPS310__SUCCEEDED)
	{
		return DPS310__FAIL_UNKNOWN;
	}

	//set TEMP SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (tempOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, dpsRegister.registers[TEMP_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, dpsRegister.registers[TEMP_SE]);
	}

	if (ret == DPS310__SUCCEEDED)
	{ //save new settings
		m_tempMr = tempMr;
		m_tempOsr = tempOsr;
	}
	else
	{
		//try to rollback on fail avoiding endless recursion
		//this is to make sure that shift enable and oversampling rate
		//are always consistent
		if (tempMr != m_tempMr || tempOsr != m_tempOsr)
		{
			configTemp(m_tempMr, m_tempOsr);
		}
	}
	return ret;
}

int16_t Dps310::configPressure(uint8_t prsMr, uint8_t prsOsr)
{
	// TODO: range check

	int16_t ret = writeByteBitfield(prsMr, dpsRegister.registers[PRS_MR]);
	ret = writeByteBitfield(prsOsr, dpsRegister.registers[PRS_OSR]);

	//abort immediately on fail
	if (ret != DPS310__SUCCEEDED)
	{
		return DPS310__FAIL_UNKNOWN;
	}

	//set PM SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (prsOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, dpsRegister.registers[PRS_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, dpsRegister.registers[PRS_SE]);
	}

	if (ret == DPS310__SUCCEEDED)
	{ //save new settings
		m_prsMr = prsMr;
		m_prsOsr = prsOsr;
	}
	else
	{ //try to rollback on fail avoiding endless recursion
		//this is to make sure that shift enable and oversampling rate
		//are always consistent
		if (prsMr != m_prsMr || prsOsr != m_prsOsr)
		{
			configPressure(m_prsMr, m_prsOsr);
		}
	}
	return ret;
}

uint16_t Dps310::calcBusyTime(uint16_t mr, uint16_t osr)
{
	// TODO: check range
	//formula from datasheet (optimized)
	return ((uint32_t)20U << mr) + ((uint32_t)16U << (osr + mr));
}

int16_t Dps310::getTemp(int32_t *result)
{
	uint8_t buffer[3] = {0};
	//read raw pressure data to buffer

	int16_t i = readBlock(dpsRegister.registerBlocks[TEMP],
						  buffer);

	//compose raw temperature value from buffer
	int32_t temp = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
	//recognize non-32-bit negative numbers
	//and convert them to 32-bit negative numbers using 2's complement
	if (temp & ((uint32_t)1 << 23))
	{
		temp -= (uint32_t)1 << 24;
	}

	//return temperature
	*result = calcTemp(temp);
	return DPS310__SUCCEEDED;
}

int16_t Dps310::getPressure(int32_t *result)
{
	uint8_t buffer[3] = {0};
	//read raw pressure data to buffer
	int16_t i = readBlock(dpsRegister.registerBlocks[PRS],
						  buffer);

	//compose raw pressure value from buffer
	int32_t prs = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
	//recognize non-32-bit negative numbers
	//and convert them to 32-bit negative numbers using 2's complement
	if (prs & ((uint32_t)1 << 23))
	{
		prs -= (uint32_t)1 << 24;
	}

	*result = calcPressure(prs);
	return DPS310__SUCCEEDED;
}

int16_t Dps310::getFIFOvalue(int32_t *value)
{
	//abort on invalid argument
	if (value == NULL)
	{
		return DPS310__FAIL_UNKNOWN;
	}

	// TODO: init buffer
	uint8_t buffer[3] = {0};
	//always read from pressure raw value register
	int16_t i = readBlock(dpsRegister.registerBlocks[PRS],
						  buffer);
	//compose raw pressure value from buffer
	*value = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
	//recognize non-32-bit negative numbers
	//and convert them to 32-bit negative numbers using 2's complement
	if (*value & ((uint32_t)1 << 23))
	{
		*value -= (uint32_t)1 << 24;
	}

	//least significant bit shows measurement type
	return buffer[2] & DPS310__LSB;
}

int32_t Dps310::calcTemp(int32_t raw)
{
	double temp = raw;

	//scale temperature according to scaling table and oversampling
	temp /= scaling_facts[m_tempOsr];

	//update last measured temperature
	//it will be used for pressure compensation
	m_lastTempScal = temp;

	//Calculate compensated temperature
	temp = m_c0Half + m_c1 * temp;

	//return temperature
	return (int32_t)temp;
}

int32_t Dps310::calcPressure(int32_t raw)
{
	double prs = raw;

	//scale pressure according to scaling table and oversampling
	prs /= scaling_facts[m_prsOsr];

	//Calculate compensated pressure
	prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));

	//return pressure
	return (int32_t)prs;
}

int16_t Dps310::readByte(uint8_t regAddress)
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
		return DPS310__FAIL_UNKNOWN; //if 0 bytes were read successfully
	}
}

int16_t Dps310::readByteSPI(uint8_t regAddress)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS310__FAIL_UNKNOWN;
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

int16_t Dps310::readBlock(RegBlock_t regBlock, uint8_t *buffer)
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

int16_t Dps310::readBlockSPI(RegBlock_t regBlock, uint8_t *buffer)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS310__FAIL_UNKNOWN;
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

int16_t Dps310::writeByte(uint8_t regAddress, uint8_t data)
{
	return writeByte(regAddress, data, 0U);
}

int16_t Dps310::writeByte(uint8_t regAddress, uint8_t data, uint8_t check)
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
		return DPS310__FAIL_UNKNOWN;
	}
	else
	{
		if (check == 0)
			return 0;					  //no checking
		if (readByte(regAddress) == data) //check if desired by calling function
		{
			return DPS310__SUCCEEDED;
		}
		else
		{
			return DPS310__FAIL_UNKNOWN;
		}
	}
}

int16_t Dps310::writeByteSpi(uint8_t regAddress, uint8_t data, uint8_t check)
{
	//this function is only made for communication via SPI
	if (m_SpiI2c != 0)
	{
		return DPS310__FAIL_UNKNOWN;
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
		return DPS310__SUCCEEDED;
	}
	//checking necessary
	if (readByte(regAddress) == data)
	{
		//check passed
		return DPS310__SUCCEEDED;
	}
	else
	{
		//check failed
		return DPS310__FAIL_UNKNOWN;
	}
}

int16_t Dps310::writeByteBitfield(uint8_t data, RegMask_t regMask)
{
	return writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

int16_t Dps310::writeByteBitfield(uint8_t data,
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

int16_t Dps310::readByteBitfield(RegMask_t regMask)
{
	int16_t ret = readByte(regMask.regAddress);
	if (ret < 0)
	{
		return ret;
	}
	return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}
