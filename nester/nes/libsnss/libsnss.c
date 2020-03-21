/**************************************************************************/
/*
      libsnss.c

      (C) 2000 The SNSS Group
      See README.TXT file for license and terms of use.

      libsnss.c,v 1.2 2001/09/11 17:57:59 tekezo Exp
*/
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libsnss.h"
#include "types.h"
#include <sys/param.h>

/*
 * 2001 Takayama Fumihiko <tekezo@catv296.ne.jp> 
 *
 * NOTE:
 *  Save data is saved as big endian (network byte order).
 *  Use hton[ls], read_n[ls] instead swap16, swap32. 
 */

/**************************************************************************/
/* support functions */
/**************************************************************************/

/* 
   read network byte order long. 
   return host endian long. 
*/
static uint32
read_nl(uint8 *p)
{
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}


/* 
   read network byte order short 
   return host endian short. 
*/
static uint16
read_ns(uint8 *p)
{
  return (p[0] << 8) + p[1];
}

/**************************************************************************/

static SNSS_RETURN_CODE
SNSS_ReadBlockHeader (SnssBlockHeader *header, SNSS_FILE *snssFile)
{
  char headerBytes[12];
  
  if (fread (headerBytes, 12, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  strncpy (header->tag, headerBytes, TAG_LENGTH);
  header->tag[TAG_LENGTH] = '\0';
  header->blockVersion = read_nl(headerBytes + 4);
  header->blockLength = read_nl(headerBytes + 8);
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE
SNSS_WriteBlockHeader (SnssBlockHeader *header, SNSS_FILE *snssFile)
{
  char headerBytes[12];
  uint32 tmp_uint32;
  
  memcpy (headerBytes, header->tag, TAG_LENGTH);
  tmp_uint32 = htonl (header->blockVersion);
  memcpy(headerBytes + 4, &tmp_uint32, 4);
  tmp_uint32 = htonl (header->blockLength);
  memcpy(headerBytes + 8, &tmp_uint32, 4);
  
  if (fwrite (headerBytes, 12, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  return SNSS_OK;
}

/**************************************************************************/

const char *
SNSS_GetErrorString (SNSS_RETURN_CODE code)
{
  switch (code)
  {
    case SNSS_OK:
      return "no error";
      
    case SNSS_BAD_FILE_TAG:
      return "not an SNSS file";
      
    case SNSS_OPEN_FAILED:
      return "could not open SNSS file";
      
    case SNSS_CLOSE_FAILED:
      return "could not close SNSS file";
      
    case SNSS_READ_FAILED:
      return "could not read from SNSS file";
      
    case SNSS_WRITE_FAILED:
      return "could not write to SNSS file";
      
    case SNSS_OUT_OF_MEMORY:
      return "out of memory";
      
    case SNSS_UNSUPPORTED_BLOCK:
      return "unsupported block type";
      
    default:
      return "unknown error";
  }
}

/**************************************************************************/
/* functions for reading and writing SNSS file headers */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadFileHeader (SNSS_FILE *snssFile)
{
  char buf[8];
  
  if (fread (buf, 8, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  if (strncmp(buf, "SNSS", 4))
    return SNSS_BAD_FILE_TAG;
  
  strncpy(snssFile->headerBlock.tag, "SNSS", 5);
  snssFile->headerBlock.numberOfBlocks = read_nl(buf + 4);
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteFileHeader (SNSS_FILE *snssFile)
{
  uint32 tmp_uint32;
  uint8 writeBuffer[8];
  
  /* always place the SNSS tag in this field */
  strncpy (writeBuffer, "SNSS", 4);
  
  tmp_uint32 = htonl(snssFile->headerBlock.numberOfBlocks);
  memcpy(writeBuffer + 4, &tmp_uint32, 4);
  
  if (fwrite (writeBuffer, 8, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  return SNSS_OK;
}

/**************************************************************************/
/* general file manipulation functions */
/**************************************************************************/
SNSS_RETURN_CODE
SNSS_OpenFile (SNSS_FILE **snssFile, const char *filename, SNSS_OPEN_MODE mode)
{
  SNSS_RETURN_CODE error_code, return_code;
  
  *snssFile = malloc(sizeof(SNSS_FILE));
  if (NULL == *snssFile)
  {
    error_code = SNSS_OUT_OF_MEMORY;
    goto error;
  }
  
  (*snssFile)->mode = mode;
  
  if (SNSS_OPEN_READ == mode)
    (*snssFile)->fp = fopen (filename, "rb");
  else 
  {
    (*snssFile)->fp = fopen (filename, "wb");
    (*snssFile)->headerBlock.numberOfBlocks = 0;
  }
  
  if (NULL == (*snssFile)->fp) 
  {
    error_code = SNSS_OPEN_FAILED;
    goto error;
  }
  
  if (SNSS_OPEN_READ == mode)
    return_code = SNSS_ReadFileHeader(*snssFile);
  else
    return_code = SNSS_WriteFileHeader(*snssFile);
  
  if (return_code != SNSS_OK)
  {
    error_code = return_code;
    goto error;
  }
  
  return SNSS_OK;
  
error:
  if (*snssFile) free(*snssFile);
  return error_code;
}

/**************************************************************************/

SNSS_RETURN_CODE
SNSS_CloseFile (SNSS_FILE **snssFile)
{
  int prevLoc;
  SNSS_RETURN_CODE code;
  
  /* file was never open, so this should indicate success- kinda. */
  if (NULL == *snssFile)
      return SNSS_OK;
  
  if (SNSS_OPEN_WRITE == (*snssFile)->mode)
  {
    prevLoc = ftell((*snssFile)->fp);
    fseek((*snssFile)->fp, 0, SEEK_SET);
    
    /* write the header again to get block count correct */
    if (SNSS_OK != (code = SNSS_WriteFileHeader(*snssFile)))
      return SNSS_CLOSE_FAILED;
    
    fseek((*snssFile)->fp, prevLoc, SEEK_SET);
  }
  
  if (fclose ((*snssFile)->fp) != 0)
    return SNSS_CLOSE_FAILED;
  
  free(*snssFile);
  *snssFile = NULL;
  
  return SNSS_OK;
}

/**************************************************************************/

SNSS_RETURN_CODE 
SNSS_GetNextBlockType (SNSS_BLOCK_TYPE *blockType, SNSS_FILE *snssFile)
{
  char tagBuffer[TAG_LENGTH + 1];
  int pos = ftell(snssFile->fp);

  if (fread (tagBuffer, TAG_LENGTH, 1, snssFile->fp) != 1) 
    return SNSS_READ_FAILED;
  tagBuffer[TAG_LENGTH] = '\0';
   
  /* reset the file pointer to the start of the block */
  if (fseek (snssFile->fp, -TAG_LENGTH, SEEK_CUR) != 0) 
    return SNSS_READ_FAILED;
  
  if (pos != ftell(snssFile->fp))
    return SNSS_READ_FAILED;
  
  /* figure out which type of block it is */
  if (strcmp (tagBuffer, "BASR") == 0) 
  {
    *blockType = SNSS_BASR;
    return SNSS_OK;
  } 
  else if (strcmp (tagBuffer, "VRAM") == 0) 
  {
    *blockType = SNSS_VRAM;
    return SNSS_OK;
  }
  else if (strcmp (tagBuffer, "SRAM") == 0) 
  {
    *blockType = SNSS_SRAM;
    return SNSS_OK;
  }
  else if (strcmp (tagBuffer, "MPRD") == 0) 
  {
    *blockType = SNSS_MPRD;
    return SNSS_OK;
  }
  else if (strcmp (tagBuffer, "CNTR") == 0) 
  {
    *blockType = SNSS_CNTR;
    return SNSS_OK;
  }
  else if (strcmp (tagBuffer, "SOUN") == 0) 
  {
    *blockType = SNSS_SOUN;
    return SNSS_OK;
  }
  else 
  {
    *blockType = SNSS_UNKNOWN_BLOCK;
    return SNSS_OK;
  }
}

/**************************************************************************/

SNSS_RETURN_CODE 
SNSS_SkipNextBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK)
    return SNSS_READ_FAILED;
  
  if (fseek (snssFile->fp, header.blockLength, SEEK_CUR) != 0)
    return SNSS_READ_FAILED;
  
  return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing base register blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadBaseBlock (SNSS_FILE *snssFile)
{
  char blockBytes[BASE_BLOCK_LENGTH]; 
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK) 
    return SNSS_READ_FAILED;
  if (strcmp(header.tag, "BASR"))
    return SNSS_READ_FAILED;
  
  if (header.blockLength != BASE_BLOCK_LENGTH)
    return SNSS_READ_FAILED;
  
  if (fread (blockBytes, BASE_BLOCK_LENGTH, 1, snssFile->fp) != 1) 
    return SNSS_READ_FAILED;
  
  snssFile->baseBlock.regA = blockBytes[0x0];
  snssFile->baseBlock.regX = blockBytes[0x1];
  snssFile->baseBlock.regY = blockBytes[0x2];
  snssFile->baseBlock.regFlags = blockBytes[0x3];
  snssFile->baseBlock.regStack = blockBytes[0x4];
  snssFile->baseBlock.regPc = read_ns(blockBytes + 0x5);
  snssFile->baseBlock.reg2000 = blockBytes[0x7];
  snssFile->baseBlock.reg2001 = blockBytes[0x8];
  memcpy (snssFile->baseBlock.cpuRam, blockBytes + 0x9, 0x800);
  memcpy (snssFile->baseBlock.spriteRam, blockBytes + 0x809, 0x100);
  memcpy (snssFile->baseBlock.ppuRam, blockBytes + 0x909, 0x1000);
  memcpy (snssFile->baseBlock.palette, blockBytes + 0x1909, 0x20);
  memcpy (snssFile->baseBlock.mirrorState, blockBytes + 0x1929, 0x4);
  snssFile->baseBlock.vramAddress = read_ns(blockBytes + 0x192D);
  snssFile->baseBlock.spriteRamAddress = blockBytes[0x192F];
  snssFile->baseBlock.tileXOffset = blockBytes[0x1930];
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteBaseBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  SNSS_RETURN_CODE returnCode;
  char blockBytes[BASE_BLOCK_LENGTH];
  uint16 tmp_uint16;
  
  strcpy (header.tag, "BASR");
  header.blockVersion = SNSS_BLOCK_VERSION;
  header.blockLength = BASE_BLOCK_LENGTH;
  if ((returnCode = SNSS_WriteBlockHeader (&header, snssFile)) != SNSS_OK)
    return returnCode;
  
  blockBytes[0x0] = snssFile->baseBlock.regA;
  blockBytes[0x1] = snssFile->baseBlock.regX;
  blockBytes[0x2] = snssFile->baseBlock.regY;
  blockBytes[0x3] = snssFile->baseBlock.regFlags;
  blockBytes[0x4] = snssFile->baseBlock.regStack;
  tmp_uint16 = htons (snssFile->baseBlock.regPc);
  memcpy (blockBytes + 0x5, &tmp_uint16, 2);
  blockBytes[0x7] = snssFile->baseBlock.reg2000;
  blockBytes[0x8] = snssFile->baseBlock.reg2001;
  memcpy (blockBytes + 0x9, snssFile->baseBlock.cpuRam, 0x800);
  memcpy (blockBytes + 0x809, snssFile->baseBlock.spriteRam, 0x100);
  memcpy (blockBytes + 0x909, snssFile->baseBlock.ppuRam, 0x1000);
  memcpy (blockBytes + 0x1909, snssFile->baseBlock.palette, 0x20);
  memcpy (blockBytes + 0x1929, snssFile->baseBlock.mirrorState, 0x4);
  tmp_uint16 = htons (snssFile->baseBlock.vramAddress);
  memcpy (blockBytes + 0x192D, &tmp_uint16, 2);
  blockBytes[0x192F] = snssFile->baseBlock.spriteRamAddress;
  blockBytes[0x1930] = snssFile->baseBlock.tileXOffset;
  
  if (fwrite (blockBytes, BASE_BLOCK_LENGTH, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  snssFile->headerBlock.numberOfBlocks++;
  
  return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing VRAM blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadVramBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK)
    return SNSS_READ_FAILED;
  if (strcmp(header.tag, "VRAM"))
    return SNSS_READ_FAILED;
  
  if (header.blockLength > VRAM_BLOCK_LENGTH)
    return SNSS_READ_FAILED;
  
  if (fread (snssFile->vramBlock.vram, header.blockLength, 1, snssFile->fp) != 1) 
    return SNSS_READ_FAILED;
  
  snssFile->vramBlock.vramSize = header.blockLength;
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteVramBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  SNSS_RETURN_CODE returnCode;
  
  strcpy (header.tag, "VRAM");
  header.blockVersion = SNSS_BLOCK_VERSION;
  header.blockLength = snssFile->vramBlock.vramSize;
  if ((returnCode = SNSS_WriteBlockHeader (&header, snssFile)) != SNSS_OK)
    return returnCode;
  
  if (fwrite (snssFile->vramBlock.vram, 
              snssFile->vramBlock.vramSize, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  snssFile->headerBlock.numberOfBlocks++;
  
  return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing SRAM blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadSramBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK) 
    return SNSS_READ_FAILED;
  
  if (strcmp(header.tag, "SRAM"))
    return SNSS_READ_FAILED;
  
  if (header.blockLength > SRAM_8K + 1)
    return SNSS_READ_FAILED;
  
  /* read blockLength - 1 bytes to get all of the SRAM */
  if (fread (&snssFile->sramBlock.sramEnabled, 1, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  /* SRAM size is the size of the block - 1 (SRAM enabled byte) */
  if (fread (&(snssFile->sramBlock.sram),
             header.blockLength - 1, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  snssFile->sramBlock.sramSize = header.blockLength - 1;
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteSramBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  SNSS_RETURN_CODE returnCode;
  
  strcpy (header.tag, "SRAM");
  header.blockVersion = SNSS_BLOCK_VERSION;
  /* length of block is size of SRAM plus SRAM enabled byte */
  header.blockLength = snssFile->sramBlock.sramSize + 1;
  if ((returnCode = SNSS_WriteBlockHeader (&header, snssFile)) != SNSS_OK) 
    return returnCode;
  
  if (fwrite (&(snssFile->sramBlock.sramEnabled), 
              1, 1, snssFile->fp) != 1) 
    return SNSS_WRITE_FAILED;
  
  if (fwrite (snssFile->sramBlock.sram, 
              snssFile->sramBlock.sramSize, 1, snssFile->fp) != 1) 
    return SNSS_WRITE_FAILED;
  
  snssFile->headerBlock.numberOfBlocks++;
  
  return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing mapper data blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadMapperBlock (SNSS_FILE *snssFile)
{
  char blockBytes[0x8 + 0x10 + 0x80];
  int i;
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK)
    return SNSS_READ_FAILED;
  if (strcmp(header.tag, "MPRD"))
    return SNSS_READ_FAILED;
  
  if (header.blockLength > MAPPER_BLOCK_LENGTH)
    return SNSS_READ_FAILED;
  
  if (fread (blockBytes, header.blockLength, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  for (i = 0; i < 4; i++)
    snssFile->mapperBlock.prgPages[i] = read_ns(blockBytes + i * 2);

  for (i = 0; i < 8; i++)
    snssFile->mapperBlock.chrPages[i] = read_ns(blockBytes + 0x8 + i * 2);
  
  memcpy (&snssFile->mapperBlock.extraData.mapperData, &blockBytes[0x18], 0x80);
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteMapperBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  char blockBytes[MAPPER_BLOCK_LENGTH];
  uint16 tmp_uint16;
  int i;
  SNSS_RETURN_CODE returnCode;
  
  strcpy (header.tag, "MPRD");
  header.blockVersion = SNSS_BLOCK_VERSION;
  header.blockLength = MAPPER_BLOCK_LENGTH;
  if ((returnCode = SNSS_WriteBlockHeader (&header, snssFile)) != SNSS_OK)
    return returnCode;
  
  for (i = 0; i < 4; i++) 
  {
    tmp_uint16 = htons (snssFile->mapperBlock.prgPages[i]);
    memcpy (blockBytes + i * 2, &tmp_uint16, 2);
  }
  
  for (i = 0; i < 8; i++) 
  {
    tmp_uint16 = htons (snssFile->mapperBlock.chrPages[i]);
    memcpy (blockBytes + 0x8 + i * 2, &tmp_uint16, 2);
  }
  
  memcpy (&blockBytes[0x18], &snssFile->mapperBlock.extraData.mapperData, 0x80);
  
  if (fwrite (blockBytes, MAPPER_BLOCK_LENGTH, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  snssFile->headerBlock.numberOfBlocks++;
  
  return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing controller data blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadControllersBlock (SNSS_FILE *snssFile)
{
   return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteControllersBlock (SNSS_FILE *snssFile)
{
   return SNSS_OK;
}

/**************************************************************************/
/* functions for reading and writing sound blocks */
/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_ReadSoundBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  
  if (SNSS_ReadBlockHeader (&header, snssFile) != SNSS_OK)
    return SNSS_READ_FAILED;
  if (strcmp(header.tag, "SOUN"))
    return SNSS_READ_FAILED;
  
  if (header.blockLength > SOUND_BLOCK_LENGTH)
    return SNSS_READ_FAILED;
  
  if (fread (snssFile->soundBlock.soundRegisters, 
             header.blockLength, 1, snssFile->fp) != 1)
    return SNSS_READ_FAILED;
  
  return SNSS_OK;
}

/**************************************************************************/

static SNSS_RETURN_CODE 
SNSS_WriteSoundBlock (SNSS_FILE *snssFile)
{
  SnssBlockHeader header;
  SNSS_RETURN_CODE returnCode;
  
  strcpy (header.tag, "SOUN");
  header.blockVersion = SNSS_BLOCK_VERSION;
  header.blockLength = SOUND_BLOCK_LENGTH;
  if ((returnCode = SNSS_WriteBlockHeader (&header, snssFile)) != SNSS_OK)
    return returnCode;
  
  if (fwrite (snssFile->soundBlock.soundRegisters, 
              SOUND_BLOCK_LENGTH, 1, snssFile->fp) != 1)
    return SNSS_WRITE_FAILED;
  
  snssFile->headerBlock.numberOfBlocks++;
  
  return SNSS_OK;
}

/**************************************************************************/
/* general functions for reading and writing SNSS data blocks */
/**************************************************************************/

SNSS_RETURN_CODE
SNSS_ReadBlock (SNSS_FILE *snssFile, SNSS_BLOCK_TYPE blockType)
{
  switch (blockType) 
  {
    case SNSS_BASR:
      return SNSS_ReadBaseBlock (snssFile);
      
    case SNSS_VRAM:
      return SNSS_ReadVramBlock (snssFile);
      
    case SNSS_SRAM:
      return SNSS_ReadSramBlock (snssFile);
      
    case SNSS_MPRD:
      return SNSS_ReadMapperBlock (snssFile);
      
    case SNSS_CNTR:
      return SNSS_ReadControllersBlock (snssFile);
      
    case SNSS_SOUN:
      return SNSS_ReadSoundBlock (snssFile);
      
    case SNSS_UNKNOWN_BLOCK:
    default:
      return SNSS_UNSUPPORTED_BLOCK;
  }
}

/**************************************************************************/

SNSS_RETURN_CODE
SNSS_WriteBlock (SNSS_FILE *snssFile, SNSS_BLOCK_TYPE blockType)
{
  switch (blockType)
  {
    case SNSS_BASR:
      return SNSS_WriteBaseBlock (snssFile);
      
    case SNSS_VRAM:
      return SNSS_WriteVramBlock (snssFile);
      
    case SNSS_SRAM:
      return SNSS_WriteSramBlock (snssFile);
      
    case SNSS_MPRD:
      return SNSS_WriteMapperBlock (snssFile);
      
    case SNSS_CNTR:
      return SNSS_WriteControllersBlock (snssFile);
      
    case SNSS_SOUN:
      return SNSS_WriteSoundBlock (snssFile);
      
    case SNSS_UNKNOWN_BLOCK:
    default:
      return SNSS_UNSUPPORTED_BLOCK;
  }
}

/*
** libsnss.c,v
** Revision 1.2  2001/09/11 17:57:59  tekezo
** delete CR after NL
**
** Revision 1.5  2000/07/09 15:37:21  matt
** all block read/write calls now pass through a common handler
**
** Revision 1.4  2000/07/09 03:39:06  matt
** minor modifications
**
** Revision 1.3  2000/07/08 16:01:39  matt
** added bald's changes, made error checking more robust
**
** Revision 1.2  2000/07/04 04:46:06  matt
** simplified handling of SNSS states
**
** Revision 1.1  2000/06/29 14:13:28  matt
** initial revision
**
*/
