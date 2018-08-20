#ifndef DPSREGISTER_H_INCLUDED
#define DPSREGISTER_H_INCLUDED

#include <Arduino.h>
#define MAX_NUM_OF_REGMASKS 50
#define MAX_NUM_OF_REGBLOCKS 5

typedef struct
{
    uint8_t regAddress;
    uint8_t mask;
    uint8_t shift;
} RegMask_t;

typedef struct
{
    uint8_t regAddress;
    uint8_t length;
} RegBlock_t;

// namespace 

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

enum RegisterBlocks_e
{
    PRS,  // pressure value
    TEMP, // temperature value
    COEF, // compensation coefficients
};

#endif