# BOXSHADE makefile for unix (tested under Linux && Solaris)
CC     = gcc
CFLAGS = -O2
LDFLAGS= -s
LIBS   = -lm
OBJ    =.o
EXE    =

HEADER = bx_types.h bx_read.h dv_all.h

CSRCbx = box.c bx_read.c bx_misc.c

CSRCdv = dv_ansi.c dv_ascii.c dv_crt.c dv_fig.c\
	 dv_hpgl.c dv_lj250.c dv_pict.c dv_ps.c\
	 dv_regis.c dv_rtf.c dv_html.c

OBJS = box$(OBJ) bx_read$(OBJ) bx_misc$(OBJ) \
       dv_ansi$(OBJ) dv_ascii$(OBJ) dv_crt$(OBJ) dv_fig$(OBJ) \
       dv_hpgl$(OBJ) dv_lj250$(OBJ) dv_pict$(OBJ) dv_ps$(OBJ)\
       dv_regis$(OBJ) dv_rtf$(OBJ) dv_html$(OBJ)

boxshade$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

depend:
	$(CC) -MM $(CSRCbx) >depend
	$(CC) -MM $(CSRCdv) >>depend

box$(OBJ):      box.c      bx_types.h bx_read.h dv_all.h version.h
bx_read$(OBJ):  bx_read.c  bx_types.h bx_read.h
bx_misc$(OBJ):  bx_misc.c  bx_types.h
dv_ansi$(OBJ):  dv_ansi.c  bx_types.h bx_read.h dv_all.h
dv_ascii$(OBJ): dv_ascii.c bx_types.h bx_read.h dv_all.h
dv_crt$(OBJ):   dv_crt.c   bx_types.h bx_read.h dv_all.h
dv_fig$(OBJ):   dv_fig.c   bx_types.h bx_read.h dv_all.h
dv_hpgl$(OBJ):  dv_hpgl.c  bx_types.h bx_read.h dv_all.h
dv_html$(OBJ):  dv_html.c  bx_types.h bx_read.h dv_all.h version.h
dv_lj250$(OBJ): dv_lj250.c bx_types.h bx_read.h dv_all.h
dv_pict$(OBJ):  dv_pict.c  bx_types.h bx_read.h dv_all.h
dv_ps$(OBJ):    dv_ps.c    bx_types.h bx_read.h dv_all.h version.h
dv_regis$(OBJ): dv_regis.c bx_types.h bx_read.h dv_all.h
dv_rtf$(OBJ):   dv_rtf.c   bx_types.h bx_read.h dv_all.h

clean:
	rm *.o
	rm boxshade$(EXE)