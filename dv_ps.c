#include "dv_all.h"
#include "version.h"
#include <math.h>
#include <time.h>
#include <stdarg.h>

/*----------------------------------------*/
/*   POSTSCRIPT  specific section         */
/*----------------------------------------*/

/* maximum line length in postscript file (approx) */
#define MAXLEN 70

/* max string length in print commands */
#define MAX_PRINT_LEN 300

typedef struct {
  double r, g, b, k;
} RGBK;

typedef struct {
  RGBK fg, bg;
} psrecord;


static char last_ps_cl;
static int act_ps_col;
static char save_sb[MAXLEN+30];
static int ps_count;
static BOOL new_y, new_x;

static psrecord ps[5];   /* POSTSCRIPT-specifics*/
static char pscc[5];
static double pscell;    /* character cell size */

static void gray(RGBK *rgbk, double gray) {
  rgbk->r = 0;
  rgbk->g = 0;
  rgbk->b = 0;
  rgbk->k = gray;
}

static void rgb(RGBK *rgbk, double r, double g, double b) {
  rgbk->r = r;
  rgbk->g = g;
  rgbk->b = b;
  rgbk->k = 0;
}

static void Code2Color(char code, RGBK *rgbk) {
  switch (code) {
    case 'W': gray(rgbk, 1.0); break;
    case '1':
    case '5': gray(rgbk, 0.8);
	      break;
    case '2':
    case '6': gray(rgbk, 0.6);
	      break;
    case '3':
    case '7': gray(rgbk, 0.4);
	      break;
    case '4':
    case '8': gray(rgbk, 0.2);
	      break;
    case 'B': gray(rgbk, 0.0); break;
    case 'R': rgb(rgbk, 1.0, 0.0, 0.0); break;
    case 'G': rgb(rgbk, 0.0, 1.0, 0.0); break;
    case 'L': rgb(rgbk, 0.0, 0.0, 1.0); break;
    case 'Y': rgb(rgbk, 1.0, 1.0, 0.0); break;
    case 'M': rgb(rgbk, 1.0, 0.0, 1.0); break;
    case 'C': rgb(rgbk, 0.0, 1.0, 1.0); break;
  }
}

