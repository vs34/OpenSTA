// OpenSTA, Static Timing Analyzer
// Copyright (c) 2025, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// 
// The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.
// 
// Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 
// This notice may not be removed or altered from any source distribution.

%{
#include <cstdlib>

#include "Report.hh"
#include "liberty/LibertyParser.hh"
#include "liberty/LibertyScanner.hh"

#undef yylex
#define yylex scanner->lex

// warning: variable 'yynerrs_' set but not used
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define loc_line(loc) loc.begin.line

void
sta::LibertyParse::error(const location_type &loc,
                         const std::string &msg)
{
  reader->report()->fileError(164, reader->filename().c_str(),
                              loc.begin.line, "%s", msg.c_str());
}

%}

%require  "3.2"
%skeleton "lalr1.cc"
%debug
%define api.namespace {sta}
%locations
%define api.location.file "LibertyLocation.hh"
%define parse.assert
%parse-param { LibertyScanner *scanner }
%parse-param { LibertyParser *reader }
%define api.parser.class {LibertyParse}

%expect 2

%union {
  char *string;
  float number;
  char ch;
  sta::LibertyAttrValue *attr_value;
  sta::LibertyAttrValueSeq *attr_values;
  sta::LibertyGroup *group;
  sta::LibertyStmt *stmt;
}

%left '+' '-' '|'
%left '*' '/' '&'
%left '^'
%left '!'

%token <number> FLOAT
%token <string> STRING KEYWORD

%type <stmt> statement complex_attr simple_attr variable group file
%type <attr_values> attr_values
%type <attr_value> attr_value
%type <string> string expr expr_term expr_term1 volt_expr
%type <ch> expr_op volt_op

%start file

%%

file:
	group
	;

group:
	KEYWORD '(' ')' '{'
	{ reader->groupBegin($1, nullptr, loc_line(@1)); }
	'}' semi_opt
	{ $$ = reader->groupEnd(); }
|	KEYWORD '(' ')' '{'
	{ reader->groupBegin($1, nullptr, loc_line(@1)); }
	statements '}' semi_opt
	{ $$ = reader->groupEnd(); }
|	KEYWORD '(' attr_values ')' '{'
	{ reader->groupBegin($1, $3, loc_line(@1)); }
	'}' semi_opt
	{ $$ = reader->groupEnd(); }
|	KEYWORD '(' attr_values ')' '{'
	{ reader->groupBegin($1, $3, loc_line(@1)); }
	statements '}' semi_opt
	{ $$ = reader->groupEnd(); }
	;

statements:
	statement
|	statements statement
	;

statement:
	simple_attr
|	complex_attr
|	group
|	variable
	;

simple_attr:
	KEYWORD ':' attr_value semi_opt
	{ $$ = reader->makeSimpleAttr($1, $3, loc_line(@1)); }
	;

complex_attr:
	KEYWORD '(' ')' semi_opt
	{ $$ = reader->makeComplexAttr($1, nullptr, loc_line(@1)); }
|	KEYWORD '(' attr_values ')' semi_opt
	{ $$ = reader->makeComplexAttr($1, $3, loc_line(@1)); }
	;

attr_values:
	attr_value
	{ $$ = new sta::LibertyAttrValueSeq;
	  $$->push_back($1);
	}
|	attr_values ',' attr_value
        { $1->push_back($3);
	  $$ = $1;
	}
|	attr_values attr_value
        { $1->push_back($2);
	  $$ = $1;
	}
	;

variable:
	string '=' FLOAT semi_opt
	{ $$ = reader->makeVariable($1, $3, loc_line(@1)); }
	;

string:
	STRING
	{ $$ = $1; }
|	KEYWORD
	{ $$ = $1; }
	;

attr_value:
	FLOAT
	{ $$ = reader->makeFloatAttrValue($1); }
|       expr
	{ $$ = reader->makeStringAttrValue($1); }
|	volt_expr
	{ $$ = reader->makeStringAttrValue($1); }
	;

/* Voltage expressions are ignored. */
/* Crafted to avoid conflicts with expr */
volt_expr:
        FLOAT volt_op FLOAT
	{ $$ = sta::stringPrint("%e%c%e", $1, $2, $3); }
|       string volt_op FLOAT
	{ $$ = sta::stringPrint("%s%c%e", $1, $2, $3);
          sta::stringDelete($1);
        }
|       FLOAT volt_op string
	{ $$ = sta::stringPrint("%e%c%s", $1, $2, $3);
          sta::stringDelete($3);
        }
|       volt_expr volt_op FLOAT
	{ $$ = sta::stringPrint("%s%c%e", $1, $2, $3);
          sta::stringDelete($1);
        }
        ;

volt_op:
	'+'
        { $$ = '+'; }
|	'-'
        { $$ = '-'; }
|	'*'
        { $$ = '*'; }
|	'/'
        { $$ = '/'; }
	;

expr:
        expr_term1
|	expr_term1 expr_op expr
	{ $$ = sta::stringPrint("%s%c%s", $1, $2, $3);
          sta::stringDelete($1);
          sta::stringDelete($3);
        }
	;

expr_term:
	string
|	'0'
	{ $$ = sta::stringPrint("0"); }
|	'1'
	{ $$ = sta::stringPrint("1"); }
|	'(' expr ')'
	{ $$ = sta::stringPrint("(%s)", $2);
          sta::stringDelete($2);
        }
	;

expr_term1:
	expr_term
|       '!' expr_term
	{ $$ = sta::stringPrint("!%s", $2);
          sta::stringDelete($2);
        }
|	expr_term '\''
	{ $$ = sta::stringPrint("%s'", $1);
          sta::stringDelete($1);
        }
	;

expr_op:
	'+'
        { $$ = '+'; }
|	'|'
        { $$ = '|'; }
|	'*'
        { $$ = '*'; }
|	'&'
        { $$ = '&'; }
|	'^'
        { $$ = '^'; }
	;

semi_opt:
	/* empty */
|	semi_opt ';'
	;

%%
