#include "sat_api.h"
#include "tools.h"
/******************************************************************************
 * We explain here the functions you need to implement
 *
 * Rules:
 * --You cannot change any parts of the function signatures
 * --You can/should define auxiliary functions to help implementation
 * --You can implement the functions in different files if you wish
 * --That is, you do not need to put everything in a single file
 * --You should carefully read the descriptions and must follow each requirement
 ******************************************************************************/

/******************************************************************************
 * Variables
 ******************************************************************************/
//returns a variable structure for the corresponding index
Var* sat_index2var(c2dSize index, const SatState* sat_state) {
  ASSERT_TEST(index>=1 && index<=sat_var_count(sat_state)); //index range test
  return sat_state->variables[index-1];
}
//returns the index of a variable
c2dSize sat_var_index(const Var* var) {
  return var->index;
}
//returns the variable of a literal
Var* sat_literal_var(const Lit* lit) {
  return lit->var;
}
//returns 1 if the variable is instantiated, 0 otherwise
//a variable is instantiated either by decision or implication (by unit resolution)
BOOLEAN sat_instantiated_var(const Var* var) {
  ASSERT_TEST(!sat_implied_literal(var->pos)||!sat_implied_literal(var->neg)) //pos, neg should not be all implied
  return (sat_implied_literal(var->pos)||sat_implied_literal(var->neg));
}
//returns 1 if all the clauses mentioning the variable are subsumed, 0 otherwise
BOOLEAN sat_irrelevant_var(const Var* var) {
  c2dSize i;
  Clause* clause;

  for (i=0; i<sat_var_occurences(var); i++) {
    clause = sat_clause_of_var(i, var);
    if (sat_subsumed_clause(clause))
      continue;
    else
      return 0;
  }
  return 1;
}
//returns the number of variables in the cnf of sat state
c2dSize sat_var_count(const SatState* sat_state) {
  return sat_state->n;
}
//returns the number of clauses mentioning a variable
//a variable is mentioned by a clause if one of its literals appears in the clause
c2dSize sat_var_occurences(const Var* var) {
  return sat_pos_literal(var)->occurSize+sat_neg_literal(var)->occurSize;
}
//returns the index^th clause that mentions a variable
//index starts from 0, and is less than the number of clauses mentioning the variable
//this cannot be called on a variable that is not mentioned by any clause
Clause* sat_clause_of_var(c2dSize index, const Var* var) {
  ASSERT_TEST(index>=0 && index<sat_var_occurences(var));
  //printf("index %ld, var %ld", index, var->index);
  c2dSize posSize = sat_pos_literal(var)->occurSize;
  if (index < posSize)
    return sat_pos_literal(var)->occurIn[index];
  else
    return sat_neg_literal(var)->occurIn[index-posSize];
}
/******************************************************************************
 * Literals 
 ******************************************************************************/
//returns a literal structure for the corresponding index
Lit* sat_index2literal(c2dLiteral index, const SatState* sat_state) {
  ASSERT_TEST(index!=0);
  if (index>0)
    return sat_pos_literal((sat_index2var(index, sat_state)));
  else
    return sat_neg_literal((sat_index2var(-index, sat_state)));
}
//returns the index of a literal
c2dLiteral sat_literal_index(const Lit* lit) {
  return lit->index;
}
//returns the positive literal of a variable
Lit* sat_pos_literal(const Var* var) {
  return var->pos;
}
//returns the negative literal of a variable
Lit* sat_neg_literal(const Var* var) {
  return var->neg;
}
//returns 1 if the literal is implied, 0 otherwise
//a literal is implied by deciding its variable, or by inference using unit resolution
BOOLEAN sat_implied_literal(const Lit* lit) {
  return lit->setLevel > 0;
}
//sets the literal to true, and then runs unit resolution
//returns a learned clause if unit resolution detected a contradiction, NULL otherwise
//
//if the current decision level is L in the beginning of the call, it should be updated 
//to L+1 so that the decision level of lit and all other literals implied by unit resolution is L+1
Clause* sat_decide_literal(Lit* lit, SatState* sat_state) {

  ASSERT_TEST(!sat_implied_literal(lit)); // this literal should not be set 
  ASSERT_TEST(!sat_implied_literal(lit->opp)); // opposite literal should not be set (conflict testing in a different way)
  
  sat_state->currentLevel++;
  
  // free(sat_state->queue);
  // sat_state->queue = (Lit**)malloc(sizeof(Lit*)*sat_state->n);

  sat_state->queueSize = 1;
  sat_state->queue[0] = lit;

  lit->inQueue = 1; // mark the lit as in the toSet list
  if (sat_unit_resolution(sat_state))
    return NULL;
  else
    return sat_state->delta[sat_clause_count(sat_state)+sat_learned_clause_count(sat_state)-1];
}
//undoes the last literal decision and the corresponding implications obtained by unit resolution
//
//if the current decision level is L in the beginning of the call, it should be updated 
//to L-1 before the call ends
void sat_undo_decide_literal(SatState* sat_state) {
  sat_undo_unit_resolution(sat_state);
  sat_state->currentLevel--;
  ASSERT_TEST(sat_state->currentLevel>=1)
  return;
}
/******************************************************************************
 * Clauses 
 ******************************************************************************/
