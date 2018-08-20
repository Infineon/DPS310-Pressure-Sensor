#ifndef DPS422_H_INCLUDED
#define DPS422_H_INCLUDED
#include "DpsClass.h"

class Dps310 : public DpsClass
{
  public:
    RegMask_t registers[MAX_NUM_OF_REGMASKS] = {
        {,,}; // PROD_ID
        {,,}; // REV_ID
        {,,}; // OPMODE
        {,,}; // TEMP_MR
        {,,}; // TEMP_OSR
        {,,}; // TEMP_SENSOR
        {,,}; // TEMP_SENSORREC
        {,,}; // TEMP_SE
        {,,}; // PRS_MR
        {,,}; // PRS_OSR
        {,,}; // PRS_SE
        {,,}; // TEMP_RDY
        {,,}; // PRS_RDY
        {,,}; // FIFO_EN
        {,,}; // FIFO_FL
        {,,}; // FIFO_EMPTY
        {,,}; // FIFO_FULL
        {,,}; // INT_HL
        {,,}; // INT_EN_FIFO
        {,,}; // INT_EN_TEMP
        {,,}; // INT_EN_PRS
        {,,}; // INT_FLAG_FIFO
        {,,}; // INT_FLAG_TEMP
        {,,}; // INT_FLAG_PRS
    };

    RegBlock_t registerBlocks[MAX_NUM_OF_REGBLOCKS] = {
        {, },
        {, },
        {, },
    };
};

#endif