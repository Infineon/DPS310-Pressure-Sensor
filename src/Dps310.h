#ifndef DPS310_H_INCLUDED
#define DPS310_H_INCLUDED

#include "DpsClass.h"

#define DPS310_NUM_OF_REGMASKS 24

class Dps310 : public DpsClass
{
  public:
    int16_t getSingleResult(float &result);
    int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount);
    int16_t setInterruptPolarity(uint8_t polarity);
    int16_t setInterruptSources(bool fifoFull, bool tempReady, bool prsReady);
    int16_t getIntStatusFifoFull(void);
    int16_t getIntStatusTempReady(void);
    int16_t getIntStatusPrsReady(void);

  protected:
    //compensation coefficients
    int32_t m_c0Half;
    int32_t m_c1;

    enum Registers_e
    {
        PROD_ID = 0,
        REV_ID,
        OPMODE,
        TEMP_MR,        // temperature measure rate
        TEMP_OSR,       // temperature oversampling rate
        TEMP_SENSOR,    // internal vs external
        TEMP_SENSORREC, //temperature sensor recommendation
        TEMP_SE,        //temperature shift enable (if temp_osr>3)
        PRS_MR,         //pressure measure rate
        PRS_OSR,        //pressure oversampling rate
        PRS_SE,         //pressure shift enable (if prs_osr>3)
        TEMP_RDY,       //temperature ready flag
        PRS_RDY,        //pressure ready flag
        FIFO_EN,        //FIFO enable
        FIFO_FL,        //FIFO flush
        FIFO_EMPTY,     //FIFO empty
        FIFO_FULL,      //FIFO full
        INT_HL,
        INT_EN_FIFO, //INT FIFO enable
        INT_EN_TEMP,
        INT_EN_PRS,
        INT_FLAG_FIFO,
        INT_FLAG_TEMP,
        INT_FLAG_PRS
    };

    RegMask_t registers[DPS310_NUM_OF_REGMASKS] = {
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

    enum RegisterBlocks_e
    {
        PRS = 0, // pressure value
        TEMP,    // temperature value
        COEF,    // compensation coefficients
    };

    RegBlock_t registerBlocks[3] = {
        {0x00, 3},
        {0x03, 3},
        {0x10, 18},
    };

    void init(void);
    int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
    int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);
    int16_t readcoeffs(void);
    int16_t setOpMode(uint8_t opMode);

    int16_t enableFIFO();
    int16_t disableFIFO();

    float calcTemp(int32_t raw);
    float calcPressure(int32_t raw);
};

#endif