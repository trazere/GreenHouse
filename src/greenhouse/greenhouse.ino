
// Open or close all 8 green house windows when internal temperature is higher
// or lower than a fixed threshold.

// History:
// 2016-04-01 Yoann Aubineau - Initial program
// 2016-08-16 Yoann Aubineau - Code cleanup before pushing to Github
// 2019-06-19 Julien Dufour - Revamped the code with the Window class
// 2019-07-07 Julien Dufour - Added support for manual control

// Todo:

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
// Led

const int ledPin = 13;

void setupLed() {
  digitalWrite(ledPin, LOW);
  pinMode(ledPin, OUTPUT);
}

void switchLedOn() {
  digitalWrite(ledPin, HIGH);
}

void switchLedOff() {
  digitalWrite(ledPin, LOW);
}

void blinkLed() {
  digitalWrite(ledPin, LOW == digitalRead(ledPin) ? HIGH : LOW);
}


// ****************************************************************************
// Manual control

const int manualOpenPin = 8;
const int manualClosePin = 9;

void setupManualControl() {
  pinMode(manualOpenPin, INPUT_PULLUP);
  pinMode(manualClosePin, INPUT_PULLUP);
}

bool isManuallyOpening() {
  return LOW == digitalRead(manualOpenPin);
}

bool isManuallyClosing() {
  return LOW == digitalRead(manualClosePin);
}


// ****************************************************************************
// Temperature

const int SENSOR_PIN = A0;
const int SENSOR_TYPE = DHT22;
DHT dht(SENSOR_PIN, SENSOR_TYPE);

const float closeTemperatureThreshold = 24.0;
const float openTemperatureThreshold = 26.0;

const unsigned long temperatureLogInterval = 2000;
unsigned long lastTemperatureLogTime = 0;


// ****************************************************************************
// Sketch

#define IDLE_STATE 0
#define MANUAL_OPEN_STATE 1
#define MANUAL_CLOSE_STATE 2
#define TEMP_OPEN_STATE 3
#define TEMP_CLOSE_STATE 4

int state = IDLE_STATE;

void setup() {
  Serial.begin(9600);
  Serial.flush();
  
  setupLed();
  setupManualControl();
  
  dht.begin();
  
  FOR_EACH_WINDOW {
    windows[i].setup();
  }
}

void loop() {
  // Autostop the windows.
  FOR_EACH_WINDOW {
    windows[i].autostop();
  }
  
  
  // Read the controls.
  bool manualOpen = isManuallyOpening();
  bool manualClose = isManuallyClosing();
  float temperature = dht.readTemperature();
  
  
  // Log the temperature.
  unsigned long now = millis();
  if (now >= lastTemperatureLogTime + temperatureLogInterval) {
    if (!isnan(temperature)) {
      Serial.println(String(temperature) + " °C");
      
      if (temperature > openTemperatureThreshold) {
        switchLedOn();     
      } else if (temperature < closeTemperatureThreshold) {
        switchLedOff();
      } else {
        blinkLed();
      }
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }
    
    lastTemperatureLogTime = now;
  }

  
  // Manually open the windows.
  if (manualOpen) {
    if (MANUAL_OPEN_STATE != state) {
      Serial.println("Manually opening all windows.");
      
      FOR_EACH_WINDOW {
        windows[i].open();
      }
      
      state = MANUAL_OPEN_STATE;
    }

  // Manually close the windows.
  } else if (manualClose) {
    if (MANUAL_CLOSE_STATE != state) {
      Serial.println("Manually closing all windows.");
      
      FOR_EACH_WINDOW {
        windows[i].close();
      }
      
      state = MANUAL_CLOSE_STATE;
    }

  // Stop the windows when temperature is unavailable.
  } else if (isnan(temperature)) {
    if (MANUAL_OPEN_STATE == state || MANUAL_CLOSE_STATE == state) {
      FOR_EACH_WINDOW {
        windows[i].stop();
      }
      
      state = IDLE_STATE;
    }
  
  // Open the windows when the temperature is high.
  } else if (temperature > openTemperatureThreshold) {
    if (TEMP_OPEN_STATE != state) {
      Serial.println("Temperature is high, opening all windows.");
      
      FOR_EACH_WINDOW {
        windows[i].open();
      }
      
      state = TEMP_OPEN_STATE;
    }
    
  // Close the windows when the temperature is low.
  } else if (temperature < closeTemperatureThreshold) {
    if (TEMP_CLOSE_STATE != state) {
      Serial.println("Temperature is low, closing all windows.");
      
      FOR_EACH_WINDOW {
        windows[i].close();
      }

      state = TEMP_CLOSE_STATE;
    }
  }


  // Wait a little
  delay(100);
}