static void menu_postscript(psrecord *ps, BOOL *lc, char *txt, char *parstring)
{
  char code;
  char instring[41];

  if (interactflag) {
    printf("\n----------------------------------------------------------------\n"
	   "Enter the text attributes for printing of **>%s<** residues:\n", txt);
    printf("First choose the color/gray-value of the letter **>background<**\n"
	   "Different letters specify different colors,\n"
	   "(B) Black, (W) white (1,2,3,4) 4 different gray values, 4 is darkest\n"
	   "(R) Red, (G) Green, (L) Blue, (Y) Yellow, (M) Magenta, (C) Cyan\n\n"
	   "choose from  BWRGLYMC1234 ( * %c * ) : ", parstring[0]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[0];
  else
    code = instring[0];
  code = toupper(code);
  if (strchr( "BWRGLYMC1234", code) == NULL)
    code = 'W';

  Code2Color(code, &(ps->bg));

  if (interactflag) {
    printf("\n\nNow choose the color/gray-value of the letter **>foreground<**\n"
	   "lowercase choices mean lowercase letters in the sequence:\n"
	   "(B,b) black  (W,w) white  (1,2,3,4) 4 different gray values, 4 is darkest\n"
	   "(R,r) red    (G,g) green  (5,6,7,8) same, but lowercase letters\n"
	   "(L,l) blue   (Y,y) yellow (M,m) magenta   (C,c) cyan\n\n"
	   "choose from  BbWwRrGgLlYyMmCc12345678 ( * %c * ) : ", parstring[1]);
    Fgets(instring, 41, stdin);
  } else
    *instring = '\0';
  if (*instring == '\0')
    code = parstring[1];
  else
    code = instring[0];
  if (strchr("BbWwRrGgLlYyMmCc12345678", code) == NULL)
    code = 'B';

  switch (code) {
    case '1' :
    case '2' :
    case '3' :
    case '4' : *lc = FALSE;
	       break;
    case '5' :
    case '6' :
    case '7' :
    case '8' : *lc = TRUE;
	       break;
    default  : *lc = islower(code);
	       break;
  }

  code = toupper(code);

  Code2Color(code, &(ps->fg));
}

static void ask(BOOL makeEPS)
{
  char parstring[6];
  char instring[41];
  int i;
  char lk;

  if (interactflag)
    printf("----------------------------------------------------------------\n");
  term_par("POSTSCRIPT");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_postscript(&ps[0], &lc[0], "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_postscript(&ps[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_postscript(&ps[2], &lc[2], "similar", parstring);
  else {
    ps[2] = ps[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_postscript(&ps[3], &lc[3], "conserved", parstring);
  else {
    ps[3] = ps[1];
    lc[3] = lc[1];
  }
  gray(&ps[4].bg, 1.0);
  gray(&ps[4].fg, 0.0);
  lc[4] = FALSE;

  lk = 'K'-1;
  for (i = 0; i <= 4; i++) {
    if (!globalflag && i == 3) continue;
    pscc[i] = ++lk;
  }

  fscanf(parfile, "%lg%*[^\n]", &pscell);
  getc(parfile);
  Fgets(parstring, 6, parfile);
  landscapeflag = toupper(parstring[0]) == 'Y';
  if (interactflag) {
    do {
      printf("Character size in Points ( * %4.1f * ) : ", pscell);
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	pscell = str2int((void *)instring);
    } while (pscell < 1 || pscell > 100);
    if (!makeEPS) {
      printf("Rotate plot  ( * %c * ) : ", YESNO(landscapeflag));
      Fgets(instring, 41, stdin);
      if (*instring != '\0')
	landscapeflag = toupper(instring[0]) == 'Y';
    }
  }

  if (!cloutflag) 
    do {
      printf("filename for POSTSCRIPT-output  : ");
      Gets(outname);
    } while (*outname == '\0');
}

static void ask_postscript(void) {
  ask(FALSE);
}

static void ask_eps(void) {
  ask(TRUE);
}

static void ps_nl(void)
{
  if (ps_count > 0)
    uwriteln(&outfile, "");
  ps_count = 0;
}

static void ps_space(void)
{
  if (ps_count > MAXLEN)
    ps_nl();
  else
  if (ps_count > 0){
    putc(' ', outfile.f);
    ps_count++;
  }
}

static void ps_string(int cmd, char *fmt, va_list arg_list) {
   char *cp1, *cp2;
   static char *buf = NULL;

   if (buf == NULL)
     buf=Malloc(MAX_PRINT_LEN);
   vsprintf(buf, fmt, arg_list);
   cp1 = buf;
   while (cp1 != NULL && *cp1 != '\0') {
     cp2 = strchr(cp1, '\n');
     if (cp2 != NULL) {
       *cp2 = '\0';
       ++cp2;
     }
     while (*cp1 != '\0') {
       if (cmd && *cp1 == ' ')
	   ps_space();
       else
       if ( (*cp1 & 0x80) != 0)
	 ps_count += fprintf(outfile.f, "\\%03o", (*cp1)&0xff);
       else {
	 putc(*cp1, outfile.f);
	 ++ps_count;
       }
       ++cp1;
     }
     if (cp2 != NULL) ps_nl();
     cp1 = cp2;
   }
   if (cmd)
     ps_space();
}

static void ps_str(char *fmt, ...) {
   va_list argptr;

   va_start(argptr, fmt);
   ps_string(FALSE, fmt, argptr);
   va_end(argptr);
}

static void ps_cmd(char *fmt, ...) {
   va_list argptr;

   va_start(argptr, fmt);
   ps_string(TRUE, fmt, argptr);
   va_end(argptr);
}

static void ps_fp(double r, int nk)
{
  char s[20], *cp;

  sprintf(s, "%1.*f", nk, r);
  if (nk > 0) {
    cp = s + strlen(s);
    while (cp-s > 0 && *(cp-1) == '0')
      --cp;
    if (cp-s > 0 && *(cp-1) == '.')
      --cp;
    *cp = '\0';
  }
  cp = s;
  while (*cp == ' ') ++cp;
  ps_cmd("%s", cp);
}

static void ps_date(void)
{
   time_t timer;
   struct tm *tblock;

   /* Gets time of day */
   timer = time(NULL);

   /* converts date/time to a structure */
   tblock = localtime(&timer);

   ps_str("%s", asctime(tblock));
}

static void PageSetup(int pn)
{
  ps_str("\n"
	 "%%%%Page: %d %d\n"
	 "%%%%BeginPageSetup\n"
	 "psetup\n"
	 "%%%%EndPageSetup\n", pn, pn);
}

static void close_sb(void)
{
  if (*save_sb != '\0') {
    int sl = strlen(save_sb);
    if (ps_count + sl > MAXLEN)
      ps_nl();
    ps_str("(%s)%c", save_sb, (sl > 1 ? 'S' : 'C'));
    ps_space();
  }
  *save_sb = '\0';
}

static void add_sb(char c)
{
  int sl = strlen(save_sb);
  save_sb[sl++] = c;
  save_sb[sl  ] = '\0';
  if (ps_count+sl > MAXLEN) {
    ps_str("(%s)%c", save_sb, (sl > 1 ? 'S' : 'C'));
    ps_space();
    *save_sb = '\0';
  }
}

static void ps_color(RGBK *c) {
  if (c->k > 0 || (c->r == 0 && c->g == 0 && c->b == 0)) {
    ps_fp(c->k, 1);
    ps_cmd("setgray");
  } else {
    ps_fp(c->r, 1);
    ps_fp(c->g, 1);
    ps_fp(c->b, 1);
    ps_cmd("setrgbcolor");
  }
}

/* static int ps_white(RGBK *c) {
  return (c->k > 0.9999);
} */

static void DVinit(BOOL makeEPS, double *xpos, double *ypos) {
  int i;
  double linelen, bbox_x0, bbox_x1, bbox_y0, bbox_y1;

  if (landscapeflag && !makeEPS) {
    dev_miny = 30.0;   /* borders of a A4 page in point units */
    dev_minx = 30.0;
    dev_maxy = 545.0;
    dev_maxx = 800.0;
  } else {
    dev_minx = 30.0;   /* borders of a A4 page in point units */
    dev_miny = 30.0;
    dev_maxx = 575.0;
    dev_maxy = 760.0;
  }
  dev_xsize = pscell * 0.7;   /* cells do overlap in x-direction */
  dev_ysize = pscell;
  *xpos = dev_minx;
  *ypos = dev_maxy - dev_ysize;   /* 0,0 is lower left corner */
  lines_per_page = (int)((dev_maxy - dev_miny) / dev_ysize);
  assert( outopen(&outfile, outname) != NULL);

  ps_str("%%!PS-Adobe-2.0%s\n", (makeEPS ? " EPSF-2.0" : "") );
  ps_str("%%%%Creator: BOXshade %s\n", BOXSHADE_ver);
  ps_str("%%%%Title: BOXSHADE document %s\n", inname);
  ps_str("%%%%CreationDate: "); ps_date(); ps_nl();

  ps_str("%%%%Pages: %s\n", (makeEPS ? "1" : "(atend)") );
  /*calculate the bounding box positions, trying to take into account the
    actual length of line and the depth on the page (if more than one
    page). Note that if the figure is more than one page, one really MUST
    use the /split option with EPSF, or the results will be a mess.*/
  if (makeEPS) {
    linelen = outlen;
    if (seqnameflag)
      linelen += seqname_outlen + 1;
    if (seqnumflag)
      linelen += seqnumlen + 1;
    bbox_x0 = dev_minx - 1;
    if (lines_left > lines_per_page)
      bbox_y0 = dev_maxy - lines_per_page * dev_ysize;
    else
      bbox_y0 = dev_maxy - lines_left * dev_ysize - 1;
   /*if the bbox is set too wide for the paper, it will be clipped by the
     printer; however, it should appear as too wide in the graphics
     program, so the user will notice it.*/
    bbox_x1 = dev_minx + linelen * dev_xsize + 1;
    bbox_y1 = dev_maxy + 1;
    ps_str("%%%%BoundingBox: %ld %ld %ld %ld\n",
       (long)bbox_x0, (long)bbox_y0, (long)bbox_x1, (long)bbox_y1);
  } else {
    if (landscapeflag) {
      ps_str("%%%%BoundingBox: %ld %ld %ld %ld\n",
	 (long)dev_miny-1, (long)dev_minx-1, (long)dev_maxy+1, (long)dev_maxx+1);
      ps_str("%%%%Orientation: landscape\n");
    } else {
      ps_str("%%%%BoundingBox: %ld %ld %ld %ld\n",
       (long)dev_minx-1, (long)dev_miny-1, (long)dev_maxx+1, (long)dev_maxy+1);
      ps_str("%%%%Orientation: portrait\n");
    }
    ps_str("%%%%PaperSize: a4\n");
  }

  ps_str("%%%%DocumentNeededFonts: Courier-Bold\n"
	 "%%%%DocumentData: Clean7Bit\n"
	 "%%%%LanguageLevel: 1\n"
	 "%%%%EndComments\n"
	 "%%%%BeginProlog\n");
  ps_cmd("/bd { bind def } bind def");
  ps_cmd("/xd { exch def } bd");
  ps_nl();
  ps_str("%%\n"
	 "%% custom color selection\n%%\n"
	 "%%   grayscale:\n%%\n"
	 "%%     '<gray> setgray'\n%%\n"
	 "%%        <gray> is a real number between\n"
	 "%%        0.0 (black) and 1.0 (white)\n%%\n");
  ps_str("%%   RGB (red/green/blue) colors:\n%%\n"
	 "%%     '<r> <g> <b> setrgbcolor'\n%%\n"
	 "%%        each color compoment <?> is a real'\n"
	 "%%        number between 0.0 (zero intensity) and\n"
	 "%%        1.0 (max intensity)\n%%\n"
	 "%% Change the following definitions for your needs !\n"
	 "%%");
  ps_nl();
  for (i = 0; i <= 4; i++) {
    if (!globalflag && i == 3) continue;
    switch (i) {
      case 0: ps_str("%% -- different residues"); break;
      case 1: ps_str("%% -- identical residues"); break;
      case 2: ps_str("%% -- similar residues"); break;
      case 3: ps_str("%% -- conserved residues"); break;
      case 4: ps_str("%% -- normal text"); break;
    }
    ps_nl();
    ps_cmd("/bg%c {", pscc[i]); ps_color(&ps[i].bg); ps_cmd("} bd");
    ps_str("%% background");
    ps_nl();
    ps_cmd("/fg%c {", pscc[i]); ps_color(&ps[i].fg); ps_cmd("} bd");
    ps_str("%% foreground");
    ps_nl();
  }
  ps_str("%%\n%% end of custom color selection\n%%");
  ps_nl();
  ps_cmd("/px 0 def");
  ps_cmd("/py 0 def");
  ps_cmd("/fg {0 setgray} bd");
  ps_cmd("/bg {1 setgray} bd");

  ps_cmd("/C {px py moveto gsave");
    ps_fp(-0.03*pscell, 1);
    ps_fp(-0.05*pscell, 1);
    ps_cmd("rmoveto");
    ps_fp(0.70*pscell, 1);
    ps_cmd("0 rlineto 0");
    ps_fp(pscell,1);
    ps_cmd("rlineto");
    ps_fp(-0.70*pscell, 1);
    ps_cmd("0 rlineto closepath "
	   "bg fill grestore fg 0 2 rmoveto show "
	   "/px px");
    ps_fp(dev_xsize, 2);
  ps_cmd("add def} bd", dev_xsize);

  ps_cmd("/X {/px xd} bd "
	 "/Y {/py xd} bd "
	 "/A {Y X} bd");

/*
  ps_cmd("/S { /strg xd /idx 0 def { strg idx 1 "
	 "getinterval C /idx idx 1 add def strg length idx "
	 "eq {exit} if } loop } bd");
*/
  ps_cmd("/S {0 1 2 index length 1 sub {"
	   "2 copy 1 getinterval C pop"
	 "} for pop} bd");

  ps_cmd("/psetup {/Courier-Bold findfont");
  ps_fp(pscell, 1);
  ps_cmd("scalefont setfont} bd");

  for (i = 0; i <= 4; i++) {
    if (!globalflag && i == 3) continue;
    ps_cmd("/%c {/bg {bg%c} bd"
		" /fg {fg%c} bd"
	      "} bd", pscc[i], pscc[i], pscc[i]);
  }
  ps_nl();

  ps_str("%%%%EndProlog\n"
	 "%%%%BeginSetup\n");
  ps_cmd("save%s", (makeEPS ? "" : " initgraphics") );
  if (landscapeflag && !makeEPS)
    ps_cmd("575 0 translate 90 rotate");
  ps_cmd("120 currentscreen 3 -1 roll pop setscreen\n");
  ps_str("%%%%EndSetup\n");

  PageSetup(1);
  last_ps_cl = ' ';
  act_ps_col = 4;
  new_x = new_y = TRUE;
  *save_sb = '\0';
  ps_count = 0;
}

static void Setcolor(int colno) {
  if (!globalflag && colno == 3) colno = 1;
  act_ps_col = colno;
}

static void Charout(char c, double *xpos, double *ypos) {
  if (pscc[act_ps_col] != last_ps_cl) {
    close_sb();
    last_ps_cl = pscc[act_ps_col];
    ps_cmd("%c", last_ps_cl);
  }
  if (new_y && new_x) {
    close_sb();
    ps_fp(*xpos, 1);
    ps_fp(*ypos, 1);
    ps_cmd("A");
    new_x = new_y = FALSE;
  } else
  if (new_y) {
    close_sb();
    ps_fp(*ypos, 1);
    ps_cmd("Y");
    new_y = FALSE;
  } else
  if (new_x) {
    close_sb();
    ps_fp(*xpos, 1);
    ps_cmd("X");
    new_x = FALSE;
  }
  add_sb(c);
  *xpos += dev_xsize;
}

static void Newline(double *xpos, double *ypos) {
  close_sb();
  *xpos = dev_minx;
  *ypos -= dev_ysize;
  new_x = new_y = TRUE;
}

static void PSnewpage(double *xpos, double *ypos) {
  close_sb();
  *xpos = dev_minx;
  ps_cmd("copypage erasepage");
  PageSetup(act_page + 1);
  *ypos = dev_maxy - dev_ysize;
  new_x = new_y = TRUE;
  last_ps_cl = ' ';
}

static void DVexit(BOOL makeEPS) {
  close_sb();
  if (!makeEPS)
    ps_cmd("copypage erasepage");
  ps_str("\n%%%%Trailer\n");
  ps_str("restore\n");
  if (!makeEPS)
    ps_str("%%%%Pages: %d\n", act_page);
  else
    ps_str("showpage\n");
  ps_str("%%%%EOF\n");
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}

static void EPSinit(double *xpos, double *ypos) { DVinit(TRUE,  xpos, ypos); }
static void PSinit(double *xpos, double *ypos)  { DVinit(FALSE, xpos, ypos); }
static void PSexit(void) { DVexit(FALSE); }
static void EPSexit(void) { DVexit(TRUE); }

GraphicsDevice Postscript = {
  "PS",
  ask_postscript,
  PSinit,
  Setcolor,
  Charout,
  GenericStringOut,
  Newline,
  PSnewpage,
  PSexit
};

GraphicsDevice Eps = {
  "EPS",
  ask_eps,
  EPSinit,
  Setcolor,
  Charout,
  GenericStringOut,
  Newline,
  Newline,
  EPSexit
};
