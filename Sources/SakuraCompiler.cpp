//-----------------------------------------------------------
// Juni Ejere
// Sakura Compiler
// SakuraCompiler.cpp
//-----------------------------------------------------------
#include <iostream>
#include <iomanip>

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <vector>
#include <time.h>

using namespace std;

/*TRACERS -- USE TO DEBUG CODE*/
//#define TRACEREADER
//#define TRACESCANNER
//#define TRACEPARSER
//#define TRACEIDENTIFIERTABLE
//#define TRACECOMPILER

#include "Sakura.h"

//-----------------------------------------------------------
typedef enum //definition of all Sakura pseudo-terminal and terminal symbols
//-----------------------------------------------------------
{
// pseudo-terminals
   IDENTIFIER,
   INTEGER,
   STRING,
   EOPTOKEN,
   UNKTOKEN,
// reserved words
   MAIN,
   INPUT,
   OUTPUT,
   ENDL,
   OR,
   NOR,
   XOR,
   AND,
   NAND,
   NOT,
   ABS,
   TRUE,
   FALSE,
   IF,
   ELSE,
   ELSEIF,
   DO,
   WHILE,
   ENDIF,
   ENDWHILE,
   INT, 
   BOOL,
   CONST,
// punctuation
   COLON, // ":"
   COMMA, // ","
   OUTSHIFT, //<< 
   INSHIFT,// >>
   ASSIGN, // <- "has"
   OPARENTHESIS, //"("
   CPARENTHESIS,  // ")"
// operators
   PIPE, // (***possibly future ABS operator****)
   LT, // LESS THAN "<"
   LTEQ, //LESS THAN OR EQ "<="
   EQ, //EQUAL "="
   GT, //GREATER THAN ">"
   GTEQ,//GREATER THAN OR EQ ">="
   NOTEQ, // NOT EQUAL "!="
   PLUS, // "+"
   MINUS, // "-"
   MULTIPLY,//  "*"
   DIVIDE,// "/"
   MODULUS, // "%"
   POWER,  // ^ and **
   INC, // ++
   DEC // --
} TOKENTYPE;
//-----------------------------------------------------------
struct TOKENTABLERECORD //defines object of token table record
//-----------------------------------------------------------
{
   TOKENTYPE type;
   char description[12+1];
   bool isReservedWord;
};

//-----------------------------------------------------------
const TOKENTABLERECORD TOKENTABLE[] = //instance of tokentable record with Sakura pseudo-terminals and terminals
//-----------------------------------------------------------
{
   { IDENTIFIER  ,"IDENTIFIER"  ,false },
   { INTEGER     ,"INTEGER"     ,false },
   { STRING      ,"STRING"      ,false },
   { EOPTOKEN    ,"EOPTOKEN"    ,false },
   { UNKTOKEN    ,"UNKTOKEN"    ,false },
   { MAIN        ,"main"        ,true  },
   { OUTPUT      ,"output"      ,true  },
   { IF          ,"if"          ,true  },
   { ELSEIF      ,"elseif"      ,true  },
   { ELSE        ,"else"        ,true  },
   { DO          ,"do"          ,true  },
   { WHILE       ,"while"       ,true  },
   { ENDIF       ,"endif"       ,true  },
   { ENDWHILE    ,"endwhile"    ,true  },
   { INPUT       ,"input"       ,true  },
   { ENDL        ,"ENDL"        ,false }, 
   { OUTSHIFT    ,"OUTSHIFT"    ,false },
   { INSHIFT     ,"INSHIFT"     ,false },
   { COLON       ,"COLON"       ,false },
   { COMMA       ,"COMMA"       ,false },
   { ASSIGN      ,"ASSIGN"      ,false },
   { INT         ,"int"         ,true  },
   { BOOL        ,"bool"        ,true  },
   { CONST       ,"const"       ,true  },
   { OR          ,"or"          ,true  },
   { NOR         ,"nor"         ,true  },
   { XOR         ,"xor"         ,true  },
   { AND         ,"and"         ,true  },
   { NAND        ,"nand"        ,true  },
   { NOT         ,"not"         ,true  },
   { ABS         ,"abs"         ,true  },
   { TRUE        ,"true"        ,true  },
   { FALSE       ,"false"       ,true  },
   { OPARENTHESIS,"OPARENTHESIS",false },
   { CPARENTHESIS,"CPARENTHESIS",false },
   { LT          ,"LT"          ,false },
   { LTEQ        ,"LTEQ"        ,false },
   { EQ          ,"EQ"          ,false },
   { GT          ,"GT"          ,false },
   { GTEQ        ,"GTEQ"        ,false },
   { NOTEQ       ,"NOTEQ"       ,false },
   { PLUS        ,"PLUS"        ,false },
   { MINUS       ,"MINUS"       ,false },
   { MULTIPLY    ,"MULTIPLY"    ,false },
   { DIVIDE      ,"DIVIDE"      ,false },
   { MODULUS     ,"MODULUS"     ,false },
   { PIPE        ,"PIPE"        ,false },
   { POWER       ,"POWER"       ,false },
   { INC         ,"INC"         ,false },
   { DEC         ,"DEC"         ,false }
};

//-----------------------------------------------------------
struct TOKEN // used to build instances of the pseudoterminals and terminals in the tokentable
//-----------------------------------------------------------
{
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH+1];
   int sourceLineNumber;
   int sourceLineIndex;
};

//--------------------------------------------------
// Global variables
//--------------------------------------------------
READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);
LISTER lister(LINESPERPAGE);
// CODEGENERATION
CODE code;
IDENTIFIERTABLE identifierTable(&lister,MAXIMUMIDENTIFIERS);
// ENDCODEGENERATION

#ifdef TRACEPARSER
int level;
#endif

//-----------------------------------------------------------
void EnterModule(const char module[])  //tracks the entrance into a new module
//-----------------------------------------------------------
{
#ifdef TRACEPARSER
   char information[SOURCELINELENGTH+1];

   level++;
   sprintf(information,"   %*s>%s",level*2," ",module);
   lister.ListInformationLine(information);
#endif
}

//-----------------------------------------------------------
void ExitModule(const char module[])  //tracks the exit from a module
//-----------------------------------------------------------
{
#ifdef TRACEPARSER
   char information[SOURCELINELENGTH+1];

   sprintf(information,"   %*s<%s",level*2," ",module);
   lister.ListInformationLine(information);
   level--;
#endif
}

//--------------------------------------------------
void ProcessCompilerError(int sourceLineNumber,int sourceLineIndex,const char errorMessage[])//outputs an error message
//--------------------------------------------------
{
   char information[SOURCELINELENGTH+1];

// Use "panic mode" error recovery technique: report error message and terminate compilation!
   sprintf(information,"     At (%4d:%3d) %s",sourceLineNumber,sourceLineIndex,errorMessage);
   lister.ListInformationLine(information);
   lister.ListInformationLine("Sakura compiler ending with compiler error!\n");
   throw( SAKURAEXCEPTION("Sakura compiler ending with compiler error!") );
}

