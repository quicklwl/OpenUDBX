/*
This library will provide common mathematical and string functions in
SQL queries using the operating system libraries or provided
definitions.  It includes the following functions:

Math: acos, asin, atan, atn2, atan2, acosh, asinh, atanh, difference,
degrees, radians, cos, sin, tan, cot, cosh, sinh, tanh, coth, exp,
log, log10, power, sign, sqrt, square, ceil, floor, pi.

String: replicate, charindex, leftstr, rightstr, ltrim, rtrim, trim,
replace, reverse, proper, padl, padr, padc, strfilter.

Aggregate: stdev, variance, mode, median, lower_quartile,
upper_quartile.

The string functions ltrim, rtrim, trim, replace are included in
recent versions of SQLite and so by default do not build.

Compilation instructions:
 Compile this C source file into a dynamic library as follows:
 * Linux:
   gcc -fPIC -lm -shared extension-functions.c -o libsqlitefunctions.so
 * Mac OS X:
   gcc -fno-common -dynamiclib extension-functions.c -o libsqlitefunctions.dylib
 (You may need to add flags
  -I /opt/local/include/ -L/opt/local/lib -lsqlite3
  if your sqlite3 is installed from Mac ports, or
  -I /sw/include/ -L/sw/lib -lsqlite3
  if installed with Fink.)
 * Windows:
  1. Install MinGW (http://www.mingw.org/) and you will get the gcc
  (gnu compiler collection)
  2. add the path to your path variable (isn't done during the
   installation!)
  3. compile:
   gcc -shared -I "path" -o libsqlitefunctions.so extension-functions.c
   (path = path of sqlite3ext.h; i.e. C:\programs\sqlite)

Usage instructions for applications calling the sqlite3 API functions:
  In your application, call sqlite3_enable_load_extension(db,1) to
  allow loading external libraries.  Then load the library libsqlitefunctions
  using sqlite3_load_extension; the third argument should be 0.
  See http://www.sqlite.org/cvstrac/wiki?p=LoadableExtensions.
  Select statements may now use these functions, as in
  SELECT cos(radians(inclination)) FROM satsum WHERE satnum = 25544;

Usage instructions for the sqlite3 program:,
  If the program is built so that loading extensions is permitted,
  the following will work:
   sqlite> SELECT load_extension('./libsqlitefunctions.so');
   sqlite> select cos(radians(45));
   0.707106781186548
  Note: Loading extensions is by default prohibited as a
  security measure; see "Security Considerations" in
  http://www.sqlite.org/cvstrac/wiki?p=LoadableExtensions.
  If the sqlite3 program and library are built this
  way, you cannot use these functions from the program, you 
  must write your own program using the sqlite3 API, and call
  sqlite3_enable_load_extension as described above, or else
  rebuilt the sqlite3 program to allow loadable extensions.

Alterations:
The instructions are for Linux, Mac OS X, and Windows; users of other
OSes may need to modify this procedure.  In particular, if your math
library lacks one or more of the needed trig or log functions, comment
out the appropriate HAVE_ #define at the top of file.  If you do not
wish to make a loadable module, comment out the define for
COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE. 

Liam Healy

History:
2010-01-06 Correct check for argc in squareFunc, and add Windows
compilation instructions.
2009-06-24 Correct check for argc in properFunc.
2008-09-14 Add check that memory was actually allocated after
sqlite3_malloc or sqlite3StrDup, call sqlite3_result_error_nomem if
not.  Thanks to Robert Simpson.
2008-06-13 Change to instructions to indicate use of the math library
and that program might work.
2007-10-01 Minor clarification to instructions.
2007-09-29 Compilation as loadable module is optional with
COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE.
2007-09-17 With help from Joe Wilson and Nuno Luca, made use of
external interfaces so that compilation is no longer dependent on
SQLite source code.  Merged source, header, and README into a single
file.  Added casts so that Mac will compile without warnings (unsigned
and signed char).
2007-09-05 Included some definitions from sqlite 3.3.13 so that this
will continue to work in newer versions of sqlite.  Completed
description of functions available.
2007-03-27 Revised description.
2007-03-23 Small cleanup and a bug fix on the code.  This was mainly
letting errno flag errors encountered in the math library and checking
the result, rather than pre-checking.  This fixes a bug in power that
would cause an error if any non-positive number was raised to any
power.
2007-02-07 posted by Mikey C to sqlite mailing list.
Original code 2006 June 05 by relicoder.

*/

//#include "config.h"

#define COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE 1
#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1
#define HAVE_SINH 1
#define HAVE_COSH 1
#define HAVE_TANH 1
#define HAVE_LOG10 1
#define HAVE_ISBLANK 1
#define SQLITE_SOUNDEX 1

#ifdef COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE
#include "sqlite3ext.h"
#endif

#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#ifndef _MAP_H_
#define _MAP_H_

#include <stdint.h>
#include "sqlite3.h"
#include <sys/types.h>
#include "error.h"
#include "atomic_ops.h"
#include "binstream.h"
#include "blobio.h"
#include "geomio.h"
#include "geom_func.h"
#include "i18n.h"
#include "sql.h"
#include "sqlite.h"
#include "spatialdb_internal.h"
#include "wkb.h"
#include "wkt.h"

/*
** Simple binary tree implementation to use in median, mode and quartile calculations
** Tree is not necessarily balanced. That would require something like red&black trees of AVL
*/

typedef int(*cmp_func)(const void *, const void *);
typedef void(*map_iterator)(void*, int64_t, void*);

typedef struct node{
  struct node *l;
  struct node *r;
  void* data;
  int64_t count;
} node;

typedef struct map{
  node *base;
  cmp_func cmp;
  short free;
} map;

/*
** creates a map given a comparison function
*/
map map_make(cmp_func cmp);

/*
** inserts the element e into map m
*/
void map_insert(map *m, void *e);

/*
** executes function iter over all elements in the map, in key increasing order
*/
void map_iterate(map *m, map_iterator iter, void* p);

/*
** frees all memory used by a map
*/
void map_destroy(map *m);

/*
** compares 2 integers
** to use with map_make
*/
int int_cmp(const void *a, const void *b);

/*
** compares 2 doubles
** to use with map_make
*/
int double_cmp(const void *a, const void *b);

#endif /* _MAP_H_ */

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef int64_t         i64;

static char *sqlite3StrDup( const char *z ) {
    char *res = sqlite3_malloc( strlen(z)+1 );
    return strcpy( res, z );
}

/*
** These are copied verbatim from fun.c so as to not have the names exported
*/

/* LMH from sqlite3 3.3.13 */
/*
** This table maps from the first byte of a UTF-8 character to the number
** of trailing bytes expected. A value '4' indicates that the table key
** is not a legal first byte for a UTF-8 character.
*/
static const u8 xtra_utf8_bytes[256]  = {
/* 0xxxxxxx */
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,

/* 10wwwwww */
4, 4, 4, 4, 4, 4, 4, 4,     4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4,     4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4,     4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4,     4, 4, 4, 4, 4, 4, 4, 4,

/* 110yyyyy */
1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1,

/* 1110zzzz */
2, 2, 2, 2, 2, 2, 2, 2,     2, 2, 2, 2, 2, 2, 2, 2,

/* 11110yyy */
3, 3, 3, 3, 3, 3, 3, 3,     4, 4, 4, 4, 4, 4, 4, 4,
};


/*
** This table maps from the number of trailing bytes in a UTF-8 character
** to an integer constant that is effectively calculated for each character
** read by a naive implementation of a UTF-8 character reader. The code
** in the READ_UTF8 macro explains things best.
*/
static const int xtra_utf8_bits[] =  {
  0,
  12416,          /* (0xC0 << 6) + (0x80) */
  925824,         /* (0xE0 << 12) + (0x80 << 6) + (0x80) */
  63447168        /* (0xF0 << 18) + (0x80 << 12) + (0x80 << 6) + 0x80 */
};

/*
** If a UTF-8 character contains N bytes extra bytes (N bytes follow
** the initial byte so that the total character length is N+1) then
** masking the character with utf8_mask[N] must produce a non-zero
** result.  Otherwise, we have an (illegal) overlong encoding.
*/
static const int utf_mask[] = {
  0x00000000,
  0xffffff80,
  0xfffff800,
  0xffff0000,
};

/* LMH salvaged from sqlite3 3.3.13 source code src/utf.c */
#define READ_UTF8(zIn, c) { \
  int xtra;                                            \
  c = *(zIn)++;                                        \
  xtra = xtra_utf8_bytes[c];                           \
  switch( xtra ){                                      \
    case 4: c = (int)0xFFFD; break;                    \
    case 3: c = (c<<6) + *(zIn)++;                     \
    case 2: c = (c<<6) + *(zIn)++;                     \
    case 1: c = (c<<6) + *(zIn)++;                     \
    c -= xtra_utf8_bits[xtra];                         \
    if( (utf_mask[xtra]&c)==0                          \
        || (c&0xFFFFF800)==0xD800                      \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }    \
  }                                                    \
}

static int sqlite3ReadUtf8(const unsigned char *z){
  int c;
  READ_UTF8(z, c);
  return c;
}

#define SKIP_UTF8(zIn) {                               \
  zIn += (xtra_utf8_bytes[*(u8 *)zIn] + 1);            \
}

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
static int sqlite3Utf8CharLen(const char *z, int nByte){
  int r = 0;
  const char *zTerm;
  if( nByte>=0 ){
    zTerm = &z[nByte];
  }else{
    zTerm = (const char *)(-1);
  }
  assert( z<=zTerm );
  while( *z!=0 && z<zTerm ){
    SKIP_UTF8(z);
    r++;
  }
  return r;
}

/*
** X is a pointer to the first byte of a UTF-8 character.  Increment
** X so that it points to the next character.  This only works right
** if X points to a well-formed UTF-8 string.
*/
#define sqliteNextChar(X)  while( (0xc0&*++(X))==0x80 ){}
#define sqliteCharVal(X)   sqlite3ReadUtf8(X)

