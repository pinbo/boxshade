#include "bx_read.h"

FILE *infile = NULL;

char *Gets(char *s) {
  fflush(stdout);
//   return Gets(s);
  if (fgets(s, sizeof s, stdin)){
    s[strcspn(s, "\n")] = '\0';
     return s;
  }
  return NULL;
}

/*
** Handle all those End-Of-Line sequences:
**
**  System   CR    LF
**   MAC      X     -
**   MSDOS    X     X
**   UNIX     -     X
**   VMS      ?     ?
**
**  Does _NOT_ return '\n' !!!
*/
char *Fgets(char *l, size_t sze, FILE *f) {
  char *cp = l;
  BOOL  cr, eoln = FALSE;

  if (f == stdin)
    return Gets(l);

  if (feof(f)) return NULL;

  cr = FALSE;
  while (sze>1 && !feof(f)) {
    int c = fgetc(f);
    if (c == EOF) {
      if (ferror(f)) return NULL;
      break;
    }
    if (cr && c != '\n') {
      /* CR end-of-line (MAC) */
      ungetc(c,f);
      eoln = TRUE;
      break;
    } else
	 cr = (c=='\r');
	 if ( cr )
      continue;
    --sze;
    if (c=='\n') { eoln = TRUE; break; }
    *(cp++) = c;
  }
  *cp = '\0';

  cr = FALSE;
  while (!eoln && !feof(f)) {
    int c = fgetc(f);
    if (c == EOF)
      break;
    if (cr && c != '\n') {
      /* CR end-of-line (MAC) */
      ungetc(c,f);
      break;
	 } else
	 cr = (c=='\r');
	 if ( cr )
		continue;
    if (c=='\n') break;
  }
  return l;
}

void term_par(char *term) {
  assert(parfile != NULL);
  rewind(parfile);
  do {
    Fgets(line_, 256, parfile);
  } while (!feof(parfile) && (*line_ != ':' || indx(line_, term) != 2) );
  assert(!feof(parfile));
}

void read_file_clustal(void)
{
  int i, j, sl;
  char nametag[41], emptytag[41];
  int taglen, seqstart, emptycount;

  sprintf(emptytag, "%*s", seqnamelen, "");

  infile = fopen(inname, TXT_RD);
  assert(infile != NULL);

  no_seq = 0;
  emptycount = 0;
  do {
    Fgets(line_, 256, infile);
    emptycount++;
  } while (strlen(line_) >= 5);
  do {
    Fgets(line_, 256, infile);
    emptycount++;
  } while (strlen(line_) <= 12);
  taglen = 0;
  do {
    if ((sl=strlen(line_)) > 12) {
      i = 1;
      do {
	i++;
      } while (!isspace(line_[i - 1]) && i < sl);
      taglen = i - 1;
      do {
	i++;
      } while (isspace(line_[i - 1]) && i < sl);
      if (taglen < 2) {
	/* seqstart:=i; */ /****** is that correct ??? ******/
	strcpy(nametag, emptytag);
      } else {
	seqstart = i;
	strncpy(nametag, line_, taglen);
	nametag[taglen] = '\0';
      }
    } else
      strcpy(nametag, emptytag);
    if (strcmp(nametag, emptytag)) {
      no_seq++;
      if (no_seq >= max_no_seq) {
	printf("OOPS, too many sequences in %s. Sorry, limit is %d!\n",
		inname, max_no_seq);
	exit(1);
      }
      strcpy(seqname[no_seq - 1], emptytag);
      startno[no_seq - 1] = 1;
      if (taglen > seqnamelen)
	taglen = seqnamelen;
      for (i = 0; i < taglen; i++)
	seqname[no_seq-1][i] = nametag[i];
    }
    if (!feof(infile)) {
      Fgets(line_, 256, infile);
    } else
      strcpy(nametag, emptytag);
  } while (strcmp(nametag, emptytag));

  rewind(infile);

  printf("%s has %d sequences. Reading ", inname, no_seq);
  fflush(stdout);

  for (i = 1; i < emptycount; i++) {
    Fgets(line_, 256, infile);
  }
  for (i = 0; i < no_seq; i++)
    seqlen[i] = 0;
  while (!feof(infile)) {
    for (i = 0; i < no_seq; i++) {
      Fgets(line_, 256, infile);
      j = 1;
      while (seqstart + j - 1 <= strlen(line_)) {
	seq[i][seqlen[i] + j - 1] = toupper(line_[seqstart + j - 2]);
	j++;
      }
      seqlen[i] += j - 1;
    }
    if (!feof(infile)) {
      Fgets(dummystring, 81, infile);
    }
    if (feof(infile))
      break;
    Fgets(dummystring, 81, infile);
    printf("."); fflush(stdout);
  }
  if (infile != NULL)
    fclose(infile);
  printf(" done\n");
}

