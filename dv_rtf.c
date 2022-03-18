#include "dv_all.h"

/*----------------------------------------*/
/*   RTF  specific section                */
/*----------------------------------------*/

typedef struct rtfrecord {
  int bgnd, fgnd;
} rtfrecord;

static rtfrecord rtfrec[5];   /*RTF-specifics*/
static int rtfcell;

static void menu_rtf(rtfrecord *rtfrec, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n-------------------------------------------------------------\n"
	   "Enter the text attributes for printing of ***>%s<*** residues:\n", txt);
    printf("First choose the color/gray-value of the letter ***>background<***\n"
	   "Different letters specify different colors,\n"
	   "!! Background colors are not supported on most RTF-programs !!\n"
	   "(R) red     (G) green   (B) blue   (C) cyan\n"
	   "(M) magenta (Y) yellow  (K) black  (W) white\n"
	   "(D) dark gray    (L) light gray\n\n"
	   "choose from  RGBCMYKWDL ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = toupper(code);
  if (strchr("RGBCMYKWDL", code) == NULL)
    code = 'W';
  switch (code) {
    case 'R': rtfrec->bgnd = 2; break;
    case 'G': rtfrec->bgnd = 3; break;
    case 'B': rtfrec->bgnd = 4; break;
    case 'C': rtfrec->bgnd = 5; break;
    case 'M': rtfrec->bgnd = 6; break;
    case 'Y': rtfrec->bgnd = 7; break;
    case 'K': rtfrec->bgnd = 0; break;
    case 'W': rtfrec->bgnd = 1; break;
    case 'D': rtfrec->bgnd = 8; break;
    case 'L': rtfrec->bgnd = 9; break;
  }
  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter ***>foreground<***\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(R) red     (G) green   (B) blue   (C) cyan\n"
	   "(M) magenta (Y) yellow  (K) black  (W) white\n"
	   "(D) dark gray    (L) light gray\n\n"
	   "choose from  RrGgBbCcMmYyKkWwDdLl ( * %c * ) : ", parstring[1]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("RGBCMYKWDL", code) == NULL)
    code = 'K';
  *lc = islower(code);
  code = toupper(code);
  switch (code) {
    case 'R': rtfrec->fgnd = 2; break;
    case 'G': rtfrec->fgnd = 3; break;
    case 'B': rtfrec->fgnd = 4; break;
    case 'C': rtfrec->fgnd = 5; break;
    case 'M': rtfrec->fgnd = 6; break;
    case 'Y': rtfrec->fgnd = 7; break;
    case 'K': rtfrec->fgnd = 0; break;
    case 'W': rtfrec->fgnd = 1; break;
    case 'D': rtfrec->fgnd = 8; break;
    case 'L': rtfrec->fgnd = 9; break;
  }
}


static void ask_rtf(void)
{
  char parstring[6];
  char instring[41];

  if (interactflag)
    printf("-------------------------------------------------------------\n");

  term_par("RTF");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_rtf(rtfrec, lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_rtf(&rtfrec[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_rtf(&rtfrec[2], &lc[2], "similar", parstring);
  else {
    rtfrec[2] = rtfrec[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_rtf(&rtfrec[3], &lc[3], "conserved", parstring);
  else {
    rtfrec[3] = rtfrec[1];
    lc[3] = lc[1];
  }
  rtfrec[4].bgnd = 1;
  rtfrec[4].fgnd = 0;
  lc[4] = FALSE;
  fscanf(parfile, "%d%*[^\n]", &rtfcell);
  getc(parfile);
  if (interactflag) {
    do {
      printf("Character size in Points ( * %2d * ) : ", rtfcell);
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	rtfcell = str2int((void *)instring);
    } while (rtfcell < 1 || rtfcell > 40);
  }
  if (!cloutflag) {
    printf("filename for RTF-output  : ");
    gets(outname);
  }
}


static void RTFinit(double *xpos, double *ypos) {
  dev_minx = 0.0;
  dev_miny = 1.0;
  dev_maxx = 10480.0;
  dev_maxy = 15000.0;
  dev_xsize = rtfcell * 120.0 / 10;
  dev_ysize = rtfcell * 20.0;
  *xpos = dev_minx;
  *ypos = dev_maxy;
  assert(outopen(&outfile, outname) != NULL);
  uwriteln(&outfile, "{\\rtf1\\ansi\\deff0");
  uwriteln(&outfile, "{\\fonttbl{\\f0\\fmodern Courier New;}}");
  uwriteln(&outfile, "{\\info{\\author BOXSHADE}}");
  uwriteln(&outfile, "{\\colortbl\n"
		     "\\red0\\green0\\blue0;"
		     "\\red255\\green255\\blue255;"
		     "\\red255\\green0\\blue0;"
		     "\\red0\\green255\\blue0;"
		     "\\red0\\green0\\blue255;"
		     "\\red0\\green255\\blue255;"
		     "\\red255\\green0\\blue255;"
		     "\\red255\\green255\\blue0;"
		     "\\red128\\green128\\blue128;"
		     "\\red192\\green192\\blue192;}");
  uwriteln(&outfile, "\\paperw11880\\paperh16820\\margl1000\\margr500");
  uwriteln(&outfile, "\\margt910\\margb910\\sectd\\cols1\\pard\\plain");
  uwriteln(&outfile, "\\fs%d", rtfcell * 2);
}

static void RTFsetcolor(int colno) {
  uwriteln(&outfile, "");
  fprintf(outfile.f, "\\highlight%d\\cf%d ",
		   rtfrec[colno].bgnd, rtfrec[colno].fgnd);
}

static void RTFcharout(char c, double *xpos, double *ypos) {
  putc(c, outfile.f);
  *xpos += dev_xsize;
}

static void RTFnewline(double *xpos, double *ypos) {
  uwriteln(&outfile, "\n\\highlight%d\\cf%d \\line",
			 rtfrec[0].bgnd, rtfrec[0].fgnd);
  *xpos = dev_minx;
  *ypos -= dev_ysize;
}

static void RTFnewpage(double *xpos, double *ypos) {
  uwriteln(&outfile, "\\page");
  *xpos = dev_minx;
  *ypos = dev_miny;
}

static void RTFexit(void) {
  uwriteln(&outfile, "}");
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}


GraphicsDevice Rtf = {
  "RTF",
  ask_rtf,
  RTFinit,
  RTFsetcolor,
  RTFcharout,
  GenericStringOut,
  RTFnewline,
  RTFnewpage,
  RTFexit
};
