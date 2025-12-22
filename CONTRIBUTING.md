# Contributing to ESP32-S3 Advanced Ride-On Car Controller

Thank you for your interest in contributing to this project! This guide will help you understand how to contribute effectively.

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [How to Contribute](#how-to-contribute)
3. [Development Workflow](#development-workflow)
4. [Coding Standards](#coding-standards)
5. [Documentation Standards](#documentation-standards)
6. [Commit Guidelines](#commit-guidelines)
7. [Testing](#testing)

## Code of Conduct

This project adheres to the Contributor Covenant Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## How to Contribute

### Reporting Bugs

- Use the issue tracker to report bugs
- Include detailed steps to reproduce the issue
- Provide your environment details (hardware, software versions)

### Suggesting Enhancements

- Open an issue to discuss new features or improvements
- Explain the problem and proposed solution
- Provide use cases and examples

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes with proper documentation
4. Ensure all tests pass
5. Submit a pull request

## Development Workflow

### Setting Up the Environment

1. Clone the repository
2. Install PlatformIO IDE or Arduino IDE with ESP32 support
3. Install required libraries (listed in platformio.ini)
4. Run `pio run` to build the project

### Branching Strategy

- `main` - Stable production code
- `develop` - Integration branch for features
- Feature branches - For individual enhancements

## Coding Standards

### C++ Style Guide

- Use camelCase for variable and function names
- Use PascalCase for class names
- Always initialize variables
- Use descriptive, meaningful names
- Keep functions small and focused
- Follow the existing code structure

### File Structure

All source files should follow this pattern:

```
src/
├── modules/
│   ├── ModuleName/
│   │   ├── ModuleName.h
│   │   └── ModuleName.cpp
└── Shared/
    └── SharedHeader.h
```

### Comments and Documentation

- Use Javadoc-style comments for all public methods
- Document complex logic with inline comments
- Update documentation when changing behavior

## Documentation Standards

### Header Files

All header files should include:

```cpp
#pragma once
#include <Arduino.h>
// Include other necessary headers

/**
 * @class ClassName
 * @brief Brief description of the class purpose
 *
 * Detailed description of what this class does and its role in the system.
 */
class ClassName {
public:
    /**
     * @brief Constructor description
     * @param param1 Description of parameter 1
     * @param param2 Description of parameter 2
     */
    ClassName(int param1, Logger* param2);

    /**
     * @brief Method description
     * @param input Description of input parameter
     * @return Description of return value
     */
    void methodName(int input);

private:
    // Private members and documentation
};
```

### Implementation Files

Implementation files should include:

```cpp
#include "ClassName.h"

/**
 * @brief Constructor implementation
 * @param param1 Description of parameter 1
 * @param param2 Description of parameter 2
 */
ClassName::ClassName(int param1, Logger* param2) {
    // Implementation details
}

/**
 * @brief Method implementation
 * @param input Description of input parameter
 */
void ClassName::methodName(int input) {
    // Implementation details
}
```

## Commit Guidelines

### Commit Message Format

Use the following format for commit messages:

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation only changes
- `style`: Changes that do not affect the meaning of the code
- `refactor`: A code change that neither fixes a bug nor adds a feature
- `test`: Adding missing tests or correcting existing tests
- `chore`: Changes to the build process or auxiliary tools

### Examples

```
feat(SystemStatus): Add color coding for different car states
fix(Accelerator): Correct acceleration curve calculation
docs(README): Update hardware requirements section
```

## Testing

### Unit Tests

All new functionality should include appropriate unit tests in the test/ directory.

### Integration Tests

Test the interaction between different modules to ensure they work together correctly.

### Hardware Testing

Verify functionality on actual hardware before merging changes.