//-----------------------------------------------------------
int main() //finds file to compile and execute
//-----------------------------------------------------------
{
   void Callback1(int sourceLineNumber,const char sourceLine[]);
   void Callback2(int sourceLineNumber,const char sourceLine[]);
   void GetNextToken(TOKEN tokens[]);
   void ParseSPLProgram(TOKEN tokens[]);

   char sourceFileName[80+1];
   TOKEN tokens[LOOKAHEAD+1];
   
   //Receive the file to compile
   cout << "Source filename? ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName); //creation of list file
      code.OpenFile(sourceFileName); //creation of stm file

// CODEGENERATION
      code.EmitBeginningCode(sourceFileName);
// ENDCODEGENERATION
      reader.SetLister(&lister);
      reader.AddCallbackFunction(Callback1);
      reader.AddCallbackFunction(Callback2);
      reader.OpenFile(sourceFileName);

   // Fill tokens[] for look-ahead
      for (int i = 0; i <= LOOKAHEAD; i++)
         GetNextToken(tokens);

#ifdef TRACEPARSER
      level = 0;
#endif
   
   //begin parsing the program
      ParseSPLProgram(tokens);
      
// CODEGENERATION
      code.EmitEndingCode();
// ENDCODEGENERATION
   }
  catch (SAKURAEXCEPTION sakuraException)
   {
      cout << "Sakura exception: " << sakuraException.GetDescription() << endl;
   }
   lister.ListInformationLine("******* Sakura Compiler ending");
   cout << "Sakura Compiler ending\n";

   system("PAUSE");
   return( 0 );
   
}

//-----------------------------------------------------------
void ParseSPLProgram(TOKEN tokens[]) //parses a sakura program
//-----------------------------------------------------------
{
   void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
   void GetNextToken(TOKEN tokens[]);
   void ParsePROGRAMDefinition(TOKEN tokens[]);

   EnterModule("SakuraProgram");

//check for global data definitions
ParseDataDefinitions(tokens,GLOBALSCOPE);

#ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
#endif

//defining the main module
   if( tokens[0].type != MAIN)
   {
   	ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting main function");
   }
   else if( tokens[1].type != COLON)
   {
   	ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting colon after main"); 
   }
   else
   //begin parsing the main module, return when parsing of entire module is complete
   	ParsePROGRAMDefinition(tokens);
   
   //when program is completed, last token is an EOPTOKEN
   if ( tokens[0].type != EOPTOKEN )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

   ExitModule("SakuraProgram");
}

