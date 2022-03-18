/*-----------------------------------------------------------------------
		      BOXSHADE 3.3       (May 1997)

this program makes multiple-aligned output from either
 - PRETTY files from old LINEUP
 - MSF files from new PILEUP
 - CLUSTAL- .ALN  files
 - MALIGNED data files
 - ESEE files
 - PHYLIP files
various kinds of shading can be applied to identical/similar residues
Output formats supported are
 - ANSI for display on terminal with ANSI.SYS loaded (PC version)
 - VT100 for display on VT100 compatible terminals
 - CRT for display on the main screen ( * PC-version only! * )
 - POSTSCRIPT (TM)
 - Encapsulated POSTSCRIPT (for export to graphics programs)
 - HPGL for output on plotters or export to graphics programs
 - RTF (Rich text format) for export to word processing programs
 - ReGIS graphics for disply on terminals with ReGIS capabilities (DEC)
 - ReGIS file for printing after conversion with RETOS
 - LJ250 coloir printer format (DEC)
 - ASCII output showing either conserved or variable residues
 - FIG file output for use with xfig 2.1 program
 - PICT file, mostly used by Mac graphics progs, also many PC progs
 - HTML output for Web publishing

This program might still contain bugs and is not particularly
user-friendly. It is completely public-domain and may be
passed around and modified without any notice to the author.

the authors addresses are:

  Kay Hofmann                              Michael D. Baron
  Bioinformatics Group                     BBSRC Institute for Animal Health
  ISREC                                    Pirbright, Surrey GU24 0NF
  CH-1066 Epalinges s/Lausanne             U.K.
  Switzerland

C port by

  Hartmut Schirmer
  Technische Fakultaet
  Kaiserstr. 2
  D-24143 Kiel
  Germany

for any comments write an E-mail to
  Kay:     khofmann@isrec-sun1.unil.ch
  Michael: baron@bbsrc.ac.uk (though kay wrote 99% of the program and is more
	      likely to be of help, especially with input and output problems)
  Hartmut: hsc@techfak.uni-kiel.de (don't send Kay or Michael any questions
				    concerning the 'C' version of boxshade)

MODIFICATION HISTORY
3.3   -converted from Pascal to C, includes several bug fixes over Pascal v3.21,
       mostly in page switching code. Rewritten Postscript driver: Compact
       output following the document structure conventions (DSC), additional
       color mode added. Enhanced memory allocation allows bigger alignments
       to be processed. Added ruler feature and /grp & /sim command line
       parameter. Added HTML output.
3.2   -added PICT output, somewhat experimental, all feedback gratefully
       received (by MDB). There are two options, either to output the Text
       with the shading (T) or just the shading (S); the latter option is for those
       who find the full file too big for their computer/program, and want to
       combine the shading with a simple formatted text version of their multiple
       sequence file e.g. the output from PRETTY (remember to set the block
       size equal to the linelength).
3.0   -major changes to shading strategy introduced by Michael Baron,
       see Documentation for details
      -addition of a new set of files (*.grp) and a few new command
       line qualifiers necessary for this new strategy
      -added support for ASCII output, showing one complete sequence
       and either conserved or deviating residues in the other sequences
      -added FIG output for use with the x-windows based public domain
       graphics program xfig 2.1
      -bug fixes in the reading routine, boxshade can now import files
       created by clustalW and the MSF format created by readseq

2.7   -unified source code for VAX/VMS, AXP/OSF1 and MSDOS (Turbo Pascal)
      -all features of previous 2.6DOS version available for OSF1 and VMS
      -reads parameter and alignment files in both DOS and UNIX format
      -writes output files in native format of the machine. UNIX or DOS
       style text files can be forced by the qualifiers /unix or /dos
      -added /check qualifier for listing of command line parameters
      -added the option to hide the master sequence (see documentation)
      -finally dropped UIS support
      -minor bug fixes

2.6DOS   -added RTF output

2.5DOS   -added HPGL output
	 -added consensus formation and output
	 -added the /NUMDEF qualifier for default numbering
	 -added the /SPLIT qualifier for splitting output to one page per file.
	 -added the /CONS and
		    /SYMBCONS qualifiers for output of consensus sequence

2.4DOS   -initial MSDOS version
	 -ported from VMS version 2.4
	 -added CRT support

2.4a  -fixed a bug (terminal size problems)

2.4   -modified POSTSCRIPT/EPS output, allows scaling of fonts.
       (.PAR file structure modified, BOX.PSP now obsolete)
      -support of vertical POSTSCRIPT output
      -support of output on VT-series terminals
      -support of input from CLUSTAL V
      -slight reordering of interactive input
      -minor bug in command line handling fixed
      -minor bug in EPS-bounding box fixed

2.3   -default parameter management
      -option for shading according to similarity to a        instead of a consensus sequence
      -support of GCG V7 MSF-format

2.2   -support of sequence numbering in output
      -support of MALIGNED data files

2.1   -output code completely rewritten for allowing easier modification
      -support of ENCAPSULATED POSTSCRIPT output
      -more compact POSTSCRIPT output files
      -multipage output for all devices
      -modifications in PRETTY and CLUSTAL reading routines
       for coping with slightly altered formats

2.0   -added POSTSCRIPT and ReGIS output routines
      -added a version that compiles without VWS (BOX_NO_UIS)
      -some minor bugfixes
      -creation of a .COM file for setting logicals

1.0   -initial version
      -CLUSTAL and PRETTY input
      -UIS/VWS and LJ250 sixel output

----------------------------------------------------------------------- */


#define BX_TYPES_G
#include "bx_types.h"

#include "bx_read.h"
#include "dv_all.h"

#include "version.h"

#define DNAPEP()  (dnaflag ? "dna" : "pep")

char *aaset    = "ACDEFGHIKLMNPQRSTVWY";
char *aasetlow = "acdefghiklmnpqrstvwy";

char *argv0=NULL;

static GraphicsDevice *OutDev = NULL;

static int cons_idx = -1;
static int ruler_idx = -1;

/*--------------------------------------------------------------------------*/
/*   user query section section                                             */
/*--------------------------------------------------------------------------*/

