// Open or close all 8 green house windows when internal temperature is higher
// or lower than a fixed threshold.

// History:
// 2016-09-04 Yoann Aubineau - Improve readability with macros
// 2016-09-03 Yoann Aubineau - Never stop window movement once initiated
// 2016-08-16 Yoann Aubineau - Cleanup code before pushing to Github
// 2016-04-01 Yoann Aubineau - Initial program

// Todo:
// - Make board LED blink when temperature is between thredsholds.
// - This code is pretty low-lever and tedious to follow. Introducing a higher
//   level `Window` class could improve readability substantially.

#include "DHT.h"

// ****************************************************************************
// WINDOWS                                     N1, N2, N3, N4, S1, S2, S3, S4

const int window_count = 8;
#define FOR_EACH_WINDOW for (int i = 0; i < window_count; i++)

const int win_direction_pins[window_count] = {22, 24, 26, 28, 30, 32, 34, 36};
const int win_action_pins[window_count]    = {38, 40, 42, 44, 46, 48, 50, 52};
const int win_opened_pins[window_count]    = {39, 41, 43, 45, 47, 49, 51, 53};
const int win_closed_pins[window_count]    = {23, 25, 27, 29, 31, 33, 35, 37};

#define SET_MOTOR_TO_OPEN  digitalWrite(win_direction_pins[i], LOW)
#define SET_MOTOR_TO_CLOSE digitalWrite(win_direction_pins[i], HIGH)
#define START_MOTOR        digitalWrite(win_action_pins[i], HIGH)
#define STOP_MOTOR         digitalWrite(win_action_pins[i], LOW)

#define OPEN_WINDOW  SET_MOTOR_TO_OPEN;  START_MOTOR
#define CLOSE_WINDOW SET_MOTOR_TO_CLOSE; START_MOTOR
#define STOP_WINDOW  STOP_MOTOR;         SET_MOTOR_TO_OPEN

#define MOTOR_IS_SET_TO_OPEN  digitalRead(win_direction_pins[i]) == LOW
#define MOTOR_IS_SET_TO_CLOSE digitalRead(win_direction_pins[i]) == HIGH
#define MOTOR_IS_STARTED      digitalRead(win_action_pins[i]) == HIGH
#define MOTOR_IS_STOPPED      digitalRead(win_action_pins[i]) == LOW

#define WINDOW_IS_OPENING MOTOR_IS_SET_TO_OPEN  && MOTOR_IS_STARTED
#define WINDOW_IS_CLOSING MOTOR_IS_SET_TO_CLOSE && MOTOR_IS_STARTED

#define WINDOW_IS_OPENED     digitalRead(win_opened_pins[i]) == LOW
#define WINDOW_IS_CLOSED     digitalRead(win_closed_pins[i]) == LOW

unsigned long win_started_at[window_count] = { 0,  0,  0,  0,  0,  0,  0,  0};
const unsigned long win_max_running_time = 30000;

#define START_TIMER win_started_at[i] = millis()
#define RUNNING_TIME_IS_OVER \
  win_started_at[i] + win_max_running_time <= millis()

// ****************************************************************************
// MESSAGE buffers used to delay writing to serial console, which is slow

String messages[window_count];
const String NULL_STRING = String("NULL_STRING");

#define CLEAR_MESSAGE_BUFFERS FOR_EACH_WINDOW { messages[i] = NULL_STRING; }
#define PRINT_MESSAGE_BUFFERS FOR_EACH_WINDOW { \
  if (messages[i] != NULL_STRING) { Serial.println(messages[i]); } }

// ****************************************************************************
// TEMPERATURE

const int SENSOR_PIN = A0;
const int SENSOR_TYPE = DHT22;
DHT dht(SENSOR_PIN, SENSOR_TYPE);

const float temperature_threshold = 25.0;
const float threshold_margin = 0.5;
const float high_temperature_threshold = temperature_threshold + threshold_margin;
const float low_temperature_threshold = temperature_threshold - threshold_margin;

#define TEMPERATURE_IS_HIGH current_temperature > high_temperature_threshold
#define TEMPERATURE_IS_LOW  current_temperature < low_temperature_threshold

