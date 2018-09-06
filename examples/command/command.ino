/* Note: even though this example works for both DPS310 and DPS422, it does not support using DPS422 with a high oversampling frequency; 
 * the example i2c_command_422.ino should be preferred if you want higher accuracy than the default setting. 
*/

// uncomment to use DPS_SPI
// #define DPS_SPI

// uncomment to use DPS422
// #define DPS422

#ifdef DPS422
#include <Dps422.h>
Dps422 DigitalPressureSensor = Dps422();
#else
#include <Dps310.h>
Dps310 DigitalPressureSensor = Dps310();
#endif

float temperature = 0;
float pressure = 0;
int16_t ret;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

#ifdef DPS_SPI
  DigitalPressureSensor.begin(SPI, PIN_SPI_SS);
#else 
  //Call begin to initialize DigitalPressureSensor
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  //DigitalPressureSensor.begin(Wire, 0x76);
  //Use the commented line below instead of the one above to use the default I2C address.
  //if you are using the Pressure 3 click Board, you need 0x76
  DigitalPressureSensor.begin(Wire);
#endif

  Serial.println("Init complete!");
}

void loop()
{
  Serial.println();

  ret = DigitalPressureSensor.measureTempOnce(temperature);
  // ret = DigitalPressureSensor.measureTempOnce(temperature, DPS__OVERSAMPLING_RATE_128);

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
  ret = DigitalPressureSensor.measurePressureOnce(pressure);
  // ret = DigitalPressureSensor.measurePressureOnce(pressure, DPS__OVERSAMPLING_RATE_128);
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
