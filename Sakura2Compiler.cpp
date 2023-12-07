//-----------------------------------------------------------
// Juni Ejere
// Sakura1 Compiler
// Sakura1Compiler.cpp
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

//#define TRACEREADER
#define TRACESCANNER
//#define TRACEPARSER
#define TRACECOMPILER

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
// punctuation
   COLON, // ":"
   OUTSHIFT, //<< ------ is it operator?
   OPARENTHESIS, //"("
   CPARENTHESIS,  // ")"
// operators
   PIPE, // | ABS operator
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
   { ENDL        ,"ENDL"        ,false }, //endl is not a reservedword so false, ^n symbol is?
   { OUTSHIFT    ,"OUTSHIFT"    ,false },
   { COLON       ,"COLON"       ,false },
   { OR          ,"or"          ,true  },
   { NOR         ,"nor"         ,true  },
   { XOR         ,"xor"         ,true  },
   { AND         ,"and"         ,true  },
   { NAND        ,"nand"        ,true  },
   { NOT         ,"not"         ,true  },
   { ABS         ,"ABS"         ,false },
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
};

//-----------------------------------------------------------
struct TOKEN
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
// ENDCODEGENERATION

#ifdef TRACEPARSER
int level;
#endif

//-----------------------------------------------------------
void EnterModule(const char module[])
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
void ExitModule(const char module[])
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
void ProcessCompilerError(int sourceLineNumber,int sourceLineIndex,const char errorMessage[])
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
int main()
//-----------------------------------------------------------
{
   void Callback1(int sourceLineNumber,const char sourceLine[]);
   void Callback2(int sourceLineNumber,const char sourceLine[]);
   void GetNextToken(TOKEN tokens[]);
   void ParseSPLProgram(TOKEN tokens[]);

   char sourceFileName[80+1];
   TOKEN tokens[LOOKAHEAD+1];
   
   cout << "Source filename? ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName);
      code.OpenFile(sourceFileName);

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
   
      ParseSPLProgram(tokens);
      
// CODEGENERATION
      code.EmitEndingCode();
// ENDCODEGENERATION
   }
  catch (SAKURAEXCEPTION sakuraException)
   {
      cout << "Sakura exception: " << sakuraException.GetDescription() << endl;
   }
   lister.ListInformationLine("******* Sakura1 Compiler ending");
   cout << "Sakura1 Compiler ending\n";

   system("PAUSE");
   return( 0 );
   
}

//-----------------------------------------------------------
void ParseSPLProgram(TOKEN tokens[])
//-----------------------------------------------------------
{
   void GetNextToken(TOKEN tokens[]);
   void ParsePROGRAMDefinition(TOKEN tokens[]);

   EnterModule("SakuraProgram");

  // if ( tokens[0].type == PROGRAM )
  //********NEED TO ADD IN COLON FUNCTIONALITY
   /*if ( tokens[0].type == MAIN ){
   	if( tokens[1].type== COLON)
      ParsePROGRAMDefinition(tokens);
    }
   else
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting main function");*/
                           
    /* makes sure program begins with main reserved word + colon*/
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
   	ParsePROGRAMDefinition(tokens);
   

   if ( tokens[0].type != EOPTOKEN )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

   ExitModule("SakuraProgram");
}

//-----------------------------------------------------------
void ParsePROGRAMDefinition(TOKEN tokens[])
//-----------------------------------------------------------
{
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

   GetNextToken(tokens);
   GetNextToken(tokens);

   //while ( tokens[0].type != END )
   while ( tokens[0].type != EOPTOKEN )
      ParseStatement(tokens);
      
// CODEGENERATION
   code.EmitFormattedLine("","RETURN");
   code.EmitUnformattedLine("; **** =========");
   sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);
   code.EmitUnformattedLine("; **** =========");
// ENDCODEGENERATION

   GetNextToken(tokens);

   ExitModule("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
   void GetNextToken(TOKEN tokens[]);
   void ParsePRINTStatement(TOKEN tokens[]);

   EnterModule("Statement");
   //GetNextToken(tokens);


   switch ( tokens[0].type )
   {
      case OUTPUT:
         ParsePRINTStatement(tokens);
         break;
      /*case EOPTOKEN:
      	break;*/
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting beginning-of-statement");
         break;
   }

   ExitModule("Statement");
}

