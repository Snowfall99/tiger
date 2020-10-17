%{
/* Lab2 Attention: You are only allowed to add code in this file and start at Line 26.*/
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

/* @function: getstr
 * @input: a string literal
 * @output: the string value for the input which has all the escape sequences 
 * translated into their meaning.
 */
char *getstr(const char *str)
{
	//optional: implement this function if you need it
  return NULL;
}

#define MAX_STR_LEN 2048
int commentLevel=0;
int len = 0;
char *res;

%}
  /* You can add lex definitions here. */

%Start COMMENT ERROR STR

%%
  /* 
  * Below is an example, which you can wipe out
  * and write reguler expressions and actions of your own.
  */ 
<INITIAL>[ \t]+ {adjust();}
<INITIAL>"\n"	    {adjust(); EM_newline();}

<INITIAL>","  {adjust(); return COMMA;}
<INITIAL>":"  {adjust(); return COLON;}
<INITIAL>";"  {adjust(); return SEMICOLON;}
<INITIAL>"("  {adjust(); return LPAREN;}
<INITIAL>")"  {adjust(); return RPAREN;}
<INITIAL>"["  {adjust(); return LBRACK;}
<INITIAL>"]"  {adjust(); return RBRACK;}
<INITIAL>"{"  {adjust(); return LBRACE;}
<INITIAL>"}"  {adjust(); return RBRACE;}
<INITIAL>"+"  {adjust(); return PLUS;}
<INITIAL>"-"  {adjust(); return MINUS;}
<INITIAL>"*"  {adjust(); return TIMES;}
<INITIAL>"/"  {adjust(); return DIVIDE;}
<INITIAL>"="  {adjust(); return EQ;}
<INITIAL>"<"  {adjust(); return LT;}
<INITIAL>">"  {adjust(); return GT;}
<INITIAL>"<=" {adjust(); return LE;}
<INITIAL>">=" {adjust(); return GE;}
<INITIAL>"<>" {adjust(); return NEQ;}
<INITIAL>"."  {adjust(); return DOT;}
<INITIAL>"&"  {adjust(); return AND;}
<INITIAL>"|"  {adjust(); return OR;}
<INITIAL>":=" {adjust(); return ASSIGN;}

<INITIAL>while    {adjust(); return WHILE;}
<INITIAL>for  	  {adjust(); return FOR;}
<INITIAL>to       {adjust(); return TO;}
<INITIAL>break    {adjust(); return BREAK;}
<INITIAL>let      {adjust(); return LET;}
<INITIAL>in       {adjust(); return IN;}
<INITIAL>end      {adjust(); return END;}
<INITIAL>function {adjust(); return FUNCTION;}
<INITIAL>var      {adjust(); return VAR;}
<INITIAL>type     {adjust(); return TYPE;}
<INITIAL>array    {adjust(); return ARRAY;}
<INITIAL>if       {adjust(); return IF;}
<INITIAL>then     {adjust(); return THEN;}
<INITIAL>else     {adjust(); return ELSE;}
<INITIAL>do       {adjust(); return DO;}
<INITIAL>of       {adjust(); return OF;}
<INITIAL>nil      {adjust(); return NIL;}

	/* error handler */
<ERROR>.|"\n" {BEGIN INITIAL; yyless(0);}

  /*  Comment */
<COMMENT>"/*"     {
                    adjust();
                    commentLevel+=1;
                  }
<COMMENT>"*/"     {
                    adjust();
                    if(--commentLevel == 0)
                        BEGIN INITIAL;
                  }
<COMMENT>[ ]      {adjust();}
<COMMENT>"\n"     {adjust(); EM_newline(); continue;}
<COMMENT>.        {adjust();}
<COMMENT><<EOF>> {EM_error(EM_tokPos, "unclosed comments"); BEGIN ERROR;}
<INITIAL>"/*"     {
                    adjust();
                    commentLevel+=1;
                    BEGIN COMMENT;
                  }
                    
    /* String */
<INITIAL>\"\" {adjust(); yylval.sval = String("(null)"); return STRING;}
<INITIAL>\"     {adjust(); BEGIN STR; res = malloc(MAX_STR_LEN); len=0;}
<STR>"\\n"   {charPos += yyleng; res[len] = '\n'; len++;}
<STR>[ ]     {charPos += yyleng; res[len] = ' '; len++;}
<STR>"\\t"   {charPos += yyleng; res[len] = '\t'; len++;}
<STR>\\[0-9]{3} {charPos += yyleng; res[len] = atoi(yytext+1); len++;}
<STR>\\\"    {charPos += yyleng; res[len] = '"'; len++;}
<STR>\\\\    {charPos += yyleng; res[len] = '\\'; len++;}
<STR>\\[ \t\n\f]+\\  {charPos+=yyleng;}
<STR>\\\^[A-Z]    {charPos += yyleng; res[len] = yytext[2] - 'A' + 1; len += yyleng;}
<STR>\\      {adjust(); EM_error(EM_tokPos, "Wrong use on \\"); BEGIN ERROR;}
<STR>\"      {charPos += yyleng; res[len] = '\0'; yylval.sval = String(res); BEGIN INITIAL; return STRING;}
<STR>.       {charPos += yyleng; strcpy(res + len, yytext); len += yyleng;}
<STR>"\n"    {charPos += yyleng; strcpy(res + len, yytext); len += yyleng;}
<INITIAL>[a-zA-Z][0-9a-zA-Z_]* {adjust(); yylval.sval=String(yytext); return ID;}
<INITIAL>(0|[1-9])[0-9]*	     {adjust(); yylval.ival=atoi(yytext); return INT;}
<INITIAL>.	        {adjust(); EM_error(EM_tokPos,"illegal token"); BEGIN ERROR;}
