#include "dv_all.h"

/*----------------------------------------*/
/*  PICTfile  specific section            */
/*----------------------------------------*/

#define CHR static char

CHR *StrBlack  = "\001\0\0\0\0\0\0";
CHR *StrWhite  = "\002\377\377\377\377\377\377"; /* \377 = 255 */
CHR *StrGrey1  = "\003\335\335\335\335\335\335"; /* \335 = 221 */
CHR *StrGrey2  = "\004\273\273\273\273\273\273"; /* \273 = 187 */
CHR *StrGrey3  = "\005\210\210\210\210\210\210"; /* \210 = 136 */
CHR *StrGrey4  = "\006\125\125\125\125\125\125"; /* \125 =  85 */
CHR *StrRed    = "\007\377\377\063\063\0\0";     /* \377 = 255, \063 =  51 */
CHR *StrPurple = "\010\231\231\0\0\314\314";     /* \231 = 153, \314 = 204 */
CHR *StrDBlue  = "\011\0\0\0\0\335\335";         /* \335 = 221 */
CHR *StrLBlue  = "\012\146\146\377\377\377\377"; /* \146 = 102, \377 = 255 */
CHR *StrGreen  = "\013\0\0\210\210\0\0";         /* \210 = 136 */
CHR *StrYellow = "\014\377\377\377\377\0\0";     /* \377 = 255 */
CHR *StrMagenta= "\015\377\377\0\0\231\231";     /* \377 = 255, \231 = 153 */

CHR *StrHeaderOp = "\0\021\002\377\014\0\377\377\377\377";

CHR *LongTextOp = "\0\050"; /* \050 =  40 */
CHR *RGBfgndOp  = "\0\032"; /* \032 =  26 */
CHR *EndPicOp   = "\0\377"; /* \377 = 255 */
CHR *PaintRectOp= "\0\061"; /* \061 =  49 */

CHR *StrHiLiteClip = "\0\036\0\001\0\012"; /* \036 = 30; \001 = 1, \012 = 10 */

CHR *StrCourierBold=
	"\0\054\0\012\0\026\007" /* \054 = 44, \012 = 10, \026 = 22 */
	"Courier"
	"\0\003\0\026\0\004\001\0\0\015\0"; /* \026 = 22, \015 = 13 */

typedef struct PictRec {
  char *bgnd, *fgnd;
} PictRec;

static PictRec pict[5];   /*PICTfile specifics*/

static int pictcell;      /*PICT files allow only integer values for char size*/
static char CurrentColour;
static PictRec *WantedColour;
static int WantedColNo, xshift, yshift, pcount;
static char StrFormat[8];
static char StrFormat2[16];

static FILE *outf = NULL;