unsigned long temperature_last_read_at = 0;
const int temperature_read_interval = 2000;

int temperature_area = 0;

#define REMEMBER_TEMPERATURE_IS_HIGH temperature_area = +1
#define REMEMBER_TEMPERATURE_IS_LOW  temperature_area = -1

#define TEMPERATURE_WAS_HIGH     temperature_area == +1
#define TEMPERATURE_WAS_LOW      temperature_area == -1

// ****************************************************************************
// BOARD LED

#define BOARD_LED_INIT digitalWrite(13, LOW); pinMode(13, OUTPUT)
#define BOARD_LED_ON   digitalWrite(13, HIGH)
#define BOARD_LED_OFF  digitalWrite(13, LOW)

// ****************************************************************************
void setup() {

  BOARD_LED_INIT;
  Serial.begin(9600);
  Serial.flush();
  dht.begin();

  FOR_EACH_WINDOW {
    STOP_WINDOW;
    pinMode(win_direction_pins[i], OUTPUT);
    pinMode(win_action_pins[i], OUTPUT);
    pinMode(win_opened_pins[i], INPUT_PULLUP);
    pinMode(win_closed_pins[i], INPUT_PULLUP);
  }
}

// ****************************************************************************
void loop() {

  // --------------------------------------------------------------------------
  // Stop motors after timeout, whatever the direction

  CLEAR_MESSAGE_BUFFERS;
  FOR_EACH_WINDOW {
    if (MOTOR_IS_STARTED && RUNNING_TIME_IS_OVER) {
      messages[i] = "Window " + String(i) + " timeout -> stopping motor.";
      STOP_WINDOW;
    }
  }
  PRINT_MESSAGE_BUFFERS;

  // --------------------------------------------------------------------------
  // Stop motors when windows are fully opened

  CLEAR_MESSAGE_BUFFERS;
  FOR_EACH_WINDOW {
    if (WINDOW_IS_OPENING && WINDOW_IS_OPENED) {
      messages[i] = "Window " + String(i) + " fully opened -> stopping motor.";
      STOP_WINDOW;
    }
  }
  PRINT_MESSAGE_BUFFERS;

  // --------------------------------------------------------------------------
  // Stop motors when windows are fully closed

  CLEAR_MESSAGE_BUFFERS;
  FOR_EACH_WINDOW {
    if (WINDOW_IS_CLOSING && WINDOW_IS_CLOSED) {
      messages[i] = "Window " + String(i) + " fully closed -> stopping motor.";
      STOP_WINDOW;
    }
  }
  PRINT_MESSAGE_BUFFERS;

  // --------------------------------------------------------------------------
  // Read temperature at fixed interval

  unsigned long current_timestamp = millis();
  if (temperature_last_read_at + temperature_read_interval > current_timestamp) {
    return;
  }
  float current_temperature = dht.readTemperature();
  temperature_last_read_at = current_timestamp;
  if (isnan(current_temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println(String(current_temperature) + " °C");

  // --------------------------------------------------------------------------
  // Open windows when temperature becomes high

  if (TEMPERATURE_IS_HIGH && !(TEMPERATURE_WAS_HIGH)) {
    REMEMBER_TEMPERATURE_IS_HIGH;
    BOARD_LED_ON;
    CLEAR_MESSAGE_BUFFERS;
    FOR_EACH_WINDOW {
      if (!(WINDOW_IS_OPENED)) {
        messages[i] = "OPENING window " + String(i);
        START_TIMER;
        OPEN_WINDOW;
      }
    }
    PRINT_MESSAGE_BUFFERS;
    return;
  }

  // --------------------------------------------------------------------------
  // Close windows when temperature becomes low

  if (TEMPERATURE_IS_LOW && !(TEMPERATURE_WAS_LOW)) {
    REMEMBER_TEMPERATURE_IS_LOW;
    BOARD_LED_OFF;
    CLEAR_MESSAGE_BUFFERS;
    FOR_EACH_WINDOW {
      if (!(WINDOW_IS_CLOSED)) {
        messages[i] = "CLOSING window " + String(i);
        START_TIMER;
        CLOSE_WINDOW;
      }
    }
    PRINT_MESSAGE_BUFFERS;
    return;
  }
}