/*
** This is a macro that facilitates writting wrappers for math.h functions
** it creates code for a function to use in SQlite that gets one numeric input
** and returns a floating point value.
**
** Could have been implemented using pointers to functions but this way it's inline
** and thus more efficient. Lower * ranking though...
** 
** Parameters:
** name:      function name to de defined (eg: sinFunc)
** function:  function defined in math.h to wrap (eg: sin)
** domain:    boolean condition that CAN'T happen in terms of the input parameter rVal
**            (eg: rval<0 for sqrt)
*/
/* LMH 2007-03-25 Changed to use errno and remove domain; no pre-checking for errors. */
#define GEN_MATH_WRAP_DOUBLE_1(name, function) \
static void name(sqlite3_context *context, int argc, sqlite3_value **argv){\
  double rVal = 0.0, val;\
  assert( argc==1 );\
  switch( sqlite3_value_type(argv[0]) ){\
    case SQLITE_NULL: {\
      sqlite3_result_null(context);\
      break;\
    }\
    default: {\
      rVal = sqlite3_value_double(argv[0]);\
      errno = 0;\
      val = function(rVal);\
      if (errno == 0) {\
        sqlite3_result_double(context, val);\
      } else {\
        sqlite3_result_error(context, strerror(errno), errno);\
      }\
      break;\
    }\
  }\
}\


/*
** Example of GEN_MATH_WRAP_DOUBLE_1 usage
** this creates function sqrtFunc to wrap the math.h standard function sqrt(x)=x^0.5
*/
GEN_MATH_WRAP_DOUBLE_1(sqrtFunc, sqrt)

/* trignometric functions */
GEN_MATH_WRAP_DOUBLE_1(acosFunc, acos)
GEN_MATH_WRAP_DOUBLE_1(asinFunc, asin)
GEN_MATH_WRAP_DOUBLE_1(atanFunc, atan)

/*
** Many of systems don't have inverse hyperbolic trig functions so this will emulate
** them on those systems in terms of log and sqrt (formulas are too trivial to demand 
** written proof here)
*/

#ifndef HAVE_ACOSH
static double acosh(double x){
  return log(x + sqrt(x*x - 1.0));
}
#endif

GEN_MATH_WRAP_DOUBLE_1(acoshFunc, acosh)

#ifndef HAVE_ASINH
static double asinh(double x){
  return log(x + sqrt(x*x + 1.0));
}
#endif

GEN_MATH_WRAP_DOUBLE_1(asinhFunc, asinh)

#ifndef HAVE_ATANH
static double atanh(double x){
  return (1.0/2.0)*log((1+x)/(1-x)) ;
}
#endif

GEN_MATH_WRAP_DOUBLE_1(atanhFunc, atanh)

/*
** math.h doesn't require cot (cotangent) so it's defined here
*/
static double cot(double x){
  return 1.0/tan(x);
}

GEN_MATH_WRAP_DOUBLE_1(sinFunc, sin)
GEN_MATH_WRAP_DOUBLE_1(cosFunc, cos)
GEN_MATH_WRAP_DOUBLE_1(tanFunc, tan)
GEN_MATH_WRAP_DOUBLE_1(cotFunc, cot)

static double coth(double x){
  return 1.0/tanh(x);
}

/*
** Many systems don't have hyperbolic trigonometric functions so this will emulate
** them on those systems directly from the definition in terms of exp
*/
#ifndef HAVE_SINH
static double sinh(double x){
  return (exp(x)-exp(-x))/2.0;
}
#endif

GEN_MATH_WRAP_DOUBLE_1(sinhFunc, sinh)

#ifndef HAVE_COSH
static double cosh(double x){
  return (exp(x)+exp(-x))/2.0;
}
#endif

GEN_MATH_WRAP_DOUBLE_1(coshFunc, cosh)

#ifndef HAVE_TANH
static double tanh(double x){
  return sinh(x)/cosh(x);
}
#endif

GEN_MATH_WRAP_DOUBLE_1(tanhFunc, tanh)

GEN_MATH_WRAP_DOUBLE_1(cothFunc, coth)

/*
** Some systems lack log in base 10. This will emulate it
*/

#ifndef HAVE_LOG10
static double log10(double x){
  static double l10 = -1.0;
  if( l10<0.0 ){
    l10 = log(10.0);
  }
  return log(x)/l10;
}
#endif

GEN_MATH_WRAP_DOUBLE_1(logFunc, log)
GEN_MATH_WRAP_DOUBLE_1(log10Func, log10)
GEN_MATH_WRAP_DOUBLE_1(expFunc, exp)

/*
** Fallback for systems where math.h doesn't define M_PI
*/
#undef M_PI
#ifndef M_PI
/*
** static double PI = acos(-1.0);
** #define M_PI (PI)
*/
#define M_PI 3.14159265358979323846
#endif

/* Convert Degrees into Radians */
static double deg2rad(double x){
  return x*M_PI/180.0;
}

/* Convert Radians into Degrees */
static double rad2deg(double x){
  return 180.0*x/M_PI;
}

GEN_MATH_WRAP_DOUBLE_1(rad2degFunc, rad2deg)
GEN_MATH_WRAP_DOUBLE_1(deg2radFunc, deg2rad)

/* constant function that returns the value of PI=3.1415... */
static void piFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  sqlite3_result_double(context, M_PI);
}

/*
** Implements the sqrt function, it has the peculiarity of returning an integer when the
** the argument is an integer.
** Since SQLite isn't strongly typed (almost untyped actually) this is a bit pedantic
*/
static void squareFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  i64 iVal = 0;
  double rVal = 0.0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      iVal = sqlite3_value_int64(argv[0]);
      sqlite3_result_int64(context, iVal*iVal);
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      rVal = sqlite3_value_double(argv[0]);
      sqlite3_result_double(context, rVal*rVal);
      break;
    }
  }
}

/*
** Wraps the pow math.h function
** When both the base and the exponent are integers the result should be integer
** (see sqrt just before this). Here the result is always double
*/
/* LMH 2007-03-25 Changed to use errno; no pre-checking for errors.  Also removes
  but that was present in the pre-checking that called sqlite3_result_error on 
  a non-positive first argument, which is not always an error. */
static void powerFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  double r1 = 0.0;
  double r2 = 0.0;
  double val;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    r1 = sqlite3_value_double(argv[0]);
    r2 = sqlite3_value_double(argv[1]);
    errno = 0;
    val = pow(r1,r2);
    if (errno == 0) {
      sqlite3_result_double(context, val);
    } else {  
      sqlite3_result_error(context, strerror(errno), errno);
    }  
  }
}

/*
** atan2 wrapper
*/
static void atn2Func(sqlite3_context *context, int argc, sqlite3_value **argv){
  double r1 = 0.0;
  double r2 = 0.0;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    r1 = sqlite3_value_double(argv[0]);
    r2 = sqlite3_value_double(argv[1]);
    sqlite3_result_double(context, atan2(r1,r2));
  }
}

/*
** Implementation of the sign() function
** return one of 3 possibilities +1,0 or -1 when the argument is respectively
** positive, 0 or negative.
** When the argument is NULL the result is also NULL (completly conventional)
*/
static void signFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  double rVal=0.0;
  i64 iVal=0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      iVal = sqlite3_value_int64(argv[0]);
      iVal = ( iVal > 0) ? 1: ( iVal < 0 ) ? -1: 0;
      sqlite3_result_int64(context, iVal);
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
 /* 2nd change below. Line for abs was: if( rVal<0 ) rVal = rVal * -1.0;  */

      rVal = sqlite3_value_double(argv[0]);
      rVal = ( rVal > 0) ? 1: ( rVal < 0 ) ? -1: 0;
      sqlite3_result_double(context, rVal);
      break;
    }
  }
}


/*
** smallest integer value not less than argument
*/
static void ceilFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  double rVal=0.0;
  i64 iVal=0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      i64 iVal = sqlite3_value_int64(argv[0]);
      sqlite3_result_int64(context, iVal);
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      rVal = sqlite3_value_double(argv[0]);
      sqlite3_result_int64(context, (i64) ceil(rVal));
      break;
    }
  }
}

/*
** largest integer value not greater than argument
*/
static void floorFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  double rVal=0.0;
  i64 iVal=0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      i64 iVal = sqlite3_value_int64(argv[0]);
      sqlite3_result_int64(context, iVal);
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      rVal = sqlite3_value_double(argv[0]);
      sqlite3_result_int64(context, (i64) floor(rVal));
      break;
    }
  }
}

/*
** Given a string (s) in the first argument and an integer (n) in the second returns the 
** string that constains s contatenated n times
*/
static void replicateFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  unsigned char *z;        /* input string */
  unsigned char *zo;       /* result string */
  i64 iCount;              /* times to repeat */
  i64 nLen;                /* length of the input string (no multibyte considerations) */ 
  i64 nTLen;               /* length of the result string (no multibyte considerations) */
  i64 i=0;

  if( argc!=2 || SQLITE_NULL==sqlite3_value_type(argv[0]) )
    return;

  iCount = sqlite3_value_int64(argv[1]);

  if( iCount<0 ){
    sqlite3_result_error(context, "domain error", -1);
  }else{

    nLen  = sqlite3_value_bytes(argv[0]);
    nTLen = nLen*iCount;
    z=sqlite3_malloc(nTLen+1);
    zo=sqlite3_malloc(nLen+1);
    if (!z || !zo){
      sqlite3_result_error_nomem(context);
      if (z) sqlite3_free(z);
      if (zo) sqlite3_free(zo);
      return;
    }
    strcpy((char*)zo, (char*)sqlite3_value_text(argv[0]));

    for(i=0; i<iCount; ++i){
      strcpy((char*)(z+i*nLen), (char*)zo);
    }

    sqlite3_result_text(context, (char*)z, -1, SQLITE_TRANSIENT);
    sqlite3_free(z);
    sqlite3_free(zo);
  }
}

