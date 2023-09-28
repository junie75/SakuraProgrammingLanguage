//-----------------------------------------------------------
// Juni Ejere & Dr. Art Hanna 
// Sakura Reader "driver" program
// SakuraReader.cpp
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

#define TRACEREADER

#include "Sakura.h"

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
   void Callback1(int sourceLineNumber,const char sourceLine[]);
   void Callback2(int sourceLineNumber,const char sourceLine[]);

   char sourceFileName[80+1]; //name of the file
   NEXTCHARACTER nextCharacter; //holds the value of the next character in the source code

   // READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);
   // READER<CALLBACKSUSED> reader(3,LOOKAHEAD);
   LISTER lister(LINESPERPAGE); //LISTER class outputs page-formatted information to the list file as a “log” of the translation process of a source file
   READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD); //READER class provides one-character-at-a-time, top-to-bottom, left-to-right traversal of the characters contained in the source file. 

    /*Input the source file to be read*/
   cout << "Source filename? ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName);
      reader.SetLister(&lister);
      reader.AddCallbackFunction(Callback1);
      reader.AddCallbackFunction(Callback2);
      reader.OpenFile(sourceFileName);

    /*while have not reached the end of the program, read each character*/
      do
      {
         nextCharacter = reader.GetNextCharacter();
      } while ( nextCharacter.character != READER<CALLBACKSUSED>::EOPC ); //EOPC = end of program character, defined in Sakura.h
    // } while ( nextCharacter.character != READER::EOPC );
   }
   catch (SAKURAEXCEPTION sakuraException)
   {
      cout << "Sakura exception: " << sakuraException.GetDescription() << endl;
   }
   lister.ListInformationLine("******* Sakura reader ending");
   cout << "Sakura reader ending\n";

   system("PAUSE");
   return( 0 );
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
}
