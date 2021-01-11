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
#line 26 "parser_impl.y"


#include <cassert>
#include "parser_impl.h"
#include "cudf/parser.hh"

#line 35 "parser_impl.c"
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
**    parserTOKENTYPE     is the data type used for minor type for terminal
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
**                       which is parserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    parserARG_SDECL     A static variable declaration for the %extra_argument
**    parserARG_PDECL     A parameter declaration for the %extra_argument
**    parserARG_STORE     Code to store %extra_argument into yypParser
**    parserARG_FETCH     Code to extract %extra_argument from yypParser
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
#define YYNOCODE 85
#define YYACTIONTYPE unsigned short int
#define parserTOKENTYPE  Parser::Token 
typedef union {
  int yyinit;
  parserTOKENTYPE yy0;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 0
#endif
#define parserARG_SDECL  Parser *pParser ;
#define parserARG_PDECL , Parser *pParser 
#define parserARG_FETCH  Parser *pParser  = yypParser->pParser 
#define parserARG_STORE yypParser->pParser  = pParser 
#define YYNSTATE             115
#define YYNRULE              111
#define YY_MAX_SHIFT         114
#define YY_MIN_SHIFTREDUCE   192
#define YY_MAX_SHIFTREDUCE   302
#define YY_MIN_REDUCE        303
#define YY_MAX_REDUCE        413
#define YY_ERROR_ACTION      414
#define YY_ACCEPT_ACTION     415
#define YY_NO_ACTION         416
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
#define YY_ACTTAB_COUNT (461)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   231,  236,  237,  232,  230,  282,   37,   28,   27,   30,
 /*    10 */    32,   45,   17,   12,    8,   13,    1,    3,    5,  212,
 /*    20 */   213,  214,  215,  216,  217,  218,  219,  220,  221,  222,
 /*    30 */   223,  224,  225,  226,  227,  234,  236,  237,  239,  114,
 /*    40 */    49,  415,   61,  248,  249,  102,  231,   96,  349,  232,
 /*    50 */   230,  104,  101,   98,   95,   92,   88,   85,   82,   79,
 /*    60 */    75,   69,   65,  108,   53,  212,  213,  214,  215,  216,
 /*    70 */   217,  218,  219,  220,  221,  222,  223,  224,  225,  226,
 /*    80 */   227,  234,  236,  237,  239,  231,  344,  309,  232,  230,
 /*    90 */   236,  237,  239,  313,  349,   86,  283,  339,  346,  340,
 /*   100 */   351,  349,  212,  213,  212,  213,  214,  215,  216,  217,
 /*   110 */   218,  219,  220,  221,  222,  223,  224,  225,  226,  227,
 /*   120 */    21,  236,  237,  239,  112,   51,  212,  213,  214,  215,
 /*   130 */   216,  217,  218,  219,  220,  221,  222,  223,  224,  225,
 /*   140 */   226,  227,  302,  236,  237,  239,   99,  351,  349,   22,
 /*   150 */   344,  312,  351,  349,  113,   38,  399,   43,   42,  315,
 /*   160 */   344,  339,  346,  340,  351,  349,   40,  236,  354,   54,
 /*   170 */   353,  339,  346,  340,  351,  349,  344,  355,  355,  319,
 /*   180 */   111,   93,   73,   72,  314,   54,  301,  339,  346,  340,
 /*   190 */   351,  349,  344,  355,  355,   70,   23,   26,   73,   72,
 /*   200 */    47,   54,  107,  339,  346,  340,  351,  349,   50,  361,
 /*   210 */   361,   57,  320,   52,   59,  344,   67,   33,   39,   56,
 /*   220 */    60,   41,   58,  281,   54,   14,  339,  346,  340,  351,
 /*   230 */   349,  344,  361,  361,  211,   66,    6,   64,  279,   67,
 /*   240 */    77,    9,  339,  346,  340,  351,  349,    4,  344,  363,
 /*   250 */    68,  277,  321,   10,   74,    2,   63,   77,   89,  339,
 /*   260 */   346,  340,  351,  349,    7,  344,  363,  275,   20,   62,
 /*   270 */    43,   15,   78,   63,   54,  273,  339,  346,  340,  351,
 /*   280 */   349,  344,  355,  355,   11,   81,  271,   71,   24,   84,
 /*   290 */    54,  269,  339,  346,  340,  351,  349,  344,  317,  317,
 /*   300 */    16,   87,   91,  267,   90,   44,   54,  344,  339,  346,
 /*   310 */   340,  351,  349,  265,  362,  362,   54,  344,  339,  346,
 /*   320 */   340,  351,  349,   94,  356,  356,   54,  263,  339,  346,
 /*   330 */   340,  351,  349,  344,   80,   80,   97,   31,  100,  261,
 /*   340 */    29,   35,  344,  259,  339,   36,  340,  351,  349,  316,
 /*   350 */   103,   48,  344,  339,  346,  340,  351,  349,   25,  109,
 /*   360 */   109,   77,  344,  339,  346,  340,  351,  349,  257,  106,
 /*   370 */   318,   77,  344,  339,  346,  340,  351,  349,   19,   34,
 /*   380 */   364,   77,   18,  339,  346,  340,  351,  349,  344,  397,
 /*   390 */    76,  305,  305,  305,  305,  305,  305,  344,  305,  339,
 /*   400 */   365,  340,  351,  349,  305,  305,  110,  304,  339,   36,
 /*   410 */   340,  351,  349,  305,  344,  305,   55,   46,  339,  344,
 /*   420 */   340,  351,  349,  305,  411,  339,   83,  340,  351,  349,
 /*   430 */   339,  105,  340,  351,  349,  344,  305,  305,  305,  305,
 /*   440 */   344,  305,  305,  305,  305,  344,  339,  366,  340,  351,
 /*   450 */   349,  339,  311,  340,  351,  349,  339,  310,  340,  351,
 /*   460 */   349,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     2,   38,   39,    5,    6,    1,    7,    8,    9,   10,
 /*    10 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   21,
 /*    20 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*    30 */    32,   33,   34,   35,   36,   37,   38,   39,   40,   55,
 /*    40 */    56,   57,   52,   45,   46,   65,    2,   68,   69,    5,
 /*    50 */     6,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*    60 */    33,   34,   35,   36,   83,   21,   22,   23,   24,   25,
 /*    70 */    26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*    80 */    36,   37,   38,   39,   40,    2,   54,   65,    5,    6,
 /*    90 */    38,   39,   40,   68,   69,   63,    1,   65,   66,   67,
 /*   100 */    68,   69,   21,   22,   21,   22,   23,   24,   25,   26,
 /*   110 */    27,   28,   29,   30,   31,   32,   33,   34,   35,   36,
 /*   120 */    61,   38,   39,   40,    5,    6,   21,   22,   23,   24,
 /*   130 */    25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*   140 */    35,   36,    3,   38,   39,   40,   67,   68,   69,   61,
 /*   150 */    54,   67,   68,   69,   60,   55,   62,   41,   42,   63,
 /*   160 */    54,   65,   66,   67,   68,   69,   55,   38,   69,   63,
 /*   170 */    69,   65,   66,   67,   68,   69,   54,   71,   72,   73,
 /*   180 */    55,   69,   76,   77,   69,   63,   47,   65,   66,   67,
 /*   190 */    68,   69,   54,   71,   72,   73,   61,   44,   76,   77,
 /*   200 */    55,   63,   49,   65,   66,   67,   68,   69,    2,   71,
 /*   210 */    72,   52,   74,   58,   52,   54,   78,   59,    4,    3,
 /*   220 */    20,    4,    3,   49,   63,   44,   65,   66,   67,   68,
 /*   230 */    69,   54,   71,   72,    4,   74,   48,   41,   49,   78,
 /*   240 */    63,   44,   65,   66,   67,   68,   69,   48,   54,   72,
 /*   250 */    41,   49,   75,   43,   41,   48,   79,   63,   50,   65,
 /*   260 */    66,   67,   68,   69,   44,   54,   72,   49,   44,   75,
 /*   270 */    41,   48,   41,   79,   63,   49,   65,   66,   67,   68,
 /*   280 */    69,   54,   71,   72,   48,   41,   49,   76,   48,   41,
 /*   290 */    63,   49,   65,   66,   67,   68,   69,   54,   71,   72,
 /*   300 */    48,   41,   41,   49,   48,   48,   63,   54,   65,   66,
 /*   310 */    67,   68,   69,   49,   71,   72,   63,   54,   65,   66,
 /*   320 */    67,   68,   69,   41,   71,   72,   63,   49,   65,   66,
 /*   330 */    67,   68,   69,   54,   71,   72,   41,   48,   41,   49,
 /*   340 */    48,   48,   54,   49,   65,   66,   67,   68,   69,   70,
 /*   350 */    41,   63,   54,   65,   66,   67,   68,   69,   48,   80,
 /*   360 */    81,   63,   54,   65,   66,   67,   68,   69,   49,   41,
 /*   370 */    72,   63,   54,   65,   66,   67,   68,   69,   48,    3,
 /*   380 */    72,   63,    3,   65,   66,   67,   68,   69,   54,    0,
 /*   390 */    72,   84,   84,   84,   84,   84,   84,   54,   84,   65,
 /*   400 */    66,   67,   68,   69,   84,   84,   53,   54,   65,   66,
 /*   410 */    67,   68,   69,   84,   54,   84,   82,   64,   65,   54,
 /*   420 */    67,   68,   69,   84,   81,   65,   66,   67,   68,   69,
 /*   430 */    65,   66,   67,   68,   69,   54,   84,   84,   84,   84,
 /*   440 */    54,   84,   84,   84,   84,   54,   65,   66,   67,   68,
 /*   450 */    69,   65,   66,   67,   68,   69,   65,   66,   67,   68,
 /*   460 */    69,
};
#define YY_SHIFT_USE_DFLT (461)
#define YY_SHIFT_COUNT    (114)
#define YY_SHIFT_MIN      (-37)
#define YY_SHIFT_MAX      (389)
static const short yy_shift_ofst[] = {
 /*     0 */     4,   -2,   -2,   44,   44,   44,   44,   44,   44,   44,
 /*    10 */    44,   44,   83,   44,   44,   44,   44,   44,   44,   83,
 /*    20 */    83,  105,  105,  105,   83,   83,   83,   83,   83,   52,
 /*    30 */    52,  -37,  -37,  119,   -1,   81,  139,   81,   95,    4,
 /*    40 */    95,    4,  129,  129,  129,  129,    4,   95,    4,  206,
 /*    50 */   461,  461,  461,   27,  116,  153,  214,  216,  217,  219,
 /*    60 */   230,  200,  174,  181,  188,  196,  189,  197,  199,  209,
 /*    70 */   202,  210,  220,  210,  207,  213,  218,  229,  223,  231,
 /*    80 */   226,  236,  244,  237,  240,  248,  242,  252,  260,  254,
 /*    90 */   208,  256,  261,  264,  257,  282,  278,  289,  295,  290,
 /*   100 */   292,  297,  294,  293,  309,  319,  310,  328,  330,  224,
 /*   110 */   376,   95,  379,  389,   95,
};
#define YY_REDUCE_USE_DFLT (-22)
#define YY_REDUCE_COUNT (52)
#define YY_REDUCE_MIN   (-21)
#define YY_REDUCE_MAX   (391)
static const short yy_reduce_ofst[] = {
 /*     0 */   -16,  106,  122,  138,  161,  177,  194,  211,  227,  243,
 /*    10 */   253,  263,  279,  298,  308,  318,   32,   96,  288,  334,
 /*    20 */   343,  353,  353,  353,  360,  365,  381,  386,  391,   79,
 /*    30 */    84,  -21,   25,   94,  -10,  -20,  -19,   22,   59,  100,
 /*    40 */    88,  111,   99,  101,  112,  115,  125,  135,  145,  155,
 /*    50 */   159,  162,  158,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   395,  414,  414,  404,  404,  406,  406,  414,  414,  414,
 /*    10 */   414,  414,  408,  414,  414,  414,  414,  414,  414,  414,
 /*    20 */   414,  305,  308,  307,  414,  414,  414,  414,  414,  414,
 /*    30 */   414,  414,  414,  414,  303,  414,  414,  414,  400,  414,
 /*    40 */   400,  414,  414,  414,  414,  414,  414,  400,  414,  306,
 /*    50 */   303,  303,  398,  414,  352,  414,  414,  414,  414,  414,
 /*    60 */   414,  414,  414,  407,  414,  391,  414,  405,  414,  389,
 /*    70 */   414,  358,  403,  357,  414,  387,  414,  352,  414,  385,
 /*    80 */   414,  414,  383,  414,  414,  381,  414,  414,  379,  414,
 /*    90 */   414,  414,  377,  414,  414,  375,  414,  414,  373,  414,
 /*   100 */   414,  371,  414,  414,  369,  414,  414,  367,  414,  409,
 /*   110 */   414,  401,  414,  414,  396,
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
  parserARG_SDECL                /* A place to hold %extra_argument */
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
void parserTrace(FILE *TraceFILE, char *zTracePrompt){
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
  "$",             "NL",            "PREAMBLE",      "COLONSP",     
  "STRING",        "PACKAGE",       "REQUEST",       "FEEDBACK_BOOL",
  "FEEDBACK_IDENT",  "FEEDBACK_ENUM",  "FEEDBACK_INT",  "FEEDBACK_NAT",
  "FEEDBACK_POSINT",  "FEEDBACK_PKGNAME",  "FEEDBACK_TYPEDECL",  "FEEDBACK_VPKG",
  "FEEDBACK_VEQPKG",  "FEEDBACK_VPKGFORMULA",  "FEEDBACK_VPKGLIST",  "FEEDBACK_VEQPKGLIST",
  "FEEDBACK_STRING",  "TRUE",          "FALSE",         "IDENT",       
  "TYPE_BOOL",     "TYPE_INT",      "TYPE_NAT",      "TYPE_POSINT", 
  "TYPE_STRING",   "TYPE_PKGNAME",  "TYPE_IDENT",    "TYPE_VPKG",   
  "TYPE_VEQPKG",   "TYPE_VPKGFORMULA",  "TYPE_VPKGLIST",  "TYPE_VEQPKGLIST",
  "TYPE_ENUM",     "PKGNAME",       "POSINT",        "NAT",         
  "INT",           "EQUAL",         "RELOP",         "BAR",         
  "COMMA",         "TRUEX",         "FALSEX",        "COLON",       
  "LBRAC",         "RBRAC",         "QUOTED",        "error",       
  "parse_string",  "parse_type",    "nonkey_ident",  "nnl",         
  "nl",            "cudf",          "preamble",      "universe",    
  "request",       "stanza",        "package",       "pkgname",     
  "property",      "bool",          "ident",         "int",         
  "nat",           "posint",        "typedecl",      "vpkg",        
  "veqpkg",        "vpkgformula",   "vpkglist",      "veqpkglist",  
  "orfla",         "andfla",        "nvpkglist",     "nveqpkglist", 
  "ntypedecl",     "typedecl1",     "identlist",     "colon",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "parse_string ::=",
 /*   1 */ "parse_type ::= nonkey_ident",
 /*   2 */ "preamble ::= PREAMBLE parse_string COLONSP STRING nnl stanza",
 /*   3 */ "preamble ::=",
 /*   4 */ "package ::= PACKAGE COLONSP pkgname nnl stanza",
 /*   5 */ "request ::= REQUEST parse_string COLONSP STRING nnl stanza",
 /*   6 */ "property ::= parse_type COLONSP FEEDBACK_BOOL bool",
 /*   7 */ "property ::= parse_type COLONSP FEEDBACK_IDENT ident",
 /*   8 */ "property ::= parse_type COLONSP FEEDBACK_ENUM ident",
 /*   9 */ "property ::= parse_type COLONSP FEEDBACK_INT int",
 /*  10 */ "property ::= parse_type COLONSP FEEDBACK_NAT nat",
 /*  11 */ "property ::= parse_type COLONSP FEEDBACK_POSINT posint",
 /*  12 */ "property ::= parse_type COLONSP FEEDBACK_PKGNAME pkgname",
 /*  13 */ "property ::= parse_type COLONSP FEEDBACK_TYPEDECL typedecl",
 /*  14 */ "property ::= parse_type COLONSP FEEDBACK_VPKG vpkg",
 /*  15 */ "property ::= parse_type COLONSP FEEDBACK_VEQPKG veqpkg",
 /*  16 */ "property ::= parse_type COLONSP FEEDBACK_VPKGFORMULA vpkgformula",
 /*  17 */ "property ::= parse_type COLONSP FEEDBACK_VPKGLIST vpkglist",
 /*  18 */ "property ::= parse_type COLONSP FEEDBACK_VEQPKGLIST veqpkglist",
 /*  19 */ "property ::= parse_type COLONSP parse_string FEEDBACK_STRING STRING",
 /*  20 */ "bool ::= TRUE",
 /*  21 */ "bool ::= FALSE",
 /*  22 */ "nonkey_ident ::= IDENT",
 /*  23 */ "nonkey_ident ::= TYPE_BOOL",
 /*  24 */ "nonkey_ident ::= TYPE_INT",
 /*  25 */ "nonkey_ident ::= TYPE_NAT",
 /*  26 */ "nonkey_ident ::= TYPE_POSINT",
 /*  27 */ "nonkey_ident ::= TYPE_STRING",
 /*  28 */ "nonkey_ident ::= TYPE_PKGNAME",
 /*  29 */ "nonkey_ident ::= TYPE_IDENT",
 /*  30 */ "nonkey_ident ::= TYPE_VPKG",
 /*  31 */ "nonkey_ident ::= TYPE_VEQPKG",
 /*  32 */ "nonkey_ident ::= TYPE_VPKGFORMULA",
 /*  33 */ "nonkey_ident ::= TYPE_VPKGLIST",
 /*  34 */ "nonkey_ident ::= TYPE_VEQPKGLIST",
 /*  35 */ "nonkey_ident ::= TYPE_ENUM",
 /*  36 */ "nonkey_ident ::= bool",
 /*  37 */ "nonkey_ident ::= int",
 /*  38 */ "ident ::= REQUEST",
 /*  39 */ "ident ::= PREAMBLE",
 /*  40 */ "ident ::= PACKAGE",
 /*  41 */ "ident ::= nonkey_ident",
 /*  42 */ "pkgname ::= PKGNAME",
 /*  43 */ "pkgname ::= ident",
 /*  44 */ "posint ::= POSINT",
 /*  45 */ "nat ::= NAT",
 /*  46 */ "nat ::= posint",
 /*  47 */ "int ::= INT",
 /*  48 */ "int ::= nat",
 /*  49 */ "veqpkg ::= pkgname",
 /*  50 */ "veqpkg ::= pkgname EQUAL posint",
 /*  51 */ "vpkg ::= pkgname RELOP posint",
 /*  52 */ "orfla ::= vpkg",
 /*  53 */ "orfla ::= orfla BAR vpkg",
 /*  54 */ "andfla ::= orfla",
 /*  55 */ "andfla ::= andfla COMMA orfla",
 /*  56 */ "vpkgformula ::= TRUEX",
 /*  57 */ "vpkgformula ::= FALSEX",
 /*  58 */ "nvpkglist ::= vpkg",
 /*  59 */ "nvpkglist ::= nvpkglist COMMA vpkg",
 /*  60 */ "nveqpkglist ::= veqpkg",
 /*  61 */ "nveqpkglist ::= nveqpkglist COMMA veqpkg",
 /*  62 */ "identlist ::= ident",
 /*  63 */ "identlist ::= identlist COMMA ident",
 /*  64 */ "typedecl1 ::= ident colon TYPE_ENUM LBRAC identlist RBRAC",
 /*  65 */ "typedecl1 ::= ident colon TYPE_ENUM LBRAC identlist RBRAC EQUAL LBRAC ident RBRAC",
 /*  66 */ "typedecl1 ::= ident colon TYPE_BOOL",
 /*  67 */ "typedecl1 ::= ident colon TYPE_BOOL EQUAL LBRAC bool RBRAC",
 /*  68 */ "typedecl1 ::= ident colon TYPE_INT",
 /*  69 */ "typedecl1 ::= ident colon TYPE_INT EQUAL LBRAC int RBRAC",
 /*  70 */ "typedecl1 ::= ident colon TYPE_NAT",
 /*  71 */ "typedecl1 ::= ident colon TYPE_NAT EQUAL LBRAC nat RBRAC",
 /*  72 */ "typedecl1 ::= ident colon TYPE_POSINT",
 /*  73 */ "typedecl1 ::= ident colon TYPE_POSINT EQUAL LBRAC posint RBRAC",
 /*  74 */ "typedecl1 ::= ident colon TYPE_STRING",
 /*  75 */ "typedecl1 ::= ident colon TYPE_STRING EQUAL LBRAC QUOTED RBRAC",
 /*  76 */ "typedecl1 ::= ident colon TYPE_PKGNAME",
 /*  77 */ "typedecl1 ::= ident colon TYPE_PKGNAME EQUAL LBRAC pkgname RBRAC",
 /*  78 */ "typedecl1 ::= ident colon TYPE_IDENT",
 /*  79 */ "typedecl1 ::= ident colon TYPE_IDENT EQUAL LBRAC ident RBRAC",
 /*  80 */ "typedecl1 ::= ident colon TYPE_VPKG",
 /*  81 */ "typedecl1 ::= ident colon TYPE_VPKG EQUAL LBRAC vpkg RBRAC",
 /*  82 */ "typedecl1 ::= ident colon TYPE_VEQPKG",
 /*  83 */ "typedecl1 ::= ident colon TYPE_VEQPKG EQUAL LBRAC veqpkg RBRAC",
 /*  84 */ "typedecl1 ::= ident colon TYPE_VPKGFORMULA",
 /*  85 */ "typedecl1 ::= ident colon TYPE_VPKGFORMULA EQUAL LBRAC vpkgformula RBRAC",
 /*  86 */ "typedecl1 ::= ident colon TYPE_VPKGLIST",
 /*  87 */ "typedecl1 ::= ident colon TYPE_VPKGLIST EQUAL LBRAC vpkglist RBRAC",
 /*  88 */ "typedecl1 ::= ident colon TYPE_VEQPKGLIST",
 /*  89 */ "typedecl1 ::= ident colon TYPE_VEQPKGLIST EQUAL LBRAC veqpkglist RBRAC",
 /*  90 */ "nnl ::= NL",
 /*  91 */ "nnl ::= nnl NL",
 /*  92 */ "nl ::=",
 /*  93 */ "nl ::= nnl",
 /*  94 */ "cudf ::= nl preamble universe request",
 /*  95 */ "universe ::=",
 /*  96 */ "universe ::= universe package",
 /*  97 */ "stanza ::=",
 /*  98 */ "stanza ::= stanza property nnl",
 /*  99 */ "vpkg ::= veqpkg",
 /* 100 */ "vpkgformula ::= andfla",
 /* 101 */ "vpkglist ::=",
 /* 102 */ "vpkglist ::= nvpkglist",
 /* 103 */ "veqpkglist ::=",
 /* 104 */ "veqpkglist ::= nveqpkglist",
 /* 105 */ "typedecl ::=",
 /* 106 */ "typedecl ::= ntypedecl",
 /* 107 */ "ntypedecl ::= typedecl1",
 /* 108 */ "ntypedecl ::= ntypedecl COMMA typedecl1",
 /* 109 */ "colon ::= COLON",
 /* 110 */ "colon ::= COLONSP",
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
** second argument to parserAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* Initialize a new parser that has already been allocated.
*/
void parserInit(void *yypParser){
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

#ifndef parser_ENGINEALWAYSONSTACK
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
** to parser and parserFree.
*/
void *parserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ) parserInit(pParser);
  return pParser;
}
#endif /* parser_ENGINEALWAYSONSTACK */


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
  parserARG_FETCH;
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
    case 1: /* NL */
    case 2: /* PREAMBLE */
    case 3: /* COLONSP */
    case 4: /* STRING */
    case 5: /* PACKAGE */
    case 6: /* REQUEST */
    case 7: /* FEEDBACK_BOOL */
    case 8: /* FEEDBACK_IDENT */
    case 9: /* FEEDBACK_ENUM */
    case 10: /* FEEDBACK_INT */
    case 11: /* FEEDBACK_NAT */
    case 12: /* FEEDBACK_POSINT */
    case 13: /* FEEDBACK_PKGNAME */
    case 14: /* FEEDBACK_TYPEDECL */
    case 15: /* FEEDBACK_VPKG */
    case 16: /* FEEDBACK_VEQPKG */
    case 17: /* FEEDBACK_VPKGFORMULA */
    case 18: /* FEEDBACK_VPKGLIST */
    case 19: /* FEEDBACK_VEQPKGLIST */
    case 20: /* FEEDBACK_STRING */
    case 21: /* TRUE */
    case 22: /* FALSE */
    case 23: /* IDENT */
    case 24: /* TYPE_BOOL */
    case 25: /* TYPE_INT */
    case 26: /* TYPE_NAT */
    case 27: /* TYPE_POSINT */
    case 28: /* TYPE_STRING */
    case 29: /* TYPE_PKGNAME */
    case 30: /* TYPE_IDENT */
    case 31: /* TYPE_VPKG */
    case 32: /* TYPE_VEQPKG */
    case 33: /* TYPE_VPKGFORMULA */
    case 34: /* TYPE_VPKGLIST */
    case 35: /* TYPE_VEQPKGLIST */
    case 36: /* TYPE_ENUM */
    case 37: /* PKGNAME */
    case 38: /* POSINT */
    case 39: /* NAT */
    case 40: /* INT */
    case 41: /* EQUAL */
    case 42: /* RELOP */
    case 43: /* BAR */
    case 44: /* COMMA */
    case 45: /* TRUEX */
    case 46: /* FALSEX */
    case 47: /* COLON */
    case 48: /* LBRAC */
    case 49: /* RBRAC */
    case 50: /* QUOTED */
{
#line 40 "parser_impl.y"
 (void)pParser; (void)(yypminor->yy0); 
#line 750 "parser_impl.c"
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
void parserFinalize(void *p){
  yyParser *pParser = (yyParser*)p;
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
}

#ifndef parser_ENGINEALWAYSONSTACK
/*
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void parserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
#ifndef YYPARSEFREENEVERNULL
  if( p==0 ) return;
#endif
  parserFinalize(p);
  (*freeProc)(p);
}
#endif /* parser_ENGINEALWAYSONSTACK */

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int parserStackPeak(void *p){
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
   parserARG_FETCH;
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
   parserARG_STORE; /* Suppress warning about unused %extra_argument var */
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
  parserTOKENTYPE yyMinor        /* The minor token to shift in */
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
  { 52, 0 },
  { 53, -1 },
  { 58, -6 },
  { 58, 0 },
  { 62, -5 },
  { 60, -6 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -4 },
  { 64, -5 },
  { 65, -1 },
  { 65, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 54, -1 },
  { 66, -1 },
  { 66, -1 },
  { 66, -1 },
  { 66, -1 },
  { 63, -1 },
  { 63, -1 },
  { 69, -1 },
  { 68, -1 },
  { 68, -1 },
  { 67, -1 },
  { 67, -1 },
  { 72, -1 },
  { 72, -3 },
  { 71, -3 },
  { 76, -1 },
  { 76, -3 },
  { 77, -1 },
  { 77, -3 },
  { 73, -1 },
  { 73, -1 },
  { 78, -1 },
  { 78, -3 },
  { 79, -1 },
  { 79, -3 },
  { 82, -1 },
  { 82, -3 },
  { 81, -6 },
  { 81, -10 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 81, -3 },
  { 81, -7 },
  { 55, -1 },
  { 55, -2 },
  { 56, 0 },
  { 56, -1 },
  { 57, -4 },
  { 59, 0 },
  { 59, -2 },
  { 61, 0 },
  { 61, -3 },
  { 71, -1 },
  { 73, -1 },
  { 74, 0 },
  { 74, -1 },
  { 75, 0 },
  { 75, -1 },
  { 70, 0 },
  { 70, -1 },
  { 80, -1 },
  { 80, -3 },
  { 83, -1 },
  { 83, -1 },
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
  parserARG_FETCH;
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
      case 0: /* parse_string ::= */
#line 47 "parser_impl.y"
{ pParser->parseString(); }
#line 1179 "parser_impl.c"
        break;
      case 1: /* parse_type ::= nonkey_ident */
#line 48 "parser_impl.y"
{ yylhsminor.yy0.index = yymsp[0].minor.yy0.index; pParser->parseType(yymsp[0].minor.yy0.index); }
#line 1184 "parser_impl.c"
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 2: /* preamble ::= PREAMBLE parse_string COLONSP STRING nnl stanza */
{  yy_destructor(yypParser,2,&yymsp[-5].minor);
#line 61 "parser_impl.y"
{ pParser->addPreamble(); }
#line 1191 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-3].minor);
  yy_destructor(yypParser,4,&yymsp[-2].minor);
}
        break;
      case 3: /* preamble ::= */
#line 62 "parser_impl.y"
{ pParser->addPreamble(); }
#line 1199 "parser_impl.c"
        break;
      case 4: /* package ::= PACKAGE COLONSP pkgname nnl stanza */
{  yy_destructor(yypParser,5,&yymsp[-4].minor);
#line 65 "parser_impl.y"
{ pParser->addPackage(yymsp[-2].minor.yy0.index); }
#line 1205 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-3].minor);
}
        break;
      case 5: /* request ::= REQUEST parse_string COLONSP STRING nnl stanza */
{  yy_destructor(yypParser,6,&yymsp[-5].minor);
#line 66 "parser_impl.y"
{ pParser->addRequest(); }
#line 1213 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-3].minor);
  yy_destructor(yypParser,4,&yymsp[-2].minor);
}
        break;
      case 6: /* property ::= parse_type COLONSP FEEDBACK_BOOL bool */
#line 71 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, pParser->mapBool(yymsp[0].minor.yy0.index)); }
#line 1221 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[-1].minor);
        break;
      case 7: /* property ::= parse_type COLONSP FEEDBACK_IDENT ident */
#line 72 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, uint32_t(yymsp[0].minor.yy0.index)); }
#line 1228 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,8,&yymsp[-1].minor);
        break;
      case 8: /* property ::= parse_type COLONSP FEEDBACK_ENUM ident */
#line 73 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, uint32_t(yymsp[0].minor.yy0.index)); }
#line 1235 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,9,&yymsp[-1].minor);
        break;
      case 9: /* property ::= parse_type COLONSP FEEDBACK_INT int */
#line 74 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, pParser->mapInt(yymsp[0].minor.yy0.index)); }
#line 1242 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,10,&yymsp[-1].minor);
        break;
      case 10: /* property ::= parse_type COLONSP FEEDBACK_NAT nat */
#line 75 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, pParser->mapInt(yymsp[0].minor.yy0.index)); }
#line 1249 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
        break;
      case 11: /* property ::= parse_type COLONSP FEEDBACK_POSINT posint */
#line 76 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, pParser->mapInt(yymsp[0].minor.yy0.index)); }
#line 1256 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,12,&yymsp[-1].minor);
        break;
      case 12: /* property ::= parse_type COLONSP FEEDBACK_PKGNAME pkgname */
#line 77 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, uint32_t(yymsp[0].minor.yy0.index)); }
#line 1263 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,13,&yymsp[-1].minor);
        break;
      case 13: /* property ::= parse_type COLONSP FEEDBACK_TYPEDECL typedecl */
#line 78 "parser_impl.y"
{ /* ignore: yymsp[-3].minor.yy0, yymsp[0].minor.yy0 */ }
#line 1270 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
        break;
      case 14: /* property ::= parse_type COLONSP FEEDBACK_VPKG vpkg */
#line 79 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, std::move(pParser->pkgRef)); }
#line 1277 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
        break;
      case 15: /* property ::= parse_type COLONSP FEEDBACK_VEQPKG veqpkg */
#line 80 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, std::move(pParser->pkgRef)); }
#line 1284 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,16,&yymsp[-1].minor);
        break;
      case 16: /* property ::= parse_type COLONSP FEEDBACK_VPKGFORMULA vpkgformula */
#line 81 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, std::move(pParser->pkgFormula)); }
#line 1291 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,17,&yymsp[-1].minor);
        break;
      case 17: /* property ::= parse_type COLONSP FEEDBACK_VPKGLIST vpkglist */
#line 82 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, std::move(pParser->pkgList)); }
#line 1298 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
        break;
      case 18: /* property ::= parse_type COLONSP FEEDBACK_VEQPKGLIST veqpkglist */
#line 83 "parser_impl.y"
{ pParser->setProperty(yymsp[-3].minor.yy0.index, std::move(pParser->pkgList)); }
#line 1305 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,19,&yymsp[-1].minor);
        break;
      case 19: /* property ::= parse_type COLONSP parse_string FEEDBACK_STRING STRING */
#line 84 "parser_impl.y"
{ pParser->setProperty(yymsp[-4].minor.yy0.index, uint32_t(yymsp[0].minor.yy0.index)); }
#line 1312 "parser_impl.c"
  yy_destructor(yypParser,3,&yymsp[-3].minor);
  yy_destructor(yypParser,20,&yymsp[-1].minor);
        break;
      case 20: /* bool ::= TRUE */
      case 21: /* bool ::= FALSE */ yytestcase(yyruleno==21);
      case 22: /* nonkey_ident ::= IDENT */ yytestcase(yyruleno==22);
      case 23: /* nonkey_ident ::= TYPE_BOOL */ yytestcase(yyruleno==23);
      case 24: /* nonkey_ident ::= TYPE_INT */ yytestcase(yyruleno==24);
      case 25: /* nonkey_ident ::= TYPE_NAT */ yytestcase(yyruleno==25);
      case 26: /* nonkey_ident ::= TYPE_POSINT */ yytestcase(yyruleno==26);
      case 27: /* nonkey_ident ::= TYPE_STRING */ yytestcase(yyruleno==27);
      case 28: /* nonkey_ident ::= TYPE_PKGNAME */ yytestcase(yyruleno==28);
      case 29: /* nonkey_ident ::= TYPE_IDENT */ yytestcase(yyruleno==29);
      case 30: /* nonkey_ident ::= TYPE_VPKG */ yytestcase(yyruleno==30);
      case 31: /* nonkey_ident ::= TYPE_VEQPKG */ yytestcase(yyruleno==31);
      case 32: /* nonkey_ident ::= TYPE_VPKGFORMULA */ yytestcase(yyruleno==32);
      case 33: /* nonkey_ident ::= TYPE_VPKGLIST */ yytestcase(yyruleno==33);
      case 34: /* nonkey_ident ::= TYPE_VEQPKGLIST */ yytestcase(yyruleno==34);
      case 35: /* nonkey_ident ::= TYPE_ENUM */ yytestcase(yyruleno==35);
      case 36: /* nonkey_ident ::= bool */ yytestcase(yyruleno==36);
      case 37: /* nonkey_ident ::= int */ yytestcase(yyruleno==37);
      case 38: /* ident ::= REQUEST */ yytestcase(yyruleno==38);
      case 39: /* ident ::= PREAMBLE */ yytestcase(yyruleno==39);
      case 40: /* ident ::= PACKAGE */ yytestcase(yyruleno==40);
      case 41: /* ident ::= nonkey_ident */ yytestcase(yyruleno==41);
      case 42: /* pkgname ::= PKGNAME */ yytestcase(yyruleno==42);
      case 43: /* pkgname ::= ident */ yytestcase(yyruleno==43);
      case 44: /* posint ::= POSINT */ yytestcase(yyruleno==44);
      case 45: /* nat ::= NAT */ yytestcase(yyruleno==45);
      case 46: /* nat ::= posint */ yytestcase(yyruleno==46);
      case 47: /* int ::= INT */ yytestcase(yyruleno==47);
      case 48: /* int ::= nat */ yytestcase(yyruleno==48);
#line 87 "parser_impl.y"
{ yylhsminor.yy0.index = yymsp[0].minor.yy0.index; }
#line 1347 "parser_impl.c"
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 49: /* veqpkg ::= pkgname */
#line 124 "parser_impl.y"
{ pParser->setPkgRef(yymsp[0].minor.yy0.index); }
#line 1353 "parser_impl.c"
        break;
      case 50: /* veqpkg ::= pkgname EQUAL posint */
      case 51: /* vpkg ::= pkgname RELOP posint */ yytestcase(yyruleno==51);
#line 125 "parser_impl.y"
{ pParser->setPkgRef(yymsp[-2].minor.yy0.index, yymsp[-1].minor.yy0.index, yymsp[0].minor.yy0.index); }
#line 1359 "parser_impl.c"
        break;
      case 52: /* orfla ::= vpkg */
      case 58: /* nvpkglist ::= vpkg */ yytestcase(yyruleno==58);
      case 60: /* nveqpkglist ::= veqpkg */ yytestcase(yyruleno==60);
#line 130 "parser_impl.y"
{ pParser->pkgList.clear(); pParser->pkgList.push_back(pParser->pkgRef); }
#line 1366 "parser_impl.c"
        break;
      case 53: /* orfla ::= orfla BAR vpkg */
#line 131 "parser_impl.y"
{ pParser->pkgList.push_back(pParser->pkgRef); }
#line 1371 "parser_impl.c"
  yy_destructor(yypParser,43,&yymsp[-1].minor);
        break;
      case 54: /* andfla ::= orfla */
#line 133 "parser_impl.y"
{ pParser->pkgFormula.clear(); pParser->pushPkgList(); }
#line 1377 "parser_impl.c"
        break;
      case 55: /* andfla ::= andfla COMMA orfla */
#line 134 "parser_impl.y"
{ pParser->pushPkgList(); }
#line 1382 "parser_impl.c"
  yy_destructor(yypParser,44,&yymsp[-1].minor);
        break;
      case 56: /* vpkgformula ::= TRUEX */
{  yy_destructor(yypParser,45,&yymsp[0].minor);
#line 137 "parser_impl.y"
{ pParser->pkgFormula.clear(); }
#line 1389 "parser_impl.c"
}
        break;
      case 57: /* vpkgformula ::= FALSEX */
{  yy_destructor(yypParser,46,&yymsp[0].minor);
#line 138 "parser_impl.y"
{ pParser->pkgFormula.clear(); pParser->pkgList.clear(); pParser->pushPkgList(); }
#line 1396 "parser_impl.c"
}
        break;
      case 59: /* nvpkglist ::= nvpkglist COMMA vpkg */
      case 61: /* nveqpkglist ::= nveqpkglist COMMA veqpkg */ yytestcase(yyruleno==61);
#line 143 "parser_impl.y"
{ pParser->pkgList.push_back(pParser->pkgRef); }
#line 1403 "parser_impl.c"
  yy_destructor(yypParser,44,&yymsp[-1].minor);
        break;
      case 62: /* identlist ::= ident */
#line 156 "parser_impl.y"
{ pParser->identList.clear(); pParser->identList.push_back(yymsp[0].minor.yy0.index); }
#line 1409 "parser_impl.c"
        break;
      case 63: /* identlist ::= identlist COMMA ident */
#line 157 "parser_impl.y"
{ pParser->identList.push_back(yymsp[0].minor.yy0.index); }
#line 1414 "parser_impl.c"
  yy_destructor(yypParser,44,&yymsp[-1].minor);
        break;
      case 64: /* typedecl1 ::= ident colon TYPE_ENUM LBRAC identlist RBRAC */
#line 162 "parser_impl.y"
{ pParser->addType(yymsp[-5].minor.yy0.index, PARSER_FEEDBACK_ENUM); }
#line 1420 "parser_impl.c"
  yy_destructor(yypParser,36,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 65: /* typedecl1 ::= ident colon TYPE_ENUM LBRAC identlist RBRAC EQUAL LBRAC ident RBRAC */
#line 163 "parser_impl.y"
{ pParser->addType(yymsp[-9].minor.yy0.index, PARSER_FEEDBACK_ENUM) = uint32_t(yymsp[-1].minor.yy0.index); }
#line 1428 "parser_impl.c"
  yy_destructor(yypParser,36,&yymsp[-7].minor);
  yy_destructor(yypParser,48,&yymsp[-6].minor);
  yy_destructor(yypParser,49,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 66: /* typedecl1 ::= ident colon TYPE_BOOL */
#line 164 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_BOOL); }
#line 1439 "parser_impl.c"
  yy_destructor(yypParser,24,&yymsp[0].minor);
        break;
      case 67: /* typedecl1 ::= ident colon TYPE_BOOL EQUAL LBRAC bool RBRAC */
#line 165 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_BOOL) = pParser->mapBool(yymsp[-1].minor.yy0.index); }
#line 1445 "parser_impl.c"
  yy_destructor(yypParser,24,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 68: /* typedecl1 ::= ident colon TYPE_INT */
#line 166 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_INT); }
#line 1454 "parser_impl.c"
  yy_destructor(yypParser,25,&yymsp[0].minor);
        break;
      case 69: /* typedecl1 ::= ident colon TYPE_INT EQUAL LBRAC int RBRAC */
#line 167 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_INT) = pParser->mapInt(yymsp[-1].minor.yy0.index); }
#line 1460 "parser_impl.c"
  yy_destructor(yypParser,25,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 70: /* typedecl1 ::= ident colon TYPE_NAT */
#line 168 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_NAT); }
#line 1469 "parser_impl.c"
  yy_destructor(yypParser,26,&yymsp[0].minor);
        break;
      case 71: /* typedecl1 ::= ident colon TYPE_NAT EQUAL LBRAC nat RBRAC */
#line 169 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_NAT) = pParser->mapInt(yymsp[-1].minor.yy0.index); }
#line 1475 "parser_impl.c"
  yy_destructor(yypParser,26,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 72: /* typedecl1 ::= ident colon TYPE_POSINT */
#line 170 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_POSINT); }
#line 1484 "parser_impl.c"
  yy_destructor(yypParser,27,&yymsp[0].minor);
        break;
      case 73: /* typedecl1 ::= ident colon TYPE_POSINT EQUAL LBRAC posint RBRAC */
#line 171 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_POSINT) = pParser->mapInt(yymsp[-1].minor.yy0.index); }
#line 1490 "parser_impl.c"
  yy_destructor(yypParser,27,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 74: /* typedecl1 ::= ident colon TYPE_STRING */
#line 172 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_STRING); }
#line 1499 "parser_impl.c"
  yy_destructor(yypParser,28,&yymsp[0].minor);
        break;
      case 75: /* typedecl1 ::= ident colon TYPE_STRING EQUAL LBRAC QUOTED RBRAC */
#line 173 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_STRING) = uint32_t(yymsp[-1].minor.yy0.index); }
#line 1505 "parser_impl.c"
  yy_destructor(yypParser,28,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 76: /* typedecl1 ::= ident colon TYPE_PKGNAME */
#line 174 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_PKGNAME); }
#line 1514 "parser_impl.c"
  yy_destructor(yypParser,29,&yymsp[0].minor);
        break;
      case 77: /* typedecl1 ::= ident colon TYPE_PKGNAME EQUAL LBRAC pkgname RBRAC */
#line 175 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_PKGNAME) = uint32_t(yymsp[-1].minor.yy0.index); }
#line 1520 "parser_impl.c"
  yy_destructor(yypParser,29,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 78: /* typedecl1 ::= ident colon TYPE_IDENT */
#line 176 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_IDENT); }
#line 1529 "parser_impl.c"
  yy_destructor(yypParser,30,&yymsp[0].minor);
        break;
      case 79: /* typedecl1 ::= ident colon TYPE_IDENT EQUAL LBRAC ident RBRAC */
#line 177 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_IDENT) = uint32_t(yymsp[-1].minor.yy0.index); }
#line 1535 "parser_impl.c"
  yy_destructor(yypParser,30,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 80: /* typedecl1 ::= ident colon TYPE_VPKG */
#line 178 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_VPKG); }
#line 1544 "parser_impl.c"
  yy_destructor(yypParser,31,&yymsp[0].minor);
        break;
      case 81: /* typedecl1 ::= ident colon TYPE_VPKG EQUAL LBRAC vpkg RBRAC */
#line 179 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_VPKG) = pParser->pkgRef; }
#line 1550 "parser_impl.c"
  yy_destructor(yypParser,31,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 82: /* typedecl1 ::= ident colon TYPE_VEQPKG */
#line 180 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_VEQPKG); }
#line 1559 "parser_impl.c"
  yy_destructor(yypParser,32,&yymsp[0].minor);
        break;
      case 83: /* typedecl1 ::= ident colon TYPE_VEQPKG EQUAL LBRAC veqpkg RBRAC */
#line 181 "parser_impl.y"
{ pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_VEQPKG) = pParser->pkgRef; }
#line 1565 "parser_impl.c"
  yy_destructor(yypParser,32,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 84: /* typedecl1 ::= ident colon TYPE_VPKGFORMULA */
#line 182 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_VPKGFORMULA); }
#line 1574 "parser_impl.c"
  yy_destructor(yypParser,33,&yymsp[0].minor);
        break;
      case 85: /* typedecl1 ::= ident colon TYPE_VPKGFORMULA EQUAL LBRAC vpkgformula RBRAC */
#line 183 "parser_impl.y"
{ Cudf::Value &val = pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_VPKGFORMULA) = Cudf::PkgFormula(); std::swap(pParser->pkgFormula, boost::any_cast<Cudf::PkgFormula&>(val)); }
#line 1580 "parser_impl.c"
  yy_destructor(yypParser,33,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 86: /* typedecl1 ::= ident colon TYPE_VPKGLIST */
#line 184 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_VPKGLIST); }
#line 1589 "parser_impl.c"
  yy_destructor(yypParser,34,&yymsp[0].minor);
        break;
      case 87: /* typedecl1 ::= ident colon TYPE_VPKGLIST EQUAL LBRAC vpkglist RBRAC */
#line 185 "parser_impl.y"
{ Cudf::Value &val = pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_VPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
#line 1595 "parser_impl.c"
  yy_destructor(yypParser,34,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 88: /* typedecl1 ::= ident colon TYPE_VEQPKGLIST */
#line 186 "parser_impl.y"
{ pParser->addType(yymsp[-2].minor.yy0.index, PARSER_FEEDBACK_VEQPKGLIST); }
#line 1604 "parser_impl.c"
  yy_destructor(yypParser,35,&yymsp[0].minor);
        break;
      case 89: /* typedecl1 ::= ident colon TYPE_VEQPKGLIST EQUAL LBRAC veqpkglist RBRAC */
#line 187 "parser_impl.y"
{ Cudf::Value &val = pParser->addType(yymsp[-6].minor.yy0.index, PARSER_FEEDBACK_VEQPKGLIST) = Cudf::PkgList(); std::swap(pParser->pkgList, boost::any_cast<Cudf::PkgList&>(val)); }
#line 1610 "parser_impl.c"
  yy_destructor(yypParser,35,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
        break;
      case 90: /* nnl ::= NL */
{  yy_destructor(yypParser,1,&yymsp[0].minor);
#line 51 "parser_impl.y"
{
}
#line 1621 "parser_impl.c"
}
        break;
      case 91: /* nnl ::= nnl NL */
#line 52 "parser_impl.y"
{
}
#line 1628 "parser_impl.c"
  yy_destructor(yypParser,1,&yymsp[0].minor);
        break;
      case 108: /* ntypedecl ::= ntypedecl COMMA typedecl1 */
#line 154 "parser_impl.y"
{
}
#line 1635 "parser_impl.c"
  yy_destructor(yypParser,44,&yymsp[-1].minor);
        break;
      case 109: /* colon ::= COLON */
{  yy_destructor(yypParser,47,&yymsp[0].minor);
#line 159 "parser_impl.y"
{
}
#line 1643 "parser_impl.c"
}
        break;
      case 110: /* colon ::= COLONSP */
{  yy_destructor(yypParser,3,&yymsp[0].minor);
#line 160 "parser_impl.y"
{
}
#line 1651 "parser_impl.c"
}
        break;
      default:
      /* (92) nl ::= */ yytestcase(yyruleno==92);
      /* (93) nl ::= nnl */ yytestcase(yyruleno==93);
      /* (94) cudf ::= nl preamble universe request */ yytestcase(yyruleno==94);
      /* (95) universe ::= */ yytestcase(yyruleno==95);
      /* (96) universe ::= universe package */ yytestcase(yyruleno==96);
      /* (97) stanza ::= */ yytestcase(yyruleno==97);
      /* (98) stanza ::= stanza property nnl */ yytestcase(yyruleno==98);
      /* (99) vpkg ::= veqpkg (OPTIMIZED OUT) */ assert(yyruleno!=99);
      /* (100) vpkgformula ::= andfla */ yytestcase(yyruleno==100);
      /* (101) vpkglist ::= */ yytestcase(yyruleno==101);
      /* (102) vpkglist ::= nvpkglist */ yytestcase(yyruleno==102);
      /* (103) veqpkglist ::= */ yytestcase(yyruleno==103);
      /* (104) veqpkglist ::= nveqpkglist */ yytestcase(yyruleno==104);
      /* (105) typedecl ::= */ yytestcase(yyruleno==105);
      /* (106) typedecl ::= ntypedecl */ yytestcase(yyruleno==106);
      /* (107) ntypedecl ::= typedecl1 (OPTIMIZED OUT) */ assert(yyruleno!=107);
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
  parserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
#line 36 "parser_impl.y"
 pParser->parseError(); 
#line 1717 "parser_impl.c"
/************ End %parse_failure code *****************************************/
  parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  parserTOKENTYPE yyminor         /* The minor type of the error token */
){
  parserARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
#line 37 "parser_impl.y"
 pParser->syntaxError(); 
#line 1736 "parser_impl.c"
/************ End %syntax_error code ******************************************/
  parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  parserARG_FETCH;
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
  parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "parserAlloc" which describes the current state of the parser.
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
void parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  parserTOKENTYPE yyminor       /* The value for the token */
  parserARG_PDECL               /* Optional %extra_argument parameter */
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
  parserARG_STORE;

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