/* 
** Some systems (win32 among others) don't have an isblank function, this will emulate it.
** This function is not UFT-8 safe since it only analyses a byte character.
*/
#ifndef HAVE_ISBLANK
int isblank(char c){
  return( ' '==c || '\t'==c );
}
#endif

static void properFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  const unsigned char *z;     /* input string */
  unsigned char *zo;          /* output string */
  unsigned char *zt;          /* iterator */
  char r;
  int c=1;

  assert( argc==1);
  if( SQLITE_NULL==sqlite3_value_type(argv[0]) ){
    sqlite3_result_null(context);
    return;
  }

  z = sqlite3_value_text(argv[0]);
  zo = (unsigned char *)sqlite3StrDup((char *) z);
  if (!zo) {
    sqlite3_result_error_nomem(context);
    return;
  }
  zt = zo;

  while( (r = *(z++))!=0 ){
    if( isblank(r) ){
      c=1;
    }else{
      if( c==1 ){
        r = toupper(r);
      }else{
        r = tolower(r);
      }
      c=0;
    }
    *(zt++) = r;
  }
  *zt = '\0';

  sqlite3_result_text(context, (char*)zo, -1, SQLITE_TRANSIENT);
  sqlite3_free(zo);
}

/*
** given an input string (s) and an integer (n) adds spaces at the begining of  s
** until it has a length of n characters.
** When s has a length >=n it's a NOP
** padl(NULL) = NULL
*/
static void padlFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  i64 ilen;          /* length to pad to */
  i64 zl;            /* length of the input string (UTF-8 chars) */
  int i = 0;
  const char *zi;    /* input string */
  char *zo;          /* output string */
  char *zt;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    zi = (char *)sqlite3_value_text(argv[0]);
    ilen = sqlite3_value_int64(argv[1]);
    /* check domain */
    if(ilen<0){
      sqlite3_result_error(context, "domain error", -1);
      return;
    }
    zl = sqlite3Utf8CharLen(zi, -1);
    if( zl>=ilen ){
      /* string is longer than the requested pad length, return the same string (dup it) */
      zo = sqlite3StrDup(zi);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    }else{
      zo = sqlite3_malloc(strlen(zi)+ilen-zl+1);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      zt = zo;
      for(i=1; i+zl<=ilen; ++i){
        *(zt++)=' ';
      }
      /* no need to take UTF-8 into consideration here */
      strcpy(zt,zi);
    }
    sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    sqlite3_free(zo);
  }
}

/*
** given an input string (s) and an integer (n) appends spaces at the end of  s
** until it has a length of n characters.
** When s has a length >=n it's a NOP
** padl(NULL) = NULL
*/
static void padrFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  i64 ilen;          /* length to pad to */
  i64 zl;            /* length of the input string (UTF-8 chars) */
  i64 zll;           /* length of the input string (bytes) */
  int i = 0;
  const char *zi;    /* input string */
  char *zo;          /* output string */
  char *zt;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    zi = (char *)sqlite3_value_text(argv[0]);
    ilen = sqlite3_value_int64(argv[1]);
    /* check domain */
    if(ilen<0){
      sqlite3_result_error(context, "domain error", -1);
      return;
    }
    zl = sqlite3Utf8CharLen(zi, -1);
    if( zl>=ilen ){
      /* string is longer than the requested pad length, return the same string (dup it) */
      zo = sqlite3StrDup(zi);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    }else{
      zll = strlen(zi);
      zo = sqlite3_malloc(zll+ilen-zl+1);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      zt = strcpy(zo,zi)+zll;
      for(i=1; i+zl<=ilen; ++i){
        *(zt++) = ' ';
      }
      *zt = '\0';
    }
    sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    sqlite3_free(zo);
  }
}

/*
** given an input string (s) and an integer (n) appends spaces at the end of  s
** and adds spaces at the begining of s until it has a length of n characters.
** Tries to add has many characters at the left as at the right.
** When s has a length >=n it's a NOP
** padl(NULL) = NULL
*/
static void padcFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  i64 ilen;           /* length to pad to */
  i64 zl;             /* length of the input string (UTF-8 chars) */
  i64 zll;            /* length of the input string (bytes) */
  int i = 0;
  const char *zi;     /* input string */
  char *zo;           /* output string */
  char *zt;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    zi = (char *)sqlite3_value_text(argv[0]);
    ilen = sqlite3_value_int64(argv[1]);
    /* check domain */
    if(ilen<0){
      sqlite3_result_error(context, "domain error", -1);
      return;
    }
    zl = sqlite3Utf8CharLen(zi, -1);
    if( zl>=ilen ){
      /* string is longer than the requested pad length, return the same string (dup it) */
      zo = sqlite3StrDup(zi);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    }else{
      zll = strlen(zi);
      zo = sqlite3_malloc(zll+ilen-zl+1);
      if (!zo){
        sqlite3_result_error_nomem(context);
        return;
      }
      zt = zo;
      for(i=1; 2*i+zl<=ilen; ++i){
        *(zt++) = ' ';
      }
      strcpy(zt, zi);
      zt+=zll;
      for(; i+zl<=ilen; ++i){
        *(zt++) = ' ';
      }
      *zt = '\0';
    }
    sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    sqlite3_free(zo);
  }
}

/*
** given 2 string (s1,s2) returns the string s1 with the characters NOT in s2 removed
** assumes strings are UTF-8 encoded
*/
static void strfilterFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  const char *zi1;        /* first parameter string (searched string) */
  const char *zi2;        /* second parameter string (vcontains valid characters) */
  const char *z1;
  const char *z21;
  const char *z22;
  char *zo;               /* output string */
  char *zot;
  int c1 = 0;
  int c2 = 0;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL ){
    sqlite3_result_null(context); 
  }else{
    zi1 = (char *)sqlite3_value_text(argv[0]);
    zi2 = (char *)sqlite3_value_text(argv[1]);
    /* 
    ** maybe I could allocate less, but that would imply 2 passes, rather waste 
    ** (possibly) some memory
    */
    zo = sqlite3_malloc(strlen(zi1)+1); 
    if (!zo){
      sqlite3_result_error_nomem(context);
      return;
    }
    zot = zo;
    z1 = zi1;
    while( (c1=sqliteCharVal((unsigned char *)z1))!=0 ){
      z21=zi2;
      while( (c2=sqliteCharVal((unsigned char *)z21))!=0 && c2!=c1 ){
        sqliteNextChar(z21);
      }
      if( c2!=0){
        z22=z21;
        sqliteNextChar(z22);
        strncpy(zot, z21, z22-z21);
        zot+=z22-z21;
      }
      sqliteNextChar(z1);
    }
    *zot = '\0';

    sqlite3_result_text(context, zo, -1, SQLITE_TRANSIENT);
    sqlite3_free(zo);
  }
}

/*
** Given a string z1, retutns the (0 based) index of it's first occurence
** in z2 after the first s characters.
** Returns -1 when there isn't a match.
** updates p to point to the character where the match occured.
** This is an auxiliary function.
*/
static int _substr(const char* z1, const char* z2, int s, const char** p){
  int c = 0;
  int rVal=-1;
  const char* zt1;
  const char* zt2;
  int c1,c2;

  if( '\0'==*z1 ){
    return -1;
  }
  
  while( (sqliteCharVal((unsigned char *)z2) != 0) && (c++)<s){
    sqliteNextChar(z2);
  }
  
  c = 0;
  while( (sqliteCharVal((unsigned char *)z2)) != 0 ){
    zt1 = z1;
    zt2 = z2;

    do{
      c1 = sqliteCharVal((unsigned char *)zt1);
      c2 = sqliteCharVal((unsigned char *)zt2);
      sqliteNextChar(zt1);
      sqliteNextChar(zt2);
    }while( c1 == c2 && c1 != 0 && c2 != 0 );

    if( c1 == 0 ){
      rVal = c;
      break; 
    }
    
    sqliteNextChar(z2);
    ++c;
  }
  if(p){
    *p=z2;
  }
  return rVal >=0 ? rVal+s : rVal;
}

/*
** given 2 input strings (s1,s2) and an integer (n) searches from the nth character
** for the string s1. Returns the position where the match occured.
** Characters are counted from 1.
** 0 is returned when no match occurs.
*/

static void charindexFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  const u8 *z1;          /* s1 string */
  u8 *z2;                /* s2 string */
  int s=0;
  int rVal=0;

  assert( argc==3 ||argc==2);

  if( SQLITE_NULL==sqlite3_value_type(argv[0]) || SQLITE_NULL==sqlite3_value_type(argv[1])){
    sqlite3_result_null(context);
    return;
  }

  z1 = sqlite3_value_text(argv[0]);
  if( z1==0 ) return;
  z2 = (u8*) sqlite3_value_text(argv[1]);
  if(argc==3){
    s = sqlite3_value_int(argv[2])-1;
    if(s<0){
      s=0;
    }
  }else{
    s = 0;
  }

  rVal = _substr((char *)z1,(char *)z2,s,NULL);
  sqlite3_result_int(context, rVal+1);
}

/*
** given a string (s) and an integer (n) returns the n leftmost (UTF-8) characters
** if the string has a length<=n or is NULL this function is NOP
*/
static void leftFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  int c=0;
  int cc=0;
  int l=0;
  const unsigned char *z;       /* input string */
  const unsigned char *zt;
  unsigned char *rz;            /* output string */

  assert( argc==2);

  if( SQLITE_NULL==sqlite3_value_type(argv[0]) || SQLITE_NULL==sqlite3_value_type(argv[1])){
    sqlite3_result_null(context);
    return;
  }

  z  = sqlite3_value_text(argv[0]);
  l  = sqlite3_value_int(argv[1]);
  zt = z;

  while( sqliteCharVal(zt) && c++<l)
    sqliteNextChar(zt);

  cc=zt-z;

  rz = sqlite3_malloc(zt-z+1);
  if (!rz){
    sqlite3_result_error_nomem(context);
    return;
  }
  strncpy((char*) rz, (char*) z, zt-z);
  *(rz+cc) = '\0';
  sqlite3_result_text(context, (char*)rz, -1, SQLITE_TRANSIENT); 
  sqlite3_free(rz);
}

