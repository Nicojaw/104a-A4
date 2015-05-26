//Nico Williams and Brandon Rullamas
//nijowill and brullama
//Assignment 4 - Symbols and Type Checking

#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#include "lyutils.h"
#include "auxlib.h"
#include "stringset.h"
#include "astree.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

void chomp(char *string, char delim){
   size_t len = strlen(string);
   if(len == 0) return;
   char *nlpos = string + len -1;
   if(*nlpos == delim) *nlpos = '\0';
}

//Scans the lines of CPP
void cpplines(FILE *pipe, char *filename){
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy(inputname, filename);
   for(;;){
      char buffer[LINESIZE];
      char *fgets_rc = fgets(buffer, LINESIZE, pipe); 
      if(fgets_rc == NULL) break;
      chomp (buffer, '\n');
      int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"",
                             &linenr, filename);
      if(sscanf_rc == 2) continue;
      char *savepos = NULL;
      char *bufptr = buffer;
      for(int tokenct = 1;; ++tokenct){
         char *token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if(token == NULL) break;
         intern_stringset(token);
      }
      ++linenr;
   }
}



//Uses getopt() to scan options from the command line and
//set the necessary flags. The function prints an error
// message if an option is incorrect.

int main(int argc, char **argv){

   set_execname(argv[0]);
   extern int yy_flex_debug;
   extern int yydebug;
   yy_flex_debug = 0;
   yydebug = 0;
   int Dopt = 0;
   int Darg = 0;
   int opt = 0;
   extern FILE* tokoutputfile;
   extern astree* yyparse_astree;

   while((opt = getopt(argc, argv, "yl:@:D:")) != -1){
      switch(opt){
      case'y':
         yydebug = 1;
         break;
      case'l':
         yy_flex_debug = 1;
         break;
      case'@':
         set_debugflags(optarg);
         break;
      case'D':
         Dopt = 1;
         Darg = optind - 1;
         break;
      default:
         fprintf(stderr, "Not a valid option!\n"); 
         set_exitstatus(1);
         break;
      }
   }

   for(int argi = optind; argi < argc; ++argi){
      char *filename = argv[argi];
      char output[512];
      char stroutput[512];
      char tokoutput[512];
      char astoutput[512];
      strcpy(output, basename(filename));
         char *periodpos = strrchr(output, '.');
         if(periodpos == NULL){
            fprintf(stderr, "Not a .oc file!\n");
            set_exitstatus(1);
            continue;
         }
         int lastperiod = periodpos - output;
         if(periodpos != NULL && 
            strcmp(&output[lastperiod], ".oc\0") != 0){
            fprintf(stderr,"Not a .oc file!\n");
            set_exitstatus(1);
            continue;
         }
         output[lastperiod] = '\0';
         strcpy(stroutput, output);
         strcpy(tokoutput, output);
         strcpy(astoutput, output);
         strcat(stroutput, ".str");
         strcat(tokoutput, ".tok");
         strcat(astoutput, ".ast");
      string command;
      if(Dopt){ //if -D opt enabled, pass it to CPP
         command = CPP + " " + argv[Darg] + " " + filename;
      }else
         command = CPP + " " + filename;
      yyin = popen(command.c_str(), "r");
      FILE *stroutputfile = fopen(stroutput, "w");
      tokoutputfile = fopen(tokoutput, "w");
      if(yyin == NULL){
         syserrprintf(command.c_str());
      }else{
         cpplines (yyin, filename);
         int pclose_rc = pclose(yyin);
         if(pclose_rc != 0) set_exitstatus(1);
         eprint_status(command.c_str(), pclose_rc);
      }
      dump_stringset(stroutputfile);
      fclose(stroutputfile);

      yyin = popen(command.c_str(), "r");
      if(yyin == NULL){
         syserrprintf(command.c_str());
      }else{
         yyparse();
         int pclose_rc = pclose(yyin);
         eprint_status(command.c_str(), pclose_rc);
      }
      fclose(tokoutputfile);

      FILE* astoutputfile = fopen(astoutput, "w");
      dump_astree_new(astoutputfile, yyparse_astree);
      fclose(astoutputfile);

   }
   return get_exitstatus();

}