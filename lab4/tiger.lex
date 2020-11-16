%{
/* Lab2 Attention: You are only allowed to add code in this file and start at Line 26.*/
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "y.tab.h"
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

static uint32_t comment_level = 0;

/* @function: getstr
 * @input: a string literal
 * @output: the string value for the input which has all the escape sequences
 * translated into their meaning.
 */
char *getstr(const char *str)
{
	//optional: implement this function if you need it
  char* ret = checked_malloc(strlen(str)+1);
  int cur = 1;
  int cnt = 0;

  if (str[cur] =='"') {
    ret = "";
    return ret;
  }

  while(str[cur]!='"')
  {
    if(str[cur]=='\\')
    {
      if(str[cur+1] >= '0' && str[cur+1] <= '9')
      {
        ret[cnt]= (str[cur+1]-'0')*10*10+(str[cur+2]-'0')*10+str[cur+3]-'0';
        cur+=4;
      }
      else{
      switch(str[cur+1]){
        case 'n': ret[cnt] ='\n';cur+=2;break;
        case 't': ret[cnt] ='\t';cur+=2;break;
        case '^': ret[cnt] = str[cur+2] -'A'+ 1 ;cur+=3;break;
        case '\\': ret[cnt] ='\\';cur+=2;break;
        case '\"': ret[cnt] = '\"';cur += 2;break;
        default: 
        cur++;
        while (str[cur] == '\n' || str[cur] == '\t' || str[cur]==' ') cur++;
        cur++;
        cnt--;
        break;
      }
      }
    }
    else{
      ret[cnt] = str[cur];
      cur++;
    }
    cnt++;
  }
  ret[cnt] = '\0';
  return ret;
}

%}
  /* You can add lex definitions here. */

%s COMMENT

%%
  /*
  * Below is an example, which you can wipe out
  * and write reguler expressions and actions of your own.
  */

"\n" {adjust(); EM_newline(); continue;}

<INITIAL>" " {adjust();}
<INITIAL>"\t" {adjust();}

<INITIAL>"," {adjust(); return COMMA;}
<INITIAL>":" {adjust(); return COLON;}
<INITIAL>";" {adjust(); return SEMICOLON;}
<INITIAL>"(" {adjust(); return LPAREN;}
<INITIAL>")" {adjust(); return RPAREN;}
<INITIAL>"[" {adjust(); return LBRACK;}
<INITIAL>"]" {adjust(); return RBRACK;}
<INITIAL>"{" {adjust(); return LBRACE;}
<INITIAL>"}" {adjust(); return RBRACE;}
<INITIAL>"." {adjust(); return DOT;}
<INITIAL>"+" {adjust(); return PLUS;}
<INITIAL>"-" {adjust(); return MINUS;}
<INITIAL>"*" {adjust(); return TIMES;}
<INITIAL>"/" {adjust(); return DIVIDE;}
<INITIAL>"=" {adjust(); return EQ;}
<INITIAL>"<>" {adjust(); return NEQ;}
<INITIAL>"<" {adjust(); return LT;}
<INITIAL>"<=" {adjust(); return LE;}
<INITIAL>">" {adjust(); return GT;}
<INITIAL>">=" {adjust(); return GE;}
<INITIAL>"&" {adjust(); return AND;}
<INITIAL>"|" {adjust(); return OR;}
<INITIAL>":=" {adjust(); return ASSIGN;}

<INITIAL>"array" {adjust(); return ARRAY;}
<INITIAL>"if" {adjust(); return IF;}
<INITIAL>"then" {adjust(); return THEN;}
<INITIAL>"else" {adjust(); return ELSE;}
<INITIAL>"while" {adjust(); return WHILE;}
<INITIAL>"for" {adjust(); return FOR;}
<INITIAL>"to" {adjust(); return TO;}
<INITIAL>"do" {adjust(); return DO;}
<INITIAL>"let" {adjust(); return LET;}
<INITIAL>"in" {adjust(); return IN;}
<INITIAL>"end" {adjust(); return END;}
<INITIAL>"of" {adjust(); return OF;}
<INITIAL>"break" {adjust(); return BREAK;}
<INITIAL>"nil" {adjust(); return NIL;}
<INITIAL>"function" {adjust(); return FUNCTION;}
<INITIAL>"var" {adjust(); return VAR;}
<INITIAL>"type" {adjust(); return TYPE;}

<INITIAL>[a-zA-Z][a-zA-Z0-9_]* {adjust();yylval.sval = String(yytext); return ID;}
<INITIAL>\"(\\\"|[^"])*\" {adjust();yylval.sval = getstr(yytext); return STRING;}
<INITIAL>[0-9]+ {adjust(); yylval.ival = atoi(yytext); return INT;}
<INITIAL><<EOF>> return 0;
<INITIAL>. {adjust(); EM_error(EM_tokPos,"illegal character");return 0;}

<INITIAL>"/*" {adjust(); comment_level++; BEGIN COMMENT;}
<COMMENT>"/*" {adjust(); comment_level++; }
<COMMENT>"*/" {adjust(); comment_level--; if(0==comment_level) BEGIN INITIAL;}
<COMMENT><<EOF>> {adjust(); EM_error(EM_tokPos,"EOF in comment"); return 1;}
<COMMENT>. {adjust();}
