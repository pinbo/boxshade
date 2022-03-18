#include "bx_types.h"
#include <stdarg.h>

#ifdef MSDOS
#include <io.h>
#else
#include <unistd.h>
#endif

MYFILE outfile = { NULL, FALSE};
FILE *parfile = NULL;

char depend_err[50] = "";

void *Malloc(size_t sze) {
  void *res = malloc(sze);
  assert(res!=NULL);
  return res;
}

int min(int a, int b)
{
  if (a < b)
    return a;
  return b;
}

static char *binpath = NULL;

void save_binpath(char *arg0) {
  int sl = strlen(arg0);
  char *ap = "";

  /* Accept three path seperators: / \ : */
  while (sl > 0 && strchr("\\/:", arg0[sl-1]) == NULL) --sl;
  if (sl <= 0) return;

  if (arg0[sl-1] == ':') ap = ":.";
  binpath = Malloc(sl + strlen(ap) );
  if (binpath == NULL) return;
  strncpy(binpath, arg0, sl-1);
  strcpy(binpath+sl-1, ap);
}

char *get_logical(char *logical)
{
  char *r;
  static char res[300];

  r = getenv(logical);
  if (r == NULL) r = "";
  strcpy(res, r);
  if (*res != '\0' || strcmp(logical, "BOXDIR")!=0 || binpath == NULL)
    return res;

  /* looking for BOXDIR -> default to exe directory ! */
  strcpy(res, binpath); /* exec path */
  return res;
}

BOOL fexist(char *fname)
{
  BOOL Result;
  FILE *f;

  f = fopen(fname, TXT_RD);
  Result = (f != NULL);
  if (f != NULL)
    fclose(f);
  return Result;
}

int indx(char *mainstring, char *substring)
{
  char *cp;

  if (*mainstring == '\0')
    return 0;
  cp = strstr(mainstring, substring);
  if (cp == NULL) return 0;
  return (int)(cp-mainstring)+1;
}

MYFILE *outopen(MYFILE *mf, char *fn) {
  mf->bin = EOLmode != EOL_default;
  mf->f = fopen(fn, (mf->bin ? BIN_WR : TXT_WR));
  return (mf->f != NULL ? mf : NULL);
}

static char *uwb = NULL;
void uwriteln(MYFILE *f, char *fmt, ...) {
  va_list argptr;

  if ( !f->bin) {
defeol:
    va_start(argptr, fmt);
    vfprintf(f->f, fmt, argptr);
    va_end(argptr);
    fputs("\n", f->f);
    return;
  }

  if (uwb == NULL) {
    uwb = malloc(4000);
    if (uwb == NULL)
      goto defeol;
  }

  {
   char *cp1, *cp2;
   int cnt;

   va_start(argptr, fmt);
   cnt = vsprintf(uwb, fmt, argptr);
   va_end(argptr);
   uwb[cnt] = '\n';
   uwb[cnt+1] = '\0';

   cp1 = uwb;
   while (cp1 != NULL && *cp1 != '\0') {
     cp2 = strchr(cp1, '\n');
     if (cp2 != NULL) {
       *cp2 = '\0';
       ++cp2;
     }
     fputs(cp1, f->f);
     if (cp2 != NULL) {
       fflush(f->f);
       switch (EOLmode) {
	 case EOL_dos  : putc('\r', f->f);
			 putc('\n', f->f);
			 break;
	 case EOL_mac  : putc('\r', f->f); break;
	 case EOL_unix :
	 default       : putc('\n', f->f); break;
       }
     }
     cp1 = cp2;
   }
  }
}


double str2real(void *s)
{
  double r;
  int err;

  err = (sscanf((char *)s, "%lg", &r) == 0);
  if (err != 0) {
    strcpy(depend_err, "str2real");
    return 0.0;
  } else
    return r;
}


int str2int(void *s)
{
  int i, err;

  err = (sscanf((char *)s, "%d", &i) == 0);
  if (err != 0) {
    strcpy(depend_err, "str2real");
    return 0;
  } else
    return i;
}


#if 0