//-----------------------------------------------------------
void ParsePRINTStatement(TOKEN tokens[])
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

   /*do
   {
      GetNextToken(tokens);

      switch ( tokens[0].type )
      {
         case STRING:
            GetNextToken(tokens);
            break;
         case ENDL:
            GetNextToken(tokens);
            break;
         default:
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                                 "Expecting string or ENDL");
      }
   } while ( tokens[0].type == OUTSHIFT );*/
   
   //outshift should be first token after OUTPUT reserved word, so evaluted FIRST
   while ( tokens[0].type == OUTSHIFT && tokens[0].type != OUTPUT)
   {
   	      GetNextToken(tokens);

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
			ParseExpression(tokens,datatype);

// CODEGENERATION
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
      }
   }

//does not need to end in period
   /*if ( tokens[0].type != PERIOD )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting '.'");*/

   //GetNextToken(tokens);

   ExitModule("PRINTStatement");
}

//-----------------------------------------------------------
void ParseExpression(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
// CODEGENERATION
/*
   An expression is composed of a collection of one or more operands (SPL calls them
      primaries) and operators (and perhaps sets of parentheses to modify the default 
      order-of-evaluation established by precedence and associativity rules).
      Expression evaluation computes a single value as the expression's result.
      The result has a specific data type. By design, the expression result is 
      "left" at the top of the run-time stack for subsequent use.
   
   SPL expressions must be single-mode with operators working on operands of
      the appropriate type (for example, boolean AND boolean) and not mixing
      modes. Static semantic analysis guarantees that operators are
      operating on operands of appropriate data type.
*/
// ENDCODEGENERATION

   void ParseConjunction(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Expression");

   ParseConjunction(tokens,datatypeLHS);

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
         ParseConjunction(tokens,datatypeRHS);
   
// CODEGENERATION
         switch ( operation )
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
   else
      datatype = datatypeLHS;

   ExitModule("Expression");
}

//-----------------------------------------------------------
void ParseConjunction(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseNegation(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Conjunction");

   ParseNegation(tokens,datatypeLHS);

   if ( (tokens[0].type ==  AND) ||
        (tokens[0].type == NAND) )
   {
      while ( (tokens[0].type ==  AND) ||
              (tokens[0].type == NAND) )
      {
         TOKENTYPE operation = tokens[0].type;
  
         GetNextToken(tokens);
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
   else
      datatype = datatypeLHS;

   ExitModule("Conjunction");
}

//-----------------------------------------------------------
void ParseNegation(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparison(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeRHS;

   EnterModule("Negation");

   if ( tokens[0].type == NOT )
   {
      GetNextToken(tokens);
      ParseComparison(tokens,datatypeRHS);

      if ( !(datatypeRHS == BOOLEANTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operand");
      code.EmitFormattedLine("","NOT");
      datatype = BOOLEANTYPE;
   }
   else
      ParseComparison(tokens,datatype);

   ExitModule("Negation");
}

//-----------------------------------------------------------
void ParseComparison(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparator(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparison");

   ParseComparator(tokens,datatypeLHS);
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
      ParseComparator(tokens,datatypeRHS);

      if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) ) //LHS and RHS at top of stack must be integers
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
      switch ( operation )
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
   else
      datatype = datatypeLHS;

   ExitModule("Comparison");
}

//-----------------------------------------------------------
void ParseComparator(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseTerm(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparator");

   ParseTerm(tokens,datatypeLHS);

   if ( (tokens[0].type ==  PLUS) ||
        (tokens[0].type == MINUS) )
   {
      while ( (tokens[0].type ==  PLUS) ||
              (tokens[0].type == MINUS) )
      {
         TOKENTYPE operation = tokens[0].type;
         
         GetNextToken(tokens);
         ParseTerm(tokens,datatypeRHS);

         if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

         switch ( operation )
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
   else
      datatype = datatypeLHS;
   
   ExitModule("Comparator");
}

//-----------------------------------------------------------
void ParseTerm(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseFactor(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Term");

   ParseFactor(tokens,datatypeLHS);
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
         ParseFactor(tokens,datatypeRHS);

         if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

         switch ( operation )
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
   else
      datatype = datatypeLHS;

   ExitModule("Term");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseSecondary(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Factor");
   TOKENTYPE operation;

   /*if ( (tokens[0].type ==  ABS) || //indicate absolute value of integer
        (tokens[0].type ==  PLUS) || //indicate positive integer
        (tokens[0].type == MINUS) //indicate negative integer
      )
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseSecondary(tokens,datatypeRHS);

      if ( datatypeRHS != INTEGERTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");*/
         
    /*if ( (tokens[0].type ==  PIPE) || //indicate absolute value of integer
        (tokens[0].type ==  PLUS) || //indicate positive integer
        (tokens[0].type == MINUS) //indicate negative integer
      )
   {
   	
   	//check if pipe is an absolute value is correct
   	if(tokens[0].type == PIPE)
   	{
   		//checks if the value after the integer is another pipe
	  if(tokens[2].type == PIPE)
	  {
	  	//DATATYPE datatypeRHS;
	  	operation = ABS;
	  }
	  else
	  	 ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Pipe can only be used as an ABS");
	}
	//token is a plus or minus
   	else
   	{
      //DATATYPE datatypeRHS;
      operation = tokens[0].type;
	}
	  DATATYPE datatypeRHS;
      GetNextToken(tokens);
      ParseSecondary(tokens,datatypeRHS);

      if ( datatypeRHS != INTEGERTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");    */ 

   /*****COME BACK TO THIS LATER****/
   /*if (tokens[0].type == PIPE)
   {
   		//get next token
		//parse ssecondary
   		//check for pipe
   }*/
   
   
   if ( (tokens[0].type ==  ABS) || //indicate absolute value of integer
        (tokens[0].type ==  PLUS) || //indicate positive integer
        (tokens[0].type == MINUS) //indicate negative integer
      )
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseSecondary(tokens,datatypeRHS);

      if ( datatypeRHS != INTEGERTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");   

      switch ( operation )
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
   else
      ParseSecondary(tokens,datatype);

   ExitModule("Factor");
}

//-----------------------------------------------------------
void ParseSecondary(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParsePrimary(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Secondary");

   ParsePrimary(tokens,datatypeLHS);

   if ( tokens[0].type == POWER ) //calculating an exponential expression
   {
      GetNextToken(tokens);

      ParsePrimary(tokens,datatypeRHS);

      if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

      code.EmitFormattedLine("","POWI");
      datatype = INTEGERTYPE;
   }
   else
      datatype = datatypeLHS;

   ExitModule("Secondary");
}

//-----------------------------------------------------------
void ParsePrimary(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Primary");

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
         ParseExpression(tokens,datatype);
         if ( tokens[0].type != CPARENTHESIS )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting )");
         GetNextToken(tokens);
         break;
         //case pipe???
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer, true, false, or (");
         break;
   }

   ExitModule("Primary");
}


//-----------------------------------------------------------
void Callback1(int sourceLineNumber,const char sourceLine[])
//-----------------------------------------------------------
{
   cout << setw(4) << sourceLineNumber << " ";
}

//-----------------------------------------------------------
void Callback2(int sourceLineNumber,const char sourceLine[])
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
void GetNextToken(TOKEN tokens[])
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
//    --line comment
      if ( (nextCharacter == '-' ) && (reader.GetLookAheadCharacter(1).character == '-') )
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
          || ((nextCharacter == '-') && (reader.GetLookAheadCharacter(1).character == '-'))
          || ((nextCharacter == '(') && (reader.GetLookAheadCharacter(1).character == '-') && (reader.GetLookAheadCharacter(2).character == ':')) );

//============================================================
// Scan token
//============================================================
   sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
   sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

// reserved words (and <identifier> ***BUT NOT YET***)
// as soon as first letter is read, looks ahead to find entire lexeme & checks if it exists in tokentable
   if ( isalpha(nextCharacter) )
   {
      //char UCLexeme[SOURCELINELENGTH+1];

      i = 0;
      lexeme[i++] = nextCharacter;
      nextCharacter = reader.GetNextCharacter().character;
      while ( isalpha(nextCharacter) || isdigit(nextCharacter) || (nextCharacter == '_') ) //fills lexeme until it encounters whitespace or invalid symbol
      {
         lexeme[i++] = nextCharacter;
         nextCharacter = reader.GetNextCharacter().character;
      }
      lexeme[i] = '\0';
      /*for (i = 0; i <= (int) strlen(lexeme); i++)
         UCLexeme[i] = toupper(lexeme[i]);*/

      bool isFound = false;

      i = 0;
      while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) ) //checks if lexeme is a reserved word
      {
         //if ( TOKENTABLE[i].isReservedWord && (strcmp(UCLexeme,TOKENTABLE[i].description) == 0) )
         if ( TOKENTABLE[i].isReservedWord && (strcmp(lexeme,TOKENTABLE[i].description) == 0) )
            isFound = true;
         else
            i++;
      }
      if ( isFound )
         type = TOKENTABLE[i].type;
      else
         type = IDENTIFIER;
   }
   
// <integer>
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
      
   else
   {
      switch ( nextCharacter )
      {
/* <string>
         case '`': 
            i = 0;
            nextCharacter = reader.GetNextCharacter().character;
            while ( (nextCharacter != '`') && (nextCharacter != READER<CALLBACKSUSED>::EOLC) )
            {
            	//check for escape sequences for \" or \\ to print '"' or '\' in a string
            	//don't need because '`' delimits a string
               /*if      ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '"') )
               {
                  lexeme[i++] = nextCharacter;
                  nextCharacter = reader.GetNextCharacter().character;
               }
               else if ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '\\') )
               {
                  lexeme[i++] = nextCharacter;
                  nextCharacter = reader.GetNextCharacter().character;
               }'*'/ 
               lexeme[i++] = nextCharacter;
               nextCharacter = reader.GetNextCharacter().character;
            }
            if ( nextCharacter == READER<CALLBACKSUSED>::EOLC )
               ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                    "Invalid string literal");
            lexeme[i] = '\0';
            type = STRING;
            reader.GetNextCharacter();
            break;*/
//===================================================================
//    ***EXAMPLE***
//     non-SPL <string> (no escape sequences)
//    <string>              ::= `{<ASCIICharacter>}*`                      || *Note* no embedded ` allowed
//===================================================================
         case '`': 
            i = 0;
            nextCharacter = reader.GetNextCharacter().character;
            while ( (nextCharacter != '`') 
                 && (nextCharacter != READER<CALLBACKSUSED>::EOLC)
                 && (nextCharacter != READER<CALLBACKSUSED>::EOPC) )
            {
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
/*		 case '|':
		 	lexeme[0] = nextCharacter;
			i = 1;
		 	nextCharacter = reader.GetLookAheadCharacter(1).character;
		 	//absolute value | int |
		 	if(isdigit(nextCharacter))
			 {
			 	i = 2;
			 	//while(isdigit(nextCharacter))
			 	while( (reader.GetLookAheadCharacter(i).character != '|') 
                 && (reader.GetLookAheadCharacter(i).character != READER<CALLBACKSUSED>::EOLC)
                 && (reader.GetLookAheadCharacter(i).character != READER<CALLBACKSUSED>::EOPC) )
			 	{ //ignore integer value
			 		//lexeme[i++] = nextCharacter;
			 		//nextCharacter = reader.GetNextCharacter().character;
					 i++;	
				}
				
				if ( (reader.GetLookAheadCharacter(i).character == READER<CALLBACKSUSED>::EOLC)
                  || (reader.GetLookAheadCharacter(i).character == READER<CALLBACKSUSED>::EOPC) )
               		ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                    "invalid use of pipe symbol");
                lexeme[1] = reader.GetLookAheadCharacter(i).character; 
                lexeme[2] = '\0';
                type = ABS;
			 }
			 else
			 {
			  	lexeme[1] = '\0';
			  	type = PIPE;
			  	//nextCharacter = reader.GetNextCharacter().character;
			 }
			 reader.GetNextCharacter();
			 //reader.GetNextCharacter();
		 	break;*/
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
//	<endl>
/*         case '^': 
         	//reader.GetLookAheadCharacter(1).character == 'n'
         	//nextCharacter = reader.GetNextCharacter().character;
         	if(reader.GetLookAheadCharacter(1).character == 'n')
			{
                //lexeme[i++] = nextCharacter;
                //nextCharacter = reader.GetNextCharacter().character;				
	            type = ENDL;
	            lexeme[0] = nextCharacter; lexeme[1] = reader.GetLookAheadCharacter(1).character; lexeme[2] = '\0';
	            reader.GetNextCharacter();
	            reader.GetNextCharacter();
            }
            break;
 */           
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
/*        case '^': 
            type = POWER;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
*/            
// outshift
/*         case '<': 
         	//reader.GetLookAheadCharacter(1).character == 'n'
         	//nextCharacter = reader.GetNextCharacter().character;
         	if(reader.GetLookAheadCharacter(1).character == '<')
			{
                //lexeme[i++] = nextCharacter;
                //nextCharacter = reader.GetNextCharacter().character;				
	            type = OUTSHIFT;
	            lexeme[0] = nextCharacter; lexeme[1] = reader.GetLookAheadCharacter(1).character; lexeme[2] = '\0';
	            reader.GetNextCharacter();
	            reader.GetNextCharacter();
            }
            break;
*/
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
            else
            {
               type = LT;
               lexeme[1] = '\0';
            }
            break;
         case '+': 
            type = PLUS;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            reader.GetNextCharacter();
            break;
         case '-': 
            type = MINUS;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
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
         default:  
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
const char *TokenDescription(TOKENTYPE type)
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
