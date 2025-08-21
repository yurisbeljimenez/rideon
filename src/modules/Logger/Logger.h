#pragma once
#include <Arduino.h>

/**
 * @class Logger
 * @brief A stateful logger to prevent spamming the Serial Monitor.
 *
 * This class creates a logger instance with a specific "scope" (name). It only
 * prints a message or a numeric value if it has changed since the last time
 * it was logged, keeping the serial output clean and readable.
 */
class Logger {
public:
  /**
   * @brief Constructor for the Logger class.
   * @param scope A C-style string that prefixes every log message (e.g., "Accelerator").
   */
  Logger(const char* scope);

  /**
   * @brief Logs a numeric (long) value if it has changed.
   * @param value The value to log.
   */
  void log(long value);

  /**
   * @brief Logs a text message (const char*) if it has changed.
   * @param message The message to log.
   */
  void log(const char* message);

private:
  const char* _scope;                 // The name of this logger instance.
  long _lastNumericValue = -999999;   // Stores the last logged number to check for changes.
  String _lastMessage;                // Stores the last logged message to check for changes.
};