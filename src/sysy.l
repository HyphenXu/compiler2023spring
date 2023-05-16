/* Options */
%option noyywrap
%option nounput
%option noinput

%{
/* Global code area */
#include <cstdlib>
#include <string>

#include "sysy.tab.hpp"     /* for token definitions in bison */

using namespace std;
%}

/* Whitespace and Comment */
WhiteSpace      [ \t\n\r]*
LineComment     "//".*
BlockComment    \/\*([^\*]|(\*+[^\*\/]))*\*+\/

/* Identifier */
Identifier      [a-zA-Z_][a-zA-Z0-9_]*

/* INT_CONST (literal) */
Decimal         [1-9][0-9]*
Octal           0[0-7]*
Hexadecimal     0[xX][0-9a-fA-F]+

/* Operators */
OrderedCompOp   [<>]=
UnorderedCompOp [!=]=
LogicAnd        "&&"
LogicOr         "||"

%%

{WhiteSpace}    { /* Ignore; nop */ }
{LineComment}   { /* Ignore; nop */ }
{BlockComment}  { /* Ignore; nop */ }

"int"           { return INT; }
"void"          { return VOID; }

"const"         { return CONST; }

"return"        { return RETURN; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

{OrderedCompOp}     { yylval.str_val = new string(yytext); return ORDEREDCOMPOP; }
{UnorderedCompOp}   { yylval.str_val = new string(yytext); return UNORDEREDCOMPOP; }
{LogicAnd}          { yylval.str_val = new string(yytext); return LOGICAND; }
{LogicOr}           { yylval.str_val = new string(yytext); return LOGICOR; }

.               { return yytext[0]; }

%%
