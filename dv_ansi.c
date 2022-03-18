#include "dv_all.h"

#define ESC "\x1b"

static char *pre_reset;
static char *pre[5];

/*------------------------------------*/
/*   ANSI-Terminal specific section   */
/*------------------------------------*/

static void menu_ansi(char **pre, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n-------------------------------------------------------------\n");
    printf("Enter the text attributes for printing of >%s< residues:\n", txt);
    printf("Different letters specify renditions,\n"
	   "lowercase choices mean lowercase residues:\n"
	   "(N,n) normal    (B,b) bold   (U,u) underscore  (R,r) reverse\n"
	   "(F,f) flashing  (E,e) reverse/bold (L,l) reverse/flashing\n\n"
	   "choose from NnBbUuRrFfEeLl ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  if (strchr("NBURELF", toupper(code)) == NULL)
    code = 'N';
  *lc = islower(code);
  code = toupper(code);
  switch (code) {
    case 'N': *pre = ESC "[0m";     break;
    case 'B': *pre = ESC "[0;1m";   break;
    case 'U': *pre = ESC "[0;4m";   break;
    case 'R': *pre = ESC "[0;7m";   break;
    case 'E': *pre = ESC "[0;1;7m"; break;
    case 'L': *pre = ESC "[0;5;7m"; break;
    case 'F': *pre = ESC "[0;5m";   break;
  }
}


static void ask(char *term)
{
  char parstring[6];

  term_par(term);
  if (interactflag)
    printf("-------------------------------------------------------------\n");
  pre_reset = ESC "[0m";
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_ansi(&pre[0], lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_ansi(&pre[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_ansi(&pre[2], &lc[2], "similar", parstring);
  else {
    pre[2] = pre[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_ansi(&pre[3], &lc[3], "conserved", parstring);
  else {
    pre[3] = pre[1];
    lc[3] = lc[1];
  }
  pre[4] = ESC "[0m";
  lc[4] = FALSE;
  if (!cloutflag) {
    printf("filename for %s-output (return for terminal)  : ", term);
    gets(outname);
  }
}

static void ask_vt(void) {
  ask("VT");
}

static void ask_ansi(void) {
  ask("ANSI");
}

/* ------------------- output functions */

static void VTinit(double *xpos, double *ypos) {
  if (*outname == '\0') {
    outfile.f = stdout;
    outfile.bin = FALSE;
  } else
    assert(outopen(&outfile, outname) != NULL);
  fputs(ESC "[2J" ESC "[0;0H", outfile.f);
  fputs(pre_reset, outfile.f);
  dev_minx = 0.0;
  dev_miny = 1.0;
  dev_maxx = 80.0;
  dev_maxy = 25.0;
  dev_xsize = 1.0;
  dev_ysize = 1.0;
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void VTsetcolor(int colno) {
  fputs(pre[colno], outfile.f);
}

static void VTcharout(char c, double *xpos, double *ypos) {
  putc(c, outfile.f);
  *xpos += dev_xsize;
}

static void VTnewline(double *xpos, double *ypos) {
  uwriteln(&outfile, pre_reset);
  *xpos = dev_minx;
  *ypos += dev_ysize;
}

static void VTnewpage(double *xpos, double *ypos) {
  printf("\007"); fflush(stdout);
  scanf("%*[^\n]");
  getchar();
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void VTexit(void) {
  uwriteln(&outfile, pre_reset);
  if (outfile.f != NULL && outfile.f != stdout)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Ansi = {
  "ANSI",
  ask_ansi,
  VTinit,
  VTsetcolor,
  VTcharout,
  GenericStringOut,
  VTnewline,
  VTnewpage,
  VTexit
};

GraphicsDevice Vt = {
  "VT100",
  ask_vt,
  VTinit,
  VTsetcolor,
  VTcharout,
  GenericStringOut,
  VTnewline,
  VTnewpage,
  VTexit
};