static char *explain_cl(char *cl)
{
  static char ncl[256];

  printf("Allowed command line parameters are:\n");
  printf(C_SEP "help           show this text\n");
  printf(C_SEP "check          show this text and extend command line\n");
  printf(C_SEP "def            use defaults, no unnecessary questions\n");
  printf(C_SEP "numdef         use default numbering\n");
  printf(C_SEP "dna            assume DNA sequences, use box_dna.par\n");
  printf(C_SEP "split          create separate files for multiple pages\n");
  printf(C_SEP "toseq=xxx      shading according to sequence No. xxx\n");
  printf(C_SEP "in=xxxxx       xxxxx is input file name\n");
  printf(C_SEP "out=xxxxx      xxxxx is output file name\n");
  printf(C_SEP "par=xxxxx      xxxxx is parameter file name\n");
  printf(C_SEP "sim=xxxxx      xxxxx is file name for similar residues def.\n");
  printf(C_SEP "grp=xxxxx      xxxxx is file name for grouping residues def.\n");
  printf(C_SEP "thr=x          x is the fraction of sequences that must agree"
			     " for a consensus\n");
  printf(C_SEP "dev=x          x is output device class (see documentation)\n");
  printf(C_SEP "type=x         x is input file format   (see documentation)\n");
  printf(C_SEP "ruler          print ruler line\n");
  printf(C_SEP "cons           create consensus line\n");
  printf(C_SEP "symbcons=xyz   xyz are consensus symbols\n");
  printf(C_SEP "symbcons=\"xyz\"    if the one above does not work, try this one\n");
  printf(C_SEP "unix           output files lines are terminated with LF only\n");
  printf(C_SEP "mac            output files lines are terminated with CR only\n");
  printf(C_SEP "dos            output files lines are terminated with CRLF\n");
/*  printf("On unix systems, use the dash (-) as parameter delimiter\n\n"); */
  if (cl != NULL) {
    printf("actual command line: %s\n", cl);
    printf("add to command line: ");
    Gets(ncl);
  } else
    *ncl = '\0';
  return ncl;
}

static char *get_cl_filename(char *Result, char *cl, char *tag)
{
  char *p, *r;
  int p1;

  p1 = indx(cl, tag);
  p = cl + p1 + strlen(tag) - 1;
  r = Result;
  while (*p != '\0' && *p != ' ' && *p != c_sep)
    *(r++) = *(p++);
  *r = '\0';
  return Result;
}


static double get_cl_real(char *cl, char *tag)
{
  char *p, *r;
  int p1;
  char dummy[256];

  p1 = indx(cl, tag);
  p = cl + p1 + strlen(tag) - 1;
  r = dummy;
  while (*p != '\0' && *p != ' ' && *p != c_sep)
    *(r++) = *(p++);
  *r = '\0';
  return( str2real(dummy) );
}


static int get_cl_int(char *cl, char *tag)
{
  char *p, *r;
  int p1;
  char dummy[256];

  p1 = indx(cl, tag);
  p = cl + p1 + strlen(tag) - 1;
  r = dummy;
  while (*p != '\0' && *p != ' ' && *p != c_sep)
    *(r++) = *(p++);
  *r = '\0';
  return( str2int(dummy) );
}


static void process_command_line(int argc, char **argv)
{
  char cl[500];
  int incr, idx;

  save_binpath(argv[0]);

  *cl = '\0';
  for (idx=1; idx < argc; idx++) {
    strcat(cl, argv[idx]);
    strcat(cl, " ");
  }

  if ( indx(cl, C_SEP "help") > 0) {
    explain_cl(NULL);
    exit(0);
  }

  if ( indx(cl, C_SEP "check") > 0) {
    strcat(cl, " ");
    strcat(cl, explain_cl(cl) );
  }
  if (indx(cl, C_SEP "dna") > 0)
    dnaflag = TRUE;
  else
    dnaflag = FALSE;
  if (indx(cl, C_SEP "def") > 0)
    interactflag = FALSE;
  else
    interactflag = TRUE;

  if (indx(cl, C_SEP "in") > 0) {
    clinflag = TRUE;
    get_cl_filename(inname, cl, C_SEP "in=");
  } else
    clinflag = FALSE;

  if (indx(cl, C_SEP "out=") > 0) {
    cloutflag = TRUE;
    get_cl_filename(outname, cl, C_SEP "out=");
  } else
    cloutflag = FALSE;

  if (indx(cl, C_SEP "par=") > 0) {
    clparflag = TRUE;
    get_cl_filename(parname, cl, C_SEP "par=");
  } else
    clparflag = FALSE;

  if (indx(cl, C_SEP "sim=") > 0) {
    clsimflag = TRUE;
    get_cl_filename(clsimname, cl, C_SEP "sim=");
  } else
    clsimflag = FALSE;

  if (indx(cl, C_SEP "grp=") > 0) {
    clgrpflag = TRUE;
    get_cl_filename(clgrpname, cl, C_SEP "grp=");
  } else
    clgrpflag = FALSE;

  if ( (idx=indx(cl, C_SEP "type=")) > 0) {
    cltypeflag = TRUE;
    inputmode = cl[idx + 5];
  } else
    cltypeflag = FALSE;

  if (indx(cl, C_SEP "thr=") > 0) {
    clthrflag = TRUE;
    thrfrac = get_cl_real(cl, C_SEP "thr=");
    if ((unsigned)thrfrac > 1)
      clthrflag = FALSE;
  } else
    clthrflag = FALSE;

  if (indx(cl, C_SEP "toseq=") > 0) {
    clseqconsflag = TRUE;
    seqconsflag = TRUE;
    consensnum = get_cl_int(cl, C_SEP "toseq=");
  } else
    clseqconsflag = FALSE;

  if ( (idx=indx(cl, C_SEP "dev=")) > 0) {
    cldevflag = TRUE;
    outputmode = cl[idx + 4];
  } else
    cldevflag = FALSE;

  if (indx(cl, C_SEP "cons") > 0) {
    consflag = TRUE;
    clconsflag = TRUE;
  } else
    clconsflag = FALSE;

  if (indx(cl, C_SEP "ruler") > 0) {
    rulerflag = TRUE;
  } else
    rulerflag = FALSE;

  if ( (idx=indx(cl, C_SEP "symbcons=")) > 0) {
    clsymbconsflag = TRUE;
    consflag = TRUE;
    clconsflag = TRUE;
    incr = 0;
    if (cl[idx + 9] == '"' ||
	cl[idx + 9] == '\'')
      incr = 1;
    symbcons[0] = cl[idx + incr + 9];
    symbcons[1] = cl[idx + incr + 10];
    symbcons[2] = cl[idx + incr + 11];
  } else
    clsymbconsflag = FALSE;

  if (indx(cl, C_SEP "split") > 0)
    splitflag = TRUE;
  else
    splitflag = FALSE;

  EOLmode = EOL_default;
  if (indx(cl, C_SEP "unix" ) > 0) EOLmode = EOL_unix; else
  if (indx(cl, C_SEP "dos") > 0)   EOLmode = EOL_dos;  else
  if (indx(cl, C_SEP "mac") > 0)   EOLmode = EOL_mac;

  if (indx(cl, C_SEP "numdef") > 0 || !interactflag)
    numdefflag = TRUE;
  else
    numdefflag = FALSE;
}

static BOOL SimGrp(char *template, char *fn, char *explain) {
  filenametype neu;

  sprintf(fn, template, DNAPEP());
  if (fexist(fn)) return TRUE;

  strcpy(neu, fn);
  sprintf(fn, "%s%c", get_logical("BOXDIR"), d_sep);
  strcat(fn, neu);
  if (fexist(fn)) return TRUE;

  if (explain == NULL) return FALSE;
  for (;;) {
    printf("%s %s does not exist, enter filename: ", explain, neu);
    Gets(fn);
    if (fexist(fn)) return TRUE;
    strcpy(neu, fn);
    printf("\007file does not exist \n");
  }
}