/*
** given a string (s) and an integer (n) returns the n rightmost (UTF-8) characters
** if the string has a length<=n or is NULL this function is NOP
*/
static void rightFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  int l=0;
  int c=0;
  int cc=0;
  const char *z;
  const char *zt;
  const char *ze;
  char *rz;

  assert( argc==2);

  if( SQLITE_NULL == sqlite3_value_type(argv[0]) || SQLITE_NULL == sqlite3_value_type(argv[1])){
    sqlite3_result_null(context);
    return;
  }

  z  = (char *)sqlite3_value_text(argv[0]);
  l  = sqlite3_value_int(argv[1]);
  zt = z;

  while( sqliteCharVal((unsigned char *)zt)!=0){
    sqliteNextChar(zt);
    ++c;
  }

  ze = zt;
  zt = z;

  cc=c-l;
  if(cc<0)
    cc=0;
  
  while( cc-- > 0 ){
    sqliteNextChar(zt);
  }

  rz = sqlite3_malloc(ze-zt+1);
  if (!rz){
    sqlite3_result_error_nomem(context);
    return;
  }
  strcpy((char*) rz, (char*) (zt));
  sqlite3_result_text(context, (char*)rz, -1, SQLITE_TRANSIENT); 
  sqlite3_free(rz);
}


/*
** given a string returns the same string but with the characters in reverse order
*/
static void reverseFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  const char *z;
  const char *zt;
  char *rz;
  char *rzt;
  int l = 0;
  int i = 0;

  assert( 1==argc );

  if( SQLITE_NULL==sqlite3_value_type(argv[0]) ){
    sqlite3_result_null(context);
    return;
  }
  z = (char *)sqlite3_value_text(argv[0]);
  l = strlen(z);
  rz = sqlite3_malloc(l+1);
  if (!rz){
    sqlite3_result_error_nomem(context);
    return;
  }
  rzt = rz+l;
  *(rzt--) = '\0';

  zt=z;
  while( sqliteCharVal((unsigned char *)zt)!=0 ){
    z=zt;
    sqliteNextChar(zt);
    for(i=1; zt-i>=z; ++i){
      *(rzt--)=*(zt-i);
    }
  }

  sqlite3_result_text(context, rz, -1, SQLITE_TRANSIENT);
  sqlite3_free(rz);
}

/*
** An instance of the following structure holds the context of a
** stdev() or variance() aggregate computation.
** implementaion of http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Algorithm_II
** less prone to rounding errors
*/
typedef struct StdevCtx StdevCtx;
struct StdevCtx {
  double rM;
  double rS;
  i64 cnt;          /* number of elements */
};

/*
** An instance of the following structure holds the context of a
** mode() or median() aggregate computation.
** Depends on structures defined in map.c (see map & map)
** These aggregate functions only work for integers and floats although
** they could be made to work for strings. This is usually considered meaningless.
** Only usuall order (for median), no use of collation functions (would this even make sense?)
*/
typedef struct ModeCtx ModeCtx;
struct ModeCtx {
  i64 riM;            /* integer value found so far */
  double rdM;         /* double value found so far */
  i64 cnt;            /* number of elements so far */
  double pcnt;        /* number of elements smaller than a percentile */
  i64 mcnt;           /* maximum number of occurrences (for mode) */
  i64 mn;             /* number of occurrences (for mode and percentiles) */
  i64 is_double;      /* whether the computation is being done for doubles (>0) or integers (=0) */
  map* m;             /* map structure used for the computation */
  int done;           /* whether the answer has been found */
};

/*
** called for each value received during a calculation of stdev or variance
*/
static void varianceStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  StdevCtx *p;

  double delta;
  double x;

  assert( argc==1 );
  p = sqlite3_aggregate_context(context, sizeof(*p));
  /* only consider non-null values */
  if( SQLITE_NULL != sqlite3_value_numeric_type(argv[0]) ){
    p->cnt++;
    x = sqlite3_value_double(argv[0]);
    delta = (x-p->rM);
    p->rM += delta/p->cnt;
    p->rS += delta*(x-p->rM);
  }
}

/*
** called for each value received during a calculation of mode of median
*/
static void modeStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  ModeCtx *p;
  i64 xi=0;
  double xd=0.0;
  i64 *iptr;
  double *dptr;
  int type;

  assert( argc==1 );
  type = sqlite3_value_numeric_type(argv[0]);

  if( type == SQLITE_NULL)
    return;
  
  p = sqlite3_aggregate_context(context, sizeof(*p));

  if( 0==(p->m) ){
    p->m = calloc(1, sizeof(map));
    if( type==SQLITE_INTEGER ){
      /* map will be used for integers */
      *(p->m) = map_make(int_cmp);
      p->is_double = 0;
    }else{
      p->is_double = 1;
      /* map will be used for doubles */
      *(p->m) = map_make(double_cmp);
    }
  }

  ++(p->cnt);

  if( 0==p->is_double ){
    xi = sqlite3_value_int64(argv[0]);
    iptr = (i64*)calloc(1,sizeof(i64));
    *iptr = xi;
    map_insert(p->m, iptr);
  }else{
    xd = sqlite3_value_double(argv[0]);
    dptr = (double*)calloc(1,sizeof(double));
    *dptr = xd;
    map_insert(p->m, dptr);
  }
}

/*
**  Auxiliary function that iterates all elements in a map and finds the mode
**  (most frequent value)
*/
static void modeIterate(void* e, i64 c, void* pp){
  i64 ei;
  double ed;
  ModeCtx *p = (ModeCtx*)pp;
  
  if( 0==p->is_double ){
    ei = *(int*)(e);

	if( p->mcnt==c ){
      ++p->mn;
    }else if( p->mcnt<c ){
      p->riM = ei;
      p->mcnt = c;
	  p->mn=1;
    }
  }else{
    ed = *(double*)(e);

	if( p->mcnt==c ){
      ++p->mn;
    }else if(p->mcnt<c){
      p->rdM = ed;
      p->mcnt = c;
	  p->mn=1;
    }
  }
}

/*
**  Auxiliary function that iterates all elements in a map and finds the median
**  (the value such that the number of elements smaller is equal the the number of 
**  elements larger)
*/
static void medianIterate(void* e, i64 c, void* pp){
  i64 ei;
  double ed;
  double iL;
  double iR;
  int il;
  int ir;
  ModeCtx *p = (ModeCtx*)pp;

  if(p->done>0)
    return;

  iL = p->pcnt;
  iR = p->cnt - p->pcnt;
  il = p->mcnt + c;
  ir = p->cnt - p->mcnt;

  if( il >= iL ){
    if( ir >= iR ){
    ++p->mn;
      if( 0==p->is_double ){
        ei = *(int*)(e);
        p->riM += ei;
      }else{
        ed = *(double*)(e);
        p->rdM += ed;
      }
    }else{
      p->done=1;
    }
  }
  p->mcnt+=c;
}

/*
** Returns the mode value
*/
static void modeFinalize(sqlite3_context *context){
  ModeCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if( p && p->m ){
    map_iterate(p->m, modeIterate, p);
    map_destroy(p->m);
    free(p->m);

    if( 1==p->mn ){
      if( 0==p->is_double )
        sqlite3_result_int64(context, p->riM);
      else
        sqlite3_result_double(context, p->rdM);
    }
  }
}

/*
** auxiliary function for percentiles
*/
static void _medianFinalize(sqlite3_context *context){
  ModeCtx *p;
  p = (ModeCtx*) sqlite3_aggregate_context(context, 0);
  if( p && p->m ){
    p->done=0;
    map_iterate(p->m, medianIterate, p);
    map_destroy(p->m);
    free(p->m);

    if( 0==p->is_double )
      if( 1==p->mn )
      	sqlite3_result_int64(context, p->riM);
      else
      	sqlite3_result_double(context, p->riM*1.0/p->mn);
    else
      sqlite3_result_double(context, p->rdM/p->mn);
  }
}

/*
** Returns the median value
*/
static void medianFinalize(sqlite3_context *context){
  ModeCtx *p;
  p = (ModeCtx*) sqlite3_aggregate_context(context, 0);
  if( p!=0 ){
    p->pcnt = (p->cnt)/2.0;
    _medianFinalize(context);
  }
}

/*
** Returns the lower_quartile value
*/
static void lower_quartileFinalize(sqlite3_context *context){
  ModeCtx *p;
  p = (ModeCtx*) sqlite3_aggregate_context(context, 0);
  if( p!=0 ){
    p->pcnt = (p->cnt)/4.0;
    _medianFinalize(context);
  }
}

/*
** Returns the upper_quartile value
*/
static void upper_quartileFinalize(sqlite3_context *context){
  ModeCtx *p;
  p = (ModeCtx*) sqlite3_aggregate_context(context, 0);
  if( p!=0 ){
    p->pcnt = (p->cnt)*3/4.0;
    _medianFinalize(context);
  }
}

/*
** Returns the stdev value
*/
static void stdevFinalize(sqlite3_context *context){
  StdevCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if( p && p->cnt>1 ){
    sqlite3_result_double(context, sqrt(p->rS/(p->cnt-1)));
  }else{
    sqlite3_result_double(context, 0.0);
  }
}

/*
** Returns the variance value
*/
static void varianceFinalize(sqlite3_context *context){
  StdevCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if( p && p->cnt>1 ){
    sqlite3_result_double(context, p->rS/(p->cnt-1));
  }else{
    sqlite3_result_double(context, 0.0);
  }
}

