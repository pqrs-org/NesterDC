#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
  long long addr;
  char name[128];
} func_addr_t;


int
main () 
{
  char buf[512];
#define FA_MAX 4096
  func_addr_t fa[FA_MAX];
  int fa_size = 0;
  
  /* setup func_addr */
  for (;;) 
  {
    if (fgets(buf, sizeof(buf), stdin) == NULL)
      break;
    
    /* 8c010000 <_start>: */
    if (strstr(buf, ">:")) {
      char *p = strchr(buf, ' ');
      
      if (p == NULL) 
      {
        fprintf(stderr, "Invalid line %s\n", buf);
        exit(1);
      }
      
      *p = '\0';
      fa[fa_size].addr = strtoll (buf, NULL, 16);
      snprintf(fa[fa_size].name, sizeof(fa[fa_size].name), "%s", p + 1);
      
      ++fa_size;
      if (fa_size > FA_MAX) 
      { 
        fprintf(stderr, "Too many function symbols...\n");
        exit(1);
      }
    }
  }
  
  fseek(stdin, 0, SEEK_SET);
  
  /* convert addr to name */
  for (;;) 
  {
    char *p;
    
    if (fgets(buf, sizeof(buf), stdin) == NULL)
      break;
    
    /* 8c010006:       24 d0           mov.l   8c010098 <init_sr>,r0   ! 0x500000f0 */
    p = strstr(buf, "! 0x");
    if (!p)
    {
      printf(buf);
    }
    else
    {
      long long addr = strtoll(p + 2, NULL, 16);
      int i;
      
      for (i = 0; i < fa_size; ++i) 
      {
        if (addr == fa[i].addr)
        {
          *(p + 2) = '\0';
          printf("%s%s", buf, fa[i].name);
          break;
        }
      }
      if (i == fa_size) 
      {
        printf(buf);
      }
    }
  }
}
