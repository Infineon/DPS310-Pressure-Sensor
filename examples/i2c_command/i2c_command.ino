#include <Dps310.h>

// Dps310 Opject
Dps310 DigitalPressureSensor = Dps310();
float temperature;
float pressure;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  //Call begin to initialize DigitalPressureSensor
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  //DigitalPressureSensor.begin(Wire, 0x76);
  //Use the commented line below instead of the one above to use the default I2C address.
  //if you are using the Pressure 3 click Board, you need 0x76
  DigitalPressureSensor.begin(Wire);

  Serial.println("Init complete!");
}

void loop()
{
  int16_t ret;
  Serial.println();

  //ret = DigitalPressureSensor.measureTempOnce(temperature);
  ret = DigitalPressureSensor.measureTempOnce(temperature, DPS__OVERSAMPLING_RATE_128);

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

  //Pressure measurement behaves like temperature measurement
  //ret = DigitalPressureSensor.measurePressureOnce(pressure);
  ret = DigitalPressureSensor.measurePressureOnce(pressure, DPS__OVERSAMPLING_RATE_128);
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
  delay(500);
}
