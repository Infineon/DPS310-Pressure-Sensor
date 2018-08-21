#include "Dps422.h"

Dps422::Dps422()
{
	registerBlocks[PRS] = {0x00, 3};
	registerBlocks[TEMP] = {0x03, 3};
	registerBlocks[COEF] = {0x20, 23};
}

int16_t Dps422::getSingleResult(int32_t &result)
{
	int16_t rdy;
	switch (m_opMode)
	{
	case CMD_TEMP: //temperature
		rdy = readByteBitfield(registers[TEMP_RDY]);
		break;
	case CMD_PRS: //pressure
		rdy = readByteBitfield(registers[PRS_RDY]);
		break;
	case CMD_BOTH:
		rdy = readByteBitfield(registers[PRS_RDY]) & readByteBitfield(registers[TEMP_RDY]);
	default: //not in command mode
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
		DpsClass::Mode oldMode = m_opMode;
		m_opMode = IDLE; //opcode was automatically reseted by DPS310
		switch (oldMode)
		{
		case CMD_TEMP:										  //temperature
			return getTemp(&result, registerBlocks[TEMP]);	//get and calculate the temperature value
		case CMD_PRS:										  //pressure
			return getPressure(&result, registerBlocks[PRS]); //get and calculate the pressure value
		case CMD_BOTH:
			return 0; // TODO
		default:
			return DPS__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS__FAIL_UNKNOWN;
}

int16_t Dps422::getContResults(int32_t *tempBuffer,
							   uint8_t &tempCount,
							   int32_t *prsBuffer,
							   uint8_t &prsCount)
{
}

int16_t Dps422::setInterruptPolarity(uint8_t polarity)
{
}

int16_t Dps422::setInterruptSources(bool fifoFull, bool tempReady, bool prsReady)
{
}

int16_t Dps422::getIntStatusFifoFull(void)
{
}
int16_t Dps422::getIntStatusTempReady(void)
{
}

int16_t Dps422::getIntStatusPrsReady(void)
{
}

void Dps422::init(void)
{
	standby();
	configTemp(DPS310__TEMP_STD_MR, DPS310__TEMP_STD_OSR);
	configPressure(DPS310__PRS_STD_MR, DPS310__PRS_STD_OSR);
}

int16_t Dps422::getFIFOvalue(int32_t *value)
{
}

int16_t Dps422::setOpMode(uint8_t opMode)
{
	if (writeByteBitfield(opMode, registers[MSR_CTRL]) == -1)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_opMode = (DpsClass::Mode)opMode;
	return DPS__SUCCEEDED;
}

int16_t Dps422::configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	int16_t ret = writeByteBitfield(tempMr, registers[TEMP_MR]);
	ret = writeByteBitfield(tempOsr, registers[TEMP_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_tempMr = tempMr;
	m_tempOsr = tempOsr;
}

int16_t Dps422::configPressure(uint8_t prsMr, uint8_t prsOsr)
{
	int16_t ret = writeByteBitfield(prsMr, registers[PRS_MR]);
	ret = writeByteBitfield(prsOsr, registers[PRS_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_prsMr = prsMr;
	m_prsOsr = prsOsr;
}

int16_t Dps422::readcoeffs(void) {}

int16_t Dps422::enableFIFO()
{
	return writeByteBitfield(1U, registers[FIFO_EN]);
}
int16_t Dps422::disableFIFO()
{
	int16_t ret = writeByteBitfield(1U, registers[FIFO_FL]);
	if (ret < 0)
	{
		return ret;
	}
	return writeByteBitfield(0U, registers[FIFO_EN]);
}