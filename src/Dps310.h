#ifndef DPS310_H_INCLUDED
#define DPS310_H_INCLUDED

#include "DpsClass.h"

#define DPS310_NUM_OF_REGMASKS 16

enum Interrupt_source_310_e
{
    DPS310_NO_INTR = 0,
    DPS310_PRS_INTR = 1,
    DPS310_TEMP_INTR = 2,
    DPS310_BOTH_INTR = 3,
    DPS310_FIFO_FULL_INTR = 4,
};

class Dps310 : public DpsClass
{
  public:
    int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount);

    /**
     * @brief Set the Interrupt Sources object
     * 
     * @param intr_source should be chosen from Interrupt_source_310_e
     * @param polarity 
     * @return int16_t 
     */
    int16_t setInterruptSources(uint8_t intr_source, uint8_t polarity = 1);

  protected:
  
    //compensation coefficients
    int32_t m_c0Half;
    int32_t m_c1;

    enum Registers_e
    {
        PROD_ID = 0,
        REV_ID,
        TEMP_SENSOR,    // internal vs external
        TEMP_SENSORREC, //temperature sensor recommendation
        TEMP_SE,        //temperature shift enable (if temp_osr>3)
        PRS_SE,         //pressure shift enable (if prs_osr>3)
        FIFO_FL,        //FIFO flush
        FIFO_EMPTY,     //FIFO empty
        FIFO_FULL,      //FIFO full
        INT_HL,
        INT_EN_FIFO, //INT FIFO enable
        INT_EN_TEMP,
        INT_EN_PRS,
    };

    RegMask_t registers[DPS310_NUM_OF_REGMASKS] = {
        {0x0D, 0x0F, 0}, // PROD_ID
        {0x0D, 0xF0, 4}, // REV_ID
        {0x07, 0x80, 7}, // TEMP_SENSOR
        {0x28, 0x80, 7}, // TEMP_SENSORREC
        {0x09, 0x08, 3}, // TEMP_SE
        {0x09, 0x04, 2}, // PRS_SE
        {0x0C, 0x80, 7}, // FIFO_FL
        {0x0B, 0x01, 0}, // FIFO_EMPTY
        {0x0B, 0x02, 1}, // FIFO_FULL
        {0x09, 0x80, 7}, // INT_HL
        {0x09, 0x40, 6}, // INT_EN_FIFO
        {0x09, 0x20, 5}, // INT_EN_TEMP
        {0x09, 0x10, 4}, // INT_EN_PRS
    };

    RegBlock_t coeffBlock = {0x10, 18};

    /////// implement pure virtual functions ///////

    void init(void);
    int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
    int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);
    int16_t readcoeffs(void);
    int16_t flushFIFO();
    float calcTemp(int32_t raw);
    float calcPressure(int32_t raw);
};

#endif