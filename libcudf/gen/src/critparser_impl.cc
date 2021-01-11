/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
#line 26 "critparser_impl.y"


#include <cassert>
#include "parser_impl.h"
#include "cudf/critparser.hh"

#line 35 "critparser_impl.c"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    critparserTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is critparserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    critparserARG_SDECL     A static variable declaration for the %extra_argument
**    critparserARG_PDECL     A parameter declaration for the %extra_argument
**    critparserARG_STORE     Code to store %extra_argument into yypParser
**    critparserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 28
#define YYACTIONTYPE unsigned char
#define critparserTOKENTYPE  CritParser::Token 
typedef union {
  int yyinit;
  critparserTOKENTYPE yy0;
  std::string const* yy28;
  Criterion* yy42;
  Criterion::Selector yy45;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 0
#endif
#define critparserARG_SDECL  CritParser *pParser ;
#define critparserARG_PDECL , CritParser *pParser 
#define critparserARG_FETCH  CritParser *pParser  = yypParser->pParser 
#define critparserARG_STORE yypParser->pParser  = pParser 
#define YYNSTATE             35
#define YYNRULE              40
#define YY_MAX_SHIFT         34
#define YY_MIN_SHIFTREDUCE   71
#define YY_MAX_SHIFTREDUCE   110
#define YY_MIN_REDUCE        111
#define YY_MAX_REDUCE        150
#define YY_ERROR_ACTION      151
#define YY_ACCEPT_ACTION     152
#define YY_NO_ACTION         153
/************* End control #defines *******************************************/

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if:
**    (1)  The yy_shift_ofst[S]+X value is out of range, or
**    (2)  yy_lookahead[yy_shift_ofst[S]+X] is not equal to X, or
**    (3)  yy_shift_ofst[S] equal YY_SHIFT_USE_DFLT.
** (Implementation note: YY_SHIFT_USE_DFLT is chosen so that
** YY_SHIFT_USE_DFLT+X will be out of range for all possible lookaheads X.
** Hence only tests (1) and (2) need to be evaluated.)
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (93)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   102,   27,   25,  103,  104,  105,  106,   22,   21,   23,
 /*    10 */    24,   20,   19,   18,   17,   16,   92,  102,  147,   10,
 /*    20 */   103,  104,  105,  106,   95,   96,   94,   93,   97,   98,
 /*    30 */    99,  100,  101,   92,   85,   86,   84,   83,   87,   88,
 /*    40 */    89,   90,   91,   34,   12,   13,   32,   30,   28,   15,
 /*    50 */    77,   78,   79,  152,   11,   11,  148,   26,    9,   14,
 /*    60 */    29,    9,   31,   33,    2,   76,  111,    3,    5,    1,
 /*    70 */   149,  113,  141,  140,  139,   74,  138,  113,  137,  136,
 /*    80 */   135,   73,  134,  113,  133,   82,   75,    4,   72,    6,
 /*    90 */   113,    7,    8,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,   21,   22,    6,    7,    8,    9,   10,   11,   12,
 /*    10 */    13,   14,   15,   16,   17,   18,   19,    3,    0,    1,
 /*    20 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*    30 */    16,   17,   18,   19,   10,   11,   12,   13,   14,   15,
 /*    40 */    16,   17,   18,    3,   22,   22,    6,    7,    8,    9,
 /*    50 */    10,   11,   12,   24,   25,   26,    0,   22,    2,   21,
 /*    60 */    21,    2,   21,   21,    1,    5,   23,    1,    4,    4,
 /*    70 */    26,   27,    5,    5,    5,    5,    5,   27,    5,    5,
 /*    80 */     5,    5,    5,   27,    5,    5,    5,    1,    5,    4,
 /*    90 */    27,    4,    4,
};
#define YY_SHIFT_USE_DFLT (93)
#define YY_SHIFT_COUNT    (34)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (88)
static const signed char yy_shift_ofst[] = {
 /*     0 */    56,   -3,   14,   14,   14,   24,   24,   24,   24,   40,
 /*    10 */    59,   18,   60,   63,   66,   64,   67,   68,   69,   71,
 /*    20 */    73,   74,   75,   77,   79,   80,   81,   86,   65,   70,
 /*    30 */    85,   76,   87,   83,   88,
};
#define YY_REDUCE_USE_DFLT (-21)
#define YY_REDUCE_COUNT (10)
#define YY_REDUCE_MIN   (-20)
#define YY_REDUCE_MAX   (44)
static const signed char yy_reduce_ofst[] = {
 /*     0 */    29,  -20,   22,   23,   35,   38,   39,   41,   42,   43,
 /*    10 */    44,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   151,  151,  151,  151,  151,  151,  151,  151,  151,  151,
 /*    10 */   151,  151,  151,  151,  151,  151,  131,  130,  129,  128,
 /*    20 */   127,  126,  125,  124,  123,  151,  151,  151,  151,  151,
 /*    30 */   121,  151,  120,  151,  151,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.
** If a construct like the following:
**
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  critparserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
  yyStackEntry *yystackEnd;            /* Last entry in the stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/*
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void critparserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = {
  "$",             "COMMA",         "SIGN",          "COUNT",       
  "LPAREN",        "RPAREN",        "NOTUPTODATE",   "UNSAT_RECOMMENDS",
  "SUM",           "ALIGNED",       "NEW",           "REMOVED",     
  "CHANGED",       "SOLUTION",      "UP",            "DOWN",        
  "INSTALLREQUEST",  "UPGRADEREQUEST",  "REQUEST",       "ATTR",        
  "error",         "selector",      "attr",          "crit",        
  "crits",         "ncrits",        "scrit",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "scrit ::= SIGN crit",
 /*   1 */ "crit ::= COUNT LPAREN selector RPAREN",
 /*   2 */ "crit ::= NOTUPTODATE LPAREN selector RPAREN",
 /*   3 */ "crit ::= UNSAT_RECOMMENDS LPAREN selector RPAREN",
 /*   4 */ "crit ::= SUM LPAREN selector COMMA attr RPAREN",
 /*   5 */ "crit ::= ALIGNED LPAREN selector COMMA attr COMMA attr RPAREN",
 /*   6 */ "crit ::= NEW",
 /*   7 */ "crit ::= REMOVED",
 /*   8 */ "crit ::= CHANGED",
 /*   9 */ "crit ::= NOTUPTODATE",
 /*  10 */ "crit ::= UNSAT_RECOMMENDS",
 /*  11 */ "crit ::= SUM LPAREN attr RPAREN",
 /*  12 */ "selector ::= SOLUTION",
 /*  13 */ "selector ::= CHANGED",
 /*  14 */ "selector ::= NEW",
 /*  15 */ "selector ::= REMOVED",
 /*  16 */ "selector ::= UP",
 /*  17 */ "selector ::= DOWN",
 /*  18 */ "selector ::= INSTALLREQUEST",
 /*  19 */ "selector ::= UPGRADEREQUEST",
 /*  20 */ "selector ::= REQUEST",
 /*  21 */ "attr ::= ATTR",
 /*  22 */ "attr ::= SOLUTION",
 /*  23 */ "attr ::= CHANGED",
 /*  24 */ "attr ::= NEW",
 /*  25 */ "attr ::= REMOVED",
 /*  26 */ "attr ::= UP",
 /*  27 */ "attr ::= DOWN",
 /*  28 */ "attr ::= INSTALLREQUEST",
 /*  29 */ "attr ::= UPGRADEREQUEST",
 /*  30 */ "attr ::= REQUEST",
 /*  31 */ "attr ::= COUNT",
 /*  32 */ "attr ::= NOTUPTODATE",
 /*  33 */ "attr ::= UNSAT_RECOMMENDS",
 /*  34 */ "attr ::= SUM",
 /*  35 */ "attr ::= ALIGNED",
 /*  36 */ "crits ::= ncrits",
 /*  37 */ "crits ::=",
 /*  38 */ "ncrits ::= ncrits COMMA scrit",
 /*  39 */ "ncrits ::= scrit",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = (yyStackEntry*)malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = (yyStackEntry*)realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0;
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to critparserAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* Initialize a new parser that has already been allocated.
*/
void critparserInit(void *yypParser){
  yyParser *pParser = (yyParser*)yypParser;
#ifdef YYTRACKMAXSTACKDEPTH
  pParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
  pParser->yytos = NULL;
  pParser->yystack = NULL;
  pParser->yystksz = 0;
  if( yyGrowStack(pParser) ){
    pParser->yystack = &pParser->yystk0;
    pParser->yystksz = 1;
  }
#endif
#ifndef YYNOERRORRECOVERY
  pParser->yyerrcnt = -1;
#endif
  pParser->yytos = pParser->yystack;
  pParser->yystack[0].stateno = 0;
  pParser->yystack[0].major = 0;
#if YYSTACKDEPTH>0
  pParser->yystackEnd = &pParser->yystack[YYSTACKDEPTH-1];
#endif
}

#ifndef critparser_ENGINEALWAYSONSTACK
/*
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to critparser and critparserFree.
*/
void *critparserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ) critparserInit(pParser);
  return pParser;
}
#endif /* critparser_ENGINEALWAYSONSTACK */


