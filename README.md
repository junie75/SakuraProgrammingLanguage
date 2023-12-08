# SakuraProgrammingLanguage ![Cherry Blossoms](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Images/cherry-blossom-clip-art-12-transparent.png)

Sakura is a simple programming language created for my CS3335 class "Survey of Programming Languages." 
Sakura is a minimal subset of SPL (Simple Programming Language), created by Dr. Arthur Hanna of St. Mary's University, which is an amalgamation of high-level programming constructs he has run into during his computing career. 
Sakura is a free-format, case-sensitive language that is inspired by programming languages such as: C++, Python, and JavaScript. 
<br><br>The main goals for my language were: <br>
 👁️ Readability <br>
 💡 Efficiency <br>
 🩷 Pleasing aesthetics <br>

 ## Table of Contents

- [Project Title](#SakuraProgrammingLanguage)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
  - [Language Syntax](#language-syntax)
  - [Usage](#usage)
  - [Example Code](#example-code)
  - [Acknowledgments](#acknowledgments)

## Features
Sakura is designed to support the following language features:
- **Simple Syntax**: Sakura has a straightforward and easy-to-understand syntax.
- **Basic Data Types**: Support for integers, boolean, and strings.
- **Arithmetic Operations**: Addition, subtraction, multiplication, remainder, and division are supported.
- **Control Flow**: Basic control flow statements such as if-else conditions and do-while/while loops.
- **I/O**: Support for basic input and output in the console.
- **Scalar Variables**: defines local and global scalar variables and constants.
- **Expressions**: Expresses computation using expression syntax, including unary and binary operators.

Some of the interesting components and stylistic choices are 
- Minimal punctuation requirements.
- Backticks as string delimiters designed to eliminate the need for escape characters.
- Nested block comments in the shape of smiley faces (-:.
- Support for multiple L-value variables in a single assignment statement.
- Support for variable declaration with multiple variables.
- Sakura source files must be saved with the .kura extension. 

 STM is the Stack-based Target Machine for the Sakura compiler. STM has a built-in assembler which assembles directly to main memory. 
 The resulting machine program is automatically executed whenver the assembled STM program contains no errors. 

## Getting Started

Follow these steps to get started with Sakura:

### Prerequisites

- Make sure you have any IDE that can run C/C++ code installed.

### Installation

1. Clone the MyLang repository:

   ```bash
   git clone https://github.com/junie75/SakuraProgrammingLanguage.git

2. Navigate to the project directory
   ```bash
   cd SakuraProgrammingLanguage

3. Compile and run the SakuraCompiler.cpp file in your IDE. This will generate the SakuraCompiler executable used to compile your source file into assembly language

4. Compile and run the STM.c file in your IDE. This will generate the STM executable used to translate your source file into machine code.

### Usage

1. Create your source file and save it with the ".kura" extension. <br>**Note: Make sure the save as type is "All Files" and that it is saved in the same folder as SakuraCompiler.cpp and STM.c**
   ![img tutorial 1](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial1.png) <br> ![img tutorial 2](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial2.png)


2. Run the SakuraCompiler executable and enter the sourcefile name when prompted. <br>**Note: The sourcefile name can be entered without the .kura extension, the compiler will find the extension automatically**
   ![img tutorial 3](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial3.png)


3. You will notice the creation of a list file and an stm file with the same name as your source file. The list file can be used to debug errors in the source code, and the stm file will translate the .kura file into machine code.
   ![img tutorial 4](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial4.png)


4. Next run the STM executable and enter the sourcefile name when prompted. <br>**Note: the sourcefile name should be entered without an extension. The STM program will find the .stm file automatically**
   ![img tutorial 5](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial5.png)
   <br>**Note: You will notice the creation of a log file with the same name as your source file. This file contains both the assembler listing file and the trace of the run-time execution of the resulting machine code.**


5. Now you are ready to start programming in Sakura! 😊 
   

## Language Syntax

A complete overview of the language syntax can be found in the Sakura1LanguageReference.doc [here](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Documentation/SakuraLanguageReference.doc).

## Example Code

## Acknowledgements

The SakuraProgrammingLanguage project builds upon concepts learned from the "Survey of Programming Languages" CS3335 course at St. Mary's University, San Antonio, Texas. <br>
The lexer and parser implementations are inspired by "Simple Programming Language (SPL)," created by Dr. Arthur Hanna. A copy of the SPL Language Reference can be found in the documentation folder. 
