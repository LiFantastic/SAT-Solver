#include "sat_api.h"
#include "tools.h"

/******************************************************************************
* unit resolution
**********************************************************************/
/******************************************************************************
* literalSet, set the literal to be true and check new subsumed clause and new
* possibility for unit resolution
*******************************************************************************/
BOOLEAN literalSet(Lit* lit, SatState* sat_state) {
  c2dSize i;
  BOOLEAN noConflict = 1;
  Clause* clause;

  ASSERT_TEST(!sat_implied_literal(lit)); // this literal should not be set 
  ASSERT_TEST(!sat_implied_literal(lit->opp)); // opposite literal should not be set (conflict testing in a different way)

  lit->setLevel = sat_state->currentLevel; // formally set the literal to true only here!!!!
  sat_state->set[sat_state->setCount++] = lit;

  for (i=0; i<lit->occurSize; i++) { // set every clause that has literal l and not already subsumed to subsumed
    clause = lit->occurIn[i];
    if (clause->setLevel==0)
      clause->setLevel = sat_state->currentLevel;
  }
  
  lit = lit->opp; // deal with the opposite literal occurrence 
  for (i=0; i<lit->occurSize; i++) { // update falseLit for every clause that has opposite literal l
    clause = lit->occurIn[i];
    clause->falseLit++;
    if (PRINT) {
      printf("C %-4ld\tFreeLit\t\t%ld", clause->index, clause->size-clause->falseLit);
      if (clause->setLevel>0)
        printf("\t\tSubsumed\t\t\n");
      else
        printf("\t\t        \t\t\n");
    }

    if (noConflict) // no conflict up till now
      noConflict = clauseTest(clause, sat_state); // test whether the new clause cause conflict
    else {
      if (PRINT)
        printf("No clauseTest here due to existing of conflict\n");
      continue; // continue here to keep updating the falseLit of other clauses
    }
  }
  if (PRINT)
    printf("------------------------------------\n");
  return noConflict;
}
/******************************************************************************
* clauseTest       :
* used for unit-resolution
* test a clause is conflict or not, or unit-resolution can be done, do the resolution and update information
*******************************************************************************/
BOOLEAN clauseTest(Clause* clause, SatState* sat_state) {
  if (clause->setLevel!=0)
    return 1; // no conflict if clause is already subsumed
  else if (clause->size == clause->falseLit) { // a conflict clause
    if (PRINT) {
      printf("~~~~~~~~~~~~~~ Conflict ~~~~~~~~~~~~~~\n");
      printClause(clause);
    }
    switch (CON) {
      case 0:
        firstUIP(clause, sat_state);
        break;
      case 1:
        conflictSet(clause, sat_state);  // find the learned clause   
        break;
    }
    return 0;
  }
  else if (clause->size == clause->falseLit+1) { // a clause where unit-resolution can be done 
    Lit* lit;
    c2dSize i;

    ASSERT_TEST(clause->size>=1); 
    lit = clause->lits[0];

    if (DEBUG) {  // debug mode, go through all literals in clause
      c2dSize tmp = 0;
      for (i=0; i<clause->size; i++) {
        if (clause->lits[i]->opp->setLevel==0)
          lit = clause->lits[i]; // the only "free" literal in this clause
        else
          tmp++;
      }
      ASSERT_TEST(tmp==clause->size-1); // test if there are clause->size-1 literals set to 'false' 
    }
    else {
      for (i=0; i<clause->size; i++) {
        if (clause->lits[i]->opp->setLevel==0) {
          lit = clause->lits[i]; // the only "free" literal in this clause
          break;
        }
      }
    }
    // update reason for implied literal -- for conflict driven
    if (!lit->inQueue && !lit->opp->inQueue) { //didn't deal with,  the literal is unset and not in the tosetList or the opp lit is already in
      lit->reason = clause;
      lit->inQueue = 1; // put the lit to the tosetList
      sat_state->queue[sat_state->queueSize++] = lit;
      //sat_state->queue[sat_state->queueSize++] = lit;
      if (PRINT)
        printf("Lit %-+4ld is put into the resolution list\n", lit->index);
    }
    else {
      if (PRINT)
        printf("Lit %-+4ld is discard due to redundancy \n", lit->index);
      ASSERT_TEST(lit->setLevel==sat_state->currentLevel || lit->setLevel==0); // or it can only be set at this level before or is still in the tosetList
    }
    return 1;
  }
  else // ordinary clause, no more information
    return 1;
}
/******************************************************************************
* conflictSet()       :
* find the conflict driven set at a given conflict
*******************************************************************************/
void conflictSet(Clause* clause, SatState* sat_state) {
  c2dSize i;
  c2dSize size = 0;

  Lit* lit;
  Lit** learned;
  Clause* ctmp;

  LitLink *head, *tmp=NULL;

  learned = (Lit**)malloc(0);

  ASSERT_TEST(clause->size>1); // a conflict clause should not have only 1 literal
  
  for (i=0; i<clause->size; i++) {
    head = (LitLink*)malloc(sizeof(LitLink));
    head->lit = clause->lits[i]->opp;  // here notice it is the opposite literal in the current conflict clause
    head->next = tmp;
    tmp = head;
  }

  while(tmp!=NULL) {   // use deep first search
    head = tmp->next; 
    lit = tmp->lit; // take one element from head out, update head, the took out element is in tmp
    free(tmp); // no use any more

    if (PRINT)
      printf("%ld\t", lit->index);

    ASSERT_TEST(lit->setLevel<=sat_state->currentLevel); // l must be set in level not greater than current sat state level 
    
    if (lit->setLevel < sat_state->currentLevel) {  // literal from lower level
      if (lit->seen==0) { // already visited before
        size++;
        learned = realloc(learned, sizeof(Lit*)*size);
        learned[size-1] = lit->opp;
        lit->seen = 1; // set it to has already added
      }
      if (PRINT)
        printf("\n");
    }
    else {  // literal from current level
      if (lit->seen == 1) {  // already visited before
        if (PRINT)
          printf("Search tree pruned here.\n");
      }
      else {
        lit->seen = 1;  // set it to already visited
        // add cause of l, in deep first manner
        ctmp = lit->reason;
        if (ctmp!=NULL) {  // if not at the decision of this level, put it's cause to the list for visiting
          for (i=0; i<ctmp->size; i++) {
            if (ctmp->lits[i]==lit)  // l itself in the cause, ignore
              continue;
            tmp = (LitLink*)malloc(sizeof(LitLink));
            tmp->lit = ctmp->lits[i]->opp;  // here notice it is the opposite literal in the current conflict clause
            tmp->next = head;
            head = tmp;  // add a new element to the head of the link-list
          }
        }
        else {  // put he root to the learned clause
          sat_state->learned = lit->opp;
          size++;
          learned = realloc(learned, sizeof(Lit*)*size);
          learned[size-1] = lit->opp;
          if (PRINT)
            printf("\n");
        }
      }
    }
    tmp = head; // take the element from the head of the link-list 
  }

  sat_state->l++;
  sat_state->delta = realloc(sat_state->delta, sizeof(Clause*)*(sat_state->m+sat_state->l));
  ctmp = clause_new(sat_state->m+sat_state->l, size, learned);

  sat_state->delta[sat_state->m+sat_state->l-1] = ctmp;

  sat_state->learned->reason = sat_state->delta[sat_state->m+sat_state->l-1];

  sat_state->assertionLevel = 1; // initial value
  for (i=0; i<size; i++) {
    //lit_addInClause(learned[i], sat_state->m+sat_state->l); //add the clause index to variables it mentioned
    if (learned[i]->opp->setLevel<sat_state->currentLevel && sat_state->assertionLevel<learned[i]->opp->setLevel)
      sat_state->assertionLevel = learned[i]->opp->setLevel;
  }

  ctmp->setLevel = sat_state->assertionLevel;

  if (PRINT) {
    // printf("Assertion Level: %ld.\n", sat_state->assertionLevel);
    printf("~~~~~~~~~~~~~~ Learned ~~~~~~~~~~~~~~\n");
    printClause(sat_state->delta[sat_state->m+sat_state->l-1]);
    printf("Assertion Level: %4ld\n", sat_state->assertionLevel);
    printf("------------------------------------\n");
  }
  // need to reset lit->seen
  for (i=0; i<sat_state->n; i++) {
    sat_state->variables[i]->pos->seen = 0;
    sat_state->variables[i]->neg->seen = 0;
  }
}
/******************************************************************************
* firstUIP()       :
* asserting clause use first UIP
*******************************************************************************/
void firstUIP(Clause* clause, SatState* sat_state) {
  c2dSize atCurrentLevel;
  Clause *conflict, *reason;
  Lit* focus;
  Lit* tmp;
  Lit** lits = (Lit**)malloc(sizeof(Lit*)*clause->size);

  c2dSize i, j;
  c2dLiteral index;

  BOOLEAN jump;

  for (i=0; i<clause->size; i++)  // deep copy, conflict = clause
    lits[i] = clause->lits[i];

  sat_state->l++;
  sat_state->delta = realloc(sat_state->delta, sizeof(Clause*)*(sat_state->m+sat_state->l));
  conflict = clause_new(sat_state->m+sat_state->l, clause->size, lits);
  sat_state->delta[sat_state->m+sat_state->l-1] = conflict;

  if (PRINT) {
    for (i=0; i<sat_state->setCount; i++)
      printf("%+-4ld\t", sat_state->set[i]->index);
    printf("\n-------------------------------------\n");
  }

  for (i=sat_state->setCount; i>0; i--) {
    focus = sat_state->set[i-1];
    reason = focus->reason;

    jump = 1;   // test if current conflict contains focus
    for (j=0; j<conflict->size; j++) {
      if (conflict->lits[j] == focus->opp) {
        jump = 0;
        break;
      }
    }
    if (jump)
      continue;
    
    index = -1;
    atCurrentLevel = 0;

    for (j=0; j<conflict->size; j++) {
      if (conflict->lits[j]->opp->setLevel == sat_state->currentLevel) {
        atCurrentLevel++;
      } // check if the conflict is the 1UIP now
      
      if (conflict->lits[j] != focus->opp) {
        conflict->lits[j]->seen = 1; // mark exist
      }
      else // the resolution literal, keep the index
        index = j;
    }
    ASSERT_TEST(index>=0);
    if (atCurrentLevel==1) { //todo
      sat_state->learned = conflict->lits[index];
      for (j=0; j<conflict->size; j++)
        conflict->lits[j]->seen = 0;
      break;
    }

    if (PRINT) {
      printf("(%ld)\t\t\t\t\t\t", atCurrentLevel);
      printClause(conflict);
      printf("On %+-4ld with \t", focus->index);
      printClause(reason);
      printf("-------------------------------------------\n");
      //printf("Index: %ld\n", index);
    }
    

    tmp = conflict->lits[index];
    conflict->lits[index] = conflict->lits[conflict->size-1];
    conflict->lits[conflict->size-1] = tmp;   // swap the focus to the last one at conflict

    conflict->size--;
    conflict->lits = realloc(conflict->lits, sizeof(Lit*)*conflict->size); // then delete existence of focus

    for (j=0; j<reason->size; j++) {
      if (!reason->lits[j]->seen && reason->lits[j]!=focus) {
        conflict->size++;
        conflict->lits = realloc(conflict->lits, sizeof(Lit*)*conflict->size);
        conflict->lits[conflict->size-1] = reason->lits[j];
      }
    }

    for (j=0; j<conflict->size; j++)
      conflict->lits[j]->seen = 0; 
  }

  sat_state->learned->reason = conflict;

  sat_state->assertionLevel = 1; // initial value
  for (i=0; i<conflict->size; i++) {
    //lit_addInClause(learned[i], sat_state->m+sat_state->l); //add the clause index to variables it mentioned
    if (conflict->lits[i]->opp->setLevel<sat_state->currentLevel && sat_state->assertionLevel<conflict->lits[i]->opp->setLevel)
      sat_state->assertionLevel = conflict->lits[i]->opp->setLevel;
  }

  conflict->setLevel = sat_state->assertionLevel;


  if (PRINT) {
    // printf("Assertion Level: %ld.\n", sat_state->assertionLevel);
    printf("~~~~~~~~~~~~~~ Learned ~~~~~~~~~~~~~~\n");
    printClause(conflict);
    //printClause(sat_state->delta[sat_state->m+sat_state->l-1]);
    printf("Assertion Level: %4ld\n", sat_state->assertionLevel);
    printf("------------------------------------\n");
  }


  
}
// /******************************************************************************
// * firstUIP()       :
// * asserting clause use first UIP
// *******************************************************************************/
// void firstUIP(Clause* clause, SatState* sat_state) {
//   c2dSize i, j;
//   Lit* lit;
//   Clause* ctmp;

