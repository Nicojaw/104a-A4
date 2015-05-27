//Nico Williams and Brandon Rullamas
//nijowill and brullama
//Assignment 4 - Symbols and Type Checking

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
/*#include "symtable.h"

SymbolTable* symbol = new SymbolTable(NULL);*/

astree* new_astree (int symbol, int filenr, int linenr,
                    int offset, const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}


static void dump_node (FILE* outfile, astree* node) {
   fprintf (outfile, "%3ld  %ld.%03ld  %-3d  %-15s (%s)  ",
            node->filenr, node->linenr, node->offset, node->symbol,
            get_yytname (node->symbol), node->lexinfo->c_str()); 
   bool need_space = false;
   for (size_t child = 0; child < node->children.size();
        ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      fprintf (outfile, "%p", node->children.at(child));
   }
}

static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size();
        ++child) {
      dump_astree_rec (outfile, root->children[child],
                       depth + 1);
   }
}

static void dump_astree_rec2 (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   int i = 0;
      while (i < depth){
         fprintf(outfile, "   ");
         i++;
      }
      fprintf(outfile, "%s", get_yytname(root->symbol));
      if(strcmp(root->lexinfo->c_str(), "") != 0)
         fprintf(outfile, " (%s)", root->lexinfo->c_str());
      fprintf(outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child){
      dump_astree_rec2 (outfile, root->children[child], depth +1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void dump_astree2(FILE* outfile, astree* root) {
   dump_astree_rec2 (outfile, root, 0);
   fflush (NULL);
}


/*
void scan(astree* root, SymbolTable* sym){
   if(root == NULL) return;
   for(size_t child = 0; child < root->children.size(); ++child){
      const char* CurSym = get_yytname(root->children[child]->symbol);
      if(strcmp(CurSym, "vardecl") != 0 && strcmp(CurSym, "block") != 0 && strcmp(CurSym, "function") != 0 && strcmp(CurSym, "struct_") != 0)
         scan(root->children[child], sym);
      else if(strcmp(CurSym, "vardecl") == 0){                         
         if(root->children[child]->children[0]->children.size() < 2){
            sym->addSymbol(root->children[child]->children[1]->lexinfo->c_str(), root->children[child]->children[0]->children[0]->children[0]->lexinfo->c_str(), root->children[child]->children[1]->filenr, root->children[child]->children[1]->linenr,
            }else{
            string type = root->children[child]->children[0]->children[0]->children[0]->lexinfo->c_str();
            type = type + "[]";
            sym->addSymbol(root->children[child]->children[1]->lexinfo->c_str(),
                              type, root->children[child]->children[1]->filenr, root->children[child]->children[1]->linenr, root->children[child]->children[1]->offset);
         }
      }else if (strcmp(CurSym, "struct_") == 0){
         string tabs;
         SymbolTable* CurTabs;
         for(size_t childOther = 0; childOther < root->children[child]->children.size(); ++childOther){
            string CurChild = get_yytname(root->children[child]->children[childOther]->symbol);
            if(CurChild == "TOK_IDENT"){
               tabs = root->children[child]->children[childOther]->lexinfo->c_str();
               structtab->addStruct(tabs);
               CurTabs = structtab->lookup2(tabs);
            }
         }
         for(size_t childOther = 0; childOther < root->children[child]->children.size(); ++childOther){
            string CurChild = get_yytname(root->children[child]->children[childOther]->symbol);
            if(CurChild == "decl"){
               CurTabs->addSymbol(  root->children[child]->children[childOther]->children[1]->lexinfo->c_str(),
                                    root->children[child]->children[childOther]->children[0]->children[0]->children[0]->lexinfo->c_str(),
                                    root->children[child]->children[childOther]->children[1]->filenr,
                                    root->children[child]->children[childOther]->children[1]->linenr,
                                    root->children[child]->children[childOther]->children[1]->offset);
            }
         }
  

      }else if(strcmp(CurSym, "block") == 0){       //if there is a block, then create a new block and start scanning that one
         scan(root->children[child], root->children[child]->blockpt = sym->enterBlock()); 
      }else if(strcmp(CurSym, "function") == 0){       //found function
         char ident[50];
         char typedecl[100];
         int blocknum;
         int numdecl = 0;
         int funcfilenr = 0;
         int funclinenr = 0;
         int funcoffset = 0;
         for(size_t childOther = 0; childOther < root->children[child]->children.size(); ++childOther){         //for all child nodes of function
            const char* currinnersymbol = get_yytname(root->children[child]->children[childOther]->symbol);
            if(strcmp(currinnersymbol, "TOK_IDENT") == 0){   //find function identifier
               funcfilenr = root->children[child]->children[childOther]->filenr;
               funclinenr = root->children[child]->children[childOther]->linenr;
               funcoffset = root->children[child]->children[childOther]->offset;
               strcpy(ident, root->children[child]->children[childOther]->lexinfo->c_str());
            }
            if(strcmp(currinnersymbol, "type") == 0){    //add type to the type/decl string
               if(root->children[child]->children[childOther]->children.size() == 2){
               strcpy(typedecl, root->children[child]->children[childOther]->children[0]->children[0]->lexinfo->c_str());
               strcat(typedecl, "[]");
               strcat(typedecl, "(");
               }else{
               strcpy(typedecl, root->children[child]->children[childOther]->children[0]->children[0]->lexinfo->c_str());
               strcat(typedecl, "(");
               }
            }
            if(strcmp(currinnersymbol, "block") == 0)
               blocknum = childOther;
         }
         for(size_t childOther = 0; childOther < root->children[child]->children.size(); ++childOther){            //get function decl types
            if(strcmp(get_yytname(root->children[child]->children[childOther]->symbol), "decl") == 0){
               if(numdecl != 0)
                  strcat(typedecl, ",");
               if(root->children[child]->children[childOther]->children[0]->children.size() == 2){
                  strcat(typedecl,root->children[child]->children[childOther]->children[0]->children[0]->children[0]->lexinfo->c_str());
                  strcat(typedecl,"[]");
               }else{
               strcat(typedecl,root->children[child]->children[childOther]->children[0]->children[0]->children[0]->lexinfo->c_str());
               }
               ++numdecl;
            }
         }
         strcat(typedecl, ")");
         SymbolTable* funcsym = sym->enterFunction(ident, typedecl, funcfilenr, funclinenr, funcoffset);
         for(size_t childOther = 0; childOther < root->children[child]->children.size(); ++childOther){           //add func declarations
            if(strcmp(get_yytname(root->children[child]->children[childOther]->symbol), "decl") == 0){
               if(root->children[child]->children[childOther]->children[0]->children.size() == 2){
                  string typearray = root->children[child]->children[childOther]->children[0]->children[0]->children[0]->lexinfo->c_str();
                  typearray = typearray + "[]";
                  funcsym->addSymbol(root->children[child]->children[childOther]->children[1]->lexinfo->c_str(),
                                     typearray,
                                     root->children[child]->children[childOther]->children[1]->filenr,
                                     root->children[child]->children[childOther]->children[1]->linenr,
                                     root->children[child]->children[childOther]->children[1]->offset);
                }
               else{
               funcsym->addSymbol(root->children[child]->children[childOther]->children[1]->lexinfo->c_str(),
                                  root->children[child]->children[childOther]->children[0]->children[0]->children[0]->lexinfo->c_str(),
                                  root->children[child]->children[childOther]->children[1]->filenr,
                                  root->children[child]->children[childOther]->children[1]->linenr,
                                  root->children[child]->children[childOther]->children[1]->offset);
               }
            }
         }
         scan(root->children[child]->children[blocknum], root->children[child]->children[blocknum]->blockpt = funcsym);
    }
   }

}

*/




























void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep) {
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n",
               get_yytname (toknum), toknum);
   }
   fflush (NULL);
}


void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}