#ifdef SQLITE_SOUNDEX

/* relicoder factored code */
/*
** Calculates the soundex value of a string
*/

static void soundex(const u8 *zIn, char *zResult){
  int i, j;
  static const unsigned char iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };

  for(i=0; zIn[i] && !isalpha(zIn[i]); i++){}
  if( zIn[i] ){
    zResult[0] = toupper(zIn[i]);
    for(j=1; j<4 && zIn[i]; i++){
      int code = iCode[zIn[i]&0x7f];
      if( code>0 ){
        zResult[j++] = code + '0';
      }
    }
    while( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
  }else{
    strcpy(zResult, "?000");
  }
}

/*
** computes the number of different characters between the soundex value fo 2 strings
*/
static void differenceFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  char zResult1[8];
  char zResult2[8];
  char *zR1 = zResult1;
  char *zR2 = zResult2;
  int rVal = 0;
  int i = 0;
  const u8 *zIn1;
  const u8 *zIn2;

  assert( argc==2 );
  
  if( sqlite3_value_type(argv[0])==SQLITE_NULL || sqlite3_value_type(argv[1])==SQLITE_NULL ){
    sqlite3_result_null(context);
    return;
  }
  
  zIn1 = (u8*)sqlite3_value_text(argv[0]);
  zIn2 = (u8*)sqlite3_value_text(argv[1]);

  soundex(zIn1, zR1);
  soundex(zIn2, zR2);

  for(i=0; i<4; ++i){
    if( sqliteCharVal((unsigned char *)zR1)==sqliteCharVal((unsigned char *)zR2) )
      ++rVal;
    sqliteNextChar(zR1);
    sqliteNextChar(zR2);
  }
  sqlite3_result_int(context, rVal);
}
#endif



#define ST_MIN_MAX(name, check, field) static void ST_##name(sqlite3_context *context, int nbArgs, sqlite3_value **args) { \
    spatialdb_t *spatialdb; \
    FUNCTION_GEOM_ARG(geomblob); \
\
    FUNCTION_START_STATIC(context, 256); \
    spatialdb = (spatialdb_t *)sqlite3_user_data(context); \
    FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0); \
 \
    if (geomblob.envelope.check == 0) { \
        if (spatialdb->fill_envelope(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob.envelope, FUNCTION_ERROR) != SQLITE_OK) { \
            if ( error_count(FUNCTION_ERROR) == 0 ) error_append(FUNCTION_ERROR, "Invalid geometry blob header");\
            goto exit; \
        } \
    } \
\
    if (geomblob.envelope.check) { \
        sqlite3_result_double(context, geomblob.envelope.field); \
    } else { \
        sqlite3_result_null(context); \
    } \
    FUNCTION_END(context); \
    FUNCTION_FREE_GEOM_ARG(geomblob); \
}

ST_MIN_MAX(MinX, has_env_x, min_x)
ST_MIN_MAX(MaxX, has_env_x, max_x)
ST_MIN_MAX(MinY, has_env_y, min_y)
ST_MIN_MAX(MaxY, has_env_y, max_y)
ST_MIN_MAX(MinZ, has_env_z, min_z)
ST_MIN_MAX(MaxZ, has_env_z, max_z)
ST_MIN_MAX(MinM, has_env_m, min_m)
ST_MIN_MAX(MaxM, has_env_m, max_m)

static void ST_SRID(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_GEOM_ARG(geomblob);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

	if (nbArgs == 1) {
		sqlite3_result_int(context, geomblob.srid);
	}
	else {
		FUNCTION_GET_INT_ARG(geomblob.srid, 1);
		if (binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0) != SQLITE_OK) {
			sqlite3_result_error(context, "Error writing geometry blob header", -1);
			goto exit;
		}
		if (spatialdb->write_blob_header(&FUNCTION_GEOM_ARG_STREAM(geomblob), &geomblob, FUNCTION_ERROR) != SQLITE_OK) {
			if (error_count(FUNCTION_ERROR) == 0) {
				error_append(FUNCTION_ERROR, "Error writing geometry blob header");
			}
			goto exit;
		}
		binstream_seek(&FUNCTION_GEOM_ARG_STREAM(geomblob), 0);
		sqlite3_result_blob(context, binstream_data(&FUNCTION_GEOM_ARG_STREAM(geomblob)), (int)binstream_available(&FUNCTION_GEOM_ARG_STREAM(geomblob)), SQLITE_TRANSIENT);
	}

	FUNCTION_END(context);
	FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsEmpty(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_GEOM_ARG(geomblob);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

	sqlite3_result_int(context, geomblob.empty);

	FUNCTION_END(context);

	FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_IsMeasured(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_WKB_ARG(wkb);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

	sqlite3_result_int(context, wkb.coord_type == GEOM_XYM || wkb.coord_type == GEOM_XYZM);

	FUNCTION_END(context);
	FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_Is3d(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_WKB_ARG(wkb);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

	sqlite3_result_int(context, wkb.coord_type == GEOM_XYZ || wkb.coord_type == GEOM_XYZM);

	FUNCTION_END(context);
	FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_CoordDim(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_WKB_ARG(wkb);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

	sqlite3_result_int(context, geom_coord_dim(wkb.coord_type));

	FUNCTION_END(context);

	FUNCTION_FREE_WKB_ARG(wkb);
}

static void ST_GeometryType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_WKB_ARG(wkb);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_WKB_ARG_UNSAFE(context, spatialdb, wkb, 0);

	const char *type_name;
	if (geom_type_name(wkb.geom_type, &type_name) == SQLITE_OK) {
		sqlite3_result_text(context, type_name, -1, SQLITE_STATIC);
	}
	else {
		error_append(FUNCTION_ERROR, "Unknown geometry type: %d", wkb.geom_type);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_WKB_ARG(wkb);
}

typedef struct {
	uint8_t *data;
	int length;
} geom_blob_auxdata;

static geom_blob_auxdata *geom_blob_auxdata_malloc() {
	return (geom_blob_auxdata *)sqlite3_malloc(sizeof(geom_blob_auxdata));
}

static void geom_blob_auxdata_free(void *auxdata) {
	if (auxdata != NULL) {
		geom_blob_auxdata *geom = (geom_blob_auxdata *)auxdata;
		sqlite3_free(geom->data);
		geom->data = NULL;
		sqlite3_free(geom);
	}
}

static void ST_AsBinary(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_GEOM_ARG(geomblob);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

	wkb_writer_t writer;
	wkb_writer_init(&writer, WKB_ISO);

	FUNCTION_RESULT = spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), wkb_writer_geom_consumer(&writer), FUNCTION_ERROR);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_blob(context, wkb_writer_getwkb(&writer), (int)wkb_writer_length(&writer), sqlite3_free);
		wkb_writer_destroy(&writer, 0);
	}
	else {
		wkb_writer_destroy(&writer, 1);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_GEOM_ARG(geomblob);
}

static void ST_AsText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_GEOM_ARG(geomblob);

	FUNCTION_START_STATIC(context, 256);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	FUNCTION_GET_GEOM_ARG_UNSAFE(context, spatialdb, geomblob, 0);

	wkt_writer_t writer;
	wkt_writer_init(&writer);

	FUNCTION_RESULT = spatialdb->read_geometry(&FUNCTION_GEOM_ARG_STREAM(geomblob), wkt_writer_geom_consumer(&writer), FUNCTION_ERROR);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_text(context, wkt_writer_getwkt(&writer), (int)wkt_writer_length(&writer), SQLITE_TRANSIENT);
	}
	wkt_writer_destroy(&writer);

	FUNCTION_END(context);

	FUNCTION_FREE_GEOM_ARG(geomblob);
}

static int geometry_is_assignable(geom_type_t expected, geom_type_t actual, errorstream_t* error) {
	if (!geom_is_assignable(expected, actual)) {
		const char* expectedName = NULL;
		const char* actualName = NULL;
		if (geom_type_name(expected, &expectedName) == SQLITE_OK && geom_type_name(actual, &actualName) == SQLITE_OK) {
			error_append(error, "Incorrect geometry type. Expected '%d' actual '%s'", expectedName, actualName);
		}
		else {
			error_append(error, "Incorrect geometry type");
		}
		return SQLITE_ERROR;
	}
	else {
		return SQLITE_OK;
	}
}

typedef int(*geometry_constructor_func)(sqlite3_context *context, void *user_data, geom_consumer_t *consumer, int nbArgs, sqlite3_value **args, errorstream_t *error);

static void geometry_constructor(sqlite3_context *context, const spatialdb_t *spatialdb, geometry_constructor_func constructor, void* user_data, geom_type_t requiredType, int nbArgs, sqlite3_value **args) {
	FUNCTION_START_STATIC(context, 256);

	geom_blob_auxdata *geom = (geom_blob_auxdata *)sqlite3_get_auxdata(context, 0);

	if (geom == NULL) {
		geom_blob_writer_t writer;

		if (sqlite3_value_type(args[nbArgs - 1]) == SQLITE_INTEGER) {
			spatialdb->writer_init_srid(&writer, sqlite3_value_int(args[nbArgs - 1]));
			nbArgs -= 1;
		}
		else {
			spatialdb->writer_init(&writer);
		}

		FUNCTION_RESULT = constructor(context, user_data, geom_blob_writer_geom_consumer(&writer), nbArgs, args, FUNCTION_ERROR);

		if (FUNCTION_RESULT == SQLITE_OK) {
			if (geometry_is_assignable(requiredType, writer.geom_type, FUNCTION_ERROR) == SQLITE_OK) {
				uint8_t *data = geom_blob_writer_getdata(&writer);
				int length = (int)geom_blob_writer_length(&writer);
				sqlite3_result_blob(context, data, length, SQLITE_TRANSIENT);
				spatialdb->writer_destroy(&writer, 0);

				geom = geom_blob_auxdata_malloc();
				if (geom != NULL) {
					geom->data = data;
					geom->length = length;
					sqlite3_set_auxdata(context, 0, geom, geom_blob_auxdata_free);
				}
			}
		}
		else {
			spatialdb->writer_destroy(&writer, 1);
		}
	}
	else {
		sqlite3_result_blob(context, geom->data, geom->length, SQLITE_TRANSIENT);
	}

	FUNCTION_END(context);
}

