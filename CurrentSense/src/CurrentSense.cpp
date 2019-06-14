#include "CurrentSense.h"
#include <Arduino.h>


double calcCurrentFlow(bool debug) {

    float nVPP;   // Voltage measured across resistor
    double nCurrThruResistorPP; // Peak Current Measured Through Resistor
    double nCurrThruResistorRMS; // RMS current through Resistor
    double nCurrentThruWire;     // Actual RMS current in Wire


    nVPP = getVPP();

    /*
    Use Ohms law to calculate current across resistor
    and express in mA
    */

    nCurrThruResistorPP = (nVPP / RESISTANCE) * 1000.0;

    /*
    Use Formula for SINE wave to convert
    to RMS
    */

    nCurrThruResistorRMS = nCurrThruResistorPP * 0.707;

    /*
    Current Transformer Ratio is 1000:1...

    Therefore current through resistor
    is multiplied by 1000 to get input current
    */

    nCurrentThruWire = ((nCurrThruResistorRMS * 1000) + ZERO_CROSSING) * SCALE_FACTOR;
    if (debug) {
        Serial.print("Volts Peak : ");
        Serial.println(nVPP, 3);

        Serial.print("Current Through Resistor (Peak) : ");
        Serial.print(nCurrThruResistorPP, 3);
        Serial.println(" mA Peak to Peak");

        Serial.print("Current Through Resistor (RMS) : ");
        Serial.print(nCurrThruResistorRMS, 3);
        Serial.println(" mA RMS");

        Serial.print("Current Through Wire : ");
        Serial.print(nCurrentThruWire, 3);
        Serial.println(" mA RMS");
    }
    return nCurrentThruWire;


}


/************************************
In order to calculate RMS current, we need to know
the peak to peak voltage measured at the output across the
200 Ohm Resistor

The following function takes one second worth of samples
and returns the peak value that is measured
*************************************/


float getVPP() {
    float result;
    int readValue;             //value read from the sensor
    int maxValue = 0;          // store max value here
    uint32_t start_time = millis();
    while ((millis() - start_time) < SAMPLING_MSEC) //sample for some non-trivial time
    {
        readValue = analogRead(A0);
        yield();
        delay(10);
        // see if you have a new maxValue
        if (readValue > maxValue) {
            /*record the maximum sensor value*/
            maxValue = readValue;
        }
    }

    // Convert the digital data to a voltage
    result = (maxValue * 5.0) / 1024.0;

    return result;
}
