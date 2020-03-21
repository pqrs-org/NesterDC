#include "types.h"
#include "nes.h"

void
load_genie_from_file(const char *filename, NES *emu)
{
  FILE *fp = NULL;
  char str[256];

  printf("load genie\n");
  
  fp = fopen (filename, "r");
  if (!fp) goto error;
  
  emu->clear_genie ();
  while (fgets(str, sizeof(str), fp))
  {
    printf("genie code = %s\n", str);
    emu->load_genie (str);
  }
  
  emu->set_genie ();
  
  fclose (fp);
  return;
  
error:
  if (fp) fclose (fp);
  return;
}