static void ask(void)
{
  unsigned i;
  BOOL ok;
  char instring[51];
  double inno;

  if (!clparflag)
    sprintf(parname, "box_%s.par", DNAPEP());
  if (!fexist(parname))
    sprintf(parname, "%s%cbox_%s.par",
		  get_logical("BOXDIR"), d_sep, DNAPEP());
  if (!fexist(parname)) {
    ok = FALSE;
    do {
      printf("Default-parameter file %s does not exist, enter filename: ",
	     parname);
      Gets(parname);
      ok = fexist(parname);
      if (!ok)
	printf("\007file does not exist \n");
    } while (!ok);
  }
  parfile = fopen(parname, TXT_RD);
  assert(parfile != NULL);
  do {
    Fgets(line_, 256, parfile);
  } while (indx(line_, ":GENERAL") != 1);
  Fgets(line_, 256, parfile);
  if (!clinflag)
    inputmode = line_[0];
  Fgets(line_, 256, parfile);
  if (!cldevflag)
    outputmode = line_[0];
  Fgets(line_, 256, parfile);
  if (line_[0] == 'Y' || line_[0] == 'y')
    seqconsflag = TRUE;
  else
    seqconsflag = FALSE;
  hideflag = FALSE;
  masternormal = FALSE;
  fscanf(parfile, "%d%*[^\n]", &outlen);
  getc(parfile);
  Fgets(line_, 256, parfile);
  if (line_[0] == 'Y' || line_[0] == 'y')
    seqnameflag = TRUE;
  else
    seqnameflag = FALSE;
  Fgets(line_, 256, parfile);
  if (line_[0] == 'Y' || line_[0] == 'y')
    seqnumflag = TRUE;
  else
    seqnumflag = FALSE;
  fscanf(parfile, "%d%*[^\n]", &interlines);
  getc(parfile);
  Fgets(line_, 256, parfile);
  if (line_[0] == 'Y' || line_[0] == 'y')
    simflag = TRUE;
  else
    simflag = FALSE;
  Fgets(line_, 256, parfile);
  if (line_[0] == 'Y' || line_[0] == 'y')
    globalflag = TRUE;
  else
    globalflag = FALSE;

  Fgets(line_, 256, parfile);
  if (!consflag) {
    if (line_[0] == 'Y' || line_[0] == 'y')
      consflag = TRUE;
    else
      consflag = FALSE;
  }

  Fgets(line_, 256, parfile);
  if (!clsymbconsflag) {
    symbcons[0] = line_[0];
    symbcons[1] = line_[1];
    symbcons[2] = line_[2];
  }

  Fgets(line_, 256, parfile);
  if (!splitflag) {
    if (line_[0] == 'Y' || line_[0] == 'y')
      splitflag = TRUE;
    else
      splitflag = FALSE;
  }

  Fgets(line_, 256, parfile);
  if (!numdefflag) {
    if (line_[0] == 'Y' || line_[0] == 'y')
      numdefflag = TRUE;
    else
      numdefflag = FALSE;
  }
  if (!clthrflag) {
    fscanf(parfile, "%lg%*[^\n]", &thrfrac);
    getc(parfile);
  } else {
    Fgets(line_, 256, parfile);
  }

  for (i = 0; i < max_no_seq; i++)
    memset(seq[i], ' ', max_no_res);

  printf("\n");
  printf("BOXSHADE %s\n", BOXSHADE_ver);
  printf(
    "This program makes multiple-aligned output from either\n"
    "PILEUP-MSF, CLUSTAL-ALN, MALIGNED-data and ESEE-save files\n"
    "(limited to a maximum of %d sequences with up to %d elements each)\n",
					       max_no_seq, max_no_res);
  printf(
    "Various kinds of shading can be applied to identical/similar residues\n"
    "Output is written to screen or to a file in the following formats:\n"
    "ANSI/VT100, PS/EPS, RTF, HPGL, ReGIS, LJ250-printer, ASCII, xFIG,\n"
    "PICT, HTML\n"
    "\n"
  );

  /**** ask for infile ****/
  *instring = '\0';
  ok = FALSE;
  do {
    if (!clinflag) {
      printf("name of aligned input-file : ");
      Gets(inname);
      printf("\n");
    }
    ok = fexist(inname);
    if (!ok) {
      printf("\007aligned input file does not exist \n");
      clinflag = FALSE;
    }
  } while (!ok);
  /**** ask for infile type ****/
  if (!cltypeflag) {
    if (indx(inname, ".pre") > 0 || indx(inname, ".PRE") > 0 ||
	indx(inname, ".msf") > 0 || indx(inname, ".MSF") > 0)
      inputmode = '1';
    else {
      if (indx(inname, ".aln") > 0 || indx(inname, ".ALN") > 0)
	inputmode = '2';
      else {
	if (indx(inname, ".mal") > 0 || indx(inname, ".MAL") > 0)
	  inputmode = '3';
	else {
	  if (indx(inname, ".ese") > 0 || indx(inname, ".ESE") > 0)
	    inputmode = '4';
	}
      }
    }
    if (indx(inname, ".phy") > 0 || indx(inname, ".PHY") > 0)
      inputmode = '5';
    if (indx(inname, ".fa") > 0 )
      inputmode = '6';
  }
  if (interactflag && !cltypeflag) {
    do {
      printf("Do you want to process (1) Lineup-PRETTY/Pileup-MSF file\n"
	     "                       (2) CLUSTAL .ALN file\n"
	     "                       (3) MALIGNED data file\n"
	     "                       (4) ESEE save file\n"
         "                       (6) fasta file\n"
	     "                       (5) PHYLIP file   (* %c *) : ",
	     inputmode);
      Fgets(instring, 51, stdin);
      if (*instring == '\0')
	sprintf(instring, "%c", inputmode);
      if (indx("123456", instring) == 0) {
	printf(" \n");
	printf("\007---> Please choose a supported type\n");
	printf(" \n");
      }
    } while (indx("123456", instring) <= 0);
  }
  printf(" \n");
  if (*instring != '\0')
    inputmode = instring[0];

  *instring = '\0';
  if (!cldevflag) {
    do {
      printf("Output suitable for (%2c) POSTSCRIPT\n", oPS);
      printf("                    (%2c) encapsulated POSTSCRIPT\n", oEPS);
      printf("                    (%2c) HPGL\n", oHPGL);
      printf("                    (%2c) RTF (Rich Text Format)\n", oRTF);
#ifdef oCRT
      printf("                    (%2c) PC-screen     (PCs only!)\n", oCRT);
#endif
      printf("                    (%2c) ANSI-screen   (PC-version)\n", oANSI);
      printf("                    (%2c) VT100-screen  (DEC-version)\n", oVT);
      printf("                    (%2c) ReGIS-screen  (25 lines each\n", oREGISt);
      printf("                    (%2c) ReGIS-file    (without breaks)\n", oREGISp);
      printf("                    (%2c) LJ250-printer file\n", oLJ250);
      printf("                    (%2c) ASCII file\n", oASCII);
      printf("                    (%2c) FIG file (for XFIG)\n", oFIG);
      printf("                    (%2c) PICT file\n", oPICT);
      printf("                    (%2c) HTML file\n", oHTML);
      printf("                      current:   (* %c *) : ", outputmode);

      Fgets(instring, 51, stdin);
      if (*instring == '\0')
	sprintf(instring, "%c", outputmode);
      if (indx(allowed_devices, instring) == 0) {
	printf("\n");
	printf("\007---> Please choose a supported type\n");
	printf("\n");
      }
    } while (indx(allowed_devices, instring) <= 0);
  }
  if (*instring != '\0')
    outputmode = instring[0];
  printf("\n");

  if (outputmode == oASCII)
    seqconsflag = TRUE;
  if (clseqconsflag)
    seqconsflag = TRUE;
  if (!seqconsflag) {
    *instring = '\0';
    if (interactflag) {
      do {
	printf(
	  "-------------------------------------------------------------------\n");
	if (seqconsflag)
	  printf("similarity to a single sequence? (* y *) : ");
	else
	  printf("similarity to a single sequence? (* n *) : ");
	Fgets(instring, 51, stdin);
      } while (indx("nNyY", instring) <= 0 && *instring != '\0');
    }
    seqconsflag = (toupper(*instring) == 'Y');
  }

  *instring = '\0';
  if (interactflag && !clconsflag) {
    do {
      printf("-------------------------------------------------------------------\n");
      printf("display consensus line ? (* %c *) : ", YESNO(consflag) );
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
    if (*instring != '\0')
      consflag = (toupper(instring[0]) == 'Y');
  } else
  if (clconsflag && !clsymbconsflag)
    printf("-------------------------------------------------------------------\n\n");

  *instring = '\0';
  if (interactflag && consflag && !clsymbconsflag) {
    printf("\n"
	   "Enter now a string of 3 symbols to be used for consensus display\n"
	   "          representing different/all-similar/all-identical residues\n"
	   "          see documentation for symbol definition, examples are:\n"
	   "          \" .*\" or \"-LU\"    (ommit quotes but use blanks)\n"
	   "SYMBOL : ");
    Fgets(instring, 51, stdin);
    if (*instring != '\0') {
      while (strlen(instring) != 3) {
	printf("\007please enter string of THREE (3) symbols !\n");
	printf("SYMBOL : ");
	Fgets(instring, 51, stdin);
      }
      symbcons[0] = instring[0];
      symbcons[1] = instring[1];
      symbcons[2] = instring[2];
    }
  }

  *instring = '\0';
  if (interactflag && !rulerflag) {
    do {
      printf("-------------------------------------------------------------------\n"
	     "display ruler line ? (* %c *) : ", YESNO(rulerflag) );
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
    if (*instring != '\0')
      rulerflag = (toupper(instring[0]) == 'Y');
  }

  *instring = '\0';
  if (interactflag && !clthrflag && outputmode != oASCII) {
    *instring = '\0';
    ok = FALSE;
    do {
      printf("--------------------------------------------------------------\n"
	     "The threshold is the fraction of residues that must be identical\n"
	     "or similar for shading to occur\n"
	     "Value for threshold (* %6.2f *):", thrfrac);
      Fgets(instring, 51, stdin);
      if (*instring == '\0')
	ok = TRUE;
      else {
	*depend_err = '\0';
	inno = str2real((void *)instring);
	if (*depend_err == '\0' && inno <= 1.0 && inno > 0.0) {
	  ok = TRUE;
	  thrfrac = inno;
	}
	if (!ok)
	  printf("The fraction must be between 0 and 1\n\n");
      }
    } while (!ok);
  }
  if (outputmode == oASCII)
    thrfrac = 0.0;

  *instring = '\0';
  if (interactflag) {
    do {
      printf("-------------------------------------------------------------------\n"
	     "\n"
	     "How many sequence characters per line (* %3d *) : ",
	     outlen);
      Fgets(instring, 51, stdin);
      if (*instring != '\0')
	outlen = str2int((void *)instring);
      printf("\n");
    } while (outlen <= min_outlen || outlen >= max_outlen);
  }

  *instring = '\0';
  if (interactflag) {
    do {
      printf("should sequence name be printed (* %c *) : ", YESNO(seqnameflag));
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring != '\0')
    seqnameflag = (toupper(instring[0]) == 'Y');

  *instring = '\0';
  if (interactflag && !rulerflag) {
    do {
      printf("should position numbers be printed (* %c *) : ", YESNO(seqnumflag));
      Fgets(instring, 51, stdin);
      printf("\n");
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring != '\0')
    seqnumflag = (toupper(instring[0]) == 'Y');

  *instring = '\0';
  if (interactflag) {
    do {
      printf("How many lines between two sequence blocks (* %2d *) : ",
	     interlines);
      Fgets(instring, 51, stdin);
      if (*instring != '\0')
	interlines = str2int((void *)instring);
      printf(" \n");
    } while (interlines <= 0 || interlines >= 100);
  }

  *instring = '\0';
  if (interactflag) {
    do {
      printf("special label for similar residues (* %c *) : ", YESNO(simflag));
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring != '\0')
    simflag = (toupper(instring[0]) == 'Y');

  if (simflag) {
    if (clsimflag)
      clsimflag = SimGrp(clsimname, simname, NULL);
    if (!clsimflag)
      SimGrp("box_%s.sim", simname, "Similarity-file");
    if (clgrpflag)
      clgrpflag = SimGrp(clgrpname, grpname, NULL);
    if (!clgrpflag)
      SimGrp("box_%s.grp", grpname, "Group-file");

#if 0
    sprintf(simname, "box_%s.sim", DNAPEP());
    sprintf(grpname, "box_%s.grp", DNAPEP());
    if (!fexist(simname))
      sprintf(simname, "%s%cbox_%s.sim",
			get_logical("BOXDIR"), d_sep, DNAPEP());
    if (!fexist(simname)) {
      ok = FALSE;
      do {
	printf("Similarity-file %s does not exist, enter filename: ", simname);
	Gets(simname);
	ok = fexist(simname);
	if (!ok)
	  printf("\007file does not exist \n");
      } while (!ok);
    }

    if (!fexist(grpname))
      sprintf(grpname, "%s%cbox_%s.grp",
	      get_logical("BOXDIR"), d_sep, DNAPEP());
    if (!fexist(grpname)) {
      ok = FALSE;
      do {
	printf("Group-file %s does not exist, enter filename: ", grpname);
	Gets(grpname);
	ok = fexist(grpname);
	if (!ok)
	  printf("\007file does not exist \n");
      } while (!ok);
    }
#endif
  }

  *instring = '\0';
  if (interactflag) {
    do {
      if (globalflag)
	printf("special label for identical residues in all sequences (* y *) : ");
      else
	printf("special label for identical residues in all sequences (* n *) : ");
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring != '\0')
    globalflag = (toupper(instring[0]) == 'Y');

  for (i = 0; i <= 4; i++)
    lc[i] = FALSE;

  switch (outputmode) {
#ifdef oCRT
    case oCRT:    OutDev = &Crt;        break;
#endif
    case oANSI:   OutDev = &Ansi;       break;
    case oPS:     OutDev = &Postscript; break;
    case oEPS:    OutDev = &Eps;        break;
    case oHPGL:   OutDev = &Hpgl;       break;
    case oRTF:    OutDev = &Rtf;        break;
    case oREGISt: OutDev = &RegisT;     break;
    case oREGISp: OutDev = &RegisP;     break;
    case oVT:     OutDev = &Vt;         break;
    case oLJ250:  OutDev = &Lj250;      break;
    case oASCII:  OutDev = &Ascii;      break;
    case oFIG:    OutDev = &Fig;        break;
    case oPICT:   OutDev = &Pict;       break;
    case oHTML:   OutDev = &Html;       break;
  }
  OutDev->Ask();

  ident_sim = FALSE;
  if (interactflag) {
    do {
      printf("create identity / similarity matrix (* n *) : ");
      Fgets(instring, 51, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring != '\0')
    ident_sim = (toupper(instring[0]) == 'Y');

  if (ident_sim) {
    /**** ask for matrix file ****/
    *instring = '\0';
    printf("name of matrix output-file : ");
    Gets(identname);
    printf("\n");
  }

  if (parfile != NULL)
    fclose(parfile);
  parfile = NULL;
}


static void ask_numbers(void)
{
  char instring[11];
  int inno, i;

  printf("-------------------------------------------------------\n"
	 "\n"
	 "You requested sequence numbering in the output.\n"
	 "Enter the number of the first sequence position\n"
	 "or confirm the suggestion of the program\n"               );

  for (i = 0; i < no_seq; i++) {
    printf("%s (* %4d *)  : ", seqname[i], startno[i]);
    Fgets(instring, 11, stdin);
    if (*instring != '\0') {
      inno = str2int((void *)instring);
      if (strcmp(depend_err, "str2int"))
	startno[i] = inno;
    } else
      startno[i] = 1;
  }
  printf("\n");
}


static void ask_seqcons(void)
{
  char instring[11];
  int i;

  if (clseqconsflag) {
    if (consensnum < 1 || consensnum > no_seq)
      clseqconsflag = FALSE;
  }

  if (!clseqconsflag)
  {  /*only go into this asking routine if nothing on the command line*/
    consensnum = 1;   /*1 is the default*/
    if (interactflag) {
      printf("-------------------------------------------------------\n"
	     " \n"
	     "You requested consensus formation to a single sequence.\n"
	     "Choose now the sequence to compare to the other ones.\n"   );
      for (i = 1; i <= no_seq; i++)
	printf("(%2d) %s\n", i, seqname[i - 1]);
      do {
	printf("No. of sequence: (* %3d *) : ", consensnum);
	Fgets(instring, 11, stdin);
	if (*instring != '\0')
	  consensnum = str2int((void *)instring);
      } while (consensnum < 1 || consensnum > no_seq);
      printf("\n");

      *instring = '\0';
      do {
	printf(
	  "-------------------------------------------------------------------\n");
	printf("hide this sequence? (* %c *) : ", YESNO(hideflag));
	Fgets(instring, 11, stdin);
      } while (indx("nNyY", instring) <= 0 && *instring != '\0');
      if (*instring != '\0')
	hideflag = (toupper(instring[0]) == 'Y');
    }
  }

  if (outputmode == oASCII) {
    masternormal = TRUE;
    return;
  }

  if (!interactflag || clseqconsflag) {
    masternormal = FALSE;
    return;
  }
  *instring = '\0';
  if (!hideflag) {
    do {
      printf("show this sequence in all-normal rendition? (* n *) : ");
      Fgets(instring, 11, stdin);
    } while (indx("nNyY", instring) <= 0 && *instring != '\0');
  }
  if (*instring == '\0')
    return;
  masternormal = (toupper(instring[0]) == 'Y');
}


/*--------------------------------------------------------------------------*/
/*   read input files and comparison-table                                  */
/*--------------------------------------------------------------------------*/

static void read_cmp(void)
{
  /* Read the .sim and .grp files */
  int i, j, p1, p2;
  char *cp;
  FILE *simfile, *grpfile;
  int ll;

  grpfile = NULL;
  simfile = NULL;
  if (simflag) {
    for (i = 0; i <= 19; i++) {
      for (j = 0; j <= 19; j++) {
	simtable[i][j] = FALSE;
	grptable[i][j] = FALSE;
      }
    }
    for (i = 0; i <= 19; i++) {
      simtable[i][i] = TRUE;
      grptable[i][i] = TRUE;
    }

    simfile = fopen(simname, TXT_RD);
    assert(simfile != NULL);
    do {
      Fgets(line_, 256, simfile);
    } while (!feof(simfile) && indx(line_, "..") <= 0);
    while (!feof(simfile)) {
      Fgets(line_, 256, simfile);
      if (*line_ == '\0') continue;
      cp = strchr(aaset, line_[0]);
      if (cp == NULL) p1 = 0;
		 else p1 = (int)(cp-aaset)+1;
      if (p1 > 0) {
	ll = strlen(line_);
	for (i = 2; i < ll; i++) {
	  cp = strchr(aaset, line_[i]);
	  if (cp == NULL) p2 = 0;
			  else p2 = (int)(cp-aaset)+1;
	  if (p2 > 0) {
	    simtable[p1 - 1][p2 - 1] = TRUE;
	    simtable[p2 - 1][p1 - 1] = TRUE;
	  }
	}
      }
    }

    grpfile = fopen(grpname, TXT_RD);
    assert(grpfile != NULL);
    do {
      Fgets(line_, 256, grpfile);
    } while (!feof(grpfile) && indx(line_, "..") <= 0);
    while (!feof(grpfile)) {
      Fgets(line_, 256, grpfile);
      if ((ll=strlen(line_)) < 2) continue;
      for (j = 1; j < ll; j++) {
	cp = strchr(aaset, line_[j-1]);
	if (cp == NULL) p1 = 0;
			else p1 = (int)(cp-aaset)+1;
	if (p1 > 0) {
	  for (i = j; i < ll; i++) {
	    cp = strchr(aaset, line_[i]);
	    if (cp == NULL) p2 = 0;
				 else p2 = (int)(cp-aaset)+1;
	    if (p2 > 0) {
	      grptable[p1 - 1][p2 - 1] = TRUE;
	      grptable[p2 - 1][p1 - 1] = TRUE;
	    }
	  }
	}
      }
    }
  }
  if (simfile != NULL)
    fclose(simfile);
  if (grpfile != NULL)
    fclose(grpfile);
}

static int aaset_idx[256];
#define IDX_aaset(ch) ( aaset_idx[(unsigned char)(ch)] )
#define IN_aaset(ch)  ( IDX_aaset(ch) >= 0 )

static void build_aaset_table(void) {
  int ch;
  char *ap;

  for (ch = 0; ch < 256; ++ch)
    aaset_idx[ch] = -1;
  ch = 0;
  for (ap = &aaset[0]; *ap != '\0'; ++ap)
    aaset_idx[*ap] = ch++;
}

static BOOL sim(char a, char b)
{
  int idx1, idx2;

  idx1 = IDX_aaset(a);
  if (idx1 < 0) return FALSE;
  idx2 = IDX_aaset(b);
  if (idx2 < 0) return FALSE;

  return simtable[idx1][idx2];
}


static BOOL grp(char a, char b)
{
  int idx1, idx2;

  idx1 = IDX_aaset(a);
  if (idx1 < 0) return FALSE;
  idx2 = IDX_aaset(b);
  if (idx2 < 0) return FALSE;

  return grptable[idx1][idx2];
}

static void make_consensus_length(void)
{
  int i;

  consenslen = 0;
  for (i = 0; i < no_seq; i++) {
    while (seq[i][seqlen[i] - 1] == '-' || seq[i][seqlen[i] - 1] == '.' ||
	   seq[i][seqlen[i] - 1] == ' ')
      seqlen[i]--;
    if (seqlen[i] > consenslen)
      consenslen = seqlen[i];
  }
  printf("consensus length is %d\n", consenslen);
}


static void make_consensus(void)
{
  int i, j, k;
  int idcount[MAX_NO_SEQ];
  int simcount[MAX_NO_SEQ];
  int pc;
  int maxidcount, maxsimcount, idindex, simindex;
  BOOL unique;

  /* set consensus length = length of longest sequence (not counting dots,
     spaces, etc. at the "far" end. May be a problem here for some strange
     cases */
  /* calculate the threshold # of sequences */
  thr = (int)(0.5 + thrfrac * no_seq);

  if (seqconsflag) {
    for (i = 0; i < consenslen; i++)
      cons[i] = seq[consensnum - 1][i];
    return;
  }

  printf("Building consensus "); fflush(stdout);

  /* build a lookup table for 'strchr(aaset,ch)-aaset' */
  build_aaset_table();

  pc = 0;
  for (i = 0; i < consenslen; i++) {
    int p = (int)((50.0*i) / consenslen);
    if (p > pc) {
      printf("."); fflush(stdout);
      pc = p;
    }
    for (j = 0; j < no_seq; j++) {
      idcount[j] = 0;
      simcount[j] = 0;
      for (k = 0; k < no_seq; k++) {
	/* increment idcount AND simcount if the two residues are the same */
	char seq_ji = seq[j][i];
	if ( IN_aaset(seq_ji)) {
	  if (seq[k][i] == seq_ji) {
	    idcount[j]++;
	    simcount[j]++;
	  } else
	  /* increment only simcount if residues are not the same but fall into
	     the same group */
	  if (grp(seq[k][i], seq_ji))
	    simcount[j]++;
	}
      }
    }
    /* Find the maximum values in idcount and simcount, along with the
       indices of those maxima */
    cons[i] = ' ';
    unique = FALSE;
    maxidcount = idcount[0];
    maxsimcount = simcount[0];
    idindex = 1;
    simindex = 1;
    for (j = 2; j <= no_seq; j++) {
      if (idcount[j - 1] > maxidcount) {
	idindex = j;
	maxidcount = idcount[j - 1];
      }
      if (simcount[j - 1] > maxsimcount) {
	maxsimcount = simcount[j - 1];
	simindex = j;
      }
    }
    /* check here for the case where several residues/types may have achieved
       the same score */
    if (maxidcount >= thr) {  /*only look if max is high enough*/
      unique = TRUE;
      for (j = 0; j < no_seq; j++) {
	if (maxidcount == idcount[j] && seq[idindex - 1][i] != seq[j][i])
	  unique = FALSE;
      }
      if (unique)
	cons[i] = seq[idindex - 1][i];
    }
    /* if there is an equally high idcount for a different residue then
       there can't be a single residue consensus */
    if (maxsimcount >= thr && !unique) {
      unique = TRUE;
      for (j = 0; j < no_seq; j++) {
	if (maxsimcount == simcount[j] &&
	    !grp(seq[simindex - 1][i], seq[j][i]))
	  unique = FALSE;
      }
      if (unique)
	cons[i] = tolower(seq[simindex - 1][i]);
      /*if maxsimcount is not unique and the other residue is NOT in the same
	similarity group then there is so consensus based on similarity. If
	the two residues with the same similarity score are in the same
	similarity group, flag that consensus position by making the
	residue lowercase*/
    }
  }
  printf(" done\n");
}


static void make_colors(void)
{
  int i, j, idcount, simcount, pc;

  printf("Colorizing "); fflush(stdout);

  pc = 0;
  for (i = 0; i < consenslen; i++) {
    int p = (int)((50.0*i) / consenslen);
    if (p > pc) {
      printf("."); fflush(stdout);
      pc = p;
    }
    idcount = 0;
    simcount = 0;
    if (strchr(aasetlow, cons[i]) != NULL)
      for (j = 0; j < no_seq; j++) {
	col[j][i] = 0;
	if (grp(seq[j][i], toupper(cons[i]))) {
	  col[j][i] = 2;
	  simcount++;
	}
    } else {
      for (j = 0; j < no_seq; j++) {
	if (seq[j][i] == cons[i])
	  idcount++;
	else {
	  if (sim(seq[j][i], cons[i]))
	    simcount++;
	}
	col[j][i] = 0;
      }  /*count the ids and sims as they are used later for consensus line*/

      if (idcount == no_seq &&
	  strchr(aaset, cons[i]) != NULL ) {
	for (j = 0; j < no_seq; j++)
	  col[j][i] = 3;
	/*if all sequences the same at this point then colour them identical(3)*/
      } else {
	if (idcount + simcount >= thr &&
	    strchr(aaset, cons[i]) != NULL ) {
	  for (j = 0; j < no_seq; j++) {
	    if (seq[j][i] == cons[i])
	      col[j][i] = 1;   /*=> conserved residue*/
	    else if (sim(seq[j][i], cons[i]))
	      col[j][i] = 2;
	    /*=> similar to consensus(2)*/
	  }
	}
      }
    }
    /*do shading and count similar residues for the case of a group
      consensus; note that in this case there cannot be any residues marked as
      'identical', by definition*/

    if (consflag) {
      if (idcount == no_seq) {
	conschar[i] = symbcons[2];
	if (toupper(conschar[i]) == 'U')
	  conschar[i] = toupper(cons[i]);
	else if (toupper(conschar[i]) == 'L')
	  conschar[i] = tolower(cons[i]);
	else if (toupper(conschar[i]) == 'B')
	  conschar[i] = ' ';
      } else if (idcount + simcount >= thr) {
	conschar[i] = symbcons[1];
	if (toupper(conschar[i]) == 'U')
	  conschar[i] = toupper(cons[i]);
	else if (toupper(conschar[i]) == 'L')
	  conschar[i] = tolower(cons[i]);
	else if (toupper(conschar[i]) == 'B')
	  conschar[i] = ' ';
      } else {
	conschar[i] = symbcons[0];
	if (toupper(conschar[i]) == 'U')
	  conschar[i] = toupper(cons[i]);
	else if (toupper(conschar[i]) == 'L')
	  conschar[i] = tolower(cons[i]);
	else if (toupper(conschar[i]) == 'B')
	  conschar[i] = ' ';
      }
    }
  }
  printf(" done\n");
}


static void make_lowcase(void)
{
  int i, j, pc;

  if ( ! (lc[0] || lc[1] || lc[2] || lc[3] || lc[4]) )
    return; /* nothing to do ! */

  printf("Lowercase "); fflush(stdout);

  pc = 0;
  for (i = 0; i < no_seq; i++) {
    int p = (int)((20.0*i) / no_seq);
    while (p > pc) {
      printf("."); fflush(stdout);
      pc++;
    }
    for (j = 0; j < seqlen[i]; j++) {
      if (lc[col[i][j]])
	seq[i][j] = tolower(seq[i][j]);
    }
  }
  printf(" done\n");
}


static void prepare_names(void)
{
  int i;
  char *cp;

  seqname_outlen = 1;
  for (i = 0; i < no_seq; i++) {
    cp = seqname[i] + strlen(seqname[i]);
    while (cp != seqname[i] && *(cp-1) == ' ')
      *(--cp) = '\0';
    if (strlen(seqname[i]) > seqname_outlen)
      seqname_outlen = strlen(seqname[i]);
  }
  for (i = 0; i < no_seq; i++) {
    int sl = strlen(seqname[i]);
    int p  = seqname_outlen - sl;
    if (p > 0)
      sprintf(seqname[i]+sl, "%*s", p, "");
  }
}


static void prepare_numbers(void)
{
  int count, bn, i, j;

  for (i = 0; i < no_seq; i++) {
    for (j = 1; j < max_no_block; j++)
      sprintf(prenum[i][j], "%*s", seqnumlen, "");
    sprintf(prenum[i][0], "%*d", seqnumlen, startno[i]);
    count = startno[i] - 1;
    bn = 1;
    for (j = 1; j <= seqlen[i]; j++) {
      if (    isupper(seq[i][j - 1])
	   || (i == cons_idx)
	   || (i == ruler_idx) )
	count++;
      if (j % outlen == 0) {
	bn++;
	if (count + 1 < seqlen[i] + startno[i])
	  sprintf(prenum[i][bn-1], "%*d", seqnumlen, count + 1);
      }
    }
  }
}


/*--------------------------------------------------------------------------*/
/*   graphics section                                                       */
/*--------------------------------------------------------------------------*/

static void graphics_init(double *xpos, double *ypos)
{
  act_page = 1;
  OutDev->Init(xpos, ypos);
  lines_per_page = (int)((dev_maxy - dev_miny) / dev_ysize);
  act_page = 1;
}

static void graphics_setcolor(int colno) {

  if (colno == act_color)
    return;
  OutDev->Setcolor(colno);
  act_color = colno;
}

static void graphics_charout(char c, double *xpos, double *ypos) {
  OutDev->Charout(c, xpos, ypos);
}

void GenericStringOut(char *s, double *xpos, double *ypos) {
  while (*s != '\0') {
     OutDev->Charout(*s, xpos, ypos);
     ++s;
  }
  OutDev->Charout(' ', xpos, ypos);
}

static void graphics_stringout(char *s, double *xpos, double *ypos) {
  OutDev->Stringout(s, xpos, ypos);
}

static void graphics_newline(double *xpos, double *ypos) {
  OutDev->Newline(xpos, ypos);
}

static void graphics_newpage(double *xpos, double *ypos) {
  OutDev->Newpage(xpos, ypos);
  act_page++;
}

static void graphics_exit(void) {
  OutDev->Exit();
}

static void graphics_out(void)
{
  double xpos, ypos;   /*current position of text cursor*/
  int i, j, k;
  char pagesymbol;
  int linecount;

  pagesymbol = '1';
  act_color = -1;
  no_blocks = (consenslen - 1) / outlen + 1;
  if (consflag && no_seq < max_no_seq) {
    cons_idx = no_seq;
    seqlen[cons_idx] = consenslen;
    strcpy(seqname[cons_idx], "consensus");
    startno[cons_idx] = 1;
    for (i = 0; i < consenslen; i++) {
      seq[cons_idx][i] = conschar[i];
      col[cons_idx][i] = 4;   /*set colour to predefined "title" type*/
    }
    no_seq++;
  }
  if (seqconsflag && masternormal) {
    for (i = 0; i < consenslen; i++)
      col[consensnum - 1][i] = 4;
  }
  if (seqconsflag && hideflag) {
    if (consensnum < no_seq) {
      for (i = consensnum; i < no_seq; i++) {
	seqlen[i - 1] = seqlen[i];
	strcpy(seqname[i - 1], seqname[i]);
	startno[i - 1] = startno[i];
	memcpy(seq[i - 1], seq[i], max_no_res * sizeof(char));
	memcpy(col[i - 1], col[i], max_no_res * sizeof(byte));
      }
    }
    no_seq--;
  }

  if (rulerflag && no_seq < max_no_seq) {
    ruler_idx = no_seq;
    seqlen[ruler_idx] = consenslen;
    strcpy(seqname[ruler_idx], "");
    startno[ruler_idx] = 1;
    for (i = 0; i < consenslen; i++) {
      seq[ruler_idx][i] = '.';
      col[ruler_idx][i] = 4;   /*set colour to predefined "title" type*/
    }
    // seq[ruler_idx][0] = '1';
    for (i=10; i <= consenslen; i+=10) {
      char no[20];
      j=i;
      sprintf(no, "%d", j);
      k = strlen(no);
    //   j=i-1;
    //   printf("start do loop\n");
    //   do {
    //     ++j;
    //     sprintf(no, "%d", j);
    //     k = strlen(no);
    //   } while ( ((j-1)%outlen)+k > outlen || j > consenslen);
    //   printf("end do loop\n");
    //   printf("for ruler: i is %d, j is %d, k is %d, no is %s\n", i, j, k, no);
    //   printf("for ruler: seq[ruler_idx] is %s\n", seq[ruler_idx]);
    //   if (j+k-1 <= consenslen && (j%10)+k < 9)
    //   if ((j%10)+k < 9)
        memcpy(&seq[ruler_idx][j-k], no, k); // replaced j-1 with j-k
    }
    no_seq++;
  }

  if (seqnameflag)
    prepare_names();
  if (seqnumflag)
    prepare_numbers();

  lines_left = no_blocks * (no_seq + interlines) - interlines;
  graphics_init(&xpos, &ypos);
  linecount = 0;
  for (i = 0; i < no_blocks; i++) {
    for (j = 0; j < no_seq; j++) {
      if (seqnameflag) {
	graphics_setcolor(4);   /*set colours to "title" type*/
/*	for (k = 0; k < seqname_outlen; k++)
	  graphics_charout(seqname[j][k], &xpos, &ypos);
	graphics_charout(' ', &xpos, &ypos);
*/       
        graphics_stringout(seqname[j], &xpos, &ypos);
      }
      if (seqnumflag) {
	graphics_setcolor(4);
/*      {
	int sl = strlen(prenum[j][i]);
	for (k = 0; k < sl; k++)
	  graphics_charout(prenum[j][i][k], &xpos, &ypos);
        }
	graphics_charout(' ', &xpos, &ypos);
*/
        graphics_stringout(prenum[j][i], &xpos, &ypos);
      }
      for (k = 0; k < outlen; k++) {
	if (i * outlen + k < consenslen) {
	  graphics_setcolor(col[j][i * outlen + k]);
	  graphics_charout(seq[j][i * outlen + k], &xpos, &ypos);
	}
      }
      linecount++;
      if (linecount >= lines_per_page) {
	lines_left -= linecount;
	linecount = 0;
	if (lines_left > 0) {
	  if (splitflag) {
	    graphics_exit();
	    pagesymbol++;
	    outname[indx(outname, ".") - 2] = pagesymbol;
	    graphics_init(&xpos, &ypos);
	  } else
	    graphics_newpage(&xpos, &ypos);
	}
      } else
	graphics_newline(&xpos, &ypos);
    } /* for j */
    if (linecount + interlines + no_seq <= lines_per_page)
    {  /*will the next block fit?*/
      for (j = 0; j < interlines; j++) {
	graphics_newline(&xpos, &ypos);
	linecount++;
      }
    } else {
      /* skipping interlines ... */
      lines_left -= interlines;
      lines_left -= linecount;
      if (lines_left > 0) {
	if (splitflag) {
	  graphics_exit();
	  pagesymbol++;
	  outname[indx(outname, ".") - 2] = pagesymbol;
	  graphics_init(&xpos, &ypos);
	  linecount = 0;
	} else {
	  graphics_newpage(&xpos, &ypos);
	  linecount = 0;
	}
      }
    }
  } /* for i */
  graphics_exit();
}


static void do_out(FILE *outf, BOOL goon)
{
  int i, j, k, id, sim, conlen;
  BOOL fl1, fl2;
  int pc;

  prepare_names();

  if (goon)
    printf("Building identity/similarity matrix "); fflush(stdout);

  pc = 0;
  for (i = 1; i <= no_seq; i++) {
    if (goon) {
      int p = (int)((20.0*i) / no_seq);
      while (p > pc) {
	printf("."); fflush(stdout);
	pc++;
      }
    }
    fprintf(outf, "%s  ", seqname[i - 1]);
    /* Identities */
    for (j = 0; j < i - 1; j++) {
      id = 0;
      conlen = 0;
      for (k = 0; k < consenslen; k++) {
	fl1 = IN_aaset(seq[i-1][k]);
	fl2 = IN_aaset(seq[j  ][k]);
	if (fl1 || fl2) {
	  conlen++;
	  if (seq[i-1][k] == seq[j][k])
	    id++;
	}
      }
      fprintf(outf, " %5.1f", 100.0 * id / conlen);
    }
    fprintf(outf, "%6s", "----");
    for (j = i; j < no_seq; j++) {
      sim = 0;
      conlen = 0;
      for (k = 0; k < consenslen; k++) {
	fl1 = IN_aaset(seq[i-1][k]);
	fl2 = IN_aaset(seq[j  ][k]);
	if (fl1 || fl2) {
	  conlen++;
	  if (seq[i-1][k] == seq[j][k] || grp(seq[i-1][k], seq[j][k]))
	    sim++;
	}
      }
      fprintf(outf, " %5.1f", 100.0 * sim / conlen);
    }
    putc('\n', outf);
  }
  if (goon)
    printf(" done\n");
}


static void ident_sim_out(void)
{
  if (*identname != '\0') {
    MYFILE idf;
    assert( outopen(&idf, identname) != NULL );
    do_out(idf.f, TRUE);
    fclose(idf.f);
  } else
    do_out(stdout, FALSE);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Allocate memory -- actually this should be done *
 * after we know how many sequences there are and  *
 * how long each is -- future work                 */
static void allocate1(void)
{
  int i;

  cons     = NULL;
  conschar = NULL;

  seq    = Malloc(MAX_NO_SEQ * sizeof(char *));
  col    = Malloc(MAX_NO_SEQ * sizeof(byte *));
  prenum = Malloc(MAX_NO_SEQ * sizeof(numtype *));

  max_no_seq = MAX_NO_SEQ;
  max_no_res = MAX_NO_RES;
  for (i = 0; i < MAX_NO_SEQ; i++) {
    seq[i] = malloc(max_no_res * sizeof(char));
    col[i] = NULL;
    prenum[i] = NULL;
    if (seq[i] == NULL) {
      if (i > 10) {
	--i;
	Free(&seq[i]);
	--i;
	Free(&seq[i]);
      }
      max_no_seq = i-1;
      break;
    }
  }
}

static void allocate2(void)
{
  int i, need;

  need = no_seq;
  if (consflag) ++need;
  if (rulerflag) ++need;
  if (need > max_no_seq) need = max_no_seq;

  for (i=max_no_seq-1; i >= need; --i)
	Free(&seq[i]);
  if (consenslen < max_no_res / 2) {
    for (i=no_seq; i >= 0; --i) {
      seq[i] = realloc(seq[i], (consenslen+1)*sizeof(char));
      assert(seq[i] != NULL);
    }
  }

  max_no_seq = need;
  max_no_res = consenslen+1;

  cons = Malloc(max_no_res * sizeof(char));
  if (consflag)
    conschar = Malloc(max_no_res * sizeof(char));

  for (i = 0; i < max_no_seq; i++) {
    col[i] = Malloc(max_no_res * sizeof(byte));
    if (seqnumflag)
      prenum[i] = Malloc(max_no_block * sizeof(numtype));
  }
}

/*--------------------------------------------------------------------------*/
/*   main program                                                           */
/*--------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  allocate1();
  //JZ: to avoid the crazy init
  if (argc < 5) {
      explain_cl(NULL);
      exit(0);
  }
  process_command_line(argc, argv);

  ask();
  switch (inputmode) {
    case '1': read_file_pretty();
	      break;
    case '2': read_file_clustal();
	      break;
    case '3': read_file_maligned();
	      break;
    case '4': read_file_esee();
	      break;
    case '5': read_file_phylip();
	      break;
    case '6': read_fasta();
	      break;
  }

  if (seqnumflag && !numdefflag)
    ask_numbers();
  if (seqconsflag)
    ask_seqcons();
  read_cmp();

  make_consensus_length();
  allocate2();
  make_consensus();

  if (ident_sim)
    ident_sim_out();

  make_colors();
  make_lowcase();
  graphics_out();
  return 0;
}