static void menu_pictfile(PictRec *pict, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n----------------------------------------------------------------\n"
	   "Enter the text attributes for printing of **>%s<** residues:\n", txt);
    printf("First choose the color/gray-value of the letter **>background<**\n"
	   "Different letters specify different colors,\n"
	   "(B) Black, (W) white (1,2,3,4) 4 different gray values, 4 is darkest\n"
	   "(R) Red, (P) Purple, (M) Magenta, (Y) Yellow, (G) Green,\n"
	   "(L) Light blue, (D) Dark blue\n\n"
	   "choose from  BWRPMYGLD1234 ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = toupper(code);
  if (strchr("BWRPMGYLD1234", code) == NULL)
    code = 'W';
  switch (code) {
    case 'B': pict->bgnd = StrBlack;   break;
    case 'W': pict->bgnd = StrWhite;   break;
    case '1': pict->bgnd = StrGrey1;   break;
    case '2': pict->bgnd = StrGrey2;   break;
    case '3': pict->bgnd = StrGrey3;   break;
    case '4': pict->bgnd = StrGrey4;   break;
    case 'R': pict->bgnd = StrRed;     break;
    case 'P': pict->bgnd = StrPurple;  break;
    case 'M': pict->bgnd = StrMagenta; break;
    case 'G': pict->bgnd = StrGreen;   break;
    case 'Y': pict->bgnd = StrYellow;  break;
    case 'L': pict->bgnd = StrLBlue;   break;
    case 'D': pict->bgnd = StrDBlue;   break;
  }
  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter **>foreground<**\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(B,b) black  (W,w) white  (1,2,3,4) 4 different gray values, 4 is darkest\n"
	   "                          (5,6,7,8) same, but lowercase letters\n"
	   "(R) Red, (P) Purple, (M) Magenta, (Y) Yellow, (G) Green,\n"
	   "(L) Light blue, (D) Dark blue\n\n"
	   "choose from  BbWwRrPpMmYyGgLlDd12345678 ( * %c * ) : ",
	   parstring[1]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("BWRPMGYLD12345678", toupper(code)) == NULL)
    code = 'B';
  *lc = islower(code);
  if (code == '4' || code == '3' || code == '2' || code == '1')
    *lc = FALSE;
  if (code == '8' || code == '7' || code == '6' || code == '5')
    *lc = TRUE;
  code = toupper(code);
  switch (code) {
    case 'B': pict->fgnd = StrBlack;   break;
    case 'W': pict->fgnd = StrWhite;   break;
    case '5':
    case '1': pict->fgnd = StrGrey1;   break;
    case '6':
    case '2': pict->fgnd = StrGrey2;   break;
    case '7':
    case '3': pict->fgnd = StrGrey3;   break;
    case '8':
    case '4': pict->fgnd = StrGrey4;   break;
    case 'R': pict->fgnd = StrRed;     break;
    case 'P': pict->fgnd = StrPurple;  break;
    case 'M': pict->fgnd = StrMagenta; break;
    case 'G': pict->fgnd = StrGreen;   break;
    case 'Y': pict->fgnd = StrYellow;  break;
    case 'L': pict->fgnd = StrLBlue;   break;
    case 'D': pict->fgnd = StrDBlue;   break;
  }
}


