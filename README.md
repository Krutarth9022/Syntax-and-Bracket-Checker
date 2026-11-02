# Syntax and Bracket Checker

A C++17 program that validates bracket balance and HTML/XML tag structure using stack-based logic. It is designed to detect syntax issues such as mismatched parentheses, unclosed braces, malformed tags, and missing closing delimiters, while pointing to the exact line and column where the problem occurs.

## Overview

This project demonstrates the use of two stack implementations:

- Array-based stack
- Linked-list stack

Both stack types implement the same interface and are used to analyze code structure for balanced delimiters and nested tags.

The checker supports:

- C++ bracket validation: `()`, `{}`, `[]`
- HTML tag validation
- XML tag validation
- Diagnostic output with line numbers and caret markers
- Detection of:
  - extra closing brackets
  - mismatched bracket pairs
  - unclosed brackets/tags
  - malformed comments and strings
  - unclosed tags and invalid nesting

## Features

### 1. Bracket checking for C++ files
The checker scans source code while ignoring comments, string literals, character literals, and raw strings. It correctly handles cases such as:

- nested braces and parentheses
- mismatched closing symbols
- unterminated strings or comments
- raw C++ string literals
- digit separators like `1'000`

### 2. Tag checking for HTML/XML files
It reads HTML or XML content and validates nested tag structure. It also handles:

- self-closing tags
- void HTML elements like `br`, `img`, `hr`, etc.
- comments (`<!-- -->`)
- CDATA sections
- DOCTYPE declarations and processing instructions

### 3. Stack-based implementation
The project compares two stack designs:

- `ArrayStack`: fixed-size stack using a dynamic array
- `LinkedStack`: variable-size stack implemented with a linked list

This makes it useful for learning data structures while also being practical for syntax validation.

## Project Structure

```text
Syntax-and-Bracket-Checker/
├── README.md
├── syntax_checker.cpp
└── (compiled binary output after building)
```

## How it works

The program:

1. Reads the selected file line by line.
2. Detects whether the file is a C++ source file or HTML/XML file.
3. Uses a stack to track opening symbols or tags.
4. Compares closing symbols against the most recent opening one.
5. Prints detailed error reports with:
   - error type
   - line number
   - message
   - source snippet
   - pointer (`^`) near the offending character

## Build and Run

### Compile

```bash
g++ -std=c++17 syntax_checker.cpp -o checker
```

### Run the interactive menu

```bash
./checker
```

### Run directly on a file

```bash
./checker myfile.cpp
./checker index.html
./checker data.xml
```

The program automatically detects file type by extension:

- `.cpp`, `.h`, `.hpp` → C++ bracket validation
- `.html`, `.htm` → HTML validation
- `.xml`, `.xhtml`, `.svg` → XML validation

## Menu Options

When you run the program without arguments, the menu includes:

1. Check brackets in a C++ file
2. Check tags in an HTML file
3. Check tags in an XML file
4. Test both stacks
5. Exit

## Example Output

```text
========================================================
 File  : sample.cpp (12 lines)
 Check : C++ brackets
 Stack : Linked list stack
========================================================

[Error 1] Brackets do not match  (line 7)
    ']' was closed by ')'.
    7 | if (x == [1, 2, 3)) {
          |                   ^

    '[' was opened here (line 7):
    7 | if (x == [1, 2, 3)) {
          |          ^

========================================================
 RESULT: 1 error(s) found. Please fix them and run again.
========================================================
```

## Example Use Cases

- Validate C++ source code before compiling
- Check HTML structure for unmatched tags
- Review XML documents for malformed nesting
- Demonstrate stack implementation behavior in learning projects

## Notes

This project is a single-file C++ application intended for learning and debugging. It is useful as a classroom example or mini utility for syntax validation.

## License

This project does not currently include a formal license file. If you plan to share or distribute it publicly, you may want to add an appropriate open-source license such as MIT or Apache-2.0.

## Future Improvements

Possible enhancements include:

- support for JavaScript or CSS validation
- line/column diagnostics for all reported issues
- GUI or web-based front end
- improved parser for more languages
- unit tests for common invalid samples

## Author

Built as a syntax and stack-checking project in C++.
