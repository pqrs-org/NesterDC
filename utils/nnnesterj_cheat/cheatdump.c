#include <stdio.h>

typedef unsigned int uint32;
typedef unsigned char uint8;

#define NNNESTERJ_CHEATINFO_SIZE 100

typedef struct {
  uint8 comment[36]; 
  uint32 address;
  uint32 data;
  uint8 check_flag;
  uint8 flag;
  uint8 size;
} nnnesterj_cheatinfo_t;


nnnesterj_cheatinfo_t nnnesterj_cheatinfo[NNNESTERJ_CHEATINFO_SIZE];
uint32 nnnesterj_cheatinfo_size;


void dump_cheatinfo(nnnesterj_cheatinfo_t *p)
{
  printf("comment: %s\n", p->comment);
  printf("address: 0x%x\n", p->address);
  printf("data: 0x%x\n", p->data);
  printf("check_flag: %d\n", p->check_flag);
  printf("flag: %d\n", p->flag);
  printf("size: %d\n", p->size);
}


int
main(int argc, char **argv)
{
  FILE *fp = NULL;
  uint32 revision;
  
  if (argc == 1) 
  {
    fprintf(stderr, "Usage: hoge cheatfile\n");
    exit(1);
  }
  
  fp = fopen(argv[1], "r");
  if (fp == NULL) goto error;
  
  revision = 0;
  if (fread (&revision, 4, 1, fp) < 1) goto error;
  
  if (fread (&nnnesterj_cheatinfo_size, 4, 1, fp) < 1) goto error;
  if (nnnesterj_cheatinfo_size > NNNESTERJ_CHEATINFO_SIZE - 1) 
    nnnesterj_cheatinfo_size = NNNESTERJ_CHEATINFO_SIZE - 1;
  
  if (fseek(fp, 0x10, SEEK_SET) < 0) goto error;
  
  if (revision == 0) 
  {
    /* revision 0 */
    int i;
    nnnesterj_cheatinfo_t *p;
    
    p = nnnesterj_cheatinfo;
    for (i = 0; i < nnnesterj_cheatinfo_size; ++i)
    {
      if (fread (p->comment, 17, 1, fp) < 1) goto error;
      if (fread (&(p->address), 4, 1, fp) < 1) goto error;
      if (fread (&(p->data), 1, 1, fp) < 1) goto error;
      if (fread (&(p->check_flag), 2, 1, fp) < 1) goto error;
      p->flag = 0;
      p->size = 0;
      
      ++p;
    }
  }
  else if (revision == 1)
  {
    /* revision 1 */
    if (fread (&nnnesterj_cheatinfo, sizeof(nnnesterj_cheatinfo_t) * nnnesterj_cheatinfo_size, 1, fp) < 1) goto error;
  } 
  else 
  {
    /* unknown */
    goto error;
  }
  
  fclose (fp);
  
  {
    int i;
    
    for (i = 0; i < nnnesterj_cheatinfo_size; ++i)
    {
      dump_cheatinfo (nnnesterj_cheatinfo + i);
    }
  }
  
  return;
  
error:
  nnnesterj_cheatinfo_size = 0;
  if (fp) { fclose(fp); fp = NULL; }
  return;
}
