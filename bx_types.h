#ifndef BX_TYPES_H
#define BX_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef BX_TYPES_G
# define vextern
#else
# define vextern extern
#endif

#if (defined(__TURBOC__) || defined(GO32))
#ifndef MSDOS
#define MSDOS
#endif
#undef UNIX
#undef MAC
#endif

#ifdef MSDOS
#define BIN_RD "rb"
#define TXT_RD "rt"
#define BIN_WR "wb"
#define TXT_WR "wt"
#define c_sep           '/'   /* command line separator */
#define C_SEP           "/"   /* command line separator */
#define d_sep           '\\'  /* directory separator    */
#else
#define BIN_RD "r"
#define TXT_RD "r"
#define BIN_WR "w"
#define TXT_WR "w"
#define c_sep           '-'   /* command line separator */
#define C_SEP           "-"   /* command line separator */
#define d_sep           '/'  /* directory separator    */
#endif


#define oPS             '1'
#define oEPS            '2'
#define oHPGL           '3'
#define oRTF            '4'
#if (defined(__TURBOC__) || defined(GO32)) && !defined(_Windows)
#define oCRT            '5'
#endif
#define oANSI           '6'
#define oVT             '7'
#define oREGISt         '8'
#define oREGISp         '9'
#define oLJ250          'a'
#define oASCII          'b'
#define oFIG            'c'
#define oPICT           'd'
#define oHTML           'e'


#ifdef oCRT
#define allowed_devices  "123456789abcde"
#else
#define allowed_devices  "12346789abcde"
#endif

/* #if defined(__TURBOC__) && !defined(_Windows)
  #define MAX_NO_RES    5000
  #else */
#define MAX_NO_RES      10000
/* #endif */

#define MAX_NO_SEQ      150
vextern unsigned max_no_seq;
vextern unsigned max_no_res;

/* if one allows a large number of residues, and someone sets a short line
   length, there will be too many blocks. Therefore, the minimum linelength
   is set here equal to max_no_res/max_no_block to prevent errors in accessing
   the prenum array. */
#define min_outlen      ((long)((double)max_no_res / max_no_block))

#define max_no_block    400

#define max_outlen      255
#define max_linelen     255   /* defines linetype */
#define seqnamelen      40    /* defines sseqnametype */
#define seqnumlen       4     /* defines numtype */

#define idpref          1
#define simpref         1


typedef int BOOL;
#ifndef TRUE
#define TRUE  ((BOOL)(1==1))
#endif
#ifndef FALSE
#define FALSE ((BOOL)(1==0))
#endif

#define YESNO(fl) ((fl) ? 'y' : 'n')

typedef struct {
  FILE *f;
  BOOL bin;
} MYFILE;

typedef unsigned char byte;

typedef char **seqtype;

typedef byte **coltype;

#define max_prenum_len (seqnumlen+1)
typedef char numtype[max_prenum_len];
typedef numtype **prenumtype;

typedef char linetype[256];

typedef char sseqnametype[41];

typedef sseqnametype seqnametype[MAX_NO_SEQ];


typedef BOOL cmptype[20][20];
typedef char filenametype[256];

typedef char prestring[21];

typedef char txtstring[21];

typedef char parstringtype[6];

typedef enum {
  Init, SetCol, CharOut, NewLine, NewPage, Exit
} actiontype;

typedef enum {
  rTERM, rPAGE
} regmodetype;

typedef enum {
  EOL_default, EOL_unix, EOL_mac, EOL_dos
} EOLstate;

typedef char cltagtype[21];

typedef char cllinetype[256];

extern char *aaset, *aasetlow;

extern MYFILE outfile;
extern FILE *parfile;

vextern char dummystring[81];

/* vextern int errcode;*/   /*turbo-specific IO-error indicator*/

vextern seqtype seq;              /*all sequences (characters)*/
vextern int consenslen;          /*maximum sequence length   */
vextern int seqlen[MAX_NO_SEQ];  /*all sequence lengths*/
vextern int startno[MAX_NO_SEQ]; /*all sequence starts*/
vextern prenumtype prenum;       /*blocknumbers for output*/
vextern coltype col;             /*all sequences (cell colors)*/
vextern char *cons;              /*the consensus sequence*/
vextern char *conschar;          /*line for consensus display*/
vextern seqnametype seqname;     /*all sequence names*/

