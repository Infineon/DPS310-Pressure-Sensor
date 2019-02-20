#include <Dps310.h>

// Dps310 Opject
Dps310 Dps310PressureSensor = Dps310();

void onFifoFull();

const unsigned char pressureLength = 50;
unsigned char pressureCount = 0;
float pressure[pressureLength];
unsigned char temperatureCount = 0;
const unsigned char temperatureLength = 50;
float temperature[temperatureLength];




void setup()
{
  //pin number of your slave select line
  //XMC2GO
  int16_t pin_cs = 3;
  //for XMC 1100 Bootkit  & XMC4700 Relax Kit uncomment the following line
  //int16_t pin_cs = 10;

  Serial.begin(9600);
  while (!Serial);

  //Call begin to initialize Dps310PressureSensor
  //The third parameter has to be 1 and enables 3-wire SPI interface
  //This is necessary, because SDO will be used to indicate interrupts
  //ATTENTION: Make sure you have connected your MISO and MOSI pins right!
  //  There may NEVER be a direct Connection between MOSI and SDI when 3-wire SPI is enabled
  //  Otherwise, you will cause shortcuts and seriously damage your equipment.
  //  For three wire interface, MISO has to be connected to SDI and there hase to be a resistor between MISO and MOSI
  //  I successfully tested this with a resistor of 1k, but I won't give you any warranty that this works for your equipment too
  Dps310PressureSensor.begin(SPI, pin_cs, 1);

  //config Dps310 for Interrupts
//  int16_t ret = Dps310PressureSensor.setInterruptPolarity(1);
  int16_t ret = Dps310PressureSensor.setInterruptSources(1, 0);
  //clear interrupt flag by reading
  Dps310PressureSensor.getIntStatusFifoFull();

  //initialization of Interrupt for Controller unit
  //SDO pin of Dps310 has to be connected with interrupt pin
  int16_t interruptPin = 2;
  pinMode(interruptPin, INPUT);
  Serial.println(digitalPinToInterrupt(interruptPin));
  attachInterrupt(digitalPinToInterrupt(interruptPin), onFifoFull, RISING);

  //start of a continuous measurement just like before
  int16_t temp_mr = 3;
  int16_t temp_osr = 2;
  int16_t prs_mr = 1;
  int16_t prs_osr = 3;
  ret = Dps310PressureSensor.startMeasureBothCont(temp_mr, temp_osr, prs_mr, prs_osr);
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

  //do other stuff
  Serial.println("loop running");
  delay(500);


  //if result arrays are full
  //This could also be in the interrupt handler, but it would take too much time for an interrupt
  if (pressureCount == pressureLength && temperatureCount == temperatureLength)
  {
    //print results
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
    Serial.println();
    Serial.println();

    //reset result counters
    pressureCount = 0;
    temperatureCount = 0;
  }

}


void onFifoFull()
{
  //message for debugging
  Serial.println("Interrupt handler called");

  //clear interrupt flag by reading
  Dps310PressureSensor.getIntStatusFifoFull();

  //calculate the number of free indexes in the result arrays
  uint8_t prs_freespace = pressureLength - pressureCount;
  uint8_t temp_freespace = temperatureLength - temperatureCount;
  //read the results from Dps310, new results will be added at the end of the arrays
  Dps310PressureSensor.getContResults(&temperature[temperatureCount], temp_freespace, &pressure[pressureCount], prs_freespace);
  //after reading the result counters are increased by the amount of new results
  pressureCount += prs_freespace;
  temperatureCount += temp_freespace;


}