//returns a clause structure for the corresponding index
Clause* sat_index2clause(c2dSize index, const SatState* sat_state) {
  ASSERT_TEST(index>=1&&index<=sat_clause_count(sat_state));
  return sat_state->delta[index-1];
}
//returns the index of a clause
c2dSize sat_clause_index(const Clause* clause) {
  return clause->index;
}
//returns the literals of a clause
Lit** sat_clause_literals(const Clause* clause) {
  return clause->lits;
}
//returns the number of literals in a clause
c2dSize sat_clause_size(const Clause* clause) {
  return clause->size;
}
//returns 1 if the clause is subsumed, 0 otherwise
BOOLEAN sat_subsumed_clause(const Clause* clause) {
  return clause->setLevel > 0;
}
//returns the number of clauses in the cnf of sat state
c2dSize sat_clause_count(const SatState* sat_state) {
  return sat_state->m;
}
//returns the number of learned clauses in a sat state (0 when the sat state is constructed)
c2dSize sat_learned_clause_count(const SatState* sat_state) {
  return sat_state->l;
}
//adds clause to the set of learned clauses, and runs unit resolution
//returns a learned clause if unit resolution finds a contradiction, NULL otherwise
//
//this function is called on a clause returned by sat_decide_literal() or sat_assert_clause()
//moreover, it should be called only if sat_at_assertion_level() succeeds
Clause* sat_assert_clause(Clause* clause, SatState* sat_state) {
  sat_state->currentLevel--;
  return sat_decide_literal(sat_state->learned, sat_state);
}
/******************************************************************************
 * A SatState should keep track of pretty much everything you will need to
 * condition/uncondition variables, perform unit resolution, and do clause learning
 *
 * Given an input cnf file you should construct a SatState
 *
 * This construction will depend on how you define a SatState
 * Still, you should at least do the following:
 * --read a cnf (in DIMACS format, possible with weights) from the file
 * --initialize variables (n of them)
 * --initialize literals  (2n of them)
 * --initialize clauses   (m of them)
 *
 * Once a SatState is constructed, all of the functions that work on a SatState
 * should be ready to use
 *
 * You should also write a function that frees the memory allocated by a
 * SatState (sat_state_free)
 ******************************************************************************/
//constructs a SatState from an input cnf file
SatState* sat_state_new(const char* file_name) {
  SatState* sat_state = (SatState*)malloc(sizeof(SatState));

  sat_state->l = 0;
  sat_state->currentLevel = 1;
  sat_state->assertionLevel = 0;
  sat_state->learned = NULL;
  sat_state->setCount = 0;
  sat_state->queueSize = 0;
  sat_state->conflict = 0; 
  /***************************************************************
  struct clause**     delta; //database, clauses for this cnf
  c2dSize             clauseCap;
  struct var**        variables; //variables in cnf
  c2dSize             n; //number of variables
  c2dSize             m; //number of clauses in original database
  struct literal**    set;  //currently implied
  struct literal**    queue; // resolution queue 
  *****************************************************************/
  FILE *fin;
  fin = fopen(file_name, "rb");
  if (fin == NULL) {
    fprintf(stderr, "ERROR! Could not open file: %s\n", file_name);
    return NULL;
  }
  char* inputStream = readFile(fin);
  parseFile(inputStream, sat_state);
  free(inputStream);

  if (PRINT) {
    c2dSize i = 0, j=0;
    printf("\nSatState constructed!\n\n============== Clauses ==============\n");
    for (i=0; i<sat_state->m; i++)
      printClause(sat_state->delta[i]);
    printf("============== Variables ==============\n");
    for (i=0; i<sat_state->n; i++) {
      printf("%+-4ld:\t\t", sat_state->variables[i]->pos->index);
      for (j=0; j<sat_state->variables[i]->pos->occurSize; j++)
        printf("%4ld\t\t", sat_state->variables[i]->pos->occurIn[j]->index);

      printf("\n%+-4ld:\t\t", sat_state->variables[i]->neg->index);
      for (j=0; j<sat_state->variables[i]->neg->occurSize; j++)
        printf("%4ld\t\t", sat_state->variables[i]->neg->occurIn[j]->index);
      printf("\n------------------------------------\n");
    }
  }
  return sat_state;
}

