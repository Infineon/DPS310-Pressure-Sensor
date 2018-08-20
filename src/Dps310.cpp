#include "Dps310.h"
int16_t Dps310::standby(void)
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
	//flush the FIFO
	ret = writeByteBitfield(1U, registers[FIFO_FL]);
	if (ret < 0)
	{
		return ret;
	}
	//disable the FIFO
	ret = writeByteBitfield(0U, registers[FIFO_EN]);
	return ret;
}

int16_t Dps310::getSingleResult(int32_t &result)
{
	//abort if initialization failed
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}

	//read finished bit for current opMode
	int16_t rdy;
	switch (m_opMode)
	{
	case CMD_TEMP: //temperature
		rdy = readByteBitfield(registers[TEMP_RDY]);
		break;
	case CMD_PRS: //pressure
		rdy = readByteBitfield(registers[PRS_RDY]);
		break;
	default: //DPS310 not in command mode
		return DPS__FAIL_TOOBUSY;
	}

	//read new measurement result
	switch (rdy)
	{
	case DPS__FAIL_UNKNOWN: //could not read ready flag
		return DPS__FAIL_UNKNOWN;
	case 0: //ready flag not set, measurement still in progress
		return DPS__FAIL_UNFINISHED;
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
			return DPS__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS__FAIL_UNKNOWN;
}

int16_t Dps310::startMeasureTempCont(uint8_t measureRate, uint8_t oversamplingRate)
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
	//enable result FIFO
	if (writeByteBitfield(1U, registers[FIFO_EN]))
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (DpsClass::setOpMode(1U, 1U, 0U))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t Dps310::startMeasurePressureCont(uint8_t measureRate, uint8_t oversamplingRate)
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
	if (writeByteBitfield(1U, registers[FIFO_EN]))
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (DpsClass::setOpMode(1U, 0U, 1U))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t Dps310::startMeasureBothCont(uint8_t tempMr,
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
	if (writeByteBitfield(1U, registers[FIFO_EN]))
	{
		return DPS__FAIL_UNKNOWN;
	}
	//Start measuring in background mode
	if (DpsClass::setOpMode(1U, 1U, 1U))
	{
		return DPS__FAIL_UNKNOWN;
	}
	return DPS__SUCCEEDED;
}

int16_t Dps310::getContResults(int32_t *tempBuffer,
							   uint8_t &tempCount,
							   int32_t *prsBuffer,
							   uint8_t &prsCount)
{
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in background mode
	if (!(m_opMode & INVAL_OP_CONT_NONE))
	{
		return DPS__FAIL_TOOBUSY;
	}

	//prepare parameters for buffer length and count
	uint8_t tempLen = tempCount;
	uint8_t prsLen = prsCount;
	tempCount = 0U;
	prsCount = 0U;

	//while FIFO is not empty
	while (readByteBitfield(registers[FIFO_EMPTY]) == 0)
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
	return DPS__SUCCEEDED;
}

int16_t Dps310::setInterruptPolarity(uint8_t polarity)
{
	//Interrupts are not supported with 4 Wire SPI
	if (!m_SpiI2c & !m_threeWire)
	{
		return DPS__FAIL_UNKNOWN;
	}
	return writeByteBitfield(polarity, registers[INT_HL]);
}

int16_t Dps310::setInterruptSources(bool fifoFull, bool tempReady, bool prsReady)
{
	//Interrupts are not supported with 4 Wire SPI
	if (!m_SpiI2c & !m_threeWire)
	{
		return DPS__FAIL_UNKNOWN;
	}

	writeByteBitfield(fifoFull, registers[INT_EN_FIFO]);
	writeByteBitfield(tempReady, registers[INT_EN_TEMP]);
	writeByteBitfield(prsReady, registers[INT_EN_PRS]);
}

int16_t Dps310::getIntStatusFifoFull(void)
{
	return readByteBitfield(registers[INT_FLAG_FIFO]);
}

int16_t Dps310::getIntStatusTempReady(void)
{
	return readByteBitfield(registers[INT_FLAG_TEMP]);
}

int16_t Dps310::getIntStatusPrsReady(void)
{
	return readByteBitfield(registers[INT_FLAG_PRS]);
}

void Dps310::init(void)
{
	// int16_t prodId = readByteBitfield(registers[PROD_ID]);
	// if (prodId < 0)
	// {
	// 	//Connected device is not a Dps310
	// 	m_initFail = 1U;
	// 	return;
	// }
	// m_productID = prodId;

	// if (m_productID == DPS310__PROD_ID)
	// {
	// 	dpsRegister = Dps310Register();
	// }
	// else if (m_productID == DPS422__PROD_ID)
	// {
	// 	dpsRegister = Dps422Register();
	// }

	int16_t revId = readByteBitfield(registers[REV_ID]);
	if (revId < 0)
	{
		m_initFail = 1U;
		return;
	}
	m_revisionID = revId;

	//find out which temperature sensor is calibrated with coefficients...
	int16_t sensor = readByteBitfield(registers[TEMP_SENSORREC]);
	if (sensor < 0)
	{
		m_initFail = 1U;
		return;
	}

	//...and use this sensor for temperature measurement
	m_tempSensor = sensor;
	if (writeByteBitfield((uint8_t)sensor, registers[TEMP_SENSOR]) < 0)
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
	int16_t ret = readBlock(registerBlocks[COEF],
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

	return DPS__SUCCEEDED;
}

int16_t Dps310::getFIFOvalue(int32_t *value)
{
	//abort on invalid argument
	if (value == NULL)
	{
		return DPS__FAIL_UNKNOWN;
	}

	// TODO: init buffer
	uint8_t buffer[3] = {0};
	//always read from pressure raw value register
	int16_t i = readBlock(registerBlocks[PRS],
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
	return buffer[2] & 0x01;
}

int16_t Dps310::getTemp(int32_t *result)
{
	uint8_t buffer[3] = {0};
	//read raw pressure data to buffer

	int16_t i = readBlock(registerBlocks[TEMP],
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
	return DPS__SUCCEEDED;
}

int16_t Dps310::setOpMode(uint8_t opMode)
{
	//Filter invalid OpModes
	if (opMode == INVAL_OP_CMD_BOTH || opMode == INVAL_OP_CONT_NONE)
	{
		return DPS__FAIL_UNKNOWN;
	}

	if (writeByteBitfield(opMode, registers[OPMODE]) == -1)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_opMode = (Dps310::Mode)opMode;
	return DPS__SUCCEEDED;
}

int16_t Dps310::configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	// TODO: check range

	int16_t ret = writeByteBitfield(tempMr, registers[TEMP_MR]);
	ret = writeByteBitfield(tempOsr, registers[TEMP_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}

	//set TEMP SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (tempOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, registers[TEMP_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, registers[TEMP_SE]);
	}

	if (ret == DPS__SUCCEEDED)
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

	int16_t ret = writeByteBitfield(prsMr, registers[PRS_MR]);
	ret = writeByteBitfield(prsOsr, registers[PRS_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}

	//set PM SHIFT ENABLE if oversampling rate higher than eight(2^3)
	if (prsOsr > DPS310__OSR_SE)
	{
		ret = writeByteBitfield(1U, registers[PRS_SE]);
	}
	else
	{
		ret = writeByteBitfield(0U, registers[PRS_SE]);
	}

	if (ret == DPS__SUCCEEDED)
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

int16_t Dps310::getPressure(int32_t *result)
{
	uint8_t buffer[3] = {0};
	//read raw pressure data to buffer
	int16_t i = readBlock(registerBlocks[PRS],
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
	return DPS__SUCCEEDED;
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