//   c2dSize currentSurfaceSize = 0;
//   Lit** currentSurface = (Lit**)malloc(0);
//   c2dSize nextSurfaceSize = 0;
//   Lit** nextSurface = (Lit**)malloc(0);
//   c2dSize seenChangedSize = 0;
//   Lit** seenChanged = (Lit**)malloc(0);

//   c2dSize size = 0;
//   Lit** learned = (Lit**)malloc(0);

//   for (i=0; i<clause->size; i++) {
//     currentSurfaceSize++;
//     currentSurface = realloc(currentSurface, sizeof(Lit*)*currentSurfaceSize);
//     currentSurface[currentSurfaceSize-1] = clause->lits[i]->opp;

//     currentSurface[currentSurfaceSize-1]->seen = 1;

//     seenChangedSize++;
//     seenChanged = realloc(seenChanged, sizeof(Lit*)*seenChangedSize);
//     seenChanged[seenChangedSize-1] = currentSurface[currentSurfaceSize-1];
//   }

//   while(currentSurfaceSize>1) {
//     if (PRINT) {
//       printf("Current Surface (%ld):\t", currentSurfaceSize);
//       for (i=0; i<currentSurfaceSize; i++)
//         printf("%+-4ld\t", currentSurface[i]->index);
//       printf("\n");
//     }
//     for (i=0; i<currentSurfaceSize; i++) {
//       ctmp = currentSurface[i]->reason;
//       if (ctmp==NULL)
//         continue;
//       for (j=0; j<ctmp->size; j++) {
//         if (ctmp->lits[j]==currentSurface[i])
//           continue;
//         else {
//           lit = ctmp->lits[j]->opp;