static int geom_from_wkb(sqlite3_context *context, void *user_data, geom_consumer_t* consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
	FUNCTION_STREAM_ARG(wkb);
	FUNCTION_START_NESTED(context, error);
	FUNCTION_GET_STREAM_ARG_UNSAFE(context, wkb, 0);

	FUNCTION_RESULT = wkb_read_geometry(&wkb, WKB_ISO, consumer, FUNCTION_ERROR);

	FUNCTION_END_NESTED(context);
	FUNCTION_FREE_STREAM_ARG(wkb);

	return FUNCTION_RESULT;
}

static void ST_GeomFromWKB(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	geometry_constructor(context, spatialdb, geom_from_wkb, NULL, GEOM_GEOMETRY, nbArgs, args);
}

typedef struct {
	volatile long ref_count;
	const spatialdb_t *spatialdb;
	i18n_locale_t *locale;
} fromtext_t;

static fromtext_t *fromtext_init(const spatialdb_t *spatialdb) {
	fromtext_t *ctx = (fromtext_t *)sqlite3_malloc(sizeof(fromtext_t));

	if (ctx == NULL) {
		return NULL;
	}

	i18n_locale_t *locale = i18n_locale_init("C");
	if (locale == NULL) {
		sqlite3_free(ctx);
		return NULL;
	}

	ctx->ref_count = 1;
	ctx->locale = locale;
	ctx->spatialdb = spatialdb;
	return ctx;
}

static void fromtext_acquire(fromtext_t *fromtext) {
	if (fromtext) {
		atomic_inc_long(&fromtext->ref_count);
	}
}

static void fromtext_release(fromtext_t *fromtext) {
	if (fromtext) {
		long newval = atomic_dec_long(&fromtext->ref_count);
		if (newval == 0) {
			i18n_locale_destroy(fromtext->locale);
			fromtext->locale = NULL;
			sqlite3_free(fromtext);
		}
	}
}

static int geom_from_wkt(sqlite3_context *context, void *user_data, geom_consumer_t* consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
	FUNCTION_TEXT_ARG(wkt);
	FUNCTION_START_NESTED(context, error);

	FUNCTION_GET_TEXT_ARG_UNSAFE(wkt, 0);

	FUNCTION_RESULT = wkt_read_geometry(wkt, FUNCTION_TEXT_ARG_LENGTH(wkt), consumer, (i18n_locale_t *)user_data, FUNCTION_ERROR);

	FUNCTION_END_NESTED(context);
	FUNCTION_FREE_TEXT_ARG(wkt);

	return FUNCTION_RESULT;
}

static void ST_GeomFromText(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	fromtext_t *fromtext = (fromtext_t *)sqlite3_user_data(context);
	geometry_constructor(context, fromtext->spatialdb, geom_from_wkt, fromtext->locale, GEOM_GEOMETRY, nbArgs, args);
}

static int point_from_coords(sqlite3_context *context, void *user_data, geom_consumer_t *consumer, int nbArgs, sqlite3_value **args, errorstream_t *error) {
	int result = SQLITE_OK;

	if (nbArgs < 2 || nbArgs > 4) {
		error_append(error, "Invalid number of coordinates: %d", nbArgs);
		result = SQLITE_ERROR;
	}
	else {
		double coord[4];
		for (int i = 0; i < nbArgs; i++) {
			coord[i] = sqlite3_value_double(args[i]);
		}

		geom_header_t header;
		header.geom_type = GEOM_POINT;
		if (nbArgs == 2) {
			header.coord_type = GEOM_XY;
			header.coord_size = 2;
		}
		else if (nbArgs == 3) {
			header.coord_type = GEOM_XYZ;
			header.coord_size = 3;
		}
		else {
			header.coord_type = GEOM_XYZM;
			header.coord_size = 4;
		}

		if (result == SQLITE_OK) {
			result = consumer->begin(consumer, error);
		}
		if (result == SQLITE_OK) {
			result = consumer->begin_geometry(consumer, &header, error);
		}
		if (result == SQLITE_OK) {
			result = consumer->coordinates(consumer, &header, 1, coord, 0, error);
		}
		if (result == SQLITE_OK) {
			result = consumer->end_geometry(consumer, &header, error);
		}
		if (result == SQLITE_OK) {
			result = consumer->end(consumer, error);
		}
	}

	return result;
}

static void ST_Point(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	fromtext_t *fromtext = (fromtext_t *)sqlite3_user_data(context);
	if (sqlite3_value_type(args[0]) == SQLITE_TEXT) {
		geometry_constructor(context, fromtext->spatialdb, geom_from_wkt, fromtext->locale, GEOM_POINT, nbArgs, args);
	}
	else if (sqlite3_value_type(args[0]) == SQLITE_BLOB) {
		geometry_constructor(context, fromtext->spatialdb, geom_from_wkb, NULL, GEOM_POINT, nbArgs, args);
	}
	else {
		geometry_constructor(context, fromtext->spatialdb, point_from_coords, NULL, GEOM_POINT, nbArgs, args);
	}
}

static void GPKG_IsAssignable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	FUNCTION_TEXT_ARG(expected_type_name);
	FUNCTION_TEXT_ARG(actual_type_name);

	FUNCTION_START(context);
	FUNCTION_GET_TEXT_ARG(context, expected_type_name, 0);
	FUNCTION_GET_TEXT_ARG(context, actual_type_name, 1);

	geom_type_t expected_type;
	FUNCTION_RESULT = geom_type_from_string(expected_type_name, &expected_type);
	if (FUNCTION_RESULT != SQLITE_OK) {
		error_append(FUNCTION_ERROR, "Invalid geometry type %s", expected_type_name);
		goto exit;
	}

	geom_type_t actual_type;
	FUNCTION_RESULT = geom_type_from_string(actual_type_name, &actual_type);
	if (FUNCTION_RESULT != SQLITE_OK) {
		error_append(FUNCTION_ERROR, "Invalid geometry type %s", actual_type_name);
		goto exit;
	}

	sqlite3_result_int(context, geom_is_assignable(expected_type, actual_type));

	FUNCTION_END(context);
	FUNCTION_FREE_TEXT_ARG(expected_type_name);
	FUNCTION_FREE_TEXT_ARG(actual_type_name);
}

static void GPKG_SpatialDBType(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;

	FUNCTION_START(context);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);

	sqlite3_result_text(context, spatialdb->name, -1, SQLITE_STATIC);

	FUNCTION_END(context);
}

static void GPKG_CheckSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_TEXT_ARG(db_name);
	FUNCTION_INT_ARG(check);
	FUNCTION_INT_ARG(type);
	FUNCTION_START(context);

	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	if (nbArgs == 0) {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
		FUNCTION_SET_INT_ARG(check, 0);
	}
	else if (nbArgs == 1) {
		FUNCTION_GET_TYPE(type, 0);
		if (type == SQLITE_TEXT) {
			FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		}
		else {
			FUNCTION_SET_TEXT_ARG(db_name, "main");
			FUNCTION_GET_INT_ARG(check, 0);
		}
	}
	else {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		FUNCTION_GET_INT_ARG(check, 1);
	}

	if (check != 0) {
		check = SQL_CHECK_ALL;
	}

	FUNCTION_RESULT = spatialdb->check_meta(FUNCTION_DB_HANDLE, db_name, check, FUNCTION_ERROR);
	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_null(context);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_TEXT_ARG(db_name);
	FUNCTION_FREE_INT_ARG(check);
	FUNCTION_FREE_INT_ARG(type);
}

static void GPKG_InitSpatialMetaData(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_TEXT_ARG(db_name);

	FUNCTION_START(context);
	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	if (nbArgs == 0) {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
	}
	else {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
	}

	FUNCTION_START_TRANSACTION(__initspatialdb);
	FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
	FUNCTION_END_TRANSACTION(__initspatialdb);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_null(context);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_TEXT_ARG(db_name);
}

