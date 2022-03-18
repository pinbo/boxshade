#include "dv_all.h"

/*--------------------------------------------------------------------------*/
/*   fig specific section                                                   */
/*--------------------------------------------------------------------------*/

typedef struct figrecord {
  int bgnd, fgnd, fontno;
} figrecord;

static int fig_fontsize;         /* XFIG specifics */
static int fig_fontno_offset;   /* 0=Tms,12=Cour,16=Helv,28=Palat */
static figrecord figrec[5];

static int fig_fontno;           /* current values go here*/
static int fig_fgnd, fig_bgnd;   /*            ... to here*/

static void menu_fig(figrecord *figrec, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[51];

  if (interactflag) {
    printf("\n----------------------------------------------------------------\n"
	   "Enter the text attributes for printing of **>%s<** residues:\n",
	   txt);
    printf("First choose the color/gray-value of the letter **>background<**\n"
	   "Different letters specify different colors,\n"
	   "(B) black  (W) white (1,2,3,4) 4 different gray values, 4 is darkest\n\n"
	   "choose from  BW1234 ( * %c * ) : ", parstring[0]);
    Fgets(instring, 51, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = toupper(code);
  if (strchr("BW1234", code) == NULL)
    code = 'W';
  switch (code) {
    case 'B': figrec->bgnd = 21; break;
    case 'W': figrec->bgnd = 0;  break;
    case '1': figrec->bgnd = 4;  break;
    case '2': figrec->bgnd = 8;  break;
    case '3': figrec->bgnd = 12; break;
    case '4': figrec->bgnd = 16; break;
  }
  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter **>foreground<**\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(B,b) black,normal    (E,e) black, emphasized\n"
	   "(W,w) white normal    (F,f) white, emphasized\n"
	   "(L,l) Blue (M,m) Magenta (Y,y) Yellow (G,g) Green (R,r) Red\n\n"
	   "choose from  BbEeWwFfLlMmYyGgRr ( * %c * ) : ", parstring[1]);
    Fgets(instring, 51, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("BEWFLMYGR", toupper(code)) == NULL)
    code = 'B';
  *lc = islower(code);
  code = toupper(code);
  switch (code) {
    case 'B':
      figrec->fgnd = 0;
      figrec->fontno = 0;
      break;
    case 'E':
      figrec->fgnd = 0;
      figrec->fontno = 2;
      break;
    case 'W':
      figrec->fgnd = 7;
      figrec->fontno = 0;
      break;
    case 'F':
      figrec->fgnd = 7;
      figrec->fontno = 2;
      break;
    case 'L':
      figrec->fgnd = 1;
      figrec->fontno = 0;
      break;
    case 'M':
      figrec->fgnd = 5;
      figrec->fontno = 0;
      break;
    case 'Y':
      figrec->fgnd = 6;
      figrec->fontno = 0;
      break;
    case 'G':
      figrec->fgnd = 2;
      figrec->fontno = 0;
      break;
    case 'R':
      figrec->fgnd = 4;
      figrec->fontno = 0;
      break;
  }
}


static void ask_fig(void)
{
  char parstring[6];
  char instring[41];

  term_par("FIG");
  if (interactflag)
    printf("-------------------------------------------------------------\n");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_fig(&figrec[0], &lc[0], "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_fig(&figrec[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_fig(&figrec[2], &lc[2], "similar", parstring);
  else {
    figrec[2] = figrec[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_fig(&figrec[3], &lc[3], "conserved", parstring);
  else {
    figrec[3] = figrec[1];
    lc[3] = lc[1];
  }
  figrec[4].bgnd = 0;
  figrec[4].fgnd = 0;
  figrec[4].fontno = 0;
  lc[4] = FALSE;

  fscanf(parfile, "%d%*[^\n]", &fig_fontno_offset);
  getc(parfile);
  fscanf(parfile, "%d%*[^\n]", &fig_fontsize);
  getc(parfile);
  if (interactflag) {
    do {
      printf("Character size in Points ( * %4d * ) : ", fig_fontsize);
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	fig_fontsize = str2int((void *)instring);
    } while (fig_fontsize < 1 || fig_fontsize > 100);
  }
  if (!cloutflag) {
    printf("name of fig-file  : ");
    Gets(outname);
  }
}


static void FIGinit(double *xpos, double *ypos) {
  assert(outopen(&outfile, outname) != NULL);
  dev_minx = fig_fontsize * 4.0;
  dev_miny = fig_fontsize * 4.0;
  dev_maxx = 10000.0;
  dev_maxy = 10000.0;
  dev_xsize = fig_fontsize + 1.0;
  dev_ysize = fig_fontsize;
  *xpos = dev_minx;
  *ypos = dev_miny;
  uwriteln(&outfile, "#FIG 2.1");
  uwriteln(&outfile, "80 2");
}

static void FIGsetcolor(int colno) {
  dev_xsize = fig_fontsize + 1.0;
  fig_fontno = fig_fontno_offset + figrec[colno].fontno;
  fig_fgnd = figrec[colno].fgnd;
  fig_bgnd = figrec[colno].bgnd;
}

static void FIGcharout(char c, double *xpos, double *ypos) {

  if (fig_bgnd > 0) {  /*draw box*/
    double bxshift, byshift;
    int bx1, by1, bx2, by2;

    bxshift = fig_fontsize * 0.25;
    byshift = fig_fontsize * 0.15;
    bx1 = (int)(*xpos + 0.5 - bxshift);
    bx2 = (int)(*xpos + dev_xsize + 0.5 - bxshift);
    by2 = (int)(*ypos + 0.5 + byshift);
    by1 = (int)(*ypos - dev_ysize + 0.5 + byshift);

    /*compound object*/
    uwriteln(&outfile, "6 %5d%5d%5d%5d", bx2, by1, bx1, by2);
    uwriteln(&outfile, "2 2 0 0 0 5 0 "    /*box*/
		       "%3d 0.0 0 0 0", fig_bgnd);
    fprintf(outfile.f, "\t ");
    fprintf(outfile.f, "%5d%5d%5d%5d%5d%5d", bx2, by2, bx2, by1, bx1, by1);
    uwriteln(&outfile, "%5d%5d%5d%5d 9999 9999", bx1, by2, bx2, by2);
  }

  fprintf(outfile.f, "4 0 %2d%3d", fig_fontno, fig_fontsize);
  fprintf(outfile.f, " 0 %2d 0 0.0 4 ", fig_fgnd);
  fprintf(outfile.f, "%3ld%3ld",(long)(dev_ysize+0.5), (long)(dev_xsize+0.5));
  fprintf(outfile.f, "%5ld%5ld %c",(long)(*xpos+0.5), (long)(*ypos+0.5), c);
  uwriteln(&outfile, "\001");

  if (fig_bgnd > 0)   /*end of compound*/
    uwriteln(&outfile, "-6");

  *xpos += dev_xsize;
}

static void FIGnewline(double *xpos, double *ypos) {
  *xpos = dev_minx;
  *ypos += dev_ysize;
}

static void FIGnewpage(double *xpos, double *ypos) {
  /* this should never happen*/
  FIGnewline(xpos, ypos);
}

static void FIGexit(void) {
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Fig = {
  "fig",
  ask_fig,
  FIGinit,
  FIGsetcolor,
  FIGcharout,
  GenericStringOut,
  FIGnewline,
  FIGnewpage,
  FIGexit
};
