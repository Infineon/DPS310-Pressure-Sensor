#include <Dps310.h>

// Dps310 Opject
Dps310 DigitalPressureSensor = Dps310();
float temperature = 0;
float pressure = 0;
int16_t oversampling = 7;
int16_t ret;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  //Call begin to initialize DigitalPressureSensor
  //The parameter pin_nr is the number of the CS pin on your Microcontroller
  DigitalPressureSensor.begin(SPI, PIN_SPI_SS);

  Serial.println("Init complete!");
}

void loop()
{
  Serial.println();

  //lets the Dps310 perform a Single temperature measurement with the last (or standard) configuration
  //The result will be written to the paramerter temperature
  //ret = DigitalPressureSensor.measureTempOnce(temperature);
  //the commented line below does exactly the same as the one above, but you can also config the precision
  //oversampling can be a value from 0 to 7
  //the Dps 310 will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
  //measurements with higher precision take more time, consult datasheet for more information
  ret = DigitalPressureSensor.measureTempOnce(temperature, oversampling);

  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" degrees of Celsius");
  }

  //ret = DigitalPressureSensor.measurePressureOnce(pressure);
  ret = DigitalPressureSensor.measurePressureOnce(pressure, oversampling);
  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pascal");
  }

  //Wait some time
  delay(1000);
}
