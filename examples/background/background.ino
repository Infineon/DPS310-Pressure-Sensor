// uncomment to use DPS422
// #define DPS422

// uncomment to use DPS_SPI
// #define DPS_SPI

#ifdef DPS422
#include <Dps422.h>
Dps422 DigitalPressureSensor = Dps422();
#else
#include <Dps310.h>
Dps310 DigitalPressureSensor = Dps310();
#endif

#define CONT_MEAS_BUFFER_SIZE 20
float pressure[CONT_MEAS_BUFFER_SIZE];
float temperature[CONT_MEAS_BUFFER_SIZE];
uint8_t temperatureCount = 0;
uint8_t pressureCount = 0;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
#ifdef DPS_SPI
  // slave select pin has to be given
  DigitalPressureSensor.begin(SPI, PIN_SPI_SS);
#else
  //Call begin to initialize DigitalPressureSensor
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  //DigitalPressureSensor.begin(Wire, 0x76);
  //Use the commented line below instead to use the default I2C address.
  DigitalPressureSensor.begin(Wire);  
#endif

  int16_t ret = DigitalPressureSensor.startMeasureBothCont(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_4, DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_4);
  //Use one of the commented lines below instead to measure only temperature or pressure
  //int16_t ret = DigitalPressureSensor.startMeasureTempCont(temp_mr, temp_osr);
  //int16_t ret = DigitalPressureSensor.startMeasurePressureCont(prs_mr, prs_osr);

  if (ret != 0)
  {
    Serial.print("Init FAILED! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.println("Init complete!");
  }
}

void loop()
{
  //This function writes the results of continuous measurements to the arrays given as parameters
  //The parameters temperatureCount and pressureCount should hold the sizes of the arrays temperature and pressure when the function is called
  //After the end of the function, temperatureCount and pressureCount hold the numbers of values written to the arrays
  //Note: The Dps310 cannot save more than 32 results. When its result buffer is full, it won't save any new measurement results
  int16_t ret = DigitalPressureSensor.getContResults(temperature, temperatureCount, pressure, pressureCount);

  if (ret != 0)
  {
    Serial.println();
    Serial.println();
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.println();
    Serial.println();
    Serial.print(temperatureCount);
    Serial.println(" temperature values found: ");
    for (int16_t i = 0; i < temperatureCount; i++)
    {
      Serial.print(temperature[i]);
      Serial.println(" degrees of Celsius");
    }

    Serial.println();
    Serial.print(pressureCount);
    Serial.println(" pressure values found: ");
    for (int16_t i = 0; i < pressureCount; i++)
    {
      Serial.print(pressure[i]);
      Serial.println(" Pascal");
    }
  }

  //Wait some time, so that the Dps310 can refill its buffer
  delay(5000);
}