/*
* Supports the following parameter lists:
* 4: table, column, type, srid
* 5: db, table, column, type, srid
* 6: table, column, type, srid, z, m
* 7: db, table, column, type, srid, z, m
*/
static void GPKG_AddGeometryColumn(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_TEXT_ARG(db_name);
	FUNCTION_TEXT_ARG(table_name);
	FUNCTION_TEXT_ARG(column_name);
	FUNCTION_TEXT_ARG(geometry_type);
	FUNCTION_INT_ARG(srs_id);
	FUNCTION_INT_ARG(z);
	FUNCTION_INT_ARG(m);
	FUNCTION_START(context);

	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	if (nbArgs == 4) {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
		FUNCTION_GET_TEXT_ARG(context, table_name, 0);
		FUNCTION_GET_TEXT_ARG(context, column_name, 1);
		FUNCTION_GET_TEXT_ARG(context, geometry_type, 2);
		FUNCTION_GET_INT_ARG(srs_id, 3);
		FUNCTION_SET_INT_ARG(z, 2);
		FUNCTION_SET_INT_ARG(m, 2);
	}
	else if (nbArgs == 5) {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		FUNCTION_GET_TEXT_ARG(context, table_name, 1);
		FUNCTION_GET_TEXT_ARG(context, column_name, 2);
		FUNCTION_GET_TEXT_ARG(context, geometry_type, 3);
		FUNCTION_GET_INT_ARG(srs_id, 4);
	}
	else if (nbArgs == 6) {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
		FUNCTION_GET_TEXT_ARG(context, table_name, 0);
		FUNCTION_GET_TEXT_ARG(context, column_name, 1);
		FUNCTION_GET_TEXT_ARG(context, geometry_type, 2);
		FUNCTION_GET_INT_ARG(srs_id, 3);
		FUNCTION_GET_INT_ARG(z, 4);
		FUNCTION_GET_INT_ARG(m, 5);
	}
	else {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		FUNCTION_GET_TEXT_ARG(context, table_name, 1);
		FUNCTION_GET_TEXT_ARG(context, column_name, 2);
		FUNCTION_GET_TEXT_ARG(context, geometry_type, 3);
		FUNCTION_GET_INT_ARG(srs_id, 4);
		FUNCTION_GET_INT_ARG(z, 5);
		FUNCTION_GET_INT_ARG(m, 6);
	}

	FUNCTION_START_TRANSACTION(__add_geom_col);

	FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);

	if (FUNCTION_RESULT == SQLITE_OK) {
		FUNCTION_RESULT = spatialdb->add_geometry_column(FUNCTION_DB_HANDLE, db_name, table_name, column_name, geometry_type, srs_id, z, m, FUNCTION_ERROR);
	}

	FUNCTION_END_TRANSACTION(__add_geom_col);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_null(context);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_TEXT_ARG(db_name);
	FUNCTION_FREE_TEXT_ARG(table_name);
	FUNCTION_FREE_TEXT_ARG(column_name);
	FUNCTION_FREE_TEXT_ARG(geometry_type);
	FUNCTION_FREE_INT_ARG(srid);
	FUNCTION_FREE_INT_ARG(z);
	FUNCTION_FREE_INT_ARG(m);
}

static void GPKG_CreateTilesTable(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_TEXT_ARG(db_name);
	FUNCTION_TEXT_ARG(table_name);
	FUNCTION_START(context);

	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	if (nbArgs == 2) {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		FUNCTION_GET_TEXT_ARG(context, table_name, 1);
	}
	else {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
		FUNCTION_GET_TEXT_ARG(context, table_name, 0);
	}

	if (spatialdb->create_tiles_table == NULL) {
		error_append(FUNCTION_ERROR, "Tiles tables are not supported in %s mode", spatialdb->name);
		goto exit;
	}

	FUNCTION_START_TRANSACTION(__create_tiles_table);

	FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
	if (FUNCTION_RESULT == SQLITE_OK) {
		FUNCTION_RESULT = spatialdb->create_tiles_table(FUNCTION_DB_HANDLE, db_name, table_name, FUNCTION_ERROR);
	}

	FUNCTION_END_TRANSACTION(__create_tiles_table);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_null(context);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_TEXT_ARG(db_name);
	FUNCTION_FREE_TEXT_ARG(table_name);
}

static void GPKG_CreateSpatialIndex(sqlite3_context *context, int nbArgs, sqlite3_value **args) {
	spatialdb_t *spatialdb;
	FUNCTION_TEXT_ARG(db_name);
	FUNCTION_TEXT_ARG(table_name);
	FUNCTION_TEXT_ARG(geometry_column_name);
	FUNCTION_TEXT_ARG(id_column_name);
	FUNCTION_START(context);

	spatialdb = (spatialdb_t *)sqlite3_user_data(context);
	if (nbArgs == 4) {
		FUNCTION_GET_TEXT_ARG(context, db_name, 0);
		FUNCTION_GET_TEXT_ARG(context, table_name, 1);
		FUNCTION_GET_TEXT_ARG(context, geometry_column_name, 2);
		FUNCTION_GET_TEXT_ARG(context, id_column_name, 3);
	}
	else {
		FUNCTION_SET_TEXT_ARG(db_name, "main");
		FUNCTION_GET_TEXT_ARG(context, table_name, 0);
		FUNCTION_GET_TEXT_ARG(context, geometry_column_name, 1);
		FUNCTION_GET_TEXT_ARG(context, id_column_name, 2);
	}

	if (spatialdb->create_spatial_index == NULL) {
		error_append(FUNCTION_ERROR, "Spatial indexes are not supported in %s mode", spatialdb->name);
		goto exit;
	}

	FUNCTION_START_TRANSACTION(__create_spatial_index);

	FUNCTION_RESULT = spatialdb->init_meta(FUNCTION_DB_HANDLE, db_name, FUNCTION_ERROR);
	if (FUNCTION_RESULT == SQLITE_OK) {
		FUNCTION_RESULT = spatialdb->create_spatial_index(FUNCTION_DB_HANDLE, db_name, table_name, geometry_column_name, id_column_name, FUNCTION_ERROR);
	}

	FUNCTION_END_TRANSACTION(__create_spatial_index);

	if (FUNCTION_RESULT == SQLITE_OK) {
		sqlite3_result_null(context);
	}

	FUNCTION_END(context);

	FUNCTION_FREE_TEXT_ARG(db_name);
	FUNCTION_FREE_TEXT_ARG(table_name);
	FUNCTION_FREE_TEXT_ARG(geometry_column_name);
	FUNCTION_FREE_TEXT_ARG(id_column_name);
}

const spatialdb_t *spatialdb_detect_schema(sqlite3 *db) {
	char message_buffer[256];
	errorstream_t error;
	error_init_fixed(&error, message_buffer, 256);

	const spatialdb_t *schemas[] = {
		spatialdb_geopackage_schema(),
		spatialdb_spatialite4_schema(),
		spatialdb_spatialite3_schema(),
		spatialdb_spatialite2_schema(),
		NULL
	};

	const spatialdb_t **schema = &schemas[0];
	while (*schema != NULL) {
		error_reset(&error);
		(*schema)->check_meta(db, "main", SQL_CHECK_PRIMARY_KEY | SQL_CHECK_NULLABLE, &error);
		if (error_count(&error) == 0) {
			break;
		}
		else {
			schema++;
		}
	}

	if (*schema != NULL) {
		return *schema;
	}
	else {
		return schemas[0];
	}
}

#define STR(x) #x