//           if (lit->setLevel < sat_state->currentLevel) {  // literal from lower level
//             if (lit->seen==0) { // already visited before
//               size++;
//               learned = realloc(learned, sizeof(Lit*)*size);
//               learned[size-1] = lit->opp;
//               lit->seen = 1; // set it to has already added
//               seenChangedSize++;
//               seenChanged = realloc(seenChanged, sizeof(Lit*)*seenChangedSize);
//               seenChanged[seenChangedSize-1] = lit;
//             }
//           }
//           else if (lit->seen==0) { // current level, not seen
//             nextSurfaceSize++;
//             nextSurface = realloc(nextSurface, sizeof(Lit*)*nextSurfaceSize);
//             nextSurface[nextSurfaceSize-1] = lit;
//             lit->seen = 1;
//             seenChangedSize++;
//             seenChanged = realloc(seenChanged, sizeof(Lit*)*seenChangedSize);
//             seenChanged[seenChangedSize-1] = lit;
//           }
//         }
//       }
//     }
//     printf("nextSurfaceSize---------------%ld\n", nextSurfaceSize);
//     currentSurfaceSize = nextSurfaceSize;
//     currentSurface = realloc(currentSurface, sizeof(Lit*)*currentSurfaceSize);
//     for (i=0; i<currentSurfaceSize; i++)
//       currentSurface[i] = nextSurface[i];
//     nextSurfaceSize = 0;
//     printf("currSurfaceSize-----~~~~~~~~~~~~~---%ld\n", currentSurfaceSize);
//   }
//   printf("First UIP: %+-4ld   %ld\n ", currentSurface[0]->index, currentSurfaceSize);

