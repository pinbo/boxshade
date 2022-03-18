#include "dv_all.h"
#include "version.h"
#include <math.h>
#include <time.h>
#include <stdarg.h>

/*----------------------------------------*/
/*   POSTSCRIPT  specific section         */
/*----------------------------------------*/

typedef struct {
  double r, g, b;
} RGB;

typedef struct {
  RGB fg, bg;
} HTMLrecord;


static HTMLrecord html[5];   /* POSTSCRIPT-specifics*/
static int open_TR = FALSE;
static int open_TABLE = FALSE;
static int Col;

static void Gray(RGB *rgb, double gray) {
  rgb->r = gray;
  rgb->g = gray;
  rgb->b = gray;
}

static void Rgb(RGB *rgb, double r, double g, double b) {
  rgb->r = r;
  rgb->g = g;
  rgb->b = b;
}

static void Code2Color(char code, RGB *rgb) {
  switch (code) {
    case 'W': Gray(rgb, 1.0); break;
    case '1':
    case '5': Gray(rgb, 0.8);
	      break;
    case '2':
    case '6': Gray(rgb, 0.6);
              break;
    case '3':
    case '7': Gray(rgb, 0.4);
	      break;
    case '4':
    case '8': Gray(rgb, 0.2);
	      break;
    case 'B': Gray(rgb, 0.0); break;
    case 'R': Rgb(rgb, 1.0, 0.0, 0.0); break;
    case 'G': Rgb(rgb, 0.0, 1.0, 0.0); break;
    case 'L': Rgb(rgb, 0.0, 0.0, 1.0); break;
    case 'Y': Rgb(rgb, 1.0, 1.0, 0.0); break;
    case 'M': Rgb(rgb, 1.0, 0.0, 1.0); break;
    case 'C': Rgb(rgb, 0.0, 1.0, 1.0); break;
  }
}

static void menu_HTML(HTMLrecord *html, BOOL *lc, char *txt, char *parstring)
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

  Code2Color(code, &(html->bg));

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

  Code2Color(code, &(html->fg));
}

static void Ask(void)
{
  char parstring[6];

  if (interactflag)
    printf("----------------------------------------------------------------\n");
  term_par("HTML");
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_HTML(&html[0], &lc[0], "different", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  menu_HTML(&html[1], &lc[1], "identical", parstring);
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (simflag)
    menu_HTML(&html[2], &lc[2], "similar", parstring);
  else {
    html[2] = html[0];
    lc[2] = lc[0];
  }
  Fgets(parstring, 6, parfile);
  parstring[2] = '\0';
  if (globalflag)
    menu_HTML(&html[3], &lc[3], "conserved", parstring);
  else {
    html[3] = html[1];
    lc[3] = lc[1];
  }
  Gray(&html[4].bg, 1.0);
  Gray(&html[4].fg, 0.0);
  lc[4] = FALSE;

  if (!cloutflag) 
    do {
      printf("filename for HTML-output  : ");
      Gets(outname);
    } while (*outname == '\0');
}

#define col255(cv) ((int)(255.0*(cv)))

static void ColWrite(RGB *rgb) {
  fprintf(outfile.f, "\"#%02X%02X%02X\"", col255(rgb->r), col255(rgb->g),  col255(rgb->b));
}

static int coleq(RGB *c1, RGB *c2) {
  return    (col255(c1->r) == col255(c2->r))
         && (col255(c1->g) == col255(c2->g))
         && (col255(c1->b) == col255(c2->b));
}

#define isBG(rgb) coleq((rgb),&html[4].bg)
#define isTC(rgb) coleq((rgb),&html[4].fg)

static void StartTR(void) {
  if (!open_TR) {
    uwriteln(&outfile, "<tr align=center>");
    open_TR = TRUE;
  }
}

static void CloseTR(void) {
  if (open_TR) {
    uwriteln(&outfile, "</tr>");
    open_TR = FALSE;
  }
}

static void StartTable(void) {
  uwriteln(&outfile, "<table border=0 cellspacing=0 CELLPADDING=0>");
  open_TR = FALSE;
  open_TABLE = TRUE;
}

static void CloseTable(void) {
  CloseTR();
  if (open_TABLE) {
    uwriteln(&outfile, "</table>");
    open_TABLE = FALSE;
  }
}

static void dv_Init(double *xpos, double *ypos) {
  dev_minx = 0.0;
  dev_miny = 0.0;
  dev_maxx = 1000.0;
  dev_maxy = 10000.0;
  dev_xsize = 1.0;
  dev_ysize = 1.0;
  *xpos = dev_minx;
  *ypos = dev_maxy - dev_ysize;
  lines_per_page = (int)((dev_maxy - dev_miny) / dev_ysize);
  assert( outopen(&outfile, outname) != NULL);

  uwriteln(&outfile, "<html>");
  uwriteln(&outfile, "<head>");
  uwriteln(&outfile, "<title>BoxShade " BOXSHADE_ver " Output</title>");
  uwriteln(&outfile, "</head>");
  fprintf(outfile.f, "<body BGCOLOR=");
    ColWrite(&html[4].bg);
    fprintf(outfile.f, " TEXT=");
    ColWrite(&html[4].fg);
  uwriteln(&outfile, ">");
}

static void Setcolor(int colno) {
  Col = colno;
}

static void StrgChar(int ch, char *s, double *xpos, double *ypos) {
  char *fc = "";
  if (!open_TABLE) StartTable();
  StartTR();
  fprintf(outfile.f, "<td%s", (s != NULL ? " align=left": "") );
  if (!isBG(&html[Col].bg)) {
    fprintf(outfile.f, " BGCOLOR=");
    ColWrite(&html[Col].bg);
  } 
  fprintf(outfile.f, ">");
  if (!isTC(&html[Col].fg)) {
    fprintf(outfile.f, "<FONT COLOR="); 
    ColWrite(&html[Col].fg);
    fprintf(outfile.f, ">");
    fc = "</FONT>";
  }
  if (s == NULL) {
    if (ch != ' ') fprintf(outfile.f, "%c%s", ch, fc);
              else fprintf(outfile.f, "&nbsp;%s", fc);
    *xpos += dev_xsize;
  } else {
    fprintf(outfile.f, "%s%s&nbsp;&nbsp;&nbsp;", s, fc);
    *xpos += strlen(s)*dev_xsize;
  }
  uwriteln(&outfile, "</TD>");
}

static void Charout(char c, double *xpos, double *ypos) {
  StrgChar(c, NULL, xpos, ypos);
}

static void Stringout(char *s, double *xpos, double *ypos) {
  StrgChar('\0', s, xpos, ypos);
}

static void Newline(double *xpos, double *ypos) {
  if (!open_TR) {
    StartTR();
    uwriteln(&outfile, "<td>&nbsp;</td>");
  }
  CloseTR();
  *xpos = dev_minx;
  *ypos -= dev_ysize;
}

static void Newpage(double *xpos, double *ypos) {
  Newline(xpos, ypos);
  *xpos = dev_minx;
  *ypos = dev_maxy - dev_ysize;
}

static void dv_Exit(void) {
  CloseTR();
  CloseTable();
  uwriteln(&outfile, "</body>");
  uwriteln(&outfile, "</html>");
  if (outfile.f != NULL)
    fclose(outfile.f);
  outfile.f = NULL;
}

GraphicsDevice Html = {
  "HTML",
  Ask,
  dv_Init,
  Setcolor,
  Charout,
  Stringout,
  Newline,
  Newpage,
  dv_Exit
};
