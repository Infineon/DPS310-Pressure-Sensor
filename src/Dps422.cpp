#include "Dps422.h"

////////   public  /////////

int16_t Dps422::measureBothOnce(float &prs, float &temp)
{
	setOpMode(CMD_BOTH);
	delay((calcBusyTime(0U, m_tempOsr) + (calcBusyTime(0U, m_prsOsr)) / DPS__BUSYTIME_SCALING));
	int16_t rdy = readByteBitfield(registers[PRS_RDY]) & readByteBitfield(registers[TEMP_RDY]);
	switch (rdy)
	{
	case DPS__FAIL_UNKNOWN: //could not read ready flag
		return DPS__FAIL_UNKNOWN;
	case 0: //ready flag not set, measurement still in progress
		standby();
		return DPS__FAIL_UNFINISHED;
	case 1: //measurement ready, expected case
		m_opMode = IDLE;
		int32_t raw_val_temp;
		int32_t raw_val_psr;
		getRawResult(&raw_val_temp, registerBlocks[TEMP]);
		getRawResult(&raw_val_psr, registerBlocks[PRS]);
		prs = calcPressure(raw_val_psr, raw_val_temp);
		temp = calcTemp(raw_val_temp);
		return DPS__SUCCEEDED; // TODO
	}
}

int16_t Dps422::getSingleResult(float &result)
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
		int32_t raw_val;
		switch (oldMode)
		{
		case CMD_TEMP: //temperature
			getRawResult(&raw_val, registerBlocks[TEMP]);
			result = calcTemp(raw_val);
			last_raw_temp = result;
			return DPS__SUCCEEDED; // TODO
		case CMD_PRS:			   //pressure
			getRawResult(&raw_val, registerBlocks[PRS]);
			result = calcPressure(raw_val, last_raw_temp);
			return DPS__SUCCEEDED; // TODO
		default:
			return DPS__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS__FAIL_UNKNOWN;
}