static long rand_m = 100000000L, rand_m1 = 10000, rand_b = 31415821L;

static long rand_a[55];
static int rand_j, filecount;

char lowcase(char c)
{
  if (c <= 90 && c >= 65)
    return (c + 32);
  else
    return c;
}

char *int2str(char *Result, int intg, int dig)
{
  char s[100];

  sprintf(s, "%*d", dig, intg);
  return strcpy(Result, s);
}


char *real2str(char *Result, double rl, int dig1, int dig2)
{
  char s[100];

  sprintf(s, "%*.*f", dig1, dig2, rl);
  return strcpy(Result, s);
}

char *get_command_line(char *Result)
{
  string255 cl;
  int i;

  *cl = '\0';
  if (P_argc > 1) {
    for (i = 1; i < P_argc; i++)
      strcat(cl, P_argv[i]);
  }
  return strcpy(Result, cl);
}

static char *getTEMPname(char *Result, char *s_)
{
  string255 s;

  strcpy(s, s_);
  s[strlen(s) - 1] = '~';
  return strcpy(Result, s);
}


void fopen_(FILE **f, char *fname_, char acc)
{
  /*acc = r,w,a */
  string255 fname;
  FILE *tmpfile;
  string255 tmpname, line;
  char c;
  char buf1[6144], buf2[6144];
  BOOL uxmode;
  char tmpfile_NAME[_FNSIZE];

  strcpy(fname, fname_);
  tmpfile = NULL;
  if (acc == 'w' || acc == 'W') {
    assign(*f, fname);
/* p2c: dep_tp.pas, line 128: Note: REWRITE does not specify a name [181] */
    if (*f != NULL)
      rewind(*f);
    else
      *f = tmpfile();
    if (*f == NULL)
      _EscIO(FileNotFound);
  } else if (acc == 'a' || acc == 'A') {
    assign(*f, fname);
/* p2c: dep_tp.pas, line 133: Note: APPEND does not specify a name [181] */
    if (*f != NULL)
      rewind(*f);
    else
      *f = tmpfile();
    if (*f == NULL)
      _EscIO(FileNotFound);
  } else {
    assign(*f, fname);
    rewind(*f);
/* p2c: dep_tp.pas, line 139:
 * Note: Turbo Pascal conditional compilation directive was ignored [218] */
    /*$IFNDEF NO_UNIX*/
    do {
      c = getc(*f);
    } while (!(P_eoln(*f) || c == '\n'));
    rewind(*f);
    if (c == '\n') {
      uxmode = TRUE;
/* p2c: dep_tp.pas, line 142:
 * Warning: Symbol 'UXMODE' is not defined [221] */
    } else
      uxmode = FALSE;
    if (uxmode) {
      setvbuf(*f, buf1, _IOFBF, 6144);
      getTEMPname(tmpname, fname);
      strcpy(tmpfile_NAME, tmpname);
      if (tmpfile != NULL)
	tmpfile = freopen(tmpfile_NAME, "w", tmpfile);
      else
	tmpfile = fopen(tmpfile_NAME, "w");
      _SETIO(tmpfile != NULL, FileNotFound);
/* p2c: dep_tp.pas, line 149:
 * Note: Make sure setvbuf() call occurs when file is open [200] */
      setvbuf(*f, buf2, _IOFBF, 6144);
      while (!P_eof(*f)) {
	ureadln(f, (void *)line);
	if (P_ioresult != 0)
	  goto _Ll_exit;
	_SETIO(fprintf(tmpfile, "%s\n", line) >= 0, FileWriteError);
      }
      if (tmpfile != NULL)
	fclose(tmpfile);
      tmpfile = NULL;
      if (*f != NULL)
	fclose(*f);
      *f = NULL;
/* p2c: dep_tp.pas, line 157: Note: Encountered an ASSIGN statement [179] */
      assign(*f, tmpname);
      rewind(*f);
_Ll_exit:
      if (P_ioresult != 0)
	strcpy(depend_err, "could not convert unix format");
    }
/* p2c: dep_tp.pas, line 145:
 * Note: Make sure setvbuf() call occurs when file is open [200] */
    /*$endif*/
  }
/* p2c: dep_tp.pas, line 127: Note: Encountered an ASSIGN statement [179] */
  if (tmpfile != NULL)
    fclose(tmpfile);

/* p2c: dep_tp.pas, line 132: Note: Encountered an ASSIGN statement [179] */
/* p2c: dep_tp.pas, line 137: Note: Encountered an ASSIGN statement [179] */
}

