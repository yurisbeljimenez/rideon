# Logger Module

The Logger module provides a stateful logging system to prevent spamming the Serial Monitor with repeated messages.

## Overview

This module creates logger instances with a specific "scope" (name). It only prints a message or numeric value if it has changed since the last time it was logged, keeping the serial output clean and readable.

## Key Features

- **Stateful Logging**: Only logs when values change
- **Scope-based Organization**: Each logger has a descriptive scope name
- **Memory Efficient**: Tracks last values to avoid duplicate logging
- **Flexible Output**: Supports both numeric values and text messages

## Implementation Details

The Logger class maintains:

- `_scope`: The name of this logger instance (used as prefix in logs)
- `_lastNumericValue`: Stores the last logged number to check for changes
- `_lastMessage`: Stores the last logged message to check for changes

## Usage Examples

```cpp
// Create a logger instance
Logger acceleratorLogger("Accelerator");

// Log numeric values (only logs when value changes)
acceleratorLogger.log(50);  // Logs: [Accelerator] Value changed: 50
acceleratorLogger.log(50);  // No output (value unchanged)
acceleratorLogger.log(60);  // Logs: [Accelerator] Value changed: 60

// Log text messages (only logs when message changes)
acceleratorLogger.log("STARTED");  // Logs: [Accelerator] State: STARTED
acceleratorLogger.log("STARTED");  // No output (message unchanged)
acceleratorLogger.log("STOPPED");  // Logs: [Accelerator] State: STOPPED
```

## Integration

The Logger module is used throughout the system to provide debug output from each component. Each module creates its own logger instance with an appropriate scope name for easy identification in the serial output.

## Dependencies

- Arduino core libraries