// JZ added
// need to get no_seq, 
// for (i = 0; i < no_seq; i++)    seqlen[i] = 0;
// strcpy(seqname[no_seq - 1], emptytag);
// for (i = 0; i < taglen; i++)	seqname[no_seq-1][i] = nametag[i];    }
// Fgets(line_, 256, parfile);


void read_fasta()
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

    // delcred in the header file
    no_seq = 0;
    fp = fopen(inname, TXT_RD);
    assert(fp != NULL);
 
	int state = 0;
    int linelen = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		/* Delete trailing newline */
		if (line[read - 1] == '\n')
			line[read - 1] = 0;
        if (line[read - 2] == '\r')
			line[read - 2] = 0;
		/* Handle comment lines*/
		if (line[0] == '>') {
            if (no_seq > 0) seqlen[no_seq - 1] = linelen;
            linelen = 0;
            no_seq ++; // number of sequences
			// if (state == 1)
			// 	printf("\n");
            strcpy(seqname[no_seq - 1], line+1); // okay
            // printf("%s: ", seqname[no_seq - 1]);
			// state = 1;
		} else {
			/* Print everything else */
			// printf("%s", line);
            // strcat(seq[no_seq - 1], line);
            // seq[i][seqlen[i] + j - 1] = toupper(line_[seqstart + j - 2]);
            int j = 0;
            while (j < strlen(line)){
                seq[no_seq - 1][linelen + j] = toupper(line[j]);
                j++;
            }
            linelen += strlen(line);
		}
	}
    seqlen[no_seq - 1] = linelen; // for the last sequence
    // init line start number
    for (int i = 0; i < no_seq; i++)
      startno[i] = 1;
	// printf("\n");
    // for (int i = 0; i < no_seq; i++)  seqlen[i] = strlen(seq[i]);
	fclose(fp);
	if (line)
		free(line);
    // check
    // for (int i = 0; i < no_seq; i++){
    //     printf("leng of seq %d is %d: \n", i, seqlen[i]);
    //     printf("name of seq %d is %s: \n", i, seqname[i]);
    //     printf("sequ of seq %d is %s: \n", i, seq[i]);
    // }
}


#define blocklen        10
#define blockno         5
#define blockdist       1


