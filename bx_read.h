#ifndef BX_READ_H
#define BX_READ_H


#include "bx_types.h"

extern char *Gets(char *s);
extern char *Fgets(char *l, size_t sze, FILE *f);

extern void read_file_clustal(void);
extern void read_file_pretty(void);
extern void read_file_maligned(void);
extern void read_file_esee(void);
extern void read_file_phylip(void);

#endif /*BX_READ_H*/

