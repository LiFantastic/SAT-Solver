#ifndef TOOL_H_
#define TOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sat_api.h"


typedef struct litlink {
  struct literal*     lit;
  struct litlink*     next;
} LitLink;

/******************************************************************************
* self-defined function tools used in sat_api.h
 ******************************************************************************/

/******************************************************************************
* for unit resolution
*******************************************************************************/
BOOLEAN literalSet(Lit*, SatState*);  // set the literal at current level
BOOLEAN clauseTest(Clause*, SatState*);
void conflictSet(Clause*, SatState*);  // conflict set method
void firstUIP(Clause*, SatState*);  // 1-UIP method

void literalUnset(Lit*, SatState*);
/******************************************************************************
* initialization function
*******************************************************************************/
void parseFile(char*, SatState*);
char* readFile(FILE *);
void skipWhitespace(char**);
void skipLine(char**);
BOOLEAN getSign(char**);
c2dSize parseNum(char**);

void addClause(char**, SatState*, c2dSize);
Lit* lit_new(c2dLiteral);
void lit_addOccur(Lit*, Clause*);
Var* var_new(c2dSize);
Clause* clause_new(c2dSize, c2dSize, Lit**);
/******************************************************************************
* // print information for clause/literals
*******************************************************************************/
void printClause(Clause*);
void printLits(SatState*);
void printInstantiated(SatState*);
/******************************************************************************/

#endif //TOOL_H_

/******************************************************************************
 * end
 ******************************************************************************/
