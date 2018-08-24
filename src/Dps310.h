#ifndef DPS310_H_INCLUDED
#define DPS310_H_INCLUDED

#include "DpsClass.h"
#include "util/dps310_config.h"

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