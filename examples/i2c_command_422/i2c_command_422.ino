#include <Dps422.h>

// Dps310 Opject
Dps422 DigitalPressureSensor = Dps422();
float temperature;
float pressure;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  DigitalPressureSensor.begin(Wire);

  Serial.println("Init complete!");
}

void loop()
{
  int16_t ret = DigitalPressureSensor.measureBothOnce(pressure, temperature);
  
  Serial.println();

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

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pascal");
  }

  //Wait some time
  delay(500);
}