vextern linetype line_;          /* line input bufer */

vextern int thr;
vextern double thrfrac;

vextern int no_seq;   /*how many sequences*/
vextern int outlen;   /*length of an output block*/
vextern int seqname_outlen;   /*output length of seqnames*/
vextern int no_blocks;   /*output how many blocks*/
vextern int lines_per_page;   /*how many lines per page*/
vextern int lines_left;   /*how many lines left to print*/

vextern filenametype inname, outname, identname, simname, grpname, parname,
		     clsimname, clgrpname;
    /*misc. filenames*/
vextern cmptype simtable, grptable;

vextern char outputmode;
    /*PS, EPS, HPGL, RTF, (CRT), ANSI, VT100, ReGISterm*/
    /*ReGISprint, LJ250, ASCII, PICT*/

vextern char inputmode;   /*1=Lineup, 2=Clustal, 3=Maligned, 4=ESEE*/

vextern int consensnum;   /* sequence to make consensus to */

vextern BOOL lc[5];

vextern BOOL ident_sim, simflag;   /*mark similar residues?*/
vextern BOOL globalflag;           /*mark globally consvd. residues?*/
vextern int interlines;            /*how many lines between blocks*/
vextern BOOL seqnameflag;          /*sequence names to be printed*/
vextern BOOL seqnumflag;           /*position numbers to be printed*/
vextern BOOL rulerflag;            /*print ruler?*/
vextern BOOL seqconsflag;          /*consensus to a master sequence*/
vextern BOOL hideflag;             /*hide master sequence*/
vextern BOOL masternormal;         /*print master sequence all-normal*/
vextern BOOL dnaflag;              /*DNA or proteins to be aligned*/
vextern BOOL interactflag;         /*interaction with user?*/
vextern BOOL clinflag;             /*input filename  in commandline?*/
vextern BOOL cloutflag;            /*output filename in commandline?*/
vextern BOOL clparflag;            /*param filename  in commandline?*/
vextern BOOL clsimflag;            /*file name for similar residues in cmdline?*/
vextern BOOL clgrpflag;            /*file name for grouping residues in cmdline?*/
vextern BOOL cltypeflag;           /*input file type in commandline?*/
vextern BOOL cldevflag;            /*output device type in comline?*/
vextern BOOL clconsflag;           /*cons requested in commandline?*/
vextern BOOL clsymbconsflag;       /*symbcons defined in commandline?*/
vextern BOOL clthrflag;            /*threshold defined in commandline?*/
vextern BOOL consflag;             /*display consensus line?*/
vextern BOOL clseqconsflag;        /*sequence to compare others to specified
							    in command line?*/
vextern BOOL splitflag;            /*multiple file output?*/
vextern BOOL numdefflag;           /*default numbering?*/

vextern EOLstate EOLmode;          /* select EOL code of text output */

vextern BOOL shadeonlyflag;        /*write only the shading to PICT file?*/
vextern char symbcons[3];          /*symbols for consensus*/

vextern double dev_maxx, dev_maxy; /*maximal device coordinates*/
vextern double dev_miny, dev_minx; /*minimal device coordinates*/
vextern double dev_ysize,dev_xsize;/*character sizes for device*/

vextern int act_color;             /*actual color*/
vextern int act_page;              /*actual page no*/

vextern BOOL landscapeflag;

#define depend_mode "tp"
extern char depend_err[];

#undef min
extern int min(int a, int b);

extern BOOL fexist(char *fname);
extern int indx(char *mainstring, char *substring);

extern MYFILE *outopen(MYFILE *mf, char *fn);
extern void uwriteln(MYFILE *f, char *fmt, ...);

extern void term_par(char *term);

extern int str2int(void *s);
extern double str2real(void *s);

extern void save_binpath(char *arg0);
extern char *get_logical(char *logical);

extern void *Malloc(size_t sze);
#define Free(p) do {    \
 if ((*(p)) != NULL) {  \
   free(*(p));          \
   (*(p)) = NULL;       \
 }                      \
} while (0)

/*extern void Free(void *p);*/

#undef vextern

#endif /*BX_TYPES_H*/

