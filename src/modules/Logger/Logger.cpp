#include "Logger.h"

// The constructor simply stores the scope name provided during creation.
Logger::Logger(const char* scope) {
  _scope = scope;
}

// Logs a numeric value.
void Logger::log(long value) {
  // The core logic: only proceed if the new value is different from the last one.
  if (value != _lastNumericValue) {
    Serial.print("[");
    Serial.print(_scope);
    Serial.print("] Value changed: ");
    Serial.println(value);

    // Update the stored value for the next check.
    _lastNumericValue = value;
  }
}

// Logs a text message.
void Logger::log(const char* message) {
  // The core logic: only proceed if the new message is different from the last one.
  if (_lastMessage != message) {
    Serial.print("[");
    Serial.print(_scope);
    Serial.print("] State: ");
    Serial.println(message);

    // Update the stored message for the next check.
    _lastMessage = message;
  }
}