//   size++;
//   learned = realloc(learned, sizeof(Lit*)*size);
//   learned[size-1] = currentSurface[0]->opp;

//   sat_state->learned = currentSurface[0]->opp;

//   free(currentSurface);
//   free(nextSurface);

//   for (i=0; i<seenChangedSize; i++)
//     seenChanged[i]->seen = 0;

//   free(seenChanged);

//   sat_state->l++;
//   sat_state->delta = realloc(sat_state->delta, sizeof(Clause*)*(sat_state->m+sat_state->l));
//   ctmp = clause_new(sat_state->m+sat_state->l, size, learned);

//   sat_state->delta[sat_state->m+sat_state->l-1] = ctmp;

//   sat_state->learned->reason = sat_state->delta[sat_state->m+sat_state->l-1];

//   sat_state->assertionLevel = 1; // initial value
//   for (i=0; i<size; i++) {
//     //lit_addInClause(learned[i], sat_state->m+sat_state->l); //add the clause index to variables it mentioned
//     if (learned[i]->opp->setLevel<sat_state->currentLevel && sat_state->assertionLevel<learned[i]->opp->setLevel)
//       sat_state->assertionLevel = learned[i]->opp->setLevel;
//   }

//   ctmp->setLevel = sat_state->assertionLevel;

