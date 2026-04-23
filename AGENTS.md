# ATM Simulator - Agent Guidelines

## Project Overview

C++20 object-oriented ATM (Automated Teller Machine) simulator with console interface.
Built with Visual Studio 2022 (v145 toolset) on Windows.

## Build Commands

### Build (MSBuild - Windows)
```bash
# Debug build (x64)
msbuild atmsimulator\atmsimulator.vcxproj /p:Configuration=Debug /p:Platform=x64

# Release build (x64)
msbuild atmsimulator\atmsimulator.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Build (Visual Studio Developer Command Prompt)
```bash
# Open Developer Command Prompt for VS 2022, then:
cd C:\Users\DELL\Desktop\atmsim\atmsimulator
msbuild atmsimulator\atmsimulator.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### Run
```bash
.\atmsimulator\x64\Debug\atmsimulator.exe
```

## Linting and Formatting

### clang-format
```bash
# Format all C++ files (if clang-format is installed)
clang-format -i atmsimulator\*.h atmsimulator\*.cpp

# Dry-run to see changes
clang-format --dry-run atmsimulator\*.cpp
```

### Compiler Warnings
- Warning Level: Level3 (`/W3`)
- SDL Checks: enabled (`/sdl`)
- Conformance Mode: enabled (`/permissive-`)

## Testing

### Run Tests (if GoogleTest is configured)
```bash
# Build tests
msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64

# Run tests
.\tests\x64\Debug\tests.exe
```

### Single Test (GoogleTest)
```bash
# Run specific test
.\tests\x64\Debug\tests.exe --gtest_filter=AccountTest.WithdrawReducesBalance
```

## Project Structure

```
atmsimulator/
├── AGENTS.md              # This file
├── atmsimulator.slnx      # Visual Studio solution
└── atmsimulator/
    ├── atmsimulator.vcxproj
    ├── atmsimulator.cpp   # Main entry point
    ├── Card.h/cpp         # Card class (card number, account ID)
    ├── Account.h/cpp      # Account class (balance, PIN, operations)
    └── ATM.h/cpp          # ATM class (state machine, menu logic)
```

## Code Style Guidelines

### File Naming
- Header files: `PascalCase.h` (e.g., `Account.h`)
- Source files: `PascalCase.cpp` (e.g., `Account.cpp`)
- Match class name in file

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `ATM`, `Account`, `Card` |
| Methods | camelCase | `withdraw()`, `checkBalance()` |
| Member variables | m_camelCase | `m_balance`, `m_isRunning` |
| Local variables | camelCase | `amount`, `userChoice` |
| Constants | UPPER_SNAKE_CASE | `MAX_DAILY_LIMIT` |
| Enums | PascalCase | `ATMState` |
| Enum values | PascalCase | `Idle`, `Authenticated` |

### Class Structure
```cpp
// Header (.h)
#pragma once
#include <string>

class Example {
public:
    Example();
    explicit Example(double value);
    ~Example() = default;

    // Getters
    double getValue() const;

    // Setters
    void setValue(double value);

    // Operations
    void performAction();

private:
    double m_value;
    std::string m_name;
};
```

### Includes Order
1. Standard library (`<iostream>`, `<string>`, `<vector>`)
2. Third-party libraries
3. Project headers (`"Account.h"`, `"Card.h"`)

```cpp
#include <iostream>
#include <string>
#include <vector>

#include "Account.h"
#include "Card.h"
```

### Formatting
- Indent: 4 spaces (no tabs)
- Braces: Allman style (opening brace on new line)
- Line length: 120 characters max
- Empty line before `public:`, `private:`, `protected:`

```cpp
class Example
{
public:
    void method()
    {
        if (condition)
        {
            doSomething();
        }
    }

private:
    int m_value;
};
```

### Error Handling

#### Input Validation
```cpp
bool ATM::validateAmount(double amount) const
{
    if (amount <= 0)
    {
        std::cout << "Invalid amount. Must be positive.\n";
        return false;
    }
    if (amount > MAX_WITHDRAWAL)
    {
        std::cout << "Amount exceeds daily limit.\n";
        return false;
    }
    return true;
}
```

#### Return Values
- Use `bool` for success/failure operations
- Use `std::optional<T>` for values that may not exist (C++17+)
- Throw exceptions only for critical errors

### Memory Management
- Prefer stack allocation over heap
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) when heap is needed
- Follow RAII principles

### Const Correctness
```cpp
// Getters should be const
double getBalance() const;

// Parameters that aren't modified should be const references
void processTransaction(const std::string& cardNumber);

// Local variables that don't change should be const
const double fee = calculateFee(amount);
```

## Architecture

### Class Responsibilities

| Class | Responsibility |
|-------|---------------|
| `Card` | Stores card number and links to account ID |
| `Account` | Manages balance, PIN validation, deposits/withdrawals |
| `ATM` | Handles user interaction, menu navigation, state management |

### State Machine (ATM)
```
Idle → CardInserted → Authenticated → Idle (after eject)
                      ↓
                 Transaction (withdraw/deposit/check balance)
```

## Hardcoded Test Data

| Card Number | PIN | Balance |
|-------------|-----|---------|
| 1234567890123456 | 1234 | 1000.00 PLN |
| 9876543210987654 | 4321 | 500.50 PLN |

## Common Tasks

### Adding a New Feature
1. Add method declaration in header file
2. Implement method in cpp file
3. Update ATM menu if user-facing
4. Test with hardcoded accounts

### Adding a New Account
Edit `ATM.cpp` in `initializeAccounts()` method:
```cpp
auto account = std::make_shared<Account>("CARD_NUMBER", "PIN", initialBalance);
m_accounts[account->getAccountId()] = account;
```

### Debugging
- Use Visual Studio debugger (F5)
- Set breakpoints in ATM.cpp for state transitions
- Check console output for error messages

## Notes

- This is a console-based simulator for educational purposes
- No real security - PIN stored in plain text
- No persistence - data lost on exit
- Single-threaded, synchronous operation