#define SPATIALDB_FUNCTION(db, pre, name, args, flags, spatialdb, err)                                                 \
  do {                                                                                                                 \
    sql_create_function(db, STR(name), pre##_##name, args, flags, (void*)spatialdb, NULL, err);                        \
    sql_create_function(db, STR(pre##_##name), pre##_##name, args, flags, (void*)spatialdb, NULL, err);                \
  } while (0)

#define SPATIALDB_ALIAS(db, pre, name, func, args, flags, spatialdb, err)                                              \
  do {                                                                                                                 \
    sql_create_function(db, STR(name), pre##_##func, args, flags, (void*)spatialdb, NULL, err);                        \
    sql_create_function(db, STR(pre##_##name), pre##_##func, args, flags, (void*)spatialdb, NULL, err);                \
  } while (0)

#define FROMTEXT_FUNCTION(db, pre, name, args, flags, ft, err)                                                         \
  do {                                                                                                                 \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(name), pre##_##name, args, flags, ft, (void(*)(void*))fromtext_release, err);          \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(pre##_##name), pre##_##name, args, flags, ft, (void(*)(void*))fromtext_release, err);  \
  } while (0)

#define FROMTEXT_ALIAS(db, pre, name, func, args, flags, ft, err)                                                      \
  do {                                                                                                                 \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(name), pre##_##func, args, flags, (void*)ft, (void(*)(void*))fromtext_release, err);   \
    fromtext_acquire(fromtext);                                                                                        \
    sql_create_function(db, STR(pre##_##name), pre##_##func, args, flags, (void*)ft, (void(*)(void*))fromtext_release, err);  \
  } while (0)

/*
** This function registered all of the above C functions as SQL
** functions.  This should be the only routine in this file with
** external linkage.
*/
int RegisterExtensionFunctions(sqlite3 *db) {
	static const struct FuncDef {
		char *zName;
		signed char nArg;
		u8 argType;           /* 0: none.  1: db  2: (-1) */
		u8 eTextRep;          /* 1: UTF-16.  0: UTF-8 */
		u8 needCollSeq;
		void(*xFunc)(sqlite3_context*, int, sqlite3_value **);
	} aFuncs[] = {
		/* math.h */
		{ "acos",               1, 0, SQLITE_UTF8,    0, acosFunc },
		{ "asin",               1, 0, SQLITE_UTF8,    0, asinFunc },
		{ "atan",               1, 0, SQLITE_UTF8,    0, atanFunc },
		{ "atn2",               2, 0, SQLITE_UTF8,    0, atn2Func },
		/* XXX alias */
		{ "atan2",              2, 0, SQLITE_UTF8,    0, atn2Func },
		{ "acosh",              1, 0, SQLITE_UTF8,    0, acoshFunc },
		{ "asinh",              1, 0, SQLITE_UTF8,    0, asinhFunc },
		{ "atanh",              1, 0, SQLITE_UTF8,    0, atanhFunc },

		{ "difference",         2, 0, SQLITE_UTF8,    0, differenceFunc },
		{ "degrees",            1, 0, SQLITE_UTF8,    0, rad2degFunc },
		{ "radians",            1, 0, SQLITE_UTF8,    0, deg2radFunc },

		{ "cos",                1, 0, SQLITE_UTF8,    0, cosFunc },
		{ "sin",                1, 0, SQLITE_UTF8,    0, sinFunc },
		{ "tan",                1, 0, SQLITE_UTF8,    0, tanFunc },
		{ "cot",                1, 0, SQLITE_UTF8,    0, cotFunc },
		{ "cosh",               1, 0, SQLITE_UTF8,    0, coshFunc },
		{ "sinh",               1, 0, SQLITE_UTF8,    0, sinhFunc },
		{ "tanh",               1, 0, SQLITE_UTF8,    0, tanhFunc },
		{ "coth",               1, 0, SQLITE_UTF8,    0, cothFunc },

		{ "exp",                1, 0, SQLITE_UTF8,    0, expFunc },
		{ "log",                1, 0, SQLITE_UTF8,    0, logFunc },
		{ "log10",              1, 0, SQLITE_UTF8,    0, log10Func },
		{ "power",              2, 0, SQLITE_UTF8,    0, powerFunc },
		{ "sign",               1, 0, SQLITE_UTF8,    0, signFunc },
		{ "sqrt",               1, 0, SQLITE_UTF8,    0, sqrtFunc },
		{ "square",             1, 0, SQLITE_UTF8,    0, squareFunc },

		{ "ceil",               1, 0, SQLITE_UTF8,    0, ceilFunc },
		{ "floor",              1, 0, SQLITE_UTF8,    0, floorFunc },

		{ "pi",                 0, 0, SQLITE_UTF8,    1, piFunc },


		/* string */
		{ "replicate",          2, 0, SQLITE_UTF8,    0, replicateFunc },
		{ "charindex",          2, 0, SQLITE_UTF8,    0, charindexFunc },
		{ "charindex",          3, 0, SQLITE_UTF8,    0, charindexFunc },
		{ "leftstr",            2, 0, SQLITE_UTF8,    0, leftFunc },
		{ "rightstr",           2, 0, SQLITE_UTF8,    0, rightFunc },
		{ "reverse",            1, 0, SQLITE_UTF8,    0, reverseFunc },
		{ "proper",             1, 0, SQLITE_UTF8,    0, properFunc },
		{ "padl",               2, 0, SQLITE_UTF8,    0, padlFunc },
		{ "padr",               2, 0, SQLITE_UTF8,    0, padrFunc },
		{ "padc",               2, 0, SQLITE_UTF8,    0, padcFunc },
		{ "strfilter",          2, 0, SQLITE_UTF8,    0, strfilterFunc },

	};
	/* Aggregate functions */
	static const struct FuncDefAgg {
		char *zName;
		signed char nArg;
		u8 argType;
		u8 needCollSeq;
		void(*xStep)(sqlite3_context*, int, sqlite3_value**);
		void(*xFinalize)(sqlite3_context*);
	} aAggs[] = {
		{ "stdev",            1, 0, 0, varianceStep, stdevFinalize },
		{ "variance",         1, 0, 0, varianceStep, varianceFinalize },
		{ "mode",             1, 0, 0, modeStep,     modeFinalize },
		{ "median",           1, 0, 0, modeStep,     medianFinalize },
		{ "lower_quartile",   1, 0, 0, modeStep,     lower_quartileFinalize },
		{ "upper_quartile",   1, 0, 0, modeStep,     upper_quartileFinalize },
	};
	int i;

	for (i = 0; i<sizeof(aFuncs) / sizeof(aFuncs[0]); i++) {
		void *pArg = 0;
		switch (aFuncs[i].argType) {
		case 1: pArg = db; break;
		case 2: pArg = (void *)(-1); break;
		}
		//sqlite3CreateFunc
		/* LMH no error checking */
		sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
			aFuncs[i].eTextRep, pArg, aFuncs[i].xFunc, 0, 0);
#if 0
		if (aFuncs[i].needCollSeq) {
			struct FuncDef *pFunc = sqlite3FindFunction(db, aFuncs[i].zName,
				strlen(aFuncs[i].zName), aFuncs[i].nArg, aFuncs[i].eTextRep, 0);
			if (pFunc && aFuncs[i].needCollSeq) {
				pFunc->needCollSeq = 1;
			}
		}
#endif
	}

	for (i = 0; i<sizeof(aAggs) / sizeof(aAggs[0]); i++) {
		void *pArg = 0;
		switch (aAggs[i].argType) {
		case 1: pArg = db; break;
		case 2: pArg = (void *)(-1); break;
		}
		//sqlite3CreateFunc
		/* LMH no error checking */
		sqlite3_create_function(db, aAggs[i].zName, aAggs[i].nArg, SQLITE_UTF8,
			pArg, 0, aAggs[i].xStep, aAggs[i].xFinalize);
#if 0
		if (aAggs[i].needCollSeq) {
			struct FuncDefAgg *pFunc = sqlite3FindFunction(db, aAggs[i].zName,
				strlen(aAggs[i].zName), aAggs[i].nArg, SQLITE_UTF8, 0);
			if (pFunc && aAggs[i].needCollSeq) {
				pFunc->needCollSeq = 1;
			}
		}
#endif
	}

	errorstream_t error;
	if (error_init(&error) != SQLITE_OK) {
		return SQLITE_ERROR;
	}

	spatialdb_t *spatialdb = NULL;
	spatialdb = spatialdb_detect_schema(db);


	if (spatialdb->init != NULL) {
		spatialdb->init(db, spatialdb, &error);
	}

	SPATIALDB_FUNCTION(db, ST, MinX, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MaxX, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MinY, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MaxY, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MinZ, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MaxZ, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MinM, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, MaxM, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, SRID, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, SRID, 2, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, Is3d, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, IsEmpty, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, IsMeasured, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, CoordDim, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, GeometryType, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, AsBinary, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, GeomFromWKB, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, GeomFromWKB, 2, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_ALIAS(db, ST, WKBToSQL, GeomFromWKB, 1, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_ALIAS(db, ST, WKBToSQL, GeomFromWKB, 2, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, ST, AsText, 1, SQL_DETERMINISTIC, spatialdb, &error);

	fromtext_t *fromtext = fromtext_init(spatialdb);
	if (fromtext != NULL) {
		FROMTEXT_FUNCTION(db, ST, GeomFromText, 1, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_FUNCTION(db, ST, GeomFromText, 2, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, WKTToSQL, GeomFromText, 1, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, WKTToSQL, GeomFromText, 2, SQL_DETERMINISTIC, fromtext, &error);

		FROMTEXT_FUNCTION(db, ST, Point, 1, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, MakePoint, Point, 1, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_FUNCTION(db, ST, Point, 2, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, MakePoint, Point, 2, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_FUNCTION(db, ST, Point, 3, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, MakePoint, Point, 3, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_FUNCTION(db, ST, Point, 4, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, MakePoint, Point, 4, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_FUNCTION(db, ST, Point, 5, SQL_DETERMINISTIC, fromtext, &error);
		FROMTEXT_ALIAS(db, ST, MakePoint, Point, 5, SQL_DETERMINISTIC, fromtext, &error);

		fromtext_release(fromtext);
	}
	else {
		error_append(&error, "Could not create fromtext function context");
	}

	SPATIALDB_FUNCTION(db, GPKG, IsAssignable, 2, SQL_DETERMINISTIC, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 0, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 1, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CheckSpatialMetaData, 2, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, InitSpatialMetaData, 0, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, InitSpatialMetaData, 1, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 4, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 5, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 6, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, AddGeometryColumn, 7, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CreateTilesTable, 1, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CreateTilesTable, 2, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CreateSpatialIndex, 3, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, CreateSpatialIndex, 4, 0, spatialdb, &error);
	SPATIALDB_FUNCTION(db, GPKG, SpatialDBType, 0, 0, spatialdb, &error);

	int result;
	if (error_count(&error) == 0) {
		result = SQLITE_OK;
	}
	else {
		result = SQLITE_ERROR;
	}
	error_destroy(&error);
	return result;
}

SQLITE_EXTENSION_INIT1

#ifdef COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE
__declspec(dllexport) 
int sqlite3_extension_init(
    sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi)
{
	SQLITE_EXTENSION_INIT2(pApi);
	RegisterExtensionFunctions(db);
	return 0;
}
#endif /* COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE */

map map_make(cmp_func cmp)
{
  map r;
  r.cmp=cmp;
  r.base = 0;
  return r;
}

void* xcalloc(size_t nmemb, size_t size, char* s){
  void* ret = calloc(nmemb, size);
  return ret;
}

void xfree(void* p){
  free(p);
}

void node_insert(node** n, cmp_func cmp, void *e){
  int c;
  node* nn;
  if(*n==0){
    nn = (node*)xcalloc(1,sizeof(node), "for node");
    nn->data = e;
    nn->count = 1;
    *n=nn;
  }else{
    c=cmp((*n)->data,e);
    if(0==c){
      ++((*n)->count);
      xfree(e);
    }else if(c>0){
      /* put it right here */
      node_insert(&((*n)->l), cmp, e);
    }else{
      node_insert(&((*n)->r), cmp, e);
    }
  }
}

void map_insert(map *m, void *e){
  node_insert(&(m->base), m->cmp, e);
}

void node_iterate(node *n, map_iterator iter, void* p){
  if(n){
    if(n->l)
      node_iterate(n->l, iter, p);
    iter(n->data, n->count, p);
    if(n->r)
      node_iterate(n->r, iter, p);
  }
}

void map_iterate(map *m, map_iterator iter, void* p){
  node_iterate(m->base, iter, p);
}

void node_destroy(node *n){
  if(0!=n){
    xfree(n->data);
    if(n->l)
      node_destroy(n->l);
    if(n->r)
      node_destroy(n->r);

    xfree(n);
  }
}

void map_destroy(map *m){
  node_destroy(m->base);
}

int int_cmp(const void *a, const void *b){
  int64_t aa = *(int64_t *)(a);
  int64_t bb = *(int64_t *)(b);
  /* printf("cmp %d <=> %d\n",aa,bb); */
  if(aa==bb)
    return 0;
  else if(aa<bb)
    return -1;
  else
    return 1;
}

int double_cmp(const void *a, const void *b){
  double aa = *(double *)(a);
  double bb = *(double *)(b);
  /* printf("cmp %d <=> %d\n",aa,bb); */
  if(aa==bb)
    return 0;
  else if(aa<bb)
    return -1;
  else
    return 1;
}

void print_elem(void *e, int64_t c, void* p){
  int ee = *(int*)(e);
  printf("%d => %lld\n", ee,c);
}

