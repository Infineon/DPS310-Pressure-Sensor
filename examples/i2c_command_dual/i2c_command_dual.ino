#include <Dps310.h>

// Example of i2c_command with two pressure sensors
// This example shows how to read out two pressure sensors via I2C

// Dps310 object for pressure sensor one
Dps310 Dps310PressureSensorOne = Dps310();

// Dps310 object for pressure sensor two
Dps310 Dps310PressureSensorTwo = Dps310();

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  // The parameter 0x76/0x77 is the bus address. The default address is 0x77 and does not need to be given.
  // Here, we show an example with I2C command with two pressure sensors Dps310
  // Please configure the Dps310 pressure sensor evaluation boards for the respective I2C addresses

  // Call begin to initialize Dps310PressureSensorOne - first pressure sensor with address 0x77
  Dps310PressureSensorOne.begin(Wire, 0x77);

  // Call begin to initialize Dps310PressureSensorTwo - second pressure sensor with address 0x76
  Dps310PressureSensorTwo.begin(Wire, 0x76);

  Serial.println("Init complete!");
}

void loop()
{
  // Please refer to the example i2c_command for more explanations
  // Basically, we are performing here the I2C command example with two pressure sensors Dps310

  // Begin of pressure sensor one read-out
  // Values are stored in variables with *One naming, e.g. pressureOne 
  
  // Pressure sensor one
  float temperatureOne;
  float pressureOne;
  uint8_t oversamplingOne = 7;
  int16_t retOne;
  Serial.println();

  // Lets the Dps310 perform a single temperature measurement with the last (or standard) configuration
  // The result will be written to the parameter temperatureOne
  // The Dps310 will perform 2^oversamplingOne internal temperature measurements and combine them to one result with higher precision
  // Measurements with higher precision take more time, consult the datasheet for more information
  retOne = Dps310PressureSensorOne.measureTempOnce(temperatureOne, oversamplingOne);

  if (retOne != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! retOne = ");
    Serial.println(retOne);
  }
  else
  {
    Serial.print("temperatureOne: ");
    Serial.print(temperatureOne);
    Serial.println(" degrees of Celsius");
  }

  // Pressure measurement behaves like temperature measurement
  retOne = Dps310PressureSensorOne.measurePressureOnce(pressureOne, oversamplingOne);
  if (retOne != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! retOne = ");
    Serial.println(retOne);
  }
  else
  {
    Serial.print("pressureOne: ");
    Serial.print(pressureOne);
    Serial.println(" Pascal");
  }
  // End of pressure sensor one read-out

  // Begin of pressure sensor two read-out
  // Values are stored in variables with *Two naming, e.g. pressureTwo
    
  // Pressure sensor two
  // Same structure as for pressure sensor one
  float temperatureTwo;
  float pressureTwo;
  uint8_t oversamplingTwo = 7;
  int16_t retTwo;
  Serial.println();

  retTwo = Dps310PressureSensorTwo.measureTempOnce(temperatureTwo, oversamplingTwo);

  if (retTwo != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! retTwo = ");
    Serial.println(retTwo);
  }
  else
  {
    Serial.print("temperatureTwo: ");
    Serial.print(temperatureTwo);
    Serial.println(" degrees of Celsius");
  }

  // Pressure measurement behaves like temperature measurement
  retTwo = Dps310PressureSensorTwo.measurePressureOnce(pressureTwo, oversamplingTwo);
  if (retTwo != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! retTwo = ");
    Serial.println(retTwo);
  }
  else
  {
    Serial.print("pressureTwo: ");
    Serial.print(pressureTwo);
    Serial.println(" Pascal");
  }
  // End of pressure sensor two read-out

  //Wait some time
  delay(500);
}
