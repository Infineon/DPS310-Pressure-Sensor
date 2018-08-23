/**
 * @brief 
 * 
 * Temperature measurements must be enabled for the DPS422 to compensate for temperature drift in the pressure measurement.
 * @file Dps422.h
 * @author Infineon Technologies
 * @date 2018-08-22
 */

#ifndef DPS422_H_INCLUDED
#define DPS422_H_INCLUDED
#include "DpsClass.h"
#include "util/dps422_consts.h"

#define DPS422_NUM_OF_REGMASKS 30

class Dps422 : public DpsClass
{
  public:
    int16_t getSingleResult(float &result);
    int16_t getContResults(float *tempBuffer, uint8_t &tempCount, float *prsBuffer, uint8_t &prsCount);
    int16_t setInterruptPolarity(uint8_t polarity);
    int16_t setInterruptSources(bool fifoFull, bool tempReady, bool prsReady);
    int16_t getIntStatusFifoFull(void);
    int16_t getIntStatusTempReady(void);
    int16_t getIntStatusPrsReady(void);
    int16_t measureBothOnce(float &prs, float &temp); // might make sense to declare in base class for future sensors

  protected:
    //compensation coefficients (for simplicity use 32 bits)
    float a_prime;
    float b_prime;
    int32_t m_c02;
    int32_t m_c12;

    int32_t last_raw_temp = 0; // for pressure compensation

    enum Registers_e
    {
        // measurement config
        TEMP_MR,  // temperature measure rate
        TEMP_OSR, // temperature measurement resolution
        PRS_MR,   // pressure measure rate
        PRS_OSR,  // pressure measurement resolution
        MSR_CTRL, // measurement control
        // flags
        TEMP_RDY,      // temperature ready flag in continuous mode
        PRS_RDY,       // pressure ready flag in continuous mode
        CONT_FLAG,     // continuous mode flag
        INIT_DONE,     // set when initialisation procedure is complete
        TEMP_RDY_INTR, // temperature ready flag in intrrupt mode
        PRS_RDY_INTR,  // pressure ready flag in intrrupt mode
        // interrupt config
        INTR_SEL, // interrupt select
        INTR_POL, // interrupt active polarity
        // fifo config
        WM,              // watermark level
        FIFO_EN,         // FIFO enable
        FIFO_FL,         // FIFO flush
        FIFO_EMPTY,      // FIFO empty
        FIFO_FULL,       // if FIFO is full or reaches watermark level
        FIFO_FULL_CONF,  // Configures FIFO behaviour when full
        FIFO_FILL_LEVEL, //contains the number of pressure and/or temperature measurements currently stored in FIFO
        // misc
        PROD_ID = 0,
        REV_ID,
        SPI_MODE, // 4- or 3-wire SPI
        SOFT_RESET,
        MUST_SET, // bit 7 of TEMP_CFG, according to datasheet should always be set
    };

    RegMask_t registers[DPS422_NUM_OF_REGMASKS] = {
        // measurement config
        {0x07, 0x70, 4}, // TEMP_MR
        {0x07, 0x07, 0}, // TEMP_OSR
        {0x06, 0x70, 4}, // PRS_MR
        {0x06, 0x07, 0}, // PRS_OSR
        {0x08, 0x07, 0}, // MSR_CTRL
        // flags
        {0x08, 0x20, 5}, // TEMP_RDY
        {0x08, 0x10, 4}, // PRS_RDY
        {0x08, 0x40, 6}, // CONT_FLAG
        {0x08, 0x80, 7}, // INIT_DONE
        {0x0A, 0x02, 1}, // TEMP_RDY_INTR
        {0x0A, 0x01, 0}, // PRS_RDY_INTR
        // interrupt config
        {0x09, 0xF0, 4}, // INTR_SEL
        {0x09, 0x80, 3}, // INTR_POL
        // /fifo config
        {0x0B, 0x1F, 0}, // WM
        {0x09, 0x02, 1}, // FIFO_EN
        {0x0D, 0x80, 7}, // FIFO_FL
        {0x0C, 0x01, 0}, // FIFO_EMPTY
        {0x0C, 0x02, 1}, // FIFO_FULL
        {0x09, 0x04, 2}, // FIFO_FULL_CONF
        {0x0C, 0xFC, 2}, // FIFO_FILL_LEVEL
        // misc
        {0x1D, 0x0F, 0}, // PROD_ID
        {0x1D, 0xF0, 0}, // REV_ID
        {0x09, 0x01, 0}, // SPI_MODE
        {0x0D, 0x0F, 0}, // SOFT_RESET
        {0x07, 0x80, 7}, // MUST_SET
    };
    enum RegisterBlocks_e
    {
        PRS = 0,   // pressure value
        TEMP,      // temperature value
        COEF_TEMP, // compensation coefficients
        COEF_PRS,
    };

    RegBlock_t registerBlocks[4] = {
        {0x00, 3},
        {0x03, 3},
        {0x20, 3},
        {0x26, 20},
    };

    void init(void);
    int16_t configTemp(uint8_t temp_mr, uint8_t temp_osr);
    int16_t configPressure(uint8_t prs_mr, uint8_t prs_osr);
    int16_t readcoeffs(void);
    int16_t setOpMode(uint8_t opMode);
    int16_t enableFIFO();
    int16_t disableFIFO();
    float calcTemp(int32_t raw);
    // in case temperature reading disabled, the default raw temperature value correspond the reference temperature of 27 degress.
    float calcPressure(int32_t raw, int32_t raw_temp = 91400);
};

#endif