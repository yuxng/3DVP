/* 
 * Name:         edge.c
 *
 * Coded:        Brian Curless 10/24/95
 *
 * Comment:      Initializes edge table containing triangle locations.
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "mc.h"
#include "lorensen_cases.h"


/* This version of InitEdgeTable uses Bill Lorensen's edge table */


void InitEdgeTable()
{
  int i, ntris;
  Index index;
  EDGE_LIST *edges, edge;

  for (index = 0; index < 256; index++) {

      edges = poly_cases[index].edges;
      ntris = 0;
      while (ntris < 5) {
	  if (edges[ntris*4] == 0 && edges[ntris*4+1] == 0 
	      && edges[ntris*4+2] == 0)
	      break;

	  ntris++;
      }

      TheEdgeTable[index].Ntriangles = ntris;
      if (ntris == 0)
	  continue;


      TheEdgeTable[index].TriangleList =
	  (Triple *) calloc(ntris, sizeof(Triple));

      for (i = 0; i < 12; i++)
	  (TheEdgeTable[index].edge)[i] = FALSE;

      for (i = 0; i < ntris; i++) {
	  edge = edges[i*4]-1;
	  TheEdgeTable[index].TriangleList[i].A = edge;
	  TheEdgeTable[index].edge[edge] = TRUE;

	  edge = edges[i*4+1]-1;
	  TheEdgeTable[index].TriangleList[i].B = edge;
	  TheEdgeTable[index].edge[edge] = TRUE;

	  edge = edges[i*4+2]-1;
	  TheEdgeTable[index].TriangleList[i].C = edge;
	  TheEdgeTable[index].edge[edge] = TRUE;
      }
  }
} /* InitEdgeTable */
