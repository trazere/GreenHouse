
// Open or close all 8 green house windows when internal temperature is higher
// or lower than a fixed threshold.

// History:
// 2016-04-01 Yoann Aubineau - Initial program
// 2016-08-16 Yoann Aubineau - Code cleanup before pushing to Github
// 2019-06-19 Julien Dufour - Revamped the code with the Window class

// Todo:
// - Make board LED blink when temperature is between thredsholds.
// - Add manual open/close control.

#include "DHT.h"
#include "Window.h"

// ****************************************************************************
// Windows

const int windowCount = 8;
const unsigned long windowMoveTimeout = 30000;

Window windows[windowCount] = {
  Window("N1", 22, 38, 39, 23, windowMoveTimeout),
  Window("N2", 24, 40, 41, 25, windowMoveTimeout),
  Window("N3", 26, 42, 43, 27, windowMoveTimeout),
  Window("N4", 28, 44, 45, 29, windowMoveTimeout),
  Window("S1", 30, 46, 47, 31, windowMoveTimeout),
  Window("S2", 32, 48, 49, 33, windowMoveTimeout),
  Window("S3", 34, 50, 51, 35, windowMoveTimeout),
  Window("S4", 36, 52, 53, 37, windowMoveTimeout),
};

#define FOR_EACH_WINDOW for (int i = 0; i < windowCount; i += 1)


// ****************************************************************************
// Temperature

const int SENSOR_PIN = A0;
const int SENSOR_TYPE = DHT22;
DHT dht(SENSOR_PIN, SENSOR_TYPE);

const float closeTemperatureThreshold = 24.0;
const float openTemperatureThreshold = 26.0;

const unsigned long temperatureReadInterval = 2000;
unsigned long lastTemperatureReadTime = 0;

// ****************************************************************************
// BOARD LED

#define BOARD_LED_INIT digitalWrite(13, LOW); pinMode(13, OUTPUT);
#define BOARD_LED_ON digitalWrite(13, HIGH);
#define BOARD_LED_OFF digitalWrite(13, LOW);

// ****************************************************************************
void setup() {

  BOARD_LED_INIT
  Serial.begin(9600);
  Serial.flush();
  dht.begin();

  FOR_EACH_WINDOW {
    windows[i].setup();
  }
}

// ****************************************************************************
void loop() {
  
  // --------------------------------------------------------------------------
  // Autostop the windows.
  
  FOR_EACH_WINDOW {
    windows[i].autostop();
  }
  
  // --------------------------------------------------------------------------
  // Read temperature at fixed interval
  
  unsigned long now = millis();
  if (lastTemperatureReadTime + temperatureReadInterval > now) {
    return;
  }
  float temperature = dht.readTemperature();
  lastTemperatureReadTime = now;
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println(String(temperature) + " °C");

  // --------------------------------------------------------------------------
  // Open windows when temperature is high

  if (temperature > openTemperatureThreshold) {
    BOARD_LED_ON
    FOR_EACH_WINDOW {
      if (!windows[i].hasTimedOut()) {
        windows[i].open();
      }
    }
    return;
  }

  // --------------------------------------------------------------------------
  // Close windows when temperature is low

  if (temperature < closeTemperatureThreshold) {
    BOARD_LED_OFF
    FOR_EACH_WINDOW {
      if (!windows[i].hasTimedOut()) {
        windows[i].close();
      }
    }
    return;
  }

  // --------------------------------------------------------------------------
  // Stop windows otherwise
  
  BOARD_LED_OFF
  FOR_EACH_WINDOW {
    windows[i].stop();
  }
  return;
}
