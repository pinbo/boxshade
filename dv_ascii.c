#include "dv_all.h"


static char acs[5];   /*ASCII sprecific vars*/
static char ascii_char;


/*------------------------------------*/
/*   ASCII specific section           */
/*------------------------------------*/

static void menu_ascii(char *acs, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n-------------------------------------------------------------\n"
	   "This output method should only be used in conjunction with\n"
	   "\"shading according to a single sequence\"\n"
	   "Enter the symbol used for printing of >%s< residues:\n", txt);
    printf("(L,l) the letter itself, in upper- or lower case, respectively\n"
	   "or type any other on-character symbol (like . or - )\n"
	   "the blank-character should be enclosed in quotes\n\n"
	   "type one character ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (strlen(instring) == 3)
    instring[0] = instring[1];
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  *lc = islower(code);
  code = toupper(code);
  *acs = code;
}


static void ask_ascii(void)
{
  char parstring[6];

  term_par("ASCII");
  if (interactflag)
    printf("-------------------------------------------------------------\n");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_ascii(acs, lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_ascii(&acs[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_ascii(&acs[2], &lc[2], "similar", parstring);
  else {
    acs[2] = acs[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_ascii(&acs[3], &lc[3], "conserved", parstring);
  else {
    acs[3] = acs[1];
    lc[3] = lc[1];
  }
  acs[4] = 'L';
  lc[4] = FALSE;
  if (!cloutflag) {
    printf("filename for ASCII-output (return for terminal)  : ");
    gets(outname);
  }
}


static void ASCinit(double *xpos, double *ypos) {
  if (*outname == '\0') {
    outfile.f = stdout;
    outfile.bin = FALSE;
  } else
    assert(outopen(&outfile, outname) != NULL);
  dev_minx = 0.0;
  dev_miny = 1.0;
  dev_maxx = 255.0;
  dev_maxy = 10000.0;
  dev_xsize = 1.0;
  dev_ysize = 1.0;
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void ASCsetcolor(int colno) {
  ascii_char = acs[colno];
}

static void ASCcharout(char c, double *xpos, double *ypos) {
  if (toupper(ascii_char) == 'L')
    putc(c, outfile.f);
  else
    putc(ascii_char, outfile.f);
  *xpos += dev_xsize;
}

static void ASCnewline(double *xpos, double *ypos) {
  uwriteln(&outfile, "");
  *xpos = dev_minx;
  *ypos += dev_ysize;
}

static void ASCnewpage(double *xpos, double *ypos) {
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void ASCexit(void) {
  uwriteln(&outfile, "");
  if (outfile.f != NULL && outfile.f != stdout)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Ascii = {
  "ASCII",
  ask_ascii,
  ASCinit,
  ASCsetcolor,
  ASCcharout,
  GenericStringOut,
  ASCnewline,
  ASCnewpage,
  ASCexit
};
