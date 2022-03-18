#include "dv_all.h"

#define CMD "\x9b"

static char *pre_reset;
static char *pre[5];

/*--------------------------------------------------------------------------*/
/*   LJ250 specific section                                                 */
/*--------------------------------------------------------------------------*/

static void menu_lj250(char **pre, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[51];

  if (interactflag) {
    printf("\n"
	   "-------------------------------------------------------------\n"
	   "Enter the text attributes for printing of >%s< residues:\n", txt);
    printf("Different letters specify different colors,\n"
	   "lowercase choices mean lowercase residues:\n"
	   "(B,b) black,normal    (E,e) black, emphasized    (L,l) Blue\n"
	   "(C,c) Cyan (M,m) Magenta (Y,y) Yellow (G,g) Green (R,r) Red\n\n"
	   "choose from BbEeLlCcMmYyGgRr (* %c *) : ", parstring[0]);
    Fgets(instring, 51, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  if (strchr("BELCMYGR", toupper(code)) == NULL)
    code = 'B';
  *lc = islower(code);
  code = tolower(code);
  switch (code) {
    case 'b': *pre = CMD "0m";      break;
    case 'e': *pre = CMD "0;1m";    break;
    case 'l': *pre = CMD "0;34m";   break;
    case 'c': *pre = CMD "0;36m";   break;
    case 'm': *pre = CMD "0;35m";   break;
    case 'y': *pre = CMD "0;33m";   break;
    case 'g': *pre = CMD "0;32m";   break;
    case 'r': *pre = CMD "0;31m";   break;
  }
}


static void ask_lj250(void)
{
  char parstring[6];

  term_par("LJ250");
  if (interactflag)
    printf("-------------------------------------------------------------\n");
  pre_reset = CMD "0m";
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_lj250(&pre[0], lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_lj250(&pre[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_lj250(&pre[2], &lc[2], "similar", parstring);
  else {
    pre[2] = pre[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_lj250(&pre[3], &lc[3], "conserved", parstring);
  else {
    pre[3] = pre[1];
    lc[3] = lc[1];
  }
  pre[4] = CMD "0m";
  lc[4] = FALSE;
  if (!cloutflag) {
    printf("name of print-file  : ");
    gets(outname);
  }
}

static void LJinit(double *xpos, double *ypos) {
  assert(outopen(&outfile, outname) != NULL);
  fputs(pre_reset, outfile.f);
  dev_minx = 0.0;
  dev_miny = 1.0;
  dev_maxx = 80.0;
  dev_maxy = 63.0;
  dev_xsize = 1.0;
  dev_ysize = 1.0;
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void LJsetcolor(int colno) {
  fputs(pre[colno], outfile.f);
}

static void LJcharout(char c, double *xpos, double *ypos) {
  putc(c, outfile.f);
  *xpos += dev_xsize;
}

static void LJnewline(double *xpos, double *ypos) {
  uwriteln(&outfile, "");
  *xpos = dev_minx;
  *ypos += dev_ysize;
}

static void LJnewpage(double *xpos, double *ypos) {
  uwriteln(&outfile, "\f");
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void LJexit(void) {
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Lj250 = {
  "LJ250",
  ask_lj250,
  LJinit,
  LJsetcolor,
  LJcharout,
  GenericStringOut,
  LJnewline,
  LJnewpage,
  LJexit
};