/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  critparserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
      /* TERMINAL Destructor */
    case 1: /* COMMA */
    case 2: /* SIGN */
    case 3: /* COUNT */
    case 4: /* LPAREN */
    case 5: /* RPAREN */
    case 6: /* NOTUPTODATE */
    case 7: /* UNSAT_RECOMMENDS */
    case 8: /* SUM */
    case 9: /* ALIGNED */
    case 10: /* NEW */
    case 11: /* REMOVED */
    case 12: /* CHANGED */
    case 13: /* SOLUTION */
    case 14: /* UP */
    case 15: /* DOWN */
    case 16: /* INSTALLREQUEST */
    case 17: /* UPGRADEREQUEST */
    case 18: /* REQUEST */
    case 19: /* ATTR */
{
#line 40 "critparser_impl.y"
 (void)pParser; (void)(yypminor->yy0); 
#line 543 "critparser_impl.c"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/*
** Clear all secondary memory allocations from the parser
*/
void critparserFinalize(void *p){
  yyParser *pParser = (yyParser*)p;
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
}

#ifndef critparser_ENGINEALWAYSONSTACK
/*
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void critparserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
#ifndef YYPARSEFREENEVERNULL
  if( p==0 ) return;
#endif
  critparserFinalize(p);
  (*freeProc)(p);
}
#endif /* critparser_ENGINEALWAYSONSTACK */

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int critparserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yytos->stateno;

  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
        iLookAhead = iFallback;
        continue;
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if(
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD && iLookAhead>0
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead],
               yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   critparserARG_FETCH;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   critparserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  critparserTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0
  if( yypParser->yytos>yypParser->yystackEnd ){
    yypParser->yytos--;
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yypParser->yytos--;
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;       /* Symbol on the left-hand side of the rule */
  signed char nrhs;     /* Negative of the number of RHS symbols in the rule */
} yyRuleInfo[] = {
  { 26, -2 },
  { 23, -4 },
  { 23, -4 },
  { 23, -4 },
  { 23, -6 },
  { 23, -8 },
  { 23, -1 },
  { 23, -1 },
  { 23, -1 },
  { 23, -1 },
  { 23, -1 },
  { 23, -4 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 21, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 22, -1 },
  { 24, -1 },
  { 24, 0 },
  { 25, -3 },
  { 25, -1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  critparserARG_FETCH;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[yysize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0
    if( yypParser->yytos>=yypParser->yystackEnd ){
      yyStackOverflow(yypParser);
      return;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        return;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* scrit ::= SIGN crit */
#line 56 "critparser_impl.y"
{ yymsp[0].minor.yy42->optimize = yymsp[-1].minor.yy0.maximize; }
#line 901 "critparser_impl.c"
        break;
      case 1: /* crit ::= COUNT LPAREN selector RPAREN */
{  yy_destructor(yypParser,3,&yymsp[-3].minor);
#line 58 "critparser_impl.y"
{ yymsp[-3].minor.yy42 = &pParser->pushCrit(Criterion::COUNT, yymsp[-1].minor.yy45); }
#line 907 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 2: /* crit ::= NOTUPTODATE LPAREN selector RPAREN */
{  yy_destructor(yypParser,6,&yymsp[-3].minor);
#line 59 "critparser_impl.y"
{ yymsp[-3].minor.yy42 = &pParser->pushCrit(Criterion::NOTUPTODATE, yymsp[-1].minor.yy45); }
#line 916 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 3: /* crit ::= UNSAT_RECOMMENDS LPAREN selector RPAREN */
{  yy_destructor(yypParser,7,&yymsp[-3].minor);
#line 60 "critparser_impl.y"
{ yymsp[-3].minor.yy42 = &pParser->pushCrit(Criterion::UNSAT_RECOMMENDS, yymsp[-1].minor.yy45); }
#line 925 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 4: /* crit ::= SUM LPAREN selector COMMA attr RPAREN */
{  yy_destructor(yypParser,8,&yymsp[-5].minor);
#line 61 "critparser_impl.y"
{ yymsp[-5].minor.yy42 = &pParser->pushCrit(Criterion::SUM, yymsp[-3].minor.yy45, yymsp[-1].minor.yy28); }
#line 934 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 5: /* crit ::= ALIGNED LPAREN selector COMMA attr COMMA attr RPAREN */
{  yy_destructor(yypParser,9,&yymsp[-7].minor);
#line 62 "critparser_impl.y"
{ yymsp[-7].minor.yy42 = &pParser->pushCrit(Criterion::ALIGNED, yymsp[-5].minor.yy45, yymsp[-3].minor.yy28, yymsp[-1].minor.yy28); }
#line 944 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-6].minor);
  yy_destructor(yypParser,1,&yymsp[-4].minor);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 6: /* crit ::= NEW */
{  yy_destructor(yypParser,10,&yymsp[0].minor);
#line 64 "critparser_impl.y"
{ yymsp[0].minor.yy42 = &pParser->pushCrit(Criterion::COUNT, Criterion::NEW); }
#line 955 "critparser_impl.c"
}
        break;
      case 7: /* crit ::= REMOVED */
{  yy_destructor(yypParser,11,&yymsp[0].minor);
#line 65 "critparser_impl.y"
{ yymsp[0].minor.yy42 = &pParser->pushCrit(Criterion::COUNT, Criterion::REMOVED); }
#line 962 "critparser_impl.c"
}
        break;
      case 8: /* crit ::= CHANGED */
{  yy_destructor(yypParser,12,&yymsp[0].minor);
#line 66 "critparser_impl.y"
{ yymsp[0].minor.yy42 = &pParser->pushCrit(Criterion::COUNT, Criterion::CHANGED); }
#line 969 "critparser_impl.c"
}
        break;
      case 9: /* crit ::= NOTUPTODATE */
{  yy_destructor(yypParser,6,&yymsp[0].minor);
#line 67 "critparser_impl.y"
{ yymsp[0].minor.yy42 = &pParser->pushCrit(Criterion::NOTUPTODATE, Criterion::SOLUTION); }
#line 976 "critparser_impl.c"
}
        break;
      case 10: /* crit ::= UNSAT_RECOMMENDS */
{  yy_destructor(yypParser,7,&yymsp[0].minor);
#line 68 "critparser_impl.y"
{ yymsp[0].minor.yy42 = &pParser->pushCrit(Criterion::UNSAT_RECOMMENDS, Criterion::SOLUTION); }
#line 983 "critparser_impl.c"
}
        break;
      case 11: /* crit ::= SUM LPAREN attr RPAREN */
{  yy_destructor(yypParser,8,&yymsp[-3].minor);
#line 69 "critparser_impl.y"
{ yymsp[-3].minor.yy42 = &pParser->pushCrit(Criterion::SUM, Criterion::SOLUTION, yymsp[-1].minor.yy28); }
#line 990 "critparser_impl.c"
  yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
        break;
      case 12: /* selector ::= SOLUTION */
{  yy_destructor(yypParser,13,&yymsp[0].minor);
#line 71 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::SOLUTION; }
#line 999 "critparser_impl.c"
}
        break;
      case 13: /* selector ::= CHANGED */
{  yy_destructor(yypParser,12,&yymsp[0].minor);
#line 72 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::CHANGED; }
#line 1006 "critparser_impl.c"
}
        break;
      case 14: /* selector ::= NEW */
{  yy_destructor(yypParser,10,&yymsp[0].minor);
#line 73 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::NEW; }
#line 1013 "critparser_impl.c"
}
        break;
      case 15: /* selector ::= REMOVED */
{  yy_destructor(yypParser,11,&yymsp[0].minor);
#line 74 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::REMOVED; }
#line 1020 "critparser_impl.c"
}
        break;
      case 16: /* selector ::= UP */
{  yy_destructor(yypParser,14,&yymsp[0].minor);
#line 75 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::UP; }
#line 1027 "critparser_impl.c"
}
        break;
      case 17: /* selector ::= DOWN */
{  yy_destructor(yypParser,15,&yymsp[0].minor);
#line 76 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::DOWN; }
#line 1034 "critparser_impl.c"
}
        break;
      case 18: /* selector ::= INSTALLREQUEST */
{  yy_destructor(yypParser,16,&yymsp[0].minor);
#line 77 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::INSTALLREQUEST; }
#line 1041 "critparser_impl.c"
}
        break;
      case 19: /* selector ::= UPGRADEREQUEST */
{  yy_destructor(yypParser,17,&yymsp[0].minor);
#line 78 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::UPGRADEREQUEST; }
#line 1048 "critparser_impl.c"
}
        break;
      case 20: /* selector ::= REQUEST */
{  yy_destructor(yypParser,18,&yymsp[0].minor);
#line 79 "critparser_impl.y"
{ yymsp[0].minor.yy45 = Criterion::REQUEST; }
#line 1055 "critparser_impl.c"
}
        break;
      case 21: /* attr ::= ATTR */
#line 81 "critparser_impl.y"
{ yylhsminor.yy28 = yymsp[0].minor.yy0.string; }
#line 1061 "critparser_impl.c"
  yymsp[0].minor.yy28 = yylhsminor.yy28;
        break;
      case 22: /* attr ::= SOLUTION */
{  yy_destructor(yypParser,13,&yymsp[0].minor);
#line 82 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("solution"); }
#line 1068 "critparser_impl.c"
}
        break;
      case 23: /* attr ::= CHANGED */
{  yy_destructor(yypParser,12,&yymsp[0].minor);
#line 83 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("changed"); }
#line 1075 "critparser_impl.c"
}
        break;
      case 24: /* attr ::= NEW */
{  yy_destructor(yypParser,10,&yymsp[0].minor);
#line 84 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("new"); }
#line 1082 "critparser_impl.c"
}
        break;
      case 25: /* attr ::= REMOVED */
{  yy_destructor(yypParser,11,&yymsp[0].minor);
#line 85 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("removed"); }
#line 1089 "critparser_impl.c"
}
        break;
      case 26: /* attr ::= UP */
{  yy_destructor(yypParser,14,&yymsp[0].minor);
#line 86 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("up"); }
#line 1096 "critparser_impl.c"
}
        break;
      case 27: /* attr ::= DOWN */
{  yy_destructor(yypParser,15,&yymsp[0].minor);
#line 87 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("down"); }
#line 1103 "critparser_impl.c"
}
        break;
      case 28: /* attr ::= INSTALLREQUEST */
{  yy_destructor(yypParser,16,&yymsp[0].minor);
#line 88 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("installrequest"); }
#line 1110 "critparser_impl.c"
}
        break;
      case 29: /* attr ::= UPGRADEREQUEST */
{  yy_destructor(yypParser,17,&yymsp[0].minor);
#line 89 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("upgraderequest"); }
#line 1117 "critparser_impl.c"
}
        break;
      case 30: /* attr ::= REQUEST */
{  yy_destructor(yypParser,18,&yymsp[0].minor);
#line 90 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("request"); }
#line 1124 "critparser_impl.c"
}
        break;
      case 31: /* attr ::= COUNT */
{  yy_destructor(yypParser,3,&yymsp[0].minor);
#line 91 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("count"); }
#line 1131 "critparser_impl.c"
}
        break;
      case 32: /* attr ::= NOTUPTODATE */
{  yy_destructor(yypParser,6,&yymsp[0].minor);
#line 92 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("notuptodate"); }
#line 1138 "critparser_impl.c"
}
        break;
      case 33: /* attr ::= UNSAT_RECOMMENDS */
{  yy_destructor(yypParser,7,&yymsp[0].minor);
#line 93 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("unsat_recommends"); }
#line 1145 "critparser_impl.c"
}
        break;
      case 34: /* attr ::= SUM */
{  yy_destructor(yypParser,8,&yymsp[0].minor);
#line 94 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("sum"); }
#line 1152 "critparser_impl.c"
}
        break;
      case 35: /* attr ::= ALIGNED */
{  yy_destructor(yypParser,9,&yymsp[0].minor);
#line 95 "critparser_impl.y"
{ yymsp[0].minor.yy28 = &pParser->string("aligned"); }
#line 1159 "critparser_impl.c"
}
        break;
      case 38: /* ncrits ::= ncrits COMMA scrit */
#line 53 "critparser_impl.y"
{
}
#line 1166 "critparser_impl.c"
  yy_destructor(yypParser,1,&yymsp[-1].minor);
        break;
      default:
      /* (36) crits ::= ncrits */ yytestcase(yyruleno==36);
      /* (37) crits ::= */ yytestcase(yyruleno==37);
      /* (39) ncrits ::= scrit (OPTIMIZED OUT) */ assert(yyruleno!=39);
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[yysize].stateno,(YYCODETYPE)yygoto);

  /* There are no SHIFTREDUCE actions on nonterminals because the table
  ** generator has simplified them to pure REDUCE actions. */
  assert( !(yyact>YY_MAX_SHIFT && yyact<=YY_MAX_SHIFTREDUCE) );

  /* It is not possible for a REDUCE to be followed by an error */
  assert( yyact!=YY_ERROR_ACTION );

  if( yyact==YY_ACCEPT_ACTION ){
    yypParser->yytos += yysize;
    yy_accept(yypParser);
  }else{
    yymsp += yysize+1;
    yypParser->yytos = yymsp;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  critparserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
#line 36 "critparser_impl.y"
 pParser->parseError(); 
#line 1219 "critparser_impl.c"
/************ End %parse_failure code *****************************************/
  critparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  critparserTOKENTYPE yyminor         /* The minor type of the error token */
){
  critparserARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
#line 37 "critparser_impl.y"
 pParser->syntaxError(); 
#line 1238 "critparser_impl.c"
/************ End %syntax_error code ******************************************/
  critparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  critparserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  critparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "critparserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void critparser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  critparserTOKENTYPE yyminor       /* The value for the token */
  critparserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  unsigned int yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  yypParser = (yyParser*)yyp;
  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  critparserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= yypParser->yystack
            && yymx != YYERRORSYMBOL
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;

#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
