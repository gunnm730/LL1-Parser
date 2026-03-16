# LL1-Parser

A LL(1) Syntax Analyzer implemented in C++ for Compiler Theory Lab 2.

## Features

- **Grammar Input**: Read grammar from input file
- **Left Recursion Elimination**: Automatically detect and eliminate left recursion
- **Backtracking Elimination**: Eliminate backtracking for LL(1) parsing
- **First & Follow Sets**: Compute FIRST and FOLLOW sets for all non-terminals
- **LL(1) Parsing Table**: Build predictive parsing table
- **Syntax Analysis**: Perform syntax analysis on input strings

## Project Structure

```
LL1-Parser/
├── src/
│   ├── Main.cpp                  # Main entry point
│   ├── Syntactic_Analysis.h     # Header file
│   └── Syntactic_Analysis.cpp   # Implementation
└── README.md
```

## Usage

1. Prepare a grammar file with productions in the format:
   ```
   S::=aAb
   A::=c|@
   ```

2. Compile the project (requires C++11 or later):
   ```bash
   g++ -std=c++11 src/*.cpp -o LL1-Parser
   ```

3. Run the program:
   ```bash
   ./LL1-Parser
   ```

4. Enter:
   - Grammar file path
   - String to analyze

## Grammar Format

- Use `::=` as production symbol
- Use `|` to separate alternatives
- Use `@` for epsilon (empty string)
- Example: `A::=aA|@` represents A → aA | ε

## Author

- Student: 马子木
- ID: B23041707

## Course

Compiler Theory - Lab 2