//   if (PRINT) {
//     // printf("Assertion Level: %ld.\n", sat_state->assertionLevel);
//     printf("~~~~~~~~~~~~~~ Learned ~~~~~~~~~~~~~~\n");
//     printClause(sat_state->delta[sat_state->m+sat_state->l-1]);
//     printf("Assertion Level: %4ld\n", sat_state->assertionLevel);
//     printf("------------------------------------\n");
//   }
// }
/******************************************************************************
* literalUnset       :
* used for undo unit-resolution
*******************************************************************************/
void literalUnset(Lit* lit, SatState* sat_state) {
  c2dSize i;
  Clause* clause;

  ASSERT_TEST(lit->opp->setLevel==0); // opposite literal should not be set (conflict testing in a different way)

  for (i=0; i<lit->occurSize; i++) { // check the clause has the lit if is set at this level
    clause = lit->occurIn[i];
    if (clause->setLevel == sat_state->currentLevel)
      clause->setLevel = 0; // set subsumed clause at this level to initial state
  }
  lit->setLevel = 0;
  lit->reason = NULL;

  lit = lit->opp; // focus on the opposite occurrence
  for (i=0; i<lit->occurSize; i++) { // update falsenum for every clause that has opposite literal l
    clause = lit->occurIn[i];
    clause->falseLit--;
    ASSERT_TEST(clause->falseLit>=0); // trivial test
  }
}
/**********************************************************************
* end of functions for unit-resolution

* file operation begin
**********************************************************************/
/**********************************************************************
* given file_name, initial the parameter for sat_state
* especially, initialize for the following:
* struct clause**     delta;
* c2dSize             clauseCap;
* struct var**        variables;
* c2dSize             n;
* c2dSize             m;
* struct literal**    set;
* struct literal**    queue;
**********************************************************************/
void parseFile(char* in, SatState* sat_state) {
  c2dSize i = 0;

  while (1) {
    skipWhitespace(&in);
    if (*in == 0)  // end of file
      break;
    else if (*in == 'c')  // comment line
      skipLine(&in);
    else if (*in == 'p') { // data input begin
      in++;
      skipWhitespace(&in);
      in += 3;   // "cnf"
      sat_state->n = parseNum(&in);
      sat_state->m = parseNum(&in);
      sat_state->delta = (Clause**)malloc(sizeof(Clause*)*sat_state->m);  // initial delta
      sat_state->variables = (Var**)malloc(sizeof(Var*)*sat_state->n);
      sat_state->set = (Lit**)malloc(sizeof(Lit*)*sat_state->n);
      sat_state->queue = (Lit**)malloc(sizeof(Lit*)*sat_state->n);
      for (i=0;i<sat_state->n;i++)
        sat_state->variables[i] = var_new(i+1);  // initial variables
      i = 0;
      skipLine(&in);  // end this line
    }
    else if (*in == '-' || (*in > '0' && *in <= '9')) {
      addClause(&in, sat_state, i+1);  // value of clause index by i+1
      i++;
    }
    else
      break;
  }
  ASSERT_TEST(i==sat_state->m);  // test number of clause read in match with m
}
/**********************************************************************
* return the data from the opened file
**********************************************************************/
char* readFile(FILE *  in) {
  char*   data = malloc(65536);
  int     cap  = 65536;
  int     size = 0;

  while (!feof(in)) {
    if (size == cap) {
      cap *= 2;
      data = realloc(data, cap); 
    }
    size += fread(&data[size], 1, 65536, in);
  }
  data = realloc(data, size+1);
  data[size] = '\0';

  return data;
}
/**********************************************************************
* whitespace 
**********************************************************************/
void skipWhitespace(char** in) {
  while ((**in >= 9 && **in <= 13) || **in == 32)
    (*in)++; 
}
/**********************************************************************
* useless line
**********************************************************************/
void skipLine(char** in) {
  for (;;) {
    if (**in == 0) return;
    if (**in == '\n') { (*in)++; return; }
    (*in)++; 
  } 
}
/**********************************************************************
* literal sign (pos / neg)
**********************************************************************/
BOOLEAN getSign(char** in) {
  skipWhitespace(in);
  if (**in == '-') {(*in)++; return 0;}
  else if (**in == '+') {(*in)++; return 1;}
  else return 1;
}
/**********************************************************************
* literal index without sign
**********************************************************************/
c2dSize parseNum(char** in) {
    c2dSize   val = 0;
    skipWhitespace(in);
    if (**in < '0' || **in > '9') fprintf(stderr, "ERROR! Unexpected char: %c in parsing.\n", **in), exit(-1);
    while (**in >= '0' && **in <= '9')
        val = val*10 + (**in - '0'),
        (*in)++;
    return val; 
}
/**********************************************************************
* file operation end

* initialization function begin
*******************************************************************************/
/**********************************************************************
* variable init
**********************************************************************/
Var* var_new(c2dSize index) {
  Var* var = (Var*)malloc(sizeof(Var));

  var->index = index;

  var->pos = lit_new(index);
  var->neg = lit_new(-index);

  var->pos->var = var;
  var->neg->var = var;

  var->pos->opp = var->neg;
  var->neg->opp = var->pos;
  var->mark = 0;
  return var;
}
/**********************************************************************
* lit init
**********************************************************************/
Lit* lit_new(c2dLiteral index) {
  Lit* lit = (Lit*)malloc(sizeof(Lit));

  lit->index = index;
  lit->setLevel = 0;

  lit->occurIn = (Clause**)malloc(0);
  lit->occurSize = 0;
  
  lit->inQueue = 0;
  lit->seen = 0;

  lit->reason = NULL;
  return lit;
}
void lit_addOccur(Lit* lit, Clause* clause) {
  lit->occurSize++;
  lit->occurIn = realloc(lit->occurIn, sizeof(Clause*)*lit->occurSize);
  lit->occurIn[lit->occurSize-1] = clause;
}
Clause* clause_new(c2dSize index, c2dSize size, Lit** lits) {
  Clause* clause = (Clause*)malloc(sizeof(Clause));

  clause->index = index;
  clause->lits = lits;  
  clause->size = size;
  clause->falseLit = 0;
  clause->setLevel = 0;
  clause->mark = 0;
  return clause;
}
/**********************************************************************
* clause init
**********************************************************************/
void addClause(char** in, SatState* sat_state, c2dSize index) {  // index is the index of current clause
  c2dSize parsedNum;
  c2dSize size=0;
  char sign;
  Var* var;
  Lit** lits;
  Lit* lit;
  Clause* clause;

  clause = (Clause*)malloc(sizeof(Clause));
  sat_state->delta[index-1] = clause;

  lits = (Lit**)malloc(0); //initial 
  while (1) {
    sign = getSign(in);
    parsedNum = parseNum(in);
    if (parsedNum == 0)
      break;  // end of this clause
    else {
      var = sat_index2var(parsedNum, sat_state);
      if (sign == 1)  // positive literal
        lit = sat_pos_literal(var);
      else
        lit = sat_neg_literal(var);
      size++;   // increase the size of this clause
      lits = realloc(lits, sizeof(Lit*)*size);  // resize lits
      lits[size-1] = lit; // add literals to clause
      lit_addOccur(lit, clause); //add the clause to literal it mentioned
    }
  }
  clause->index = index;
  clause->lits = lits;
  clause->size = size;
  clause->falseLit = 0;
  clause->setLevel = 0;
  clause->mark = 0;

  if (size==1 && lits[0]->inQueue==0) { //unit-clause at init level
    sat_state->queue[sat_state->queueSize++] = lits[0];
    lits[0]->inQueue = 1;
  }
}
/******************************************************************************
* // print information for clause
*******************************************************************************/
void printClause(Clause* clause) {
  if (clause!=NULL) {
    c2dSize i;
    printf("C %-4ld:\t\t\t", clause->index);
    for (i=0; i<clause->size; i++) {
      printf("%+-4ld\t\t\t", clause->lits[i]->index);
    }
    printf("\n");
  }
  else
    printf("Decision or Unit clause\n");
}
/******************************************************************************
* // print information for literals
*******************************************************************************/
void printLits(SatState* s) {
  c2dSize i;
  printf("============== Literals ==============\nName\t\t-\t\t+\t\t\n");
  for (i=0; i<s->n; i++)
    printf("%ld\t\t%ld\t\t%ld\t\t\n", s->variables[i]->index, s->variables[i]->pos->setLevel, s->variables[i]->neg->setLevel);
  printf("\nModel: ");
  for (i=0; i<s->n; i++) {
    if (sat_implied_literal(s->variables[i]->pos))
      printf("%ld\t", s->variables[i]->pos->index);
    else
      printf("%ld\t", s->variables[i]->neg->index);
  }
  printf("\n\n");
}
void printInstantiated(SatState* sat_state) {
  c2dSize i;
  printf("Start of level %-4ld, already instantiated:\t", sat_state->currentLevel);
  for (i=0; i<sat_state->setCount; i++)
    printf("%+-4ld\t", sat_state->set[i]->index);
  printf("\n-------------------------------------\n");
}
/******************************************************************************
 * end
 ******************************************************************************/
