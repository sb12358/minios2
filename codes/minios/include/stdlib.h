#ifndef LIBC_STDLIB_INCLUDED
#define LIBC_STDLIB_INCLUDED

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
int abs(int a);
int atoi(char *str);
int atol(char *str);
int rand ();
int srand(uint32 seed);

#endif
