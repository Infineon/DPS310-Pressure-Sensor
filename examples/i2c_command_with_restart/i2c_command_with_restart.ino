#include <Dps310.h>

/* Example of the DPS310 Pressure Sensor which restarts the software interface 
 * if an error occurs. If an error code is given back on a DPS310 operation, 
 * the sensor handler is resetted and sensor initialisation is started again.
 */

// The I2C address of the DPS310 pressure sensor (0x77 or 0x76)
// Change here to set different address
#define DPS_ADDRESS 0x77

// Global variables

// Dps310 Object
Dps310 Dps310PressureSensor = Dps310();

// State variable whether something went wrong
bool restartRequired = false;
uint16_t numberRestartTries = 0;

// Functions

// This function is executed once
void setup()
{
    // Initialize the Arduino serial interface and wait until the interface is ready.
    Serial.begin(9600);
    while (!Serial)
        ;

    // Call Dps310PressureSensor.begin to initialize DPS310 Pressure Sensor
    // Wire is the I2C interface which is handed over to the DPS310 sensor class

    // Use the commented line below to use the default I2C address (do not use this line and the above one simultaneously).
    Dps310PressureSensor.begin(Wire, DPS_ADDRESS);

    // Initialization of pressure sensor done - reply on serial interface
    Serial.println("Init complete!");
}

// This function is continuously executed
void loop()
{

   /*
    * Read out the pressure sensor and print values on the serial interface
    */

    // Variables for storing the results
    float temperature;
    float pressure;

    // The oversampling setting
    uint8_t oversampling = 7;

    // The return error code
    int16_t ret;

    // Print an empty line on the serial interface
    Serial.println();

    // Only read-out the sensor if no restart is required
    if ((restartRequired == false))
    {

        // The following command makes the DPS310 perform a single temperature measurement
        // with the last (or standard) configuration and the result is written to temperature.

        // ret = Dps310PressureSensor.measureTempOnce(temperature);

        // The commented line below does exactly the same as the one above, but you can also
        // config the oversampling which can be a value from 0 to 7
        // The DPS310 will perform 2^oversampling internal temperature measurements and combine
        // them to one result with higher precision (but measurement takes longer).
        // Consult the datasheet for more information

        ret = Dps310PressureSensor.measureTempOnce(temperature, oversampling);

        // Check the return code from the measureTempOnce() execution
        // Enter only the following conditional if no restart is required (restartRequired == false)
        if (ret != DPS__SUCCEEDED)
        {
            // Something went wrong
            // Look at the library code /src/util/dps_config.h for more information about return status codes
            Serial.print("FAIL TEMPERATURE! ret = ");
            Serial.println(ret);

            // Something went wrong, we have to restart the sensor interface
            restartRequired = true;
        }
        else if (ret == DPS__SUCCEEDED)
        {
            // Everything is all right, no error has been returned and no restart is pending (restartRequired == false)
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" degrees of Celsius");
        }
    }

    // Only read-out the sensor if no restart is required
    // Do not read out the sensor or interact with it if a restart is required as an error occurred earlier
    if ((restartRequired == false))
    {

        // Pressure measurement function is similar like temperature measurement
        // Use the following line if oversampling shall not be configured
        // ret = Dps310PressureSensor.measurePressureOnce(pressure);

        ret = Dps310PressureSensor.measurePressureOnce(pressure, oversampling);

        // Check the return code from the measurePressureOnce() execution
        // Enter only the following conditional if no restart is required (restartRequired == false)
        if (ret != DPS__SUCCEEDED)
        {
            // Something went wrong
            // Look at the library code /src/util/dps_config.h for more information about return status codes
            Serial.print("FAIL PRESSURE! ret = ");
            Serial.println(ret);

            // Something went wrong, we have to restart the sensor software interface
            restartRequired = true;
        }
        else if (ret == DPS__SUCCEEDED)
        {
            // Everything is all right, no error has been returned and no software sensor interface restart is pending (restartRequired == false)
            Serial.print("Pressure: ");
            Serial.print(pressure);
            Serial.println(" Pascal");
        }
    }

  /*
   * Check the restart conditional and execute it if needed
   */

    // Check whether restart is required and execute it
    // Do not read out the sensor or interact with it if a restart is required as an error occurred earlier
    if ((restartRequired == true))
    {
        // Increment the number of restart tries
        numberRestartTries++;

        // Set the DPS310 to idle modus and stop the sensor interface
        ret = Dps310PressureSensor.standby();

        // Check whether the sensor software interface could be stopped
        if (ret != DPS__SUCCEEDED)
        {
            // Something went wrong when putting the device into idle state
            // Look at the library code /src/util/dps_config.h for more information about return status codes
            Serial.print("FAIL Restart! ret = ");
            Serial.println(ret);
            Serial.print("Restarting the sensor software interface FAILED, current restart tries: ");
            Serial.println(numberRestartTries);
            delay(50);

            Serial.println("Will try to execute begin() although restart failed.");
            Serial.println("CAUTION: No proper restart cycling.");
            Dps310PressureSensor.begin(Wire, DPS_ADDRESS);

            // Something went wrong, we have to continue restarting the sensor software interface
            restartRequired = false;
        }
        else if (ret == DPS__SUCCEEDED)
        {
            Dps310PressureSensor.begin(Wire, DPS_ADDRESS);
            Serial.print("Restarting the sensor software interface SUCCEEDED, current restart tries: ");
            Serial.println(numberRestartTries);
            delay(50);

            // Everything is all right, we can continue with reading out the sensor
            restartRequired = false;
        }
        // Continue with the loop() function
    }

    // Wait some time before starting a new measurement
    delay(500);
}