static void ask_pict(void)
{
  char parstring[6];
  char instring[41];

  splitflag = TRUE;

  /*although you can have multipage PICTs, I have defined the boundary of
    the ones created here as an A4 page. Very large PICTs, in any event, with
    1000s of objects, might give the average drawing program a headache!*/

  if (interactflag)
    printf("----------------------------------------------------------------\n");
  term_par("PICTFILE");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_pictfile(&pict[0], &lc[0], "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_pictfile(&pict[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_pictfile(&pict[2], &lc[2], "similar", parstring);
  else {
    pict[2] = pict[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_pictfile(&pict[3], &lc[3], "conserved", parstring);
  else {
    pict[3] = pict[1];
    lc[3] = lc[1];
  }
  pict[4].bgnd = StrWhite;
  pict[4].fgnd = StrBlack;
  lc[4] = FALSE;
  fscanf(parfile, "%d%*[^\n]", &pictcell);
  getc(parfile);
  Fgets(parstring, 6, parfile);
  shadeonlyflag = toupper(parstring[0]) == 'S';
  Fgets(parstring, 6, parfile);
  landscapeflag = toupper(parstring[0]) == 'Y';
  if (interactflag) {
    do {
      printf("Character size in Points(5-20) ( * %4d * ) : ", pictcell);
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	pictcell = str2int((void *)instring);
    } while (pictcell < 5 || pictcell > 20);
    /*rather arbitrary limitation, but should cover most people's needs*/

    printf("Save shading (S) or Shading+Text (T) (* %c *) ",
						(shadeonlyflag ? 'S' : 'T'));
    Fgets(instring, 41, stdin);
    if (*instring != '\0')
      shadeonlyflag = toupper(instring[0]) == 'S';

    printf("Rotate page  ( * %c * ) : ", YESNO(landscapeflag));
    Fgets(instring, 41, stdin);
    if (*instring != '\0')
      landscapeflag = toupper(instring[0]) == 'Y';
  }
  if (!cloutflag) {
    printf("filename for PICT file output  : ");
    Gets(outname);
  }
}


static void MakePoint(int *xpoint, int *ypoint)
{
  /*converts a pair of points into a string containing the values as hex*/
  /*in the order yvalue (hi), yvalue(lo), xvalue(hi), xvalue(lo)*/
  putc( (*ypoint >> 8)&255, outf);
  putc( (*ypoint     )&255, outf);
  putc( (*xpoint >> 8)&255, outf);
  putc( (*xpoint     )&255, outf);
}


static void PICTinit(double *xpos, double *ypos) {
  int i, linelen, tlx, tly, brx, bry;

  if (landscapeflag) {
    dev_miny = 30.0;   /* borders of an A4 page in point units */
    dev_minx = 30.0;
    dev_maxy = 565.0;
    dev_maxx = 800.0;
  } else {
    dev_minx = 30.0;   /* borders of an A4 page in point units */
    dev_miny = 30.0;
    dev_maxx = 565.0;
    dev_maxy = 800.0;
  }
  dev_xsize = pictcell * 0.85;   /* cells do overlap in x-direction */
  dev_ysize = pictcell;
  *xpos = dev_minx;
  *ypos = dev_miny + dev_ysize;
  /* 0,0 is top left corner, and we increase ypos to go down the page*/
  lines_per_page = (int)((dev_maxy - dev_miny) / dev_ysize);

  if (pictcell < 15) { xshift = 1; yshift = 2; }
		else { xshift = 2; yshift = 3; }
  /* as the size of the unit cell Gets larger, the shift of the
     character to place it nicely in the "box" changes; I haven't worked
     out a nice formula that gives integer values that work for any
     number of points */

  outf = fopen(outname, BIN_WR);
  assert(outf != NULL);

  /* Calculate here the TopLeft and BottomRight corners of the PICT, based on
     the size of the image and the page dimensions entered above: changing
     these to reflect, e.g. US Letter paper will result in a changed 'window'
     in which the program will draw the PICT. Note that BOXSHADE allows both
     PICT and EPSF to be *wider* than the defined page (i.e. it doesn't check
     for user errors over what will fit), but it does split the output into
     separate files of one 'page-full' (or less) */

  linelen = outlen;
  if (seqnameflag)
    linelen += seqname_outlen + 1;
  if (seqnumflag)
    linelen += seqnumlen + 1;
  tly = (int)dev_minx;
  tlx = (int)dev_miny;

  if (lines_left < lines_per_page)
      brx = tly + (int)(0.5 + (lines_left + 1) * dev_ysize);
  else
      brx = tly + (int)(0.5 + lines_per_page * dev_ysize);

  bry = tlx + (int)(0.5 + linelen * dev_xsize);

  StrFormat[0] = tlx / 256;
  StrFormat[1] = tlx & 255;
  StrFormat[2] = tly / 256;
  StrFormat[3] = tly & 255;
  StrFormat[4] = brx / 256;
  StrFormat[5] = brx & 255;
  StrFormat[6] = bry / 256;
  StrFormat[7] = bry & 255;

  for (i = 0; i <= 15; i++)
      StrFormat2[i] = '\0';  /*yes, I know I'm being lazy!*/

  StrFormat2[4] = tlx / 256;
  StrFormat2[5] = tlx & 255;
  StrFormat2[0] = tly / 256;
  StrFormat2[1] = tly & 255;
  StrFormat2[12] = brx / 256;
  StrFormat2[13] = brx & 255;
  StrFormat2[8] = bry / 256;
  StrFormat2[9] = bry & 255;

  /* I have to write a header of 512 bytes plus 2 bytes for the
     PICT size field, which fortunately is never read. */

  for (i = 1; i <= 514; i++)
      putc(0, outf);

  /* now we write the various header opcodes and size for the file */
  for (i = 0; i <= 7; i++)
      putc(StrFormat[i], outf);
  for (i = 0; i <= 9; i++)
      putc(StrHeaderOp[i], outf);
  for (i = 0; i <= 15; i++)
      putc(StrFormat2[i], outf);
  for (i = 1; i <= 4; i++)
      putc(0, outf);
  for (i = 0; i <= 5; i++)
      putc(StrHiLiteClip[i], outf);
  for (i = 0; i <= 7; i++)
      putc(StrFormat[i], outf);
  for (i = 0; i <= 24; i++)
      putc(StrCourierBold[i], outf);
  putc(pictcell, outf);
  pcount = 592;
}

static void PICTsetcolor(int colno) {
  WantedColour = &pict[colno];
  WantedColNo = colno;
}

static void PICTcharout(char c, double *xpos, double *ypos) {
  int xpoint, ypoint, i;

  if (WantedColNo < 4) {
     if (CurrentColour != WantedColour->bgnd[0]) {
       if (pcount > 1016) {
	 for (i = pcount; i <= 1023; i++)
	     putc(0, outf);
	 putc('\n', outf);
	 pcount = 0;
       }
       fprintf(outf, "%c%c", RGBfgndOp[0], RGBfgndOp[1]);
       for (i = 1; i <= 6; i++)
	   putc(WantedColour->bgnd[i], outf);
       CurrentColour = WantedColour->bgnd[0];
       pcount += 8;
     }

     if (pcount > 1014) {
       for (i = pcount; i <= 1023; i++)
	   putc(0, outf);
       putc('\n', outf);
       pcount = 0;
     }
     fprintf(outf, "%c%c", PaintRectOp[0], PaintRectOp[1]);
     xpoint = (int)(*xpos - xshift + 0.5);
     ypoint = (int)(*ypos + yshift - dev_ysize + 0.5);
     MakePoint(&xpoint, &ypoint);
     xpoint = (int)(*xpos - xshift + dev_xsize + 0.5);
     ypoint = (int)(*ypos + yshift + 0.5);
     MakePoint(&xpoint, &ypoint);
     pcount += 10;
  }

  if (!shadeonlyflag) {  /*print the text*/
     if (CurrentColour != WantedColour->fgnd[0]) {
       if (pcount > 1016) {
	      for (i = pcount; i <= 1023; i++)
		     putc(0, outf);
	      putc('\n', outf);
	      pcount = 0;
       }
       fprintf(outf, "%c%c", RGBfgndOp[0], RGBfgndOp[1]);
       for (i = 1; i <= 6; i++)
	      putc(WantedColour->fgnd[i], outf);
       CurrentColour = WantedColour->fgnd[0];
       pcount += 8;
     }

     if (pcount > 1016) {
       for (i = pcount; i <= 1023; i++)
	      putc(0, outf);
       putc('\n', outf);
       pcount = 0;
     }
     fprintf(outf, "%c%c", LongTextOp[0], LongTextOp[1]);
     xpoint = (int)(*xpos + 0.5);
     ypoint = (int)(*ypos + 0.5);
     MakePoint(&xpoint, &ypoint);
     fprintf(outf, "\001%c", c);
     pcount += 8;
  }
  *xpos += dev_xsize;   /*need to move anyway*/
}

static void PICTnewline(double *xpos, double *ypos) {
  *xpos = dev_minx;
  *ypos += dev_ysize;
}

static void PICTnewpage(double *xpos, double *ypos) {
  /* this should never be accessed - /split is enforced with
     PICT files to stop them becoming too large */
  *xpos = dev_minx;
  *ypos = dev_miny + dev_ysize;
}

static void PICTexit(void) {
  int i;
  if (pcount > 1022) {
	  for (i = pcount; i <= 1023; i++)
	    putc(0, outf);
	  putc('\n', outf);
	  pcount = 0;
  }
  fprintf(outf, "%c%c", EndPicOp[0], EndPicOp[1]);
  for (i = pcount; i <= 1021; i++)
	  putc(0, outf);
  if (outf != NULL)
	  fclose(outf);
  outf = NULL;
}


GraphicsDevice Pict = {
  "PICT",
  ask_pict,
  PICTinit,
  PICTsetcolor,
  PICTcharout,
  GenericStringOut,
  PICTnewline,
  PICTnewpage,
  PICTexit
};
