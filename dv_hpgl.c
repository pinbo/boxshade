#include "dv_all.h"

#include <stdarg.h>

/*----------------------------------------*/
/*   HPGL  specific section               */
/*----------------------------------------*/

typedef struct hpglrecord {
  char *hatch;
  int bgnd, fgnd;
} hpglrecord;

static hpglrecord hpglrec[5];       /* HPGL-specifics*/
static double hpglcell;       /* character cell size */
static int actcol;               /* actual color No. */
static int hpglpen;        /* actual pen No. (1-8/0) */
static char *hpglhatch;         /* actual hatch type */

#define HATCHES 4
static char *hatches[HATCHES] = { "", "3,0.8,45", "4,0.8,0", "1" };

/* -------------- VERY simple output optimizer */

typedef struct {
  char   c;
  byte   col;
  float  px, py;
} STORE;

#define MAX_STORE 256

static STORE store[MAX_STORE];
static int   store_idx = 0;
static int   print_len = 0;

static char buf[100];
static void print(char *fmt, ...) {
   va_list argptr;
   int len;

   va_start(argptr, fmt);
   len = vsprintf(buf, fmt, argptr);
   va_end(argptr);
   if (print_len+len > 78) {
     uwriteln(&outfile, "");
     print_len = 0;
   }
   fprintf(outfile.f, "%s", buf);
   print_len += len;
}


/* select a new pen */
static int select_pen(int pen) {
  if (pen != hpglpen) {
    hpglpen = pen;
    print("SP%d;", hpglpen);
    return TRUE;
  }
  return FALSE;
}

/* write out all stored characters */
static void flush_fg(int pen) {
  int idx;

  for (idx = 0; idx < store_idx; ++idx) {
    if (   store[idx].c != ' '
	&& hpglrec[store[idx].col].fgnd == pen ) {
      select_pen(pen);
      print("PU%1.2f,%1.2f;", store[idx].px, store[idx].py);
      print("LB%c\003;", store[idx].c);
    }
  }
}

/* draw all saved background shadings */
static void flush_bg(int pen) {
  int idx, h;

  if (pen <= 0) return;
  for (idx = 0; idx < store_idx; ++idx) {
    if (hpglrec[store[idx].col].bgnd != pen) continue;
    for (h = 0; h < HATCHES; ++h) {
      if (hpglrec[store[idx].col].hatch == hatches[h]) {
	double act_x = store[idx].px - dev_xsize/8;
	double act_y = store[idx].py - dev_ysize/4;
	select_pen(pen);
	print("PU%1.2f,%1.2f;", act_x, act_y);
	if (hatches[h] != hpglhatch) {
	  hpglhatch = hatches[h];
	  print("FT%s;", hpglhatch);
	}
	print("RR%1.2f,%1.2f;", dev_xsize, dev_ysize);
      }
    }
  }
}

/* save stored character cells */
static void store_flush(void) {
  int p, sp;

  /* first background */
  sp = hpglpen;
  flush_bg(sp); /* actual pen first */
  for (p=1; p < 10; ++p)
    if (p!=sp)
      flush_bg(p);

  /* then foreground */
  sp = hpglpen;
  flush_fg(sp); /* actual pen first */
  for (p=0; p < 10; ++p)
    if (p!=sp)
      flush_fg(p);

  store_idx = 0;
}

/* save a character for optimizing HPGL output */
static void store_char(char c, int col, double px, double py) {
  store[store_idx].c  = c;
  store[store_idx].px = (float) px;
  store[store_idx].py = (float) py;
  store[store_idx].col= (byte) col;
  ++store_idx;
  if (store_idx == MAX_STORE)
    store_flush();
}

/* - - - - - - - - - - - - - - */