char *substr_(char *Result, void *original, int pos1, int count)
{
  return strsub(Result, (char[256])original, pos1, count);
}


static long mult(long p, long q)
{
  long p1, p0, q1, q0;

  p1 = p / rand_m1;
  p0 = p % rand_m1;
/* p2c: dep_tp.pas, line 182:
 * Note: Using % for possibly-negative arguments [317] */
  q1 = q / rand_m1;
  q0 = q % rand_m1;
/* p2c: dep_tp.pas, line 183:
 * Note: Using % for possibly-negative arguments [317] */
  return (((p0 * q1 + p1 * q0) % rand_m1 * rand_m1 + p0 * q0) % rand_m);
/* p2c: dep_tp.pas, line 184:
 * Note: Using % for possibly-negative arguments [317] */
/* p2c: dep_tp.pas, line 184:
 * Note: Using % for possibly-negative arguments [317] */
}


void rand_init(int seed)
{
  rand_a[0] = seed;
  rand_j = 0;
  do {
    rand_j++;
    rand_a[rand_j] = (mult(rand_b, rand_a[rand_j - 1]) + 1) % rand_m;
/* p2c: dep_tp.pas, line 193:
 * Note: Using % for possibly-negative arguments [317] */
  } while (rand_j != 54);
}


int rand_int(int limit)
{
  /*from 0 to limit-1*/
  rand_j = (rand_j + 1) % 55;
/* p2c: dep_tp.pas, line 198:
 * Note: Using % for possibly-negative arguments [317] */
  rand_a[rand_j] = (rand_a[(rand_j + 23) % 55] + rand_a[(rand_j + 54) % 55]) %
		   rand_m;
/* p2c: dep_tp.pas, line 199:
 * Note: Using % for possibly-negative arguments [317] */
/* p2c: dep_tp.pas, line 200:
 * Note: Using % for possibly-negative arguments [317] */
/* p2c: dep_tp.pas, line 200:
 * Note: Using % for possibly-negative arguments [317] */
  return (rand_a[rand_j] / rand_m1 * limit / rand_m1);
}


double rand_real(void)
{
  /* from 0(incl) to 1(excl) */
  return (rand_int(32000) / 32000.0);
}


typedef char string1[2];


void ureadln(FILE **f, void *line)
{
  BOOL done;
  char c;
  char STR1[256];

  done = FALSE;
  *(string1)line = '\0';
  do {
    c = getc(*f);
    if (c == '\n')
      done = TRUE;
    else if (c != '\015') {
      sprintf(STR1, "%s%c", (char[256])line, c);
      strcpy((char *)(&line), STR1);
    }
    if (P_eoln(*f)) {
      fscanf(*f, "%*[^\n]");
      getc(*f);
      done = TRUE;
    }
  } while (!done);
}


BOOL getunixmode(char *fname)
{
  BOOL Result;
  char c;
  FILE *f;
  char f_NAME[_FNSIZE];

  f = NULL;
  strcpy(f_NAME, fname);
  if (f != NULL)
    f = freopen(f_NAME, "r", f);
  else
    f = fopen(f_NAME, "r");
  if (f == NULL)
    _EscIO(FileNotFound);
  do {
    c = getc(f);
  } while (!(c == '\n' || P_eoln(f)));
  if (c == '\n')
    Result = TRUE;
  else
    Result = FALSE;
  if (f != NULL)
    fclose(f);
  f = NULL;
  if (f != NULL)
    fclose(f);
  return Result;
}

void _dep_tp_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;
  filecount = 100;
  rand_init(1);
}
#endif