//-----------------------------------------------------------
void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope) //parses data definitions
//-----------------------------------------------------------
{
   void GetNextToken(TOKEN tokens[]);

   EnterModule("DataDefinitions");

   //check for variables being defined
   while ((tokens[0].type == INT) || (tokens[0].type == BOOL) || (tokens[0].type == CONST) )
   {
      switch ( tokens[0].type )
      {
         case INT: //if an integer variable is declared
             do
             {
                 char identifier[MAXIMUMLENGTHIDENTIFIER + 1];
                 char literal[MAXIMUMLENGTHIDENTIFIER+1];
                 char reference[MAXIMUMLENGTHIDENTIFIER + 1];
                 DATATYPE datatype;
                 bool isInTable;
                 int index;
                 bool initialized = false;

                 //set datatype
                 datatype = INTEGERTYPE;

                 GetNextToken(tokens);

                 //check for name of variable after
                 if (tokens[0].type != IDENTIFIER)
                     ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
                 strcpy(identifier, tokens[0].lexeme);
                 GetNextToken(tokens);

                 //check if variable is assigned a value
                 if (tokens[0].type == ASSIGN)
                 {
                    GetNextToken(tokens);

                    //if the value is an integer, save the literal value
                    if (tokens[0].type == INTEGER)
                    {
                        strcpy(literal, "0D");
                        strcat(literal, tokens[0].lexeme);
                        initialized = true;
                    }
                    else
                        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");
                        
                    GetNextToken(tokens);
                 }

                 //check if identifier is already defined
                 index = identifierTable.GetIndex(identifier, isInTable);
                 if (isInTable && identifierTable.IsInCurrentScope(index))
                     ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier");

                  //save variable based on the scope
                 switch (identifierScope)
                 {
                  
                 case GLOBALSCOPE: //if the value is a global variable
                    if(initialized) //if it has a value, save it is as a DW in static data
                 	{
                 	 // CODEGENERATION
                 	 code.AddDWToStaticData(literal,identifier,reference);
                 	 // CODEGENERATION
					   }
                 	else //if it is not assigned a value, save it as a RW in static data
                 	{
                     // CODEGENERATION
                     code.AddRWToStaticData(1, identifier, reference);
                     // ENDCODEGENERATION
                	}
                  //add the variable to the identifier table
                     identifierTable.AddToTable(identifier, GLOBAL_VARIABLE, datatype, reference);
                     break;
                  
                 case PROGRAMMODULESCOPE: //if the value is a local variable
                    if(initialized)
                 	{
                 	 // CODEGENERATION
                 	 code.AddDWToStaticData(literal,identifier,reference);
                 	 // CODEGENERATION
					   }
                 	else
                 	{
                     // CODEGENERATION
                     code.AddRWToStaticData(1, identifier, reference);
                     // ENDCODEGENERATION
                	}
                     identifierTable.AddToTable(identifier, PROGRAMMODULE_VARIABLE, datatype, reference);
                     break;
                 }
             
             } while (tokens[0].type == COMMA); // can declare multiple variables with the same datatype i.e. int x, y, num2
             break;
         case BOOL: //if a boolean variable is declared
             do
             {
                 char identifier[MAXIMUMLENGTHIDENTIFIER + 1];
                 char literal[MAXIMUMLENGTHIDENTIFIER+1];
                 char reference[MAXIMUMLENGTHIDENTIFIER + 1];
                 DATATYPE datatype;
                 bool isInTable;
                 int index;
                 bool initialized = false;

                 //set datatype
                 datatype = BOOLEANTYPE;

                 GetNextToken(tokens);

                 //check for name of variable after
                 if (tokens[0].type != IDENTIFIER)
                     ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
                 strcpy(identifier, tokens[0].lexeme);
                 GetNextToken(tokens);

                 //check if variable is assigned a value
                 if (tokens[0].type == ASSIGN)
                 {
                     GetNextToken(tokens);
                     //if the value is a boolean, save the literal 
                     if ((tokens[0].type == TRUE) || (tokens[0].type == FALSE))
                     {
                         strcpy(literal, tokens[0].lexeme);
                         initialized = true;
                     }
                     else
                         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");

                     GetNextToken(tokens);
                 }

                 //check if identifier is already defined
                 index = identifierTable.GetIndex(identifier, isInTable);
                 if (isInTable && identifierTable.IsInCurrentScope(index))
                     ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier");

                 switch (identifierScope)
                 {
                 case GLOBALSCOPE:
                 	if(initialized)
                 	{
                 	 // CODEGENERATION
                 	 code.AddDWToStaticData(literal,identifier,reference);
                 	 // CODEGENERATION
					}
                 	else
                 	{
                     // CODEGENERATION
                     code.AddRWToStaticData(1, identifier, reference);
                     // ENDCODEGENERATION
                	}
                     identifierTable.AddToTable(identifier, GLOBAL_VARIABLE, datatype, reference);
                     break;
                 case PROGRAMMODULESCOPE:
                 	if(initialized)
                 	{
                 	 // CODEGENERATION
                 	 code.AddDWToStaticData(literal,identifier,reference);
                 	 // CODEGENERATION
					}
					else
					{
                     // CODEGENERATION
                     code.AddRWToStaticData(1, identifier, reference);
                     // ENDCODEGENERATION
                 	}
                     identifierTable.AddToTable(identifier, PROGRAMMODULE_VARIABLE, datatype, reference);
                     break;
                 }

             } while (tokens[0].type == COMMA); 
             break;
         case CONST: //if a constant variable has been declared
            do
            {
               char identifier[MAXIMUMLENGTHIDENTIFIER+1];
               char literal[MAXIMUMLENGTHIDENTIFIER+1];
               char reference[MAXIMUMLENGTHIDENTIFIER+1];
               DATATYPE datatype;
               bool isInTable;
               int index;
      
               GetNextToken(tokens);
      
               //check if constant int or bool is being declared and save the datatype
               switch ( tokens[0].type )
               {
                  case INT:
                     datatype = INTEGERTYPE;
                     break;
                  case BOOL:
                     datatype = BOOLEANTYPE;
                     break;
                  default:
                     ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting INT or BOOL");
               }
               GetNextToken(tokens);

               //get the name of the variable
               if (tokens[0].type != IDENTIFIER)
                   ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
               strcpy(identifier, tokens[0].lexeme);
               GetNextToken(tokens);

               //all constant variables must be initialized; check for equal sign
               if (tokens[0].type != ASSIGN)
                   ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '<-'");
               GetNextToken(tokens);
         
               //if variable is an integer and the const value is integer, save the literal
               if      ( (datatype == INTEGERTYPE) && (tokens[0].type == INTEGER) )
               {
                  strcpy(literal,"0D");
                  strcat(literal,tokens[0].lexeme);
               }
               //if variable is a boolean and the const value is a bool, save the literal
               else if ( ((datatype == BOOLEANTYPE) && (tokens[0].type ==    TRUE))
                      || ((datatype == BOOLEANTYPE) && (tokens[0].type ==   FALSE))  )
                 strcpy(literal,tokens[0].lexeme);
               else
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Data type mismatch");
               GetNextToken(tokens);
          
               //check if variable is already defined
               index = identifierTable.GetIndex(identifier,isInTable);
               if ( isInTable && identifierTable.IsInCurrentScope(index) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");

               switch ( identifierScope )
               {
                  case GLOBALSCOPE:
// CODEGENERATION
                     code.AddDWToStaticData(literal,identifier,reference);
// ENDCODEGENERATION
                     identifierTable.AddToTable(identifier,GLOBAL_CONSTANT,datatype,reference);
                     break;
                  case PROGRAMMODULESCOPE:
// CODEGENERATION
                     code.AddDWToStaticData(literal,identifier,reference);
// ENDCODEGENERATION
                     identifierTable.AddToTable(identifier,PROGRAMMODULE_CONSTANT,datatype,reference);
                     break;
               }
            } while ( tokens[0].type == COMMA);

            break;
       }
   }

   ExitModule("DataDefinitions");

}

//-----------------------------------------------------------
void ParsePROGRAMDefinition(TOKEN tokens[]) //parses a program module
//-----------------------------------------------------------
{
   void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
   void GetNextToken(TOKEN tokens[]);
   void ParseStatement(TOKEN tokens[]);
   
   char line[SOURCELINELENGTH+1];
   char label[SOURCELINELENGTH+1];
   char reference[SOURCELINELENGTH+1];
   
   EnterModule("PROGRAMDefinition");
   
// CODEGENERATION
   code.EmitUnformattedLine("; **** =========");
   sprintf(line,"; **** PROGRAM module (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
   code.EmitFormattedLine("PROGRAMMAIN","EQU"  ,"*");

   code.EmitFormattedLine("","PUSH" ,"#RUNTIMESTACK","set SP");
   code.EmitFormattedLine("","POPSP");
   code.EmitFormattedLine("","PUSHA","STATICDATA","set SB");
   code.EmitFormattedLine("","POPSB");
   code.EmitFormattedLine("","PUSH","#HEAPBASE","initialize heap");
   code.EmitFormattedLine("","PUSH","#HEAPSIZE");
   code.EmitFormattedLine("","SVC","#SVC_INITIALIZE_HEAP");
   sprintf(label,"PROGRAMBODY%04d",code.LabelSuffix());
   code.EmitFormattedLine("","CALL",label);
   code.AddDSToStaticData("Normal program termination","",reference);
   code.EmitFormattedLine("","PUSHA",reference);
   code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
   code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
   code.EmitFormattedLine("","PUSH","#0D0","terminate with status = 0");
   code.EmitFormattedLine("","SVC" ,"#SVC_TERMINATE");
   code.EmitUnformattedLine("");
   code.EmitFormattedLine(label,"EQU","*");
// ENDCODEGENERATION

   //push past the main and the colon token
   GetNextToken(tokens);
   GetNextToken(tokens);
   
   //parse local data definitions
   identifierTable.EnterNestedStaticScope();
   ParseDataDefinitions(tokens,PROGRAMMODULESCOPE);

   //parse statements of the file until the program is complete
   while ( tokens[0].type != EOPTOKEN )
      ParseStatement(tokens);
      
// CODEGENERATION
   code.EmitFormattedLine("","RETURN");
   code.EmitUnformattedLine("; **** =========");
   sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
// ENDCODEGENERATION

#ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
#endif

   identifierTable.ExitNestedStaticScope();

   GetNextToken(tokens);

   ExitModule("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement(TOKEN tokens[]) //parses a statement and determines what type of statement
//-----------------------------------------------------------
{
   void GetNextToken(TOKEN tokens[]);
   void ParsePRINTStatement(TOKEN tokens[]);
   void ParseINPUTStatement(TOKEN tokens[]);
   void ParseAssignmentStatement(TOKEN tokens[]);
   void ParseIFStatement(TOKEN tokens[]);
   void ParseDOWHILEStatement(TOKEN tokens[]);


   EnterModule("Statement");

   //check for the type of statement based on beginning keyword
   switch ( tokens[0].type )
   {
      case OUTPUT:
         ParsePRINTStatement(tokens);
         break;
      case INPUT:
         ParseINPUTStatement(tokens);
         break;
      case IDENTIFIER:
         ParseAssignmentStatement(tokens);
         break;
      case IF:
         ParseIFStatement(tokens);
         break;
      case DO:
         ParseDOWHILEStatement(tokens);
         break;
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting beginning-of-statement");
         break;
   }

   ExitModule("Statement");
}

//-----------------------------------------------------------
void ParsePRINTStatement(TOKEN tokens[]) //parses a print statement
//-----------------------------------------------------------
{
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);
   
   char line[SOURCELINELENGTH+1];
   DATATYPE datatype;
   
   EnterModule("PRINTStatement");
   
// CODEGENERATION
   sprintf(line,"; **** PRINT statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
// ENDCODEGENERATION

   GetNextToken(tokens);
   
   //outshift "<<" should be first token after OUTPUT reserved word, so evaluted FIRST
   //evaluates what is being outputted until print statement is complete
   while ( tokens[0].type == OUTSHIFT && tokens[0].type != OUTPUT)
   {
   	GetNextToken(tokens);

      //check if a string, endline, or expression is being printed
      switch ( tokens[0].type )
      {
         case STRING:
         	
// CODEGENERATION
            char reference[SOURCELINELENGTH+1];

            code.AddDSToStaticData(tokens[0].lexeme,"",reference);
            code.EmitFormattedLine("","PUSHA",reference);
            code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
// ENDCODEGENERATION

            GetNextToken(tokens);
            break;

         case ENDL:   	
// CODEGENERATION
            code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
// ENDCODEGENERATION
            GetNextToken(tokens);
            break;

         default:
         {
            //if it is not a string or endline, assumes it is an expression
			ParseExpression(tokens,datatype);
// CODEGENERATION
            //after datatype of the token is received, output it via service request
            switch ( datatype )
            {
               case INTEGERTYPE:
                  code.EmitFormattedLine("","SVC","#SVC_WRITE_INTEGER");
                  break;
               case BOOLEANTYPE:
                  code.EmitFormattedLine("","SVC","#SVC_WRITE_BOOLEAN");
                  break;
            }
// ENDCODEGENERATION
		   } 

      } //end switch
   } //end while

   ExitModule("PRINTStatement");
}

//-----------------------------------------------------------
void ParseINPUTStatement(TOKEN tokens[]) //parses an input statement
//-----------------------------------------------------------
{
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char reference[SOURCELINELENGTH+1];
   char line[SOURCELINELENGTH+1];
   DATATYPE datatype;

   EnterModule("INPUTStatement");

   sprintf(line,"; **** INPUT statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens);

   //checks for the optional prompt outputted to the user, otherwise will prompt "? "
   if ( tokens[0].type == STRING )
   {
// CODEGENERATION
      code.AddDSToStaticData(tokens[0].lexeme,"",reference);
      code.EmitFormattedLine("","PUSHA",reference);
      code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
// ENDCODEGENERATION
      GetNextToken(tokens);
   }

   //checks for syntax error of not including ">>"
   if ( tokens[0].type != INSHIFT )
       ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
           "Expecting '>>'");
    
    GetNextToken(tokens);

   //parses the variable the user will input into
   ParseVariable(tokens,true,datatype);

// CODEGENERATION
   //what datatype will be read from the user is based on the datatype of the variable
   switch ( datatype )
   {
      case INTEGERTYPE:
         code.EmitFormattedLine("","SVC","#SVC_READ_INTEGER");
         break;
      case BOOLEANTYPE:
         code.EmitFormattedLine("","SVC","#SVC_READ_BOOLEAN");
         break;
   }
   code.EmitFormattedLine("","POP","@SP:0D1");
   code.EmitFormattedLine("","DISCARD","#0D1");
// ENDCODEGENERATION

   ExitModule("INPUTStatement");
}

//-----------------------------------------------------------
void ParseAssignmentStatement(TOKEN tokens[]) //parses an assignment statement
//-----------------------------------------------------------
{
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   DATATYPE datatypeLHS,datatypeRHS;
   int n; //check for the number of variables being assigned a value

   EnterModule("AssignmentStatement");

   sprintf(line,"; **** assignment statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   //parse the variable being assigned a value
   ParseVariable(tokens,true,datatypeLHS);
   n = 1;

   //assigns each variable after the comma the same value 
   while ( tokens[0].type == COMMA )
   {
      DATATYPE datatype;

      GetNextToken(tokens);
      //gets the next variable to be assigned and increments n
      ParseVariable(tokens,true,datatype);
      n++;

      //all variables must be of the same datatype as the first
      if ( datatype != datatypeLHS )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Mixed-mode variables not allowed");
   }

   //after comma seperated list of variables, assign token "<-" must be next
   if ( tokens[0].type != ASSIGN )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '<-'");
   GetNextToken(tokens);

   //parse the value being assigned to the variables
   ParseExpression(tokens,datatypeRHS);

   //value must be of the same datatype as the variables
   if ( datatypeLHS != datatypeRHS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Data type mismatch");

// CODEGENERATION
   for (int i = 1; i <= n; i++)
   {
      code.EmitFormattedLine("","MAKEDUP");
      code.EmitFormattedLine("","POP","@SP:0D2");
      code.EmitFormattedLine("","SWAP");
      code.EmitFormattedLine("","DISCARD","#0D1");
   }
   code.EmitFormattedLine("","DISCARD","#0D1");
// ENDCODEGENERATION

   ExitModule("AssignmentStatement");
}

//-----------------------------------------------------------
void ParseIFStatement(TOKEN tokens[]) //parses an if statement
//-----------------------------------------------------------
{
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void ParseStatement(TOKEN tokens[]);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   char Ilabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
   DATATYPE datatype;

   EnterModule("IFStatement");

   sprintf(line,"; **** IF statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens);

   //expression after the if statement must be in paranthesis
   if ( tokens[0].type != OPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
   GetNextToken(tokens);

   //parse the expression 
   ParseExpression(tokens,datatype);

   if ( tokens[0].type != CPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
   GetNextToken(tokens);

   //expression must result in a boolean value
   if ( datatype != BOOLEANTYPE )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

// CODEGENERATION
/* 
   Plan for the generalized IF statement with n ELIFs and 1 ELSE (*Note* n
      can be 0 and the ELSE may be missing and the plan still "works.")

   ...expression...           ; boolean expression on top-of-stack
      SETT
      DISCARD   #0D1
      JMPNT     I???1
   ...statements...
      JMP       E????
I???1 EQU       *             ; 1st ELIF clause
   ...expression...
      SETT
      DISCARD   #0D1
      JMPNT     I???2
   ...statements...
      JMP       E????
      .
      .
I???n EQU       *             ; nth ELIF clause
   ...expression...
      SETT
      DISCARD   #0D1
      JMPNT     I????
   ...statements...
      JMP       E????
I???? EQU       *             ; ELSE clause
   ...statements...
E???? EQU       *
*/
   sprintf(Elabel,"E%04d",code.LabelSuffix());
   code.EmitFormattedLine("","SETT");
   code.EmitFormattedLine("","DISCARD","#0D1");
   sprintf(Ilabel,"I%04d",code.LabelSuffix());
   code.EmitFormattedLine("","JMPNT",Ilabel);
// ENDCODEGENERATION

   while ( (tokens[0].type != ELSEIF) && 
           (tokens[0].type != ELSE) && 
           (tokens[0].type != ENDIF) )
      //parse the statements to be performed if the condition evaluates to true
      ParseStatement(tokens);

// CODEGENERATION
   code.EmitFormattedLine("","JMP",Elabel);
   code.EmitFormattedLine(Ilabel,"EQU","*");
// ENDCODEGENERATION

   //checks for elseif statement
   while ( tokens[0].type == ELSEIF )
   {
      GetNextToken(tokens);
      if ( tokens[0].type != OPARENTHESIS )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
      GetNextToken(tokens);
      ParseExpression(tokens,datatype);
      if ( tokens[0].type != CPARENTHESIS )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
      GetNextToken(tokens);

      if ( datatype != BOOLEANTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

// CODEGENERATION
      code.EmitFormattedLine("","SETT");
      code.EmitFormattedLine("","DISCARD","#0D1");
      sprintf(Ilabel,"I%04d",code.LabelSuffix());
      code.EmitFormattedLine("","JMPNT",Ilabel);
// ENDCODEGENERATION

      while ( (tokens[0].type != ELSEIF) && 
              (tokens[0].type != ELSE) && 
              (tokens[0].type != ENDIF) )
         //parse the statements to be executed if the else if is true
         ParseStatement(tokens);

// CODEGENERATION
      code.EmitFormattedLine("","JMP",Elabel);
      code.EmitFormattedLine(Ilabel,"EQU","*");
// ENDCODEGENERATION

   }
   //checks for else statement
   if ( tokens[0].type == ELSE )
   {
      GetNextToken(tokens);
      while ( tokens[0].type != ENDIF)
         //parse the statements to be executed if the else is true
         ParseStatement(tokens);
   }

   GetNextToken(tokens);

// CODEGENERATION
      code.EmitFormattedLine(Elabel,"EQU","*");
// ENDCODEGENERATION

   ExitModule("IFStatement");
}

//-----------------------------------------------------------
void ParseDOWHILEStatement(TOKEN tokens[]) //parses a do while statement
//-----------------------------------------------------------
{
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void ParseStatement(TOKEN tokens[]);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   char Dlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
   DATATYPE datatype;

   EnterModule("DOWHILEStatement");

   sprintf(line,"; **** DO-WHILE statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens);

// CODEGENERATION
/*
D???? EQU       *
   ...statements...
   ...expression...
      SETT
      DISCARD   #0D1
      JMPNT     E????
   ...statements...
      JMP       D????
E???? EQU       *
*/

   sprintf(Dlabel,"D%04d",code.LabelSuffix());
   sprintf(Elabel,"E%04d",code.LabelSuffix());
   code.EmitFormattedLine(Dlabel,"EQU","*");
// ENDCODEGENERATION

   //parse the DO statements before the "while"
   while ( tokens[0].type != WHILE )
      ParseStatement(tokens);
   GetNextToken(tokens);

   //parse the expression in the while statement
   if ( tokens[0].type != OPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
   GetNextToken(tokens);
   ParseExpression(tokens,datatype);
   if ( tokens[0].type != CPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
   GetNextToken(tokens);

   if ( datatype != BOOLEANTYPE )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

// CODEGENERATION
   code.EmitFormattedLine("","SETT");
   code.EmitFormattedLine("","DISCARD","#0D1");
   code.EmitFormattedLine("","JMPNT",Elabel);
// ENDCODEGENERATION

   //parse the statements performed if the while statement evaluates to true
   while ( tokens[0].type != ENDWHILE )
      ParseStatement(tokens);

   GetNextToken(tokens);

// CODEGENERATION
   code.EmitFormattedLine("","JMP",Dlabel);
   code.EmitFormattedLine(Elabel,"EQU","*");
// ENDCODEGENERATION

   ExitModule("DOWHILEStatement");
}

//-----------------------------------------------------------
void ParseExpression(TOKEN tokens[],DATATYPE &datatype) //evaluates an expression and all of its tokens
//-----------------------------------------------------------
{
// CODEGENERATION
/*
   An expression is composed of a collection of one or more operands (Sakura calls them
      primaries) and operators (and perhaps sets of parentheses to modify the default 
      order-of-evaluation established by precedence and associativity rules).
      Expression evaluation computes a single value as the expression's result.
      The result has a specific data type. By design, the expression result is 
      "left" at the top of the run-time stack for subsequent use.
   
   Sakura expressions must be single-mode with operators working on operands of
      the appropriate type (for example, boolean AND boolean) and not mixing
      modes. Static semantic analysis guarantees that operators are
      operating on operands of appropriate data type.
*/
// ENDCODEGENERATION

   void ParseConjunction(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Expression");

   //get the left hand side operand
   ParseConjunction(tokens,datatypeLHS);

   //check for the disjunction
   if ( (tokens[0].type ==  OR) ||
        (tokens[0].type == NOR) ||
        (tokens[0].type == XOR) )
   {
      while ( (tokens[0].type ==  OR) ||
              (tokens[0].type == NOR) ||
              (tokens[0].type == XOR) )
      {
         TOKENTYPE operation = tokens[0].type;

         GetNextToken(tokens);
         //get the right hand side operand
         ParseConjunction(tokens,datatypeRHS);
   
// CODEGENERATION
         switch ( operation ) //perform the boolean operation on the operands
         {
            case OR:

// STATICSEMANTICS : LHS and RHS on the stack must both be booleans
               if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","OR");
               datatype = BOOLEANTYPE;
               break;
            case NOR:
   
// STATICSEMANTICS
               if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","NOR");
               datatype = BOOLEANTYPE;
               break;
            case XOR:
   
// STATICSEMANTICS
               if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","XOR");
               datatype = BOOLEANTYPE;
               break;
         }
      }
// CODEGENERATION

   }
   else //if there is no disjunction, save the datatype and return
      datatype = datatypeLHS;

   ExitModule("Expression");
}

//-----------------------------------------------------------
void ParseConjunction(TOKEN tokens[],DATATYPE &datatype) //checks if token is a conjunction
//-----------------------------------------------------------
{
   void ParseNegation(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Conjunction");

   //get left hand side operand
   ParseNegation(tokens,datatypeLHS);

   //check for conjunction
   if ( (tokens[0].type ==  AND) ||
        (tokens[0].type == NAND) )
   {
      while ( (tokens[0].type ==  AND) ||
              (tokens[0].type == NAND) )
      {
         TOKENTYPE operation = tokens[0].type;
  
         GetNextToken(tokens);
         //get right hand side operand
         ParseNegation(tokens,datatypeRHS);
   
         switch ( operation )
         {
            case AND:
               if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
               code.EmitFormattedLine("","AND");
               datatype = BOOLEANTYPE;
               break;
            case NAND:
               if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
               code.EmitFormattedLine("","NAND");
               datatype = BOOLEANTYPE;
               break;
         }
      }
   }
   else //if no conjunction, save datatype and return
      datatype = datatypeLHS;

   ExitModule("Conjunction");
}

//-----------------------------------------------------------
void ParseNegation(TOKEN tokens[],DATATYPE &datatype) //checks if token is a negation
//-----------------------------------------------------------
{
   void ParseComparison(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeRHS;

   EnterModule("Negation");

   //check for negation
   if ( tokens[0].type == NOT )
   {
      GetNextToken(tokens);
      //get the value being negated
      ParseComparison(tokens,datatypeRHS);

      //value must be a boolean
      if ( !(datatypeRHS == BOOLEANTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operand");
      code.EmitFormattedLine("","NOT");
      datatype = BOOLEANTYPE;
   }
   else //if no negation, check for comparison
      ParseComparison(tokens,datatype);

   ExitModule("Negation");
}

//-----------------------------------------------------------
void ParseComparison(TOKEN tokens[],DATATYPE &datatype) //checks if token is a comparison
//-----------------------------------------------------------
{
   void ParseComparator(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparison");

   //get the left hand side operand
   ParseComparator(tokens,datatypeLHS);

   //check for comparison operator
   if ( (tokens[0].type ==    LT) ||
        (tokens[0].type ==  LTEQ) ||
        (tokens[0].type ==    EQ) ||
        (tokens[0].type ==    GT) ||
        (tokens[0].type ==  GTEQ) ||
        (tokens[0].type == NOTEQ)
      )
   {
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      //get the right hand side operand
      ParseComparator(tokens,datatypeRHS);

      //LHS and RHS at top of stack must be integers
      if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) ) 
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");
/*
      CMPI
      JMPXX     T????         ; XX = L,E,G,LE,NE,GE (as required)
      PUSH      #0X0000       ; push FALSE
      JMP       E????         ;    or 
T???? PUSH      #0XFFFF       ; push TRUE (as required)
E???? EQU       *
*/
      char Tlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];

      code.EmitFormattedLine("","CMPI");
      sprintf(Tlabel,"T%04d",code.LabelSuffix());
      sprintf(Elabel,"E%04d",code.LabelSuffix());
      switch ( operation ) //perform the comparison
      {
         case LT:
            code.EmitFormattedLine("","JMPL",Tlabel);
            break;
         case LTEQ:
            code.EmitFormattedLine("","JMPLE",Tlabel);
            break;
         case EQ:
            code.EmitFormattedLine("","JMPE",Tlabel);
            break;
         case GT:
            code.EmitFormattedLine("","JMPG",Tlabel);
            break;
         case GTEQ:
            code.EmitFormattedLine("","JMPGE",Tlabel);
            break;
         case NOTEQ:
            code.EmitFormattedLine("","JMPNE",Tlabel);
            break;
      }
      datatype = BOOLEANTYPE;
      code.EmitFormattedLine("","PUSH","#0X0000");
      code.EmitFormattedLine("","JMP",Elabel);
      code.EmitFormattedLine(Tlabel,"PUSH","#0XFFFF");
      code.EmitFormattedLine(Elabel,"EQU","*");
   }
   else //if there is no comparison, save the datatype and return
      datatype = datatypeLHS;

   ExitModule("Comparison");
}

//-----------------------------------------------------------
void ParseComparator(TOKEN tokens[],DATATYPE &datatype) //checks if token is a comparator
//-----------------------------------------------------------
{
   void ParseTerm(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparator");

   //parse the left hand side operand
   ParseTerm(tokens,datatypeLHS);

   //checks for addition or subtraction
   if ( (tokens[0].type ==  PLUS) ||
        (tokens[0].type == MINUS) )
   {
      while ( (tokens[0].type ==  PLUS) ||
              (tokens[0].type == MINUS) )
      {
         TOKENTYPE operation = tokens[0].type;
         
         GetNextToken(tokens);
         //gets the right hand side operand
         ParseTerm(tokens,datatypeRHS);

         if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

         switch ( operation ) //performs the addition or subtraction
         {
            case PLUS:
               code.EmitFormattedLine("","ADDI");
               break;
            case MINUS:
               code.EmitFormattedLine("","SUBI");
               break;
         }
         datatype = INTEGERTYPE;
      }
   }
   else //if no addition or subtraction, save the datatype of the operand and return
      datatype = datatypeLHS;
   
   ExitModule("Comparator");
}

//-----------------------------------------------------------
void ParseTerm(TOKEN tokens[],DATATYPE &datatype) //checks if token is a term
//-----------------------------------------------------------
{
   void ParseFactor(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Term");

   //gets the left hand side operand
   ParseFactor(tokens,datatypeLHS);
   //checks for multiplication, division, or remainder operators
   if ( (tokens[0].type == MULTIPLY) ||
        (tokens[0].type ==   DIVIDE) ||
        (tokens[0].type ==  MODULUS) )
   {
      while ( (tokens[0].type == MULTIPLY) ||
              (tokens[0].type ==   DIVIDE) ||
              (tokens[0].type ==  MODULUS) )
      {
         TOKENTYPE operation = tokens[0].type;
         
         GetNextToken(tokens);
         //gets the right hand side operand
         ParseFactor(tokens,datatypeRHS);

         if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

         switch ( operation ) //performs the multiplication, division, or remainder
         {
            case MULTIPLY:
               code.EmitFormattedLine("","MULI");
               break;
            case DIVIDE:
               code.EmitFormattedLine("","DIVI");
               break;
            case MODULUS:
               code.EmitFormattedLine("","REMI");
               break;
         }
         datatype = INTEGERTYPE;
      }
   }
   else //if not multiplying, dividing, or remaindering, save the datatype and return
      datatype = datatypeLHS;

   ExitModule("Term");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[],DATATYPE &datatype) //checks if token is a factor
//-----------------------------------------------------------
{
   void ParseSecondary(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Factor");
   TOKENTYPE operation;

   //checks for unary operators
   if ( (tokens[0].type ==  ABS) || //indicate absolute value of integer
        (tokens[0].type ==  PLUS) || //indicate positive integer
        (tokens[0].type == MINUS) //indicate negative integer
      )
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      //gets right hand operand
      ParseSecondary(tokens,datatypeRHS);

      if ( datatypeRHS != INTEGERTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");   


      switch ( operation ) //performs absolute value-ing, positive-ing, or negative-ing
      {
         case ABS:
/*
      SETNZPI
      JMPNN     E????
      NEGI                    ; NEGI or NEGF (as required)
E???? EQU       *
*/
            {
               char Elabel[SOURCELINELENGTH+1];
         
               sprintf(Elabel,"E%04d",code.LabelSuffix());
               code.EmitFormattedLine("","SETNZPI");
               code.EmitFormattedLine("","JMPNN",Elabel);
               code.EmitFormattedLine("","NEGI");
               code.EmitFormattedLine(Elabel,"EQU","*");
            }
            break;
         case PLUS:
         // Do nothing (identity operator)
            break;
         case MINUS:
            code.EmitFormattedLine("","NEGI");
            break;
      }
      datatype = INTEGERTYPE;
   }
   else //if not unary operator, check if value is a secondary
      ParseSecondary(tokens,datatype);

   ExitModule("Factor");
}

//-----------------------------------------------------------
void ParseSecondary(TOKEN tokens[],DATATYPE &datatype) //checks if token is a secondary
//-----------------------------------------------------------
{
   void ParsePrefix(TOKEN tokens[], DATATYPE & datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Secondary");

   //get the left hand side operand 
   ParsePrefix(tokens, datatypeLHS);

   //checks for exponentiation
   if ( tokens[0].type == POWER ) //calculating an exponential expression
   {
      GetNextToken(tokens);

      //get the right hand side operand (the exponent)
      ParsePrefix(tokens, datatypeLHS);

      if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

      code.EmitFormattedLine("","POWI");
      datatype = INTEGERTYPE;
   }
   else //if not exponent, save datatype of operand and return
      datatype = datatypeLHS;

   ExitModule("Secondary");
}

//-----------------------------------------------------------
void ParsePrefix(TOKEN tokens[], DATATYPE& datatype) //checks if token is a prefix
//-----------------------------------------------------------
{
    void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE & datatype);
    void ParsePrimary(TOKEN tokens[], DATATYPE & datatype);
    void GetNextToken(TOKEN tokens[]);

    EnterModule("Prefix");

   //checks for increment or decrement operator (prefix only)
    if ((tokens[0].type == INC) ||
        (tokens[0].type == DEC)
		)
    {
        DATATYPE datatypeRHS;
        TOKENTYPE operation = tokens[0].type;

        GetNextToken(tokens);
        //gets value to be incremented or decremented
        ParseVariable(tokens, true, datatypeRHS);

        if (datatypeRHS != INTEGERTYPE)
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operand");

        switch (operation) //increments the integer
        {
        case INC:
            code.EmitFormattedLine("", "PUSH", "@SP:0D0");
            code.EmitFormattedLine("", "PUSH", "#0D1");
            code.EmitFormattedLine("", "ADDI");
            code.EmitFormattedLine("", "POP", "@SP:0D1");       // side-effect
            code.EmitFormattedLine("", "PUSH", "@SP:0D0");
            code.EmitFormattedLine("", "SWAP");
            code.EmitFormattedLine("", "DISCARD", "#0D1");      // value
            break;
        case DEC:
            code.EmitFormattedLine("", "PUSH", "@SP:0D0");
            code.EmitFormattedLine("", "PUSH", "#0D1");
            code.EmitFormattedLine("", "SUBI");
            code.EmitFormattedLine("", "POP", "@SP:0D1");       // side-effect
            code.EmitFormattedLine("", "PUSH", "@SP:0D0");
            code.EmitFormattedLine("", "SWAP");
            code.EmitFormattedLine("", "DISCARD", "#0D1");      // value
            break;
        }
        datatype = INTEGERTYPE;
    }
    else //if no prefix, value must be a primary
        ParsePrimary(tokens, datatype);

    ExitModule("Prefix");
}


//-----------------------------------------------------------
void ParsePrimary(TOKEN tokens[],DATATYPE &datatype) //checks if token is a primary
//-----------------------------------------------------------
{
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Primary");

   //checks if value is a primary (integer, boolean, identifier, or parenthesis)
   switch ( tokens[0].type )
   {
      case INTEGER:
         {
            char operand[SOURCELINELENGTH+1];
            
            sprintf(operand,"#0D%s",tokens[0].lexeme);
            code.EmitFormattedLine("","PUSH",operand);
            datatype = INTEGERTYPE;
            GetNextToken(tokens);
         }
         break;
      case TRUE:
         code.EmitFormattedLine("","PUSH","#0XFFFF");
         datatype = BOOLEANTYPE;
         GetNextToken(tokens);
         break;
      case FALSE:
         code.EmitFormattedLine("","PUSH","#0X0000");
         datatype = BOOLEANTYPE;
         GetNextToken(tokens);
         break;
      case OPARENTHESIS:
         GetNextToken(tokens);
         ParseExpression(tokens,datatype); //if parenthesis, evaluates expression in paranthesis
         if ( tokens[0].type != CPARENTHESIS )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
         GetNextToken(tokens);
         break;
      case IDENTIFIER:
         ParseVariable(tokens,false,datatype); //if it is an identifier, parse the variable as a rvalue
         break;
      default: //if not any of these, send an error
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting integer, true, false, '(', variable");
         break;
   }

   ExitModule("Primary");
}

//-----------------------------------------------------------
void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE& datatype) //checks if token is a variable
//-----------------------------------------------------------
{
    /*
    Syntax "locations"                 l- or r-value
    ---------------------------------  -------------
    <expression>                       r-value
    <prefix>                           l-value
    <INPUTStatement>                   l-value
    LHS of <assignmentStatement>       l-value

    r-value ( read-only): value is pushed on run-time stack
    l-value (read/write): address of value is pushed on run-time stack
    */
    void GetNextToken(TOKEN tokens[]);

    bool isInTable;
    int index;
    IDENTIFIERTYPE identifierType;

    EnterModule("Variable");

   //value must be an identifier
    if (tokens[0].type != IDENTIFIER)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");

    //checks if it is a valid identifier
    // STATICSEMANTICS
    index = identifierTable.GetIndex(tokens[0].lexeme, isInTable);
    if (!isInTable)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Undefined identifier");

    identifierType = identifierTable.GetType(index);
    datatype = identifierTable.GetDatatype(index);

   //checks if identifier is a global/local variable or constant
    if (!((identifierType == GLOBAL_VARIABLE) ||
        (identifierType == GLOBAL_CONSTANT) ||
        (identifierType == PROGRAMMODULE_VARIABLE) ||
        (identifierType == PROGRAMMODULE_CONSTANT)))
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting variable or constant identifier");

    //makes sure constant is not being redefined
    if (asLValue && ((identifierType == GLOBAL_CONSTANT) || (identifierType == PROGRAMMODULE_CONSTANT)))
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Constant may not be l-value");
    // ENDSTATICSEMANTICS

    // CODEGENERATION
    if (asLValue)
        code.EmitFormattedLine("", "PUSHA", identifierTable.GetReference(index));
    else
        code.EmitFormattedLine("", "PUSH", identifierTable.GetReference(index));
    // ENDCODEGENERATION

    GetNextToken(tokens);

    ExitModule("Variable");
}

//-----------------------------------------------------------
void Callback1(int sourceLineNumber,const char sourceLine[]) //prints formatted sourceline
//-----------------------------------------------------------
{
   cout << setw(4) << sourceLineNumber << " ";
}

//-----------------------------------------------------------
void Callback2(int sourceLineNumber,const char sourceLine[]) //prints unformatted sourceline
//-----------------------------------------------------------
{
   cout << sourceLine << endl;
    char line[SOURCELINELENGTH+1];

// CODEGENERATION
   sprintf(line,"; %4d %s",sourceLineNumber,sourceLine);
   code.EmitUnformattedLine(line);
// ENDCODEGENERATION
}

//-----------------------------------------------------------
void GetNextToken(TOKEN tokens[]) //builds each token lexeme
//-----------------------------------------------------------
{
   const char *TokenDescription(TOKENTYPE type);

   int i;
   TOKENTYPE type;
   char lexeme[SOURCELINELENGTH+1];
   int sourceLineNumber;
   int sourceLineIndex;
   char information[SOURCELINELENGTH+1];

//============================================================
// Move look-ahead "window" to make room for next token-and-lexeme
//============================================================
   for (int i = 1; i <= LOOKAHEAD; i++)
      tokens[i-1] = tokens[i];

   char nextCharacter = reader.GetLookAheadCharacter(0).character;

//============================================================
// "Eat" white space and comments
//============================================================
   do
   {
//    "Eat" any white-space (blanks and EOLCs and TABCs) 
      while ( (nextCharacter == ' ')
           || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
           || (nextCharacter == READER<CALLBACKSUSED>::TABC) )
         nextCharacter = reader.GetNextCharacter().character;

//    "Eat" line comment
//    ~~line comment
      if ( (nextCharacter == '~' ) && (reader.GetLookAheadCharacter(1).character == '~') )
      {

#ifdef TRACESCANNER
   sprintf(information,"At (%4d:%3d) begin line comment",
      reader.GetLookAheadCharacter(0).sourceLineNumber,
      reader.GetLookAheadCharacter(0).sourceLineIndex);
   lister.ListInformationLine(information);
#endif

         do
            nextCharacter = reader.GetNextCharacter().character;
         while ( nextCharacter != READER<CALLBACKSUSED>::EOLC );
      } 

//    "Eat" block comments (nesting allowed) 
//     (-: 'blocked comment' :-)
      if ( (nextCharacter == '(') && (reader.GetLookAheadCharacter(1).character == '-') && (reader.GetLookAheadCharacter(2).character == ':' ) )
      {
         int depth = 0;

         do
         {
            if ( (nextCharacter == '(') && (reader.GetLookAheadCharacter(1).character == '-') && (reader.GetLookAheadCharacter(2).character == ':' ) )
            {
               depth++;

#ifdef TRACESCANNER
   sprintf(information,"At (%4d:%3d) begin block comment depth = %d",
      reader.GetLookAheadCharacter(0).sourceLineNumber,
      reader.GetLookAheadCharacter(0).sourceLineIndex,
      depth);
   lister.ListInformationLine(information);
#endif

               nextCharacter = reader.GetNextCharacter().character;
               nextCharacter = reader.GetNextCharacter().character;
            }
            else if ( (nextCharacter == ':') && (reader.GetLookAheadCharacter(1).character == '-') && (reader.GetLookAheadCharacter(2).character == ')') )
            {

#ifdef TRACESCANNER
   sprintf(information,"At (%4d:%3d)   end block comment depth = %d",
      reader.GetLookAheadCharacter(0).sourceLineNumber,
      reader.GetLookAheadCharacter(0).sourceLineIndex,
      depth);
   lister.ListInformationLine(information);
#endif

               depth--;
               nextCharacter = reader.GetNextCharacter().character;
               nextCharacter = reader.GetNextCharacter().character;
               nextCharacter = reader.GetNextCharacter().character;
            }
            else
               nextCharacter = reader.GetNextCharacter().character;
         }
         while ( (depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC) );
         if ( depth != 0 ) 
            ProcessCompilerError(reader.GetLookAheadCharacter(0).sourceLineNumber,
                                 reader.GetLookAheadCharacter(0).sourceLineIndex,
                                 "Unexpected end-of-program");
      }
   } while ( (nextCharacter == ' ')
          || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
          || (nextCharacter == READER<CALLBACKSUSED>::TABC)
          || ((nextCharacter == '~') && (reader.GetLookAheadCharacter(1).character == '~'))
          || ((nextCharacter == '(') && (reader.GetLookAheadCharacter(1).character == '-') && (reader.GetLookAheadCharacter(2).character == ':')) );

//============================================================
// Scan token
//============================================================
   sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
   sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

// reserved words and <identifier> 
// as soon as first letter is read, looks ahead to find entire lexeme & checks if it exists in tokentable
   if ( isalpha(nextCharacter) )
   {

      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;
      while ( isalpha(nextCharacter) || isdigit(nextCharacter) || (nextCharacter == '_') ) 
      //fills lexeme until it encounters whitespace or invalid symbol
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }
      lexeme[i] = '\0';

      //initializes isFound variable used to check if token is a reserved word
      bool isFound = false;

      i = 0;
      while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) ) 
      //checks if lexeme is a reserved word
      {
         if ( TOKENTABLE[i].isReservedWord && (strcmp(lexeme,TOKENTABLE[i].description) == 0) )
            isFound = true;
         else
            i++;
      }
      if ( isFound )
         type = TOKENTABLE[i].type;
      else //if it is not a reserved word, then it is an identifier
         type = IDENTIFIER;
   }
   
// checks if token is an <integer>
   else if ( isdigit(nextCharacter) )
   {
      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;
      while ( isdigit(nextCharacter) )
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }
      lexeme[i] = '\0';
      type = INTEGER;
   }
      
   else //checks if token is a string or operator
   {
      switch ( nextCharacter )
      {
//===================================================================
//     Sakura <string> (no escape sequence for double quote needed)
//    <string>              ::= `{<ASCIICharacter>}*`                      || *Note* no embedded ` allowed
//===================================================================
         case '`': 
            i = 0;
            nextCharacter = reader.GetNextCharacter().character;
            while ( (nextCharacter != '`') 
                 && (nextCharacter != READER<CALLBACKSUSED>::EOLC)
                 && (nextCharacter != READER<CALLBACKSUSED>::EOPC) )
            {
               if(nextCharacter == '"') //create escape characters for double quotes
			   {
               	lexeme[i++] = '\\';
			   }
               lexeme[i++] = nextCharacter;
               nextCharacter = reader.GetNextCharacter().character;
            }
            if ( (nextCharacter == READER<CALLBACKSUSED>::EOLC)
              || (nextCharacter == READER<CALLBACKSUSED>::EOPC) )
               ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                    "Un-terminated string literal");
            lexeme[i] = '\0';
            type = STRING;
            reader.GetNextCharacter();
            break;
//===================================================================
		 case '|': 
            type = PIPE;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case READER<CALLBACKSUSED>::EOPC: 
            {
               static int count = 0;
   
               if ( ++count > (LOOKAHEAD+3) ) //changed from 1, 2, to 3--WHY????
                  ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                       "Unexpected end-of-program");
               else
               {
                  type = EOPTOKEN;
                  reader.GetNextCharacter();
                  lexeme[0] = '\0';
               }
            }
            break;
         case '(': 
            type = OPARENTHESIS;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case ')': 
            type = CPARENTHESIS;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case '=': 
            type = EQ;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case '>':
            lexeme[0] = nextCharacter;
            nextCharacter = reader.GetNextCharacter().character;
            if ( nextCharacter == '=' )
            {
               type = GTEQ;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               reader.GetNextCharacter();
            }
            else if (nextCharacter == '>')
            {
                type = INSHIFT;
                lexeme[1] = nextCharacter; lexeme[2] = '\0';
                reader.GetNextCharacter();
            }
            else
            {
               type = GT;
               lexeme[1] = '\0';
            }
            break;
      // use character look-ahead to "find" '='
         case '!':
            lexeme[0] = nextCharacter;
            if ( reader.GetLookAheadCharacter(1).character == '=' )
            {
               nextCharacter = reader.GetNextCharacter().character;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               reader.GetNextCharacter();
               type = NOTEQ;
            }
            else
            {
               type = UNKTOKEN;
               lexeme[1] = '\0';
               reader.GetNextCharacter();
            }
            break;
         case ':': 
            type = COLON;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;        
        case '^': 
            lexeme[0] = nextCharacter;
            nextCharacter = reader.GetNextCharacter().character;
            if ( nextCharacter == 'n' )
            {
               type = ENDL;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               reader.GetNextCharacter();
            }
            else
            {
               type = POWER;
               lexeme[1] = '\0';
            }
            break;          
         case '<': 
            lexeme[0] = nextCharacter;
            nextCharacter = reader.GetNextCharacter().character;
            if ( nextCharacter == '=' )
            {
               type = LTEQ;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               reader.GetNextCharacter();
            }
            else if ( nextCharacter == '<' )
            {
               type = OUTSHIFT;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               reader.GetNextCharacter();
            }
            else if (nextCharacter == '-')
            {
                type = ASSIGN;
                lexeme[1] = nextCharacter; lexeme[2] = '\0';
                reader.GetNextCharacter();
            }
            else
            {
               type = LT;
               lexeme[1] = '\0';
            }
            break;
         case '+': 
             lexeme[0] = nextCharacter;
             if (reader.GetLookAheadCharacter(1).character == '+')
             {
                 nextCharacter = reader.GetNextCharacter().character;
                 lexeme[1] = nextCharacter; lexeme[2] = '\0';
                 type = INC;
             }
             else
             {
                 type = PLUS;
                 lexeme[0] = nextCharacter; lexeme[1] = '\0';
             }
             reader.GetNextCharacter();
             break;
         case '-': 
             lexeme[0] = nextCharacter;
             if (reader.GetLookAheadCharacter(1).character == '-')
             {
                 nextCharacter = reader.GetNextCharacter().character;
                 lexeme[1] = nextCharacter; lexeme[2] = '\0';
                 type = DEC;
             }
             else
             {
                 type = MINUS;
                 lexeme[0] = nextCharacter; lexeme[1] = '\0';
             }
             reader.GetNextCharacter();
             break;
      // use character look-ahead to "find" other '*'
         case '*': 
            lexeme[0] = nextCharacter;
            if ( reader.GetLookAheadCharacter(1).character == '*' )
            {
               nextCharacter = reader.GetNextCharacter().character;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               type = POWER;
            }
            else
            {
               type = MULTIPLY;
               lexeme[0] = nextCharacter; lexeme[1] = '\0';
            }
            reader.GetNextCharacter();
            break;
         case '/': 
            type = DIVIDE;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case '%': 
            type = MODULUS;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case ',':
             type = COMMA;
             lexeme[0] = nextCharacter; lexeme[1] = '\0';
             reader.GetNextCharacter();
             break;
         default:  //if token not found, save as an unknown token
            type = UNKTOKEN;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
      }
   }

   tokens[LOOKAHEAD].type = type;
   strcpy(tokens[LOOKAHEAD].lexeme,lexeme);
   tokens[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
   tokens[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

#ifdef TRACESCANNER
   sprintf(information,"At (%4d:%3d) token = %12s lexeme = |%s|",
      tokens[LOOKAHEAD].sourceLineNumber,
      tokens[LOOKAHEAD].sourceLineIndex,
      TokenDescription(type),lexeme);
   lister.ListInformationLine(information);
#endif

}

//-----------------------------------------------------------
const char *TokenDescription(TOKENTYPE type) //return the description of the token type
//-----------------------------------------------------------
{
   int i;
   bool isFound;
   
   isFound = false;
   i = 0;
   while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) )
   {
      if ( TOKENTABLE[i].type == type )
         isFound = true;
      else
         i++;
   }
   return ( isFound ? TOKENTABLE[i].description : "???????" );
}