int16_t Dps422::getContResults(float *tempBuffer,
							   uint8_t &tempCount,
							   float *prsBuffer,
							   uint8_t &prsCount)
{
	// code duplication
	if (m_initFail)
	{
		return DPS__FAIL_INIT_FAILED;
	}
	//abort if device is not in background mode
	if (!(m_opMode & 0x04))
	{
		return DPS__FAIL_TOOBUSY;
	}

	if (!tempBuffer || !prsBuffer)
	{
		return DPS__FAIL_UNKNOWN;
	}

	// change
	uint8_t tempLen = tempCount;
	uint8_t prsLen = prsCount;
	tempCount = 0U;
	prsCount = 0U;

	//while FIFO is not empty
	while (readByteBitfield(registers[FIFO_EMPTY]) == 0)
	{
		int32_t raw_result;
		float result;
		//read next result from FIFO
		int16_t type = getFIFOvalue(&raw_result, registerBlocks[PRS]);
		switch (type)
		{
		case 0: //temperature
			if (tempCount < tempLen)
			{
				result = calcTemp(raw_result);
				tempBuffer[tempCount++] = result;
				last_raw_temp = result;
			}
			break;
		case 1: //pressure
			if (prsCount < prsLen)
			{
				result = calcPressure(raw_result, last_raw_temp);
				prsBuffer[prsCount++] = result;
			}
			break;
		case -1: //read failed
			break;
		}
	}
	return DPS__SUCCEEDED;
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

////////   private  /////////
void Dps422::init(void)
{
	readcoeffs();
	standby();
	writeByteBitfield(0x01, registers[MUST_SET]);
	configTemp(DPS310__TEMP_STD_MR, DPS310__TEMP_STD_OSR);
	configPressure(DPS310__PRS_STD_MR, DPS310__PRS_STD_OSR);
	correctTemp();
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
	// two accesses to the same register; for readability
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

int16_t Dps422::readcoeffs(void)
{
	uint8_t buffer_temp[3];
	uint8_t buffer_prs[20];
	readBlock(registerBlocks[COEF_TEMP], buffer_temp);
	readBlock(registerBlocks[COEF_PRS], buffer_prs);

	// refer to datasheet
	// 1. read T_Vbe, T_dVbe and T_gain
	int32_t t_gain = buffer_temp[0];													 // 8 bits
	int32_t t_dVbe = (uint32_t)buffer_temp[1] >> 1;										 // 7 bits
	int32_t t_Vbe = ((uint32_t)buffer_temp[1] & 0x01) | ((uint32_t)buffer_temp[2] << 1); // 9 bits

	getTwosComplement(&t_gain, 8);
	getTwosComplement(&t_dVbe, 7);
	getTwosComplement(&t_Vbe, 9);

	// 2. Vbe, dVbe and Aadc
	float Vbe = t_Vbe * 1.05031e-4 + 0.463232422;
	float dVbe = t_dVbe * 1.25885e-5 + 0.04027621;
	float Aadc = t_gain * 8.4375e-5 + 0.675;
	// 3. Vbe_cal and dVbe_cal
	float Vbe_cal = Vbe / Aadc;
	float dVbe_cal = dVbe / Aadc;
	// 4. T_calib
	float T_calib = DPS422_A_0 * dVbe_cal - 273.15;
	// 5. Vbe_cal(T_ref): Vbe value at reference temperature
	float Vbe_cal_tref = Vbe_cal - (T_calib - DPS422_T_REF) * DPS422_T_C_VBE;
	// 6. alculate PTAT correction coefficient
	float k_ptat = (DPS422_V_BE_TARGET - Vbe_cal_tref) * DPS422_K_PTAT_CORNER + DPS422_K_PTAT_CURVATURE;
	// 7. calculate A' and B'
	a_prime = DPS422_A_0 * (Vbe_cal + DPS422_ALPHA * dVbe_cal) * (1 + k_ptat);
	b_prime = -273.15 * (1 + k_ptat) - k_ptat * T_calib;

	// c00, c01, c02, c10 : 20 bits
	// c11, c12: 17 bits
	// c20: 15 bits; c21: 14 bits; c30 12 bits
	m_c00 = ((uint32_t)buffer_prs[0] << 12) | ((uint32_t)buffer_prs[1] << 4) | (((uint32_t)buffer_prs[2] & 0xF0) >> 4);
	m_c10 = ((uint32_t)(buffer_prs[2] & 0x0F) << 16) | ((uint32_t)buffer_prs[3] << 8) | (uint32_t)buffer_prs[4];
	m_c01 = ((uint32_t)buffer_prs[5] << 12) | ((uint32_t)buffer_prs[6] << 4) | (((uint32_t)buffer_prs[7] & 0xF0) >> 4);
	m_c02 = ((uint32_t)(buffer_prs[7] & 0x0F) << 16) | ((uint32_t)buffer_prs[8] << 8) | (uint32_t)buffer_prs[9];
	m_c20 = ((uint32_t)(buffer_prs[10] & 0x7F) << 8) | (uint32_t)buffer_prs[11];
	m_c30 = ((uint32_t)(buffer_prs[12] & 0x0F) << 8) | (uint32_t)buffer_prs[13];
	m_c11 = ((uint32_t)buffer_prs[14] << 9) | ((uint32_t)buffer_prs[15] << 1) | (((uint32_t)buffer_prs[16] & 0x80) >> 7);
	m_c12 = (((uint32_t)buffer_prs[16] & 0x7F) << 10) | ((uint32_t)buffer_prs[17] << 2) | (((uint32_t)buffer_prs[18] & 0xC0) >> 6);
	m_c21 = (((uint32_t)buffer_prs[18] & 0x3F) << 8) | ((uint32_t)buffer_prs[19]);

	getTwosComplement(&m_c00, 20);
	getTwosComplement(&m_c01, 20);
	getTwosComplement(&m_c02, 20);
	getTwosComplement(&m_c10, 20);
	getTwosComplement(&m_c11, 17);
	getTwosComplement(&m_c12, 17);
	getTwosComplement(&m_c20, 15);
	getTwosComplement(&m_c21, 14);
	getTwosComplement(&m_c30, 12);

	return DPS__SUCCEEDED;
}

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

float Dps422::calcTemp(int32_t raw)
{
	float t_cal = raw;
	t_cal /= 1048576;
	float u = t_cal / (1 + DPS422_ALPHA * t_cal);
	return (a_prime * u + b_prime);
}

float Dps422::calcPressure(int32_t raw_prs, int32_t raw_temp)
{
	// TODO: pressure calculation requires temperature values - what happens if temp reading disabled?
	float prs = raw_prs;
	prs /= scaling_facts[m_prsOsr];

	float tempx = raw_temp;
	tempx /= 1048576;
	float temp = (8.5 * tempx) / (1 + 8.8 * tempx);

	prs = m_c00 + m_c10 * prs + m_c01 * temp + m_c20 * prs * prs + m_c02 * temp * temp + m_c30 * prs * prs * prs +
		  m_c11 * temp * prs + m_c12 * prs * temp * temp + m_c21 * prs * prs * temp;
	return prs;
}