void read_file_pretty(void)
{
  int blockstart, i, j, k;
  int offs;
  BOOL found;
  char nametag[256], tempname[256];
  char emptytag[81];
  BOOL msfflag, num_mode;

  infile = NULL;
  num_mode = FALSE;   /*new*/
  infile = fopen( inname, TXT_RD);
  assert(infile != NULL);
  do {
    Fgets(line_, 256, infile);
  } while (!(indx(line_, "..") > 0 || feof(infile)));
  do {
    Fgets(line_, 256, infile);
  } while (!(indx(line_, "//") > 0 || feof(infile)));
  if (!feof(infile))
    msfflag = TRUE;
  else {
    msfflag = FALSE;
    fseek(infile, 0, SEEK_SET);
    assert(infile != NULL);
    do {
      Fgets(line_, 256, infile);
    } while (indx(line_, "..") <= 0);
  }

  Fgets(dummystring, 81, infile);
  Fgets(line_, 256, infile);
  /*numbers or first sequence*/
  if (indx(line_, "  1    ") > 0) {
    Fgets(line_, 256, infile);
    num_mode = TRUE;
  }

  blockstart = 1;
  while (line_[blockstart - 1] == ' ')
    blockstart++;
  while (line_[blockstart - 1] != ' ')
    blockstart++;
  while (line_[blockstart - 1] == ' ')
    blockstart++;
  sprintf(emptytag, "%*s", blockstart, "");
  no_seq = 0;

  fseek(infile, 0, SEEK_SET);
  assert(infile != NULL);
  do {
    Fgets(line_, 256, infile);
  } while (!(indx(line_, "..") > 0 || feof(infile)));
  do {
    Fgets(line_, 256, infile);
  } while (!(indx(line_, "//") > 0 || feof(infile)));
  while (Fgets(line_, 256, infile) != NULL) {
    if (strlen(line_) < blockstart)
      continue;
/*  if (!strcmp(substr_(STR1, (void *)line_, 1, blockstart - 1), emptytag)) */
    if (! strncmp(line_, emptytag, blockstart - 1) )
      continue;
    *tempname = '\0';
    i = 1;
    while (line_[i - 1] == ' ')
      i++;
    while (line_[i - 1] != ' ') {
      sprintf(tempname + strlen(tempname), "%c", line_[i - 1]);
      i++;
    }
    if (no_seq == 0) {
      no_seq = 1;
      strcpy(seqname[0], tempname);
      continue;
    }
    found = FALSE;
    for (i = 0; i < no_seq; i++) {
      if (!strcmp(tempname, seqname[i]))
	found = TRUE;
    }
    if (found == FALSE) {
      no_seq++;
      strcpy(seqname[no_seq - 1], tempname);
    }
  }
  for (i = 0; i < no_seq; i++)
    startno[i] = 1;

  fseek(infile, 0, SEEK_SET);
  assert(infile != NULL);
  do {
    Fgets(line_, 256, infile);
  } while (indx(line_, "..") <= 0);
  if (msfflag) {
    do {
      Fgets(line_, 256, infile);
    } while (indx(line_, "//") <= 0);
  }
  Fgets(dummystring, 81, infile);
  if (num_mode) {
    Fgets(dummystring, 81, infile);
  }
  offs = 1;
  do {
    i = 1;
    if (!feof(infile)) {
      Fgets(line_, 256, infile);
    }
    do {
      if (strlen(line_) >= blockstart) {
	if (strncmp(line_, emptytag, blockstart-1)) {
/*      if (strcmp(substr_(STR1, (void *)line_, 1, blockstart - 1), emptytag)) { */
	  char *cp = nametag;
	  j = 0;
	  while (line_[j] == ' ')
	    j++;
	  while (line_[j] != ' ') {
	    *(cp++) = line_[j];
	    j++;
	  }
	  *cp = '\0';
	  while (line_[j] == ' ')
	    j++;
	  blockstart = min(blockstart, j+1);
	} else
	  *nametag = '\0';
      } else
	*nametag = '\0';
      if (strcmp(nametag, seqname[i - 1])) {
	for (j = -1; j <= blocklen * blockno - 2; j++)
	  seq[i - 1][offs + j] = ' ';
      } else {
	for (j = 0; j < blockno; j++) {
	  for (k = -1; k <= blocklen - 2; k++) {
	    if (blockstart + j * (blocklen + blockdist) + k + 1 <= strlen(line_))
	      seq[i - 1][offs + j * blocklen + k] = toupper(
		  line_[blockstart + j * (blocklen + blockdist) + k]);
	  }
	}
	if (!feof(infile)) {
	  Fgets(line_, 256, infile);
	}
      }
      i++;
    } while (i <= no_seq);
    offs += blocklen * blockno;
    if (num_mode && !feof(infile)) {
      Fgets(line_, 256, infile);
    }
  } while (!feof(infile));
  for (i = 0; i < no_seq; i++) {
    int sl;
    seqlen[i] = offs - 1;
    if ((sl=strlen(seqname[i])) < seqnamelen)
      sprintf(seqname[i]+sl, "%*s", seqnamelen-sl, "");
  }
  if (infile != NULL)
    fclose(infile);
  infile = NULL;
}

#undef blocklen
#undef blockno
#undef blockdist


void read_file_maligned(void)
{
  int i, j, sl;
  char emptytag[256];

  infile = NULL;
  sprintf(emptytag, "%*s", seqnamelen, "");
  infile = fopen(inname, TXT_RD);
  assert(infile != NULL);
  no_seq = 0;
  Fgets(line_, 256, infile);
  Fgets(line_, 256, infile);
  while (Fgets(line_, 256, infile) != NULL) {
    if (!feof(infile)) {   /*filename*/
      Fgets(line_, 256, infile);
    }
    if (feof(infile))
      continue;
    no_seq++;
    strcpy(seqname[no_seq - 1], emptytag);
    Fgets(line_, 256, infile);
    j = min(strlen(line_), seqnamelen);
    for (i = 0; i < j; i++)
      seqname[no_seq - 1][i] = line_[i];
    fscanf(infile, "%d%*[^\n]", &startno[no_seq - 1]);
    getc(infile);   /*seqstart, seqend*/
    seqlen[no_seq - 1] = 0;
    while (!feof(infile) && *line_ != '\0') {
      Fgets(line_, 256, infile);
      if (*line_ == '\0')
	break;
      sl = strlen(line_);
      for (j = 0; j < sl; j++) {
	seq[no_seq-1][seqlen[no_seq-1] + j] = toupper(line_[j]);
	seqlen[no_seq-1] += j+1;
      }
    }
  }
  if (infile != NULL)
    fclose(infile);
}


void read_file_esee(void)
{
  int i, j;
  char emptytag[256];
  byte b, versno, snl;

  infile = NULL;
  sprintf(emptytag, "%*s", seqnamelen, "");
  infile = fopen(inname, BIN_RD);
  assert(infile != NULL);
  fread(&versno, sizeof(byte), 1, infile);
  fread(&b, sizeof(byte), 1, infile);
  no_seq = b;
  fread(&b, sizeof(byte), 1, infile);
  no_seq += b * 256;
  for (i = 0; i < no_seq; i++) {
    fread(&b, sizeof(byte), 1, infile);
    seqlen[i] = b;
    fread(&b, sizeof(byte), 1, infile);
    seqlen[i] += b * 256 - 1;
    for (j = 0; j < seqlen[i]; j++) {
      fread(&b, sizeof(byte), 1, infile);
      seq[i][j] = toupper(b);
    }
    fread(&b, sizeof(byte), 1, infile);
    if (b != 1)
      printf("ESEE format error in seq:%2d\n", i + 1);
    fread(&b, sizeof(byte), 1, infile);
    snl = b;
    strcpy(seqname[i], emptytag);
    startno[i] = 1;
    for (j = 1; j <= 64; j++) {
      fread(&b, sizeof(byte), 1, infile);
      if (j <= snl && j <= seqnamelen)
	seqname[i][j-1] = b;
    }
    for (j = 1; j <= 6; j++)   /*linesum*/
      fread(&b, sizeof(byte), 1, infile);
    for (j = 1; j <= 6; j++)   /*start*/
      fread(&b, sizeof(byte), 1, infile);
    fread(&b, sizeof(byte), 1, infile);   /*type*/
    fread(&b, sizeof(byte), 1, infile);   /*NrOFF*/
    fread(&b, sizeof(byte), 1, infile);   /*reverse*/
  }
  if (infile != NULL)
    fclose(infile);
}


void read_file_phylip(void)
{
  int i, j, sl;
  FILE *infile;
  BOOL firstline;

  infile = NULL;
  infile = fopen(inname, TXT_RD);
  assert(infile != NULL);
  no_seq = 0;
  firstline = TRUE;
  fscanf(infile, "%d%*[^\n]", &no_seq);
  getc(infile);
  while (!feof(infile)) {
    for (i = 0; i < no_seq; i++) {
      Fgets(line_, 256, infile);
      if (firstline) {
	startno[i] = 1;
	seqlen[i] = 0;
	strncpy(seqname[i], line_, 10);
	seqname[i][10] = '\0';
	memmove(line_,line_+11, sizeof(line_)-11);
      }
      sl = strlen(line_);
      for (j = 0; j < sl; j++) {
	if (line_[j] != ' ') {
	  seq[i][seqlen[i]] = line_[j];
	  seqlen[i]++;
	}
      }
    }
    Fgets(dummystring, 81, infile);
    firstline = FALSE;
  }
  if (infile != NULL)
    fclose(infile);
}

