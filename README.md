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
  - [License](#license)
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

1. Create your source file and save it with the ".kura" extension. <br>***Note: Make sure the save as type is "All Files" and that it is saved in the same folder as SakuraCompiler.cpp and STM.c***
   ![img tutorial 1](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial1.png) <br> ![img tutorial 2](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial2.png)


2. Run the SakuraCompiler executable and enter the sourcefile name when prompted. <br>***Note: The sourcefile name can be entered without the .kura extension, the compiler will find the extension automatically***
   ![img tutorial 3](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial3.png)


3. You will notice the creation of a list file and an stm file with the same name as your source file. The list file can be used to debug errors in the source code, and the stm file will translate the .kura file into machine code.
   ![img tutorial 4](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial4.png)


4. Next run the STM executable and enter the sourcefile name when prompted. <br>**Note: the sourcefile name should be entered without an extension. The STM program will find the .stm file automatically**
   ![img tutorial 5](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/Tutorial5.png)
   <br>***Note: You will notice the creation of a log file with the same name as your source file. This file contains both the assembler listing file and the trace of the run-time execution of the resulting machine code.***


5. Now you are ready to start programming in Sakura! 😊 
   

## Language Syntax

A complete overview of the language syntax can be found in the Sakura1LanguageReference.doc [here](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Documentation/SakuraLanguageReference.doc).

## Example Code

Some sample .kura programs can be found in the SampleCode folder. Right now, we will run through P6.kura
![Example Program](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/P6Code.png)

1. Comments

   ```
   (-:---------------------------------------------------------:-)
   ~~ Juni Ejere
   ~~ Problem #6
   ~~ P6.kura
   (-:---------------------------------------------------------:-)

 Sakura supports both single line and block comments. Single line comments can be created using a double tilda "~~" symbol. 
 <br>Block comments can be created with an east-pointing smiley face "(-:" and closed with a west-pointing smiley face ":-)".

 2. Main and Variable Declaractions

    ```
    main:
       int x, y
       bool m, z <- false
       const bool j <- true 

A Sakura main program module is defined with the keyword "main" and a colon. A reminder that Sakura is case sensitive, and all reserved words are in lower case.
<br>Global variables can be defined before the main program module. In this example we are declaring 5 variables local to the main program module. 
<br> Sakura supports variable declaration with multiple variables. The statement "int x, y" declares two integer variables named "x" and "y".
<br> Sakura variables can be initialized when being defined. Sakura also supports multiple L-value variables in a single assignment statement. The statement "bool m, z <- false" declares two 
boolean variables named "m" and "z", and initializes both of them to false. The statement "const bool j <- true" declares a boolean constant   named "j" and initializes it to true. 
<br>Sakura constants must be initialized upon declaration and their value cannot be changed. 

3. Loops and User input
   ```
   do
     input `x? ` >> x
   while (x != 0)
This segment of the code is an example of a classic post-test unbounded loop in Sakura. Here, the program prompts the user with the "x? " string and takes that input and places it into the variable x with what is named an inshift operator ">>" 
<br> The program continues to prompt the user until the user enters 0 for x. 
***Note: Strings in Sakura are delimited with
a backtick " ` " rather than double quotes, so that if the programmer wanted to print a double quote or single quote character, it can be done so without the need for escape characters.*** <br><br>


4. Conditions and Output
   ```
   do
      input `x? ` >> x
   while ( x != 0 )
      input `y? ` >> y
      if ( x < y ) 
         output << `x  < y` << ^n
      elseif ( x = y )
         output << `x  = y` << ^n
      elseif ( x > y ) 
         output << `x  > y` << ^n
      endif
   endwhile
In the remainder of the do-while loop we began, if the user's input for x is not 0, we will prompt the user to enter a value for y, compare the two integers, and print out the result of the comparison. The first if-statement
checks to see if x is less than y. if it is, it prints out "x < y" and an endline character "^n". If that condition evaluates to false, it moves on to the elseif-statement, which checks if the two variables are equal. The last 
else-if ( which could technically be an else since this is the only option remaining) checks if x is larger than y, and prints the result. If-statements, or a series of if-elseif-else blocks are ended with the "endif" keyword. 
Do While/while loops are ended with the "endwhile" keyword. <br>
Print statements in Sakura are similar to input statement, except the flow of information (and the direction of the arrows) point to the output keyword with what is called the outshift operator "<<", 
while the flow of information in input statements point to the variable ">>". <br> In the output statements in this example, we send the string we want to print, and then an endline. 
<br>Sakura can print strings, variables, endlines, or expressions and each must be seperated with the outshift operator.  

5. Logical operators and Expressions
   ```
   output << `"(m or j) and z = ` << (m or j) and z << ` "\n`
The last bit of code in this program is a print statement using logical operators to compare boolean values, and displaying the result. The parenthesis in the expression "(m or j) and z" will evaluate first, which will result to true
because m is false, j is true, and false or true = true. Then the result of that comparison is and-ed with z, which is also false. Therefore, the entire expression results to false, because true and false = false. 
<br> You will notice a double quote character in the first string, and a double quote character in the last string plus a newline character "\n". I wanted to display Sakura's capability to print double quote characters without the need to put
a backslash before the quote ' \" ' as with most escape sequences. However, I wanted the programmer to have the capability of using other escape characters (such as \n, \t, \a, etc.) in their programs. 
<br> The resulting output of this statement should be
  "(m or j) and z = F ".
<br><br> This is the output for P6.kura! <br><br> ![Example Output](https://github.com/junie75/SakuraProgrammingLanguage/blob/master/Tutorial/P6EXE.png)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

The SakuraProgrammingLanguage project builds upon concepts learned from the "Survey of Programming Languages" CS3335 course at St. Mary's University, San Antonio, Texas. <br>
The lexer and parser implementations are inspired by "Simple Programming Language (SPL)," created by Dr. Arthur Hanna. A copy of the SPL Language Reference can be found in the documentation folder. 
