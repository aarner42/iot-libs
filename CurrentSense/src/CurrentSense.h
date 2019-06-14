#ifndef CURRENTSENSE_H
#define CURRENTSENSE_H

/* CT Sensor consts */
#define CURRENT_FLOW_NONZERO_THRESHOLD 250
#define RESISTANCE 82.0
#define SAMPLING_MSEC 333
#define ZERO_CROSSING -126.2
#define SCALE_FACTOR .3  //compensate for the triple-wired coil

#include <Arduino.h>

float getVPP();
double calcCurrentFlow(bool debug);

#endif