//frees the SatState
void sat_state_free(SatState* sat_state) {
  c2dSize i;
  
  for (i=1; i<=sat_var_count(sat_state); i++) {
    free(sat_pos_literal(sat_index2var(i, sat_state))->occurIn);
    free(sat_pos_literal(sat_index2var(i, sat_state)));
    free(sat_neg_literal(sat_index2var(i, sat_state))->occurIn);
    free(sat_neg_literal(sat_index2var(i, sat_state)));
    free(sat_index2var(i, sat_state));
  }
  for (i=0; i<sat_clause_count(sat_state)+sat_learned_clause_count(sat_state); i++) {
    free(sat_state->delta[i]->lits);
    free(sat_state->delta[i]);
  }
  free(sat_state->delta);
  free(sat_state->variables);
  free(sat_state->queue);
  free(sat_state->set);
  return;
}
/******************************************************************************
 * Given a SatState, which should contain data related to the current setting
 * (i.e., decided literals, subsumed clauses, decision level, etc.), this function 
 * should perform unit resolution at the current decision level 
 *
 * It returns 1 if succeeds, 0 otherwise (after constructing an asserting
 * clause)
 *
 * There are three possible places where you should perform unit resolution:
 * (1) after deciding on a new literal (i.e., in sat_decide_literal())
 * (2) after adding an asserting clause (i.e., in sat_assert_clause(...)) 
 * (3) neither the above, which would imply literals appearing in unit clauses
 *
 * (3) would typically happen only once and before the other two cases
 * It may be useful to distinguish between the above three cases
 * 
 * Note if the current decision level is L, then the literals implied by unit
 * resolution must have decision level L
 *
 * This implies that there must be a start level S, which will be the level
 * where the decision sequence would be empty
 *
 * We require you to choose S as 1, then literals implied by (3) would have 1 as
 * their decision level (this level will also be the assertion level of unit
 * clauses)
 *
 * Yet, the first decided literal must have 2 as its decision level
 ******************************************************************************/
//applies unit resolution to the cnf of sat state
//returns 1 if unit resolution succeeds, 0 if it finds a contradiction
BOOLEAN sat_unit_resolution(SatState* sat_state) {
  c2dSize i;
  BOOLEAN noConflict = 1;
  Lit* lit;

  if (PRINT)
    printInstantiated(sat_state);

  for(i=0; i<sat_state->queueSize; i++) {
    lit = sat_state->queue[i];

    if (PRINT) {
      printf("Literal %+-4ld is set at level %ld \n", lit->index, sat_state->currentLevel);
      printf("Reason for %-+4ld ------->   ", lit->index);
      printClause(lit->reason);
      printf("------------------------------------\n");
    }

    noConflict = literalSet(lit, sat_state);
    if (!noConflict)
      break;
  }

  for(i=0; i<sat_state->queueSize; i++) {
    lit = sat_state->queue[i];
    lit->inQueue = 0; // back to init
    if (lit->setLevel==0)
      lit->reason = NULL; //maybe changed but literal not implied 
  }
  sat_state->queueSize = 0; //clear all possible implied
  return noConflict;
}
//undoes sat_unit_resolution(), leading to un-instantiating variables that have been instantiated
//after sat_unit_resolution()
void sat_undo_unit_resolution(SatState* sat_state) {
  c2dSize i, count = 0;
  Lit* lit;

  for (i=sat_state->setCount; i>0; i--) {
    lit = sat_state->set[i-1];
    if (lit->setLevel == sat_state->currentLevel) {
      literalUnset(lit, sat_state);
      count++;
    }
    else
      break;
  }

  sat_state->setCount -= count;
}
//returns 1 if the decision level of the sat state equals to the assertion level of clause,
//0 otherwise
//
//this function is called after sat_decide_literal() or sat_assert_clause() returns clause.
//it is used to decide whether the sat state is at the right decision level for adding clause.
BOOLEAN sat_at_assertion_level(const Clause* clause, const SatState* sat_state) {
  return sat_state->currentLevel == sat_state->assertionLevel;
}

/******************************************************************************
 * The functions below are already implemented for you and MUST STAY AS IS
 ******************************************************************************/
//returns the weight of a literal (which is 1 for our purposes)
c2dWmc sat_literal_weight(const Lit* lit) {
  return 1;
}
//returns 1 if a variable is marked, 0 otherwise
BOOLEAN sat_marked_var(const Var* var) {
  return var->mark;
}
//marks a variable (which is not marked already)
void sat_mark_var(Var* var) {
  var->mark = 1;
}
//unmarks a variable (which is marked already)
void sat_unmark_var(Var* var) {
  var->mark = 0;
}
//returns 1 if a clause is marked, 0 otherwise
BOOLEAN sat_marked_clause(const Clause* clause) {
  return clause->mark;
}
//marks a clause (which is not marked already)
void sat_mark_clause(Clause* clause) {
  clause->mark = 1;
}
//unmarks a clause (which is marked already)
void sat_unmark_clause(Clause* clause) {
  clause->mark = 0;
}
/******************************************************************************
 * end
 ******************************************************************************/
