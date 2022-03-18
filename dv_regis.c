#include "dv_all.h"
#include <math.h>

/*--------------------------------------------------------------------------*/
/*   ReGIS specific section                                                 */
/*--------------------------------------------------------------------------*/

#define ESC "\x1b"

typedef char regisstring[31];

static regisstring regstr[5];   /*ReGIS-specifics*/

static void menu_regis(char *regstr, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[51];

  *regstr = '\0';
  if (interactflag) {
    printf("\n-----------------------------------------------------------------------\n"
	   "Enter the text attributes for printing of **>%s<** residues:\n", txt);
    printf("First choose the color/gray-value of the letter **>background<**\n"
	   "Different letters specify different colors,\n"
	   "(B) black  (W) white       (L) Blue (G) Green   (R) Red\n"
	   "                           (C) Cyan (M) Magenta (Y) Yellow \n"
	   "(1,2,3,4) 4 different gray values, 4 is darkest\n\n"
	   "choose from  BWLGRCMY1234 (* %c *) : ", parstring[0]);
    Fgets(instring, 51, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = tolower(code);
  if (strchr("bwlgrcmy1234", code) == NULL)
    code = 'w';
  switch (code) {
    case 'b': strcat(regstr, "S(I(D));");        break;
    case 'w': strcat(regstr, "S(I(W));");        break;
    case 'l': strcat(regstr, "S(I(B));");        break;
    case 'g': strcat(regstr, "S(I(G));");        break;
    case 'r': strcat(regstr, "S(I(R));");        break;
    case 'c': strcat(regstr, "S(I(C));");        break;
    case 'm': strcat(regstr, "S(I(M));");        break;
    case 'y': strcat(regstr, "S(I(Y));");        break;
    case '1': strcat(regstr, "S(I(H0L80S0));");  break;
    case '2': strcat(regstr, "S(I(H0L60S0));");  break;
    case '3': strcat(regstr, "S(I(H0L40S0));");  break;
    case '4': strcat(regstr, "S(I(H0L20S0));");  break;
  }
  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter **>foreground<**\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(B,b) black  (W,w) white   (L,l) Blue  (G,g) Green  (R,r) Red\n"
	   "(C,c) Cyan   (M,m) Magenta (Y,y) Yellow \n"
	   "(1,2,3,4) 4 different gray values, 4 is darkest\n"
	   "(5,6,7,8) 4 different gray values, 8 is darkest, but lowercase letters\n\n"
	   "choose from  BWLGRCMY1234/bwlgrcmy5678 (* %c *) : ", parstring[1]);
    Fgets(instring, 51, stdin);
    printf("\n\n");
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("BWLGRCMY12345678", toupper(code)) == NULL)
    code = 'B';
  *lc = islower(code);
  if (code == '4' || code == '3' || code == '2' || code == '1')
    *lc = FALSE;
  code = tolower(code);
  switch (code) {
    case 'b': strcat(regstr, "W(I(D));");       break;
    case 'w': strcat(regstr, "W(I(W));");       break;
    case 'l': strcat(regstr, "W(I(B));");       break;
    case 'g': strcat(regstr, "W(I(G));");       break;
    case 'r': strcat(regstr, "W(I(R));");       break;
    case 'c': strcat(regstr, "W(I(C));");       break;
    case 'm': strcat(regstr, "W(I(M));");       break;
    case 'y': strcat(regstr, "W(I(Y));");       break;
    case '1':
    case '5': strcat(regstr, "W(I(H0L80S0));"); break;
    case '2':
    case '6': strcat(regstr, "W(I(H0L60S0));"); break;
    case '3':
    case '7': strcat(regstr, "W(I(H0L40S0));"); break;
    case '4':
    case '8': strcat(regstr, "W(I(H0L20S0));"); break;
  }
}


static void ask_regis(void)
{
  char parstring[6];

  if (interactflag)
    printf("-----------------------------------------------------------------------\n");
  term_par("ReGIS");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_regis(regstr[0], &lc[0], "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_regis(regstr[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_regis(regstr[2], &lc[2], "similar", parstring);
  else {
    strcpy(regstr[2], regstr[0]);
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_regis(regstr[3], &lc[3], "conserved", parstring);
  else {
    strcpy(regstr[3], regstr[1]);
    lc[3] = lc[1];
  }
  strcpy(regstr[4], "S(I(W));W(I(D));");
  lc[4] = FALSE;
  if (!cloutflag) {
    printf("filename for REGIS-output  : ");
    gets(outname);
  }
}


static void DVinit(regmodetype regmode, double *xpos, double *ypos) {
  dev_minx = 0.0;
  dev_miny = 0.0;
  dev_maxx = 799.0;
  if (regmode == rTERM) dev_maxy = 479.0;
		   else dev_maxy = 1199.0;
  dev_xsize = 9.0;
  dev_ysize = 20.0;
  assert(outopen(&outfile, outname) != NULL);
  uwriteln(&outfile, "%s", ESC "[?38l" ESC "Pp");
  uwriteln(&outfile, "S(I(W)E);S(C0);S(A[0,0][%ld,%ld]);",
	   (long)floor(dev_maxx + 0.5), (long)floor(dev_maxy + 0.5));
  uwriteln(&outfile, "P[0,0];W(R);T(S1)");
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void RTinit(double *xpos, double *ypos) { DVinit(rTERM, xpos, ypos); }
static void RPinit(double *xpos, double *ypos) { DVinit(rPAGE, xpos, ypos); }

static void Rsetcolor(int colno) {
  fputs(regstr[colno], outfile.f);
}

static void Rcharout(char c, double *xpos, double *ypos) {
  uwriteln(&outfile, "T\"%c\";", c);
  *xpos += dev_xsize;
}

static void Rnewline(double *xpos, double *ypos) {
  *xpos = dev_minx;
  *ypos += dev_ysize;
  uwriteln(&outfile, "P[0,%ld];", (long)floor(*ypos + 0.5));
}

static void RPnewpage(double *xpos, double *ypos) {
  uwriteln(&outfile, "S(FI(W)E);S(C0);S(A[0,0][%ld,%ld]);",
	  (long)floor(dev_maxx + 0.5), (long)floor(dev_maxy + 0.5));
  uwriteln(&outfile, "P[0,0];W(R);T(S1)");
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void RTnewpage(double *xpos, double *ypos) {
  putchar('\007');
  scanf("%*[^\n]");
  getchar();
  RPnewpage(xpos, ypos);
}

static void Rexit(void) {
  uwriteln(&outfile, "%s", ESC "\\" ESC "[23;1H");
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}



GraphicsDevice RegisT = {
  "REGISt",
  ask_regis,
  RTinit,
  Rsetcolor,
  Rcharout,
  GenericStringOut,
  Rnewline,
  RTnewpage,
  Rexit
};

GraphicsDevice RegisP = {
  "REGISp",
  ask_regis,
  RPinit,
  Rsetcolor,
  Rcharout,
  GenericStringOut,
  Rnewline,
  RPnewpage,
  Rexit
};