static void menu_hpgl(hpglrecord *hpglrec, BOOL *lc, char *txt, char *parstring)
{
  char code1, code2;
  char instring[41];

  if (interactflag) {
    printf("\n-------------------------------------------------------------\n"
	   "Enter the text attributes for printing of **>%s<** residues:\n", txt);
    printf("First choose color and hatching type of the letter **>background<**\n"
	   "NUMBERS specify plotter Pens,\n"
	   "LETTERS specify hatching types.\n"
	   "input has to be composed of a number and a letter\n"
	   "1-9, 0 : PEN No. (0 means no background)\n"
	   "(N) : no hatching  (S) : slash\n"
	   "(C) : crosshatch   (F) : fill\n\n"
	   "choose a number and a letter ( * %c%c * ) : ",
				   parstring[0], parstring[1]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0') {
    code1 = parstring[0];
    code2 = parstring[1];
  } else {
    code1 = instring[0];
    code2 = instring[1];
  }
  code1 = toupper(code1);
  code2 = toupper(code2);
  if (strchr("NSCF", code2) == NULL)
    code2 = 'N';
  switch (code2) {
    case 'N':
      hpglrec->hatch = hatches[0];
      break;
    case 'S':
      hpglrec->hatch = hatches[1];
      break;
    case 'C':
      hpglrec->hatch = hatches[2];
      break;
    case 'F':
      hpglrec->hatch = hatches[3];
      break;
  }
  hpglrec->bgnd = code1-'0';
  if (interactflag) {
    printf("\n\nEnter the text attributes for printing of **>%s<** residues:\n",
	   txt);
    printf("Now choose color and case of the letter **>foreground<**\n"
	   "NUMBERS specify plotter Pens,\n"
	   "A letter \"n\" behind the number means \"normal printing\"\n"
	   "A letter \"l\" behind the number means \"lowercase printing\"\n"
	   "1-9, 0 : PEN No. (0 means no printing)\n"
	   "e.g. \"1n\" means : PEN #1 is to be used\n"
	   "e.g. \"2l\" means : PEN #2 and lowercase types are to be used\n\n"
	   "choose a number and a letter ( * %c%c * ) : ",
				  parstring[2], parstring[3]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0') {
    code1 = parstring[2];
    code2 = parstring[3];
  } else {
    code1 = instring[0];
    code2 = instring[1];
  }
  if (code2 == 'L' || code2 == 'l')
    *lc = TRUE;
  else
    *lc = FALSE;
  hpglrec->fgnd = code1 - '0';
}


static void ask_hpgl(void)
{
  char parstring[6];
  char instring[41];

  term_par("HPGL");
  if (interactflag)
    printf(
      "-----------------------------------------------------------------------\n");
  Fgets(parstring, 6, parfile);
  parstring[4] = '\0';
  menu_hpgl(hpglrec, lc, "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[4] = '\0';
  menu_hpgl(&hpglrec[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[4] = '\0';
  if (simflag)
    menu_hpgl(&hpglrec[2], &lc[2], "similar", parstring);
  else {
    hpglrec[2] = hpglrec[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[4] = '\0';
  if (globalflag)
    menu_hpgl(&hpglrec[3], &lc[3], "conserved", parstring);
  else {
    hpglrec[3] = hpglrec[1];
    lc[3] = lc[1];
  }
  hpglrec[4].bgnd = 0;
  hpglrec[4].hatch = hatches[0];
  hpglrec[4].fgnd = hpglrec[0].fgnd;
  lc[4] = FALSE;
  fscanf(parfile, "%lg%*[^\n]", &hpglcell);
  getc(parfile);
  Fgets(parstring, 6, parfile);
  if (parstring[0] == 'Y' || parstring[0] == 'y')
    landscapeflag = TRUE;
  else
    landscapeflag = FALSE;
  if (interactflag) {
    do {
      printf("Character size in Points ( * %4.1f * ) : ", hpglcell);
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	hpglcell = str2int((void *)instring);
    } while (hpglcell < 1 || hpglcell > 100);
    printf("Rotate plot  ( * %c * ) : ", YESNO(landscapeflag));
    Fgets(instring, 41, stdin);
    if (*instring != '\0') {
      if (instring[0] == 'Y' || instring[0] == 'y')
	landscapeflag = TRUE;
      else
	landscapeflag = FALSE;
    }
  }
  if (!cloutflag) {
    printf("filename for HPGL-output  : ");
    Gets(outname);
  }
}


static void HPGLinit(double *xpos, double *ypos) {
  double xd, yd;

  if (landscapeflag) {
    dev_minx = 10.75;   /* borders of a A4 page in mm */
    dev_miny = 5.0;
    dev_maxx = 260.75;
    dev_maxy = 185.0;
  } else {
    dev_minx = 5.0;   /* borders of a A4 page in mm */
    dev_miny = 10.75;
    dev_maxx = 185.0;
    dev_maxy = 260.75;
  }
  dev_xsize = hpglcell * 0.351;
  dev_ysize = hpglcell * 0.351 * 2.0 / 1.5;
  hpglpen = 0;
  hpglhatch = hatches[0];
  *xpos = dev_minx;
  *ypos = dev_maxy;   /* 0,0 is lower left corner */
  assert(outopen(&outfile, outname) != NULL);
  xd = dev_xsize * 100 / 1.5 / (dev_maxx - dev_minx);
  yd = dev_ysize * 100 / 2.0 / (dev_maxy - dev_miny);
  print("IN;%sIP;SI%5.3f;", (landscapeflag?"RO 90;":""), hpglcell*0.351);
  print("SC%1.2f,%1.2f,%1.2f,%1.2f;", dev_minx,dev_maxx, dev_miny,dev_maxy);
  print("SR%1.4f,%1.4f;", xd, yd);
  uwriteln(&outfile, "PU;");
  print_len = 0;
}

static void HPGLsetcolor(int colno) {
  actcol  = colno;
}

static void HPGLcharout(char c, double *xpos, double *ypos) {
  store_char(c, actcol, *xpos, *ypos);
  *xpos += dev_xsize;
}

static void HPGLnewline(double *xpos, double *ypos) {
  *xpos = dev_minx;
  *ypos -= dev_ysize;
}

static void HPGLnewpage(double *xpos, double *ypos) {
  store_flush();
  uwriteln(&outfile, "PG");
  print_len = 0;
  *xpos = dev_minx;
  *ypos = dev_maxy;
  print("PU;");
}

static void HPGLexit(void) {
  store_flush();
  uwriteln(&outfile, "PG");
  print_len = 0;
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Hpgl = {
  "HPGL",
  ask_hpgl,
  HPGLinit,
  HPGLsetcolor,
  HPGLcharout,
  GenericStringOut,
  HPGLnewline,
  HPGLnewpage,
  HPGLexit
};
