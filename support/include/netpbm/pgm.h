/* pgm.h - header file for libpgm portable graymap library
*/

#ifndef _PGM_H_
#define _PGM_H_

#include "pbm.h"

/* The following definition has nothing to do with the format of a PGM file */
typedef unsigned int gray;

/* Since April 2000, we are capable of reading and generating raw
   (binary) PGM files with maxvals up to 65535.  However, before that
   the maximum (as usually implemented) was 255, and people still want
   to generate files with a maxval of no more than 255 in most cases
   (because then old Netpbm programs can process them, and they're
   only half as big).

   So we keep PGM_MAXMAXVAL = 255, even though it's kind of a misnomer.  

   Note that one could always write a file with maxval > PGM_MAXMAXVAL and
   it would just go into plain (text) format instead of raw (binary) format.
   Along with the expansion to 16 bit raw files, we took away that ability.
   Unless you specify 'forceplain' on the pgm_writepgminit() call, it will
   fail if you specify a maxval > PGM_OVERALLMAXVAL.  I made this design
   decision because I don't think anyone really wants to get a plain format
   file with samples larger than 65535 in it.  However, it should be possible
   just to increase PGM_OVERALLMAXVAL and get that old function back for
   maxvals that won't fit in 16 bits.  I think the only thing really
   constraining PGM_OVERALLMAXVAL is the size of the 'gray' data structure,
   which is generally 32 bits.
*/

#define PGM_OVERALLMAXVAL 65535
#define PGM_MAXMAXVAL 255


/* Magic constants. */

#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)
#define PGM_TYPE PGM_FORMAT

/* For the alpha-mask variant of PGM: */
#define PGM_TRANSPARENT 0

/* Macro for turning a format number into a type number. */

#define PGM_FORMAT_TYPE(f) ((f) == PGM_FORMAT || (f) == RPGM_FORMAT ? PGM_TYPE : PBM_FORMAT_TYPE(f))


/* Declarations of routines. */

void pgm_init ARGS(( int* argcP, char* argv[] ));

#define pgm_allocarray( cols, rows ) ((gray**) pm_allocarray( cols, rows, sizeof(gray) ))
#define pgm_allocrow( cols ) ((gray*) pm_allocrow( cols, sizeof(gray) ))
#define pgm_freearray( grays, rows ) pm_freearray( (char**) grays, rows )
#define pgm_freerow( grayrow ) pm_freerow( (char*) grayrow )

gray** pgm_readpgm ARGS(( FILE* file, int* colsP, int* rowsP, gray* maxvalP ));
void pgm_readpgminit ARGS(( FILE* file, int* colsP, int* rowsP, gray* maxvalP, int* formatP ));
void pgm_readpgmrow ARGS(( FILE* file, gray* grayrow, int cols, gray maxval, int format ));

void pgm_writepgm ARGS(( FILE* file, gray** grays, int cols, int rows, gray maxval, int forceplain ));
void pgm_writepgminit ARGS(( FILE* file, int cols, int rows, gray maxval, int forceplain ));
void pgm_writepgmrow ARGS(( FILE* file, gray* grayrow, int cols, gray maxval, int forceplain ));


void
pgm_nextimage(FILE * const file, int * const eofP);

void
pgm_check(FILE * file, const enum pm_check_type check_type, 
          const int format, const int cols, const int rows, const int maxval,
          enum pm_check_code * const retval_p);

#endif /*_PGM_H_*/
