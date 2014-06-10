/*
 * Name :   mcfile.c
 *
 * Coded :   Paul Ning  9/18/89
 *
 * Comment :  Library routines for Marching Cubes File Package
 */

#include <stdlib.h>
#include <stdio.h>
#include "mcfile.h"


/*
 * GENERAL
 */

static void Fatal(message)
     char *message;
{
  printf("MCFILE : %s\n",message);
  fflush(stdout);
  exit(-1);
}  /* Fatal */


/*
 * HEADER STUFF 
 */

Boolean IsMCFile(fp)
     FILE *fp;
{
  int magic;

  rewind(fp);
  if ((fread(&magic,sizeof(int),1,fp) != 1) || (magic != MC_MAGIC))
    return(FALSE);
  else
    return(TRUE);
}  /* IsMCFile */

void MC_InitHeader(fp)
     FILE *fp;
{
  mcfile header;

  /* These lines inserted so that this compiles.  
     I don't understand what this function really does, other than
     write out an unitialized header.  (Brian Curless, June 5, 2006) */

  header.mc_length = 0;
  header.mc_tmaptype = 0;
  header.mc_type = 0;
  header.mc_magic = 0;

  MC_WriteHeader(fp,header);
}  /* MC_InitHeader */

void MC_WriteHeader(fp,header)
     FILE *fp;
     mcfile header;
{
  rewind(fp);
  if (fwrite(&header,sizeof(mcfile),1,fp) != 1) 
    Fatal("Error in writing MC header");
}  /* MC_WriteHeader */

mcfile MC_ReadHeader(fp)
     FILE *fp;
{
  mcfile header;

  rewind(fp);
  if (fread(&header,sizeof(mcfile),1,fp) != 1)
    Fatal("Error in reading MC header");
  return(header);
}  /* MC_ReadHeader */


/* 
 * SURFACE DATA STUFF
 */

void MC_WriteTriangleVertex(fp,vertex)
     FILE *fp;
     TriangleVertex vertex;
{
  if (fwrite(&vertex,sizeof(TriangleVertex),1,fp) != 1)
    Fatal("Error in writing vertex data");
}  /* MC_WriteTriangleVertex */

TriangleVertex MC_ReadTriangleVertex(fp)
     FILE *fp;
{
  TriangleVertex vertex;

  if (fread(&vertex,sizeof(TriangleVertex),1,fp) != 1)
    Fatal("Error in reading vertex data");
  return(vertex);
}  /* MC_ReadTriangleVertex */


