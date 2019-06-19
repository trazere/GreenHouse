
#include "Arduino.h"
#include "Window.h"

// States.
#define IDLE_STATE 0
#define OPENING_STATE 1
#define CLOSING_STATE 2
#define TIMEOUT_STATE 3

Window::Window(const char *description, int directionPin, int motorPin, int openedPin, int closedPin, unsigned long moveTimeout) {
  this->description = description;
  _directionPin = directionPin;
  _motorPin = motorPin;
  _openedPin = openedPin;
  _closedPin = closedPin;
  _moveTimeout = moveTimeout;
  
  _state = IDLE_STATE;
  _moveDeadline = 0;
}

void Window::setup() {
  digitalWrite(_directionPin, LOW);
  pinMode(_directionPin, OUTPUT);
  
  digitalWrite(_motorPin, LOW);
  pinMode(_motorPin, OUTPUT);
  
  pinMode(_openedPin, INPUT_PULLUP);
  pinMode(_closedPin, INPUT_PULLUP);
}

void Window::open() {
  if (OPENING_STATE != _state && !isOpen()) {
    Serial.println("Opening window " + String(description) + ".");
    
    digitalWrite(_directionPin, LOW);
    digitalWrite(_motorPin, HIGH);
    
    _state = OPENING_STATE;
    _moveDeadline = millis() + _moveTimeout;
  }
}

void Window::close() {
  if (CLOSING_STATE != _state && !isClosed()) {
    Serial.println("Closing window " + String(description) + ".");
    
    digitalWrite(_directionPin, HIGH);
    digitalWrite(_motorPin, HIGH);
    
    _state = CLOSING_STATE;
    _moveDeadline = millis() + _moveTimeout;
  }
}

void Window::stop() {
  if (IDLE_STATE != _state) {
    Serial.println("Stopping window " + String(description) + ".");
    
    digitalWrite(_directionPin, LOW);
    digitalWrite(_motorPin, LOW);
    
    _state = IDLE_STATE;
    _moveDeadline = 0;
  }
}

void Window::autostop() {
  // Stop opening window when it has opened.
  if (OPENING_STATE == _state && isOpen()) {
    Serial.println("Window " + String(description) + " has been opened.");
    
    digitalWrite(_directionPin, LOW);
    digitalWrite(_motorPin, LOW);
    
    _state = IDLE_STATE;
    _moveDeadline = 0;
    
  // Stop closing window when it has closed.
  } else if (CLOSING_STATE == _state && isClosed()) {
    Serial.println("Window " + String(description) + " has been closed.");
    
    digitalWrite(_directionPin, LOW);
    digitalWrite(_motorPin, LOW);
    
    _state = IDLE_STATE;
    _moveDeadline = 0;

  // Stop moving window when it has timed out.
  } else if (isMoving() && millis() >= _moveDeadline) {
    Serial.println("Window " + String(description) + " is moving too slowly, stopping.");
    
    digitalWrite(_directionPin, LOW);
    digitalWrite(_motorPin, LOW);
    
    _state = TIMEOUT_STATE;
    _moveDeadline = 0;
  }
}

bool Window::isOpen() {
  return LOW == digitalRead(_openedPin);
}

bool Window::isClosed() {
  return LOW == digitalRead(_closedPin);
}

bool Window::isMoving() {
  return OPENING_STATE == _state || CLOSING_STATE == _state;
}

bool Window::hasTimedOut() {
  return TIMEOUT_STATE == _state;
}
