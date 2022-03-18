#include "dv_all.h"

#ifdef oCRT

typedef struct crtrecord {
  byte bgnd, fgnd;
} crtrecord;
static crtrecord crtrec[5];   /* CRT-specifics*/
static struct text_info ti;

#include <conio.h>

/*----------------------------------------*/
/*      CRT  specific section             */
/*----------------------------------------*/

static void menu_crt(crtrecord *crtrec, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n-------------------------------------------------------------\n"
	   "Enter the text attributes for printing of ***>%s<*** residues:\n",
	   txt);
    printf("First choose the color/gray-value of the letter ***>background<***\n"
	   "Different letters specify different colors,\n"
	   "(R) red     (G) green         (U) blue \n"
	   "(B) black   (L) light gray\n\n"
	   "choose from  RGUYDL ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = toupper(code);
  if (strchr("RGUYDL", code) == NULL)
    code = 'R';
  switch (code) {
    case 'B': crtrec->bgnd = BLACK;      break;
    case 'R': crtrec->bgnd = RED;        break;
    case 'G': crtrec->bgnd = GREEN;      break;
    case 'U': crtrec->bgnd = BLUE;       break;
    case 'L': crtrec->bgnd = LIGHTGRAY;  break;
  }
  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter **>foreground<**\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(R,r) red    (G,g) green  (U,u) blue        (Y,y) yellow\n"
	   "(B,b) black  (W,w) white  (D,d) dark gray   (L,l) light gray\n\n"
	   "choose from  RrGgUuYyBbWwDdLl ( * %c * ) : ", parstring[1]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("BWRGUYDL", toupper(code)) == NULL)
    code = 'B';

  *lc = islower(code);

  code = toupper(code);
  switch (code) {
    case 'B': crtrec->fgnd = BLACK;     break;
    case 'W': crtrec->fgnd = WHITE;     break;
    case 'R': crtrec->fgnd = RED;       break;
    case 'G': crtrec->fgnd = GREEN;     break;
    case 'U': crtrec->fgnd = BLUE;      break;
    case 'Y': crtrec->fgnd = YELLOW;    break;
    case 'D': crtrec->fgnd = DARKGRAY;  break;
    case 'L': crtrec->fgnd = LIGHTGRAY; break;
  }
}


static void ask_crt(void)
{
  char parstring[6];

  term_par("CRT");
  if (interactflag)
    printf("-------------------------------------------------------------\n");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_crt(crtrec, lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_crt(&crtrec[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_crt(&crtrec[2], &lc[2], "similar", parstring);
  else {
    crtrec[2] = crtrec[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_crt(&crtrec[3], &lc[3], "conserved", parstring);
  else {
    crtrec[3] = crtrec[1];
    lc[3] = lc[1];
  }
  crtrec[4] = crtrec[0];
  lc[4] = lc[0];
}


static void CRTinit(double *xpos, double *ypos) {
  gettextinfo(&ti);
  dev_minx = 0.0;
  dev_miny = 1.0;
  dev_maxx = ti.screenwidth;
  dev_maxy = ti.screenheight;
  dev_xsize = 1.0;
  dev_ysize = 1.0;
  *xpos = dev_minx;
  *ypos = dev_maxy;
  textbackground(crtrec[0].bgnd);
  textcolor(crtrec[0].fgnd);
  clrscr();
}

static void CRTsetcolor(int colno) {
  textbackground(crtrec[colno].bgnd);
  textcolor(crtrec[colno].fgnd);
}

static void CRTcharout(char c, double *xpos, double *ypos) {
  putch(c);
  *xpos += dev_xsize;
}

static void CRTnewline(double *xpos, double *ypos) {
  textbackground(crtrec[0].bgnd);
  textcolor(crtrec[0].fgnd);
  cprintf("\r\n");
  *xpos = dev_minx;
  *ypos -= dev_ysize;
}

static void CRTnewpage(double *xpos, double *ypos) {
  scanf("%*[^\n]");
  getchar();
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void CRTexit(void) {
  normvideo();
  clreol();
}

GraphicsDevice Crt = {
  "CRT",
  ask_crt,
  CRTinit,
  CRTsetcolor,
  CRTcharout,
  GenericStringOut,
  CRTnewline,
  CRTnewpage,
  CRTexit
};

#endif
