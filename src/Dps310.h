#ifndef DPS310_H_INCLUDED
#define DPS310_H_INCLUDED

#include "DpsClass.h"

class Dps310 : public DpsClass
{
  public:
    RegMask_t registers[MAX_NUM_OF_REGMASKS] = {
        {0x0D, 0x0F, 0}, // PROD_ID
        {0x0D, 0xF0, 4}, // REV_ID
        {0x08, 0x07, 0}, // OPMODE
        {0x07, 0x70, 4}, // TEMP_MR
        {0x07, 0x07, 0}, // TEMP_OSR
        {0x07, 0x80, 7}, // TEMP_SENSOR
        {0x28, 0x80, 7}, // TEMP_SENSORREC
        {0x09, 0x08, 3}, // TEMP_SE
        {0x06, 0x70, 4}, // PRS_MR
        {0x06, 0x07, 0}, // PRS_OSR
        {0x09, 0x04, 2}, // PRS_SE
        {0x08, 0x20, 5}, // TEMP_RDY
        {0x08, 0x10, 4}, // PRS_RDY
        {0x09, 0x02, 1}, // FIFO_EN
        {0x0C, 0x80, 7}, // FIFO_FL
        {0x0B, 0x01, 0}, // FIFO_EMPTY
        {0x0B, 0x02, 1}, // FIFO_FULL
        {0x09, 0x80, 7}, // INT_HL
        {0x09, 0x40, 6}, // INT_EN_FIFO
        {0x09, 0x20, 5}, // INT_EN_TEMP
        {0x09, 0x10, 4}, // INT_EN_PRS
        {0x0A, 0x04, 2}, // INT_FLAG_FIFO
        {0x0A, 0x02, 1}, // INT_FLAG_TEMP
        {0x0A, 0x01, 0}, // INT_FLAG_PRS
    };

    RegBlock_t registerBlocks[MAX_NUM_OF_REGBLOCKS] = {
        {0x00, 3},
        {0x03, 3},
        {0x10, 18},
    };

    int16_t standby(void);
    int16_t getSingleResult(int32_t &result);
    int16_t startMeasureTempCont(uint8_t measureRate, uint8_t oversamplingRate);
    int16_t startMeasurePressureCont(uint8_t measureRate, uint8_t oversamplingRate);
    int16_t startMeasureBothCont(uint8_t tempMr, uint8_t tempOsr, uint8_t prsMr, uint8_t prsOsr);
    int16_t getContResults(int32_t *tempBuffer, uint8_t &tempCount, int32_t *prsBuffer, uint8_t &prsCount);
    int16_t setInterruptPolarity(uint8_t polarity);
    int16_t setInterruptSources(bool fifoFull, bool tempReady, bool prsReady);
    int16_t getIntStatusFifoFull(void);
    int16_t getIntStatusTempReady(void);
    int16_t getIntStatusPrsReady(void);

  protected:
    void init(void);
    int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
    int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);
    int16_t readBlock(RegBlock_t regBlock, uint8_t *buffer);
    int16_t readcoeffs(void);
    int16_t setOpMode(uint8_t opMode);
    int16_t getTemp(int32_t *result);
    int16_t getPressure(int32_t *result);
    int16_t getFIFOvalue(int32_t *value);
};

#endif