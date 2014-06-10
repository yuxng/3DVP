/*

Convert any PLY file to either a binary or an ascii PLY file, depending on
which of WRITE_ASCII or WRITE_BINARY is defined during compilation.

This program does NOT use the other_element routines because they require
that storing the entire model.  Here, each individual element is saved
right away and then the memory is used again for the next element.  This
means that running out of memory is not a problem.

Greg Turk, June 1994

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <ply.h>


#ifdef WRITE_ASCII
void usage(char *progname) {
  fprintf (stderr, "usage: %s [flags] [infile] > outfile\n", progname);
  fprintf (stderr, "   or: %s [flags] < infile > outfile\n", progname);
  fprintf (stderr, "          -p (print element labels)\n");
  exit (-1);
}
#endif

#ifdef WRITE_BINARY
void usage(char *progname) {
  fprintf (stderr, "usage: %s [infile] > outfile\n", progname);
  fprintf (stderr, "   or: %s < infile > outfile\n", progname);
  exit (-1);
}
#endif


/******************************************************************************
Convert binary PLY files to ascii.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j,k;
  PlyFile *in_ply;
  PlyFile *out_ply;
  int nelems;
  char **elist;
  int file_type;
  float version;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  PlyProperty **plist_copy;
  PlyProperty *prop;
  char *elem_name;
  int num_comments;
  char **comments;
  int num_obj_info;
  char **obj_info;
  char *data;
  int offset;
  int *offset_list;
  int **lists;
  int *list_count;
  char **list_ptr;
  int verbose_flag = 0;
  char *progname;
  FILE *inFile = stdin;

#ifndef WRITE_ASCII
#ifndef WRITE_BINARY

  fprintf (stderr, "'%s' compiled incorrectly.\n", argv[0]);
  fprintf (stderr,
           "Must have WRITE_ASCII or WRITE_BINARY defined during compile.\n");
  exit (-1);

#endif
#endif

  /* remember executable name */
  progname = argv[0];
  argc--;
  argv++;

  /* Parse args */

#ifdef WRITE_ASCII
  while (argc > 0) {
    if (equal_strings (argv[0], "-h")) {
      usage(progname);
      exit(-1);
    } else if (equal_strings (argv[0], "-p")) {
      verbose_flag = 1;
    } else if (argv[0][0] != '-') {
      /* Open input file */
      inFile = fopen(argv[0], "r");
      if (inFile == NULL) {
	fprintf(stderr, "Error: Could not open input ply file %s\n", argv[0]);
	usage(progname);
	exit(-1);
      }
    } else {
      fprintf(stderr, "Error, unhandled arg: %s\n", argv[0]);
      usage(progname);
      exit(-1);
    }
    argc--;
    argv++;
  }

#endif

#ifdef WRITE_BINARY
  while (argc > 0) {
    if (equal_strings (argv[0], "-h")) {
      usage(progname);
      exit(-1);
    } else if (argv[0][0] != '-') {
      /* Open input file */
      inFile = fopen(argv[0], "r");
      if (inFile == NULL) {
	fprintf(stderr, "Error: Could not open input ply file %s\n", argv[0]);
	usage(progname);
	exit(-1);
      }
    } else {
      fprintf(stderr, "Error, unhandled arg: %s\n", argv[0]);
      usage(progname);
      exit(-1);
    }
    argc--;
    argv++;
  }

#endif

  /* open the input and output files */

  in_ply  = ply_read  (inFile,  &nelems, &elist);

#ifdef WRITE_ASCII
  out_ply = ply_write (stdout, nelems, elist, PLY_ASCII);
#endif

#ifdef WRITE_BINARY
  out_ply = ply_write (stdout, nelems, elist, PLY_BINARY_BE);
#endif

  /* allocate space for various lists that keep track of the elements */

  plist_copy = (PlyProperty **) malloc (sizeof (PlyProperty *) * nelems);
  offset_list = (int *) malloc (sizeof (int) * nelems);
  lists = (int **) malloc (sizeof (int *) * nelems);
  list_count = (int *) malloc (sizeof (int));

  /* go through each kind of element that we learned is in the file */
  /* and come up with a list that has offsets for all properties */

  for (i = 0; i < nelems; i++) {

    /* get the description of the element */
    elem_name = elist[i];
    plist = ply_get_element_description(in_ply, elem_name, &num_elems, &nprops);

    /* make room for a list of the lists in an element, so that */
    /* we can later easily free up the space created by malloc'ed lists */

    list_count[i] = 0;
    lists[i] = (int *) malloc (sizeof (int) * nprops);

    /* set up pointers into data */

    offset = 0;
    for (j = 0; j < nprops; j++) {
      plist[j]->offset = offset;
      offset += 8;
      if (plist[j]->is_list) {
        plist[j]->count_offset = offset;
        lists[i][list_count[i]] = offset - 8;
        list_count[i]++;
        offset += 8;
      }
    }

    offset_list[i] = offset;

    /* copy the property list */

    plist_copy[i] = (PlyProperty *) malloc (sizeof (PlyProperty) * nprops);
    prop = plist_copy[i];

    for (j = 0; j < nprops; j++) {
      prop->name = plist[j]->name;
      prop->external_type = plist[j]->external_type;
      prop->internal_type = plist[j]->external_type;
      prop->offset = plist[j]->offset;
      prop->is_list = plist[j]->is_list;
      prop->count_external = plist[j]->count_external;
      prop->count_internal = plist[j]->count_external;
      prop->count_offset = plist[j]->count_offset;
      prop++;
    }

    ply_describe_element (out_ply, elem_name, num_elems, nprops, plist_copy[i]);
  }

  /* copy the comments and obj_info */

  comments = ply_get_comments (in_ply, &num_comments);
  for (i = 0; i < num_comments; i++)
    ply_put_comment (out_ply, comments[i]);

  obj_info = ply_get_obj_info (in_ply, &num_obj_info);
  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (out_ply, obj_info[i]);

  /* finish the header for the output file */
  ply_header_complete (out_ply);

  /* copy all the element information */

  for (i = 0; i < nelems; i++) {

    /* get the description of the element */
    elem_name = elist[i];
    plist = ply_get_element_description(in_ply, elem_name, &num_elems, &nprops);

    /* allocate space for an element */
    data = (char *) malloc (8 * offset_list[i]);

    /* set up for getting elements */
    ply_get_element_setup (in_ply, elem_name, nprops, plist_copy[i]);
    ply_put_element_setup (out_ply, elem_name);

    /* possibly print out name of element */
    if (verbose_flag)
      fprintf (out_ply->fp, "%s:\n", elem_name);

    /* copy all the elements */

    if (list_count[i]) {
      /* need to free the lists */
      for (j = 0; j < num_elems; j++) {
        ply_get_element (in_ply, (void *) data);
        ply_put_element (out_ply, (void *) data);
        for (k = 0; k < list_count[i]; k++) {
          list_ptr = (char **) (data + lists[i][k]);
          free (*list_ptr);
        }
      }
    }
    else {
      /* no lists */
      for (j = 0; j < num_elems; j++) {
        ply_get_element (in_ply, (void *) data);
        ply_put_element (out_ply, (void *) data);
      }
    }

  }

  /* close the PLY files */

  ply_close (in_ply);
  ply_close (out_ply);

  exit(0);

  return 0;

}

