/*

Header for PLY polygon files.

- Greg Turk, March 1994

A PLY file contains a single polygonal _object_.

An object is composed of lists of _elements_.  Typical elements are
vertices, faces, edges and materials.

Each type of element for a given object has one or more _properties_
associated with the element type.  For instance, a vertex element may
have as properties three floating-point values x,y,z and three unsigned
chars for red, green and blue.

---------------------------------------------------------------

Copyright (c) 1994 The Board of Trustees of The Leland Stanford
Junior University.  All rights reserved.   
  
Permission to use, copy, modify and distribute this software and its   
documentation for any purpose is hereby granted without fee, provided   
that the above copyright notice and this permission notice appear in   
all copies of this software and that you do not sell the software.   
  
THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.   

*/

#ifndef __PLY_H__
#define __PLY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>

#ifdef LINUX
#include <sys/types.h>
#endif

#define PLY_ASCII      1        /* ascii PLY file */
#define PLY_BINARY_BE  2        /* binary PLY file, big endian */
#define PLY_BINARY_LE  3        /* binary PLY file, little endian */

#define PLY_OKAY    0           /* ply routine worked okay */
#define PLY_ERROR  -1           /* error in ply routine */

/* Define BIG/LITTLE_ENDIAN FLAGS if not already */

#ifndef BIG_ENDIAN
#define BIG_ENDIAN      4321	/* Flag for native Big Endian machines */
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   1234	/* Flag for native Little Endian machines */
#endif

  /* Define BYTE_ORDER for big/little endian machines */
/* This assumes Linux/Win32 are on PC, all else is SGI... */
#ifndef BYTE_ORDER
#if defined (LINUX) || defined (WIN32)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif

/* scalar data types supported by PLY format */

#define PLY_START_TYPE 0
#define PLY_CHAR       1
#define PLY_SHORT      2
#define PLY_INT        3
#define PLY_UCHAR      4
#define PLY_USHORT     5
#define PLY_UINT       6
#define PLY_FLOAT      7
#define PLY_DOUBLE     8
#define PLY_END_TYPE   9

#define  PLY_SCALAR  0
#define  PLY_LIST    1


typedef struct PlyProperty {    /* description of a property */

  char *name;                           /* property name */
  int external_type;                    /* file's data type */
  int internal_type;                    /* program's data type */
  int offset;                           /* offset bytes of prop in a struct */

  int is_list;                          /* 1 = list, 0 = scalar */
  int count_external;                   /* file's count type */
  int count_internal;                   /* program's count type */
  int count_offset;                     /* offset byte for list count */

} PlyProperty;

typedef struct PlyElement {     /* description of an element */
  char *name;                   /* element name */
  int num;                      /* number of elements in this object */
  int size;                     /* size of element (bytes) or -1 if variable */
  int nprops;                   /* number of properties for this element */
  PlyProperty **props;          /* list of properties in the file */
  char *store_prop;             /* flags: property wanted by user? */
  int other_offset;             /* offset to un-asked-for props, or -1 if none*/
  int other_size;               /* size of other_props structure */
} PlyElement;

typedef struct PlyOtherProp {   /* describes other properties in an element */
  char *name;                   /* element name */
  int size;                     /* size of other_props */
  int nprops;                   /* number of properties in other_props */
  PlyProperty **props;          /* list of properties in other_props */
} PlyOtherProp;

typedef struct OtherData { /* for storing other_props for an other element */
  void *other_props;
} OtherData;

typedef struct OtherElem {     /* data for one "other" element */
  char *elem_name;             /* names of other elements */
  int elem_count;              /* count of instances of each element */
  OtherData **other_data;      /* actual property data for the elements */
  PlyOtherProp *other_props;   /* description of the property data */
} OtherElem;

typedef struct PlyOtherElems {  /* "other" elements, not interpreted by user */
  int num_elems;                /* number of other elements */
  OtherElem *other_list;        /* list of data for other elements */
} PlyOtherElems;

typedef struct PlyFile {        /* description of PLY file */
  FILE *fp;                     /* file pointer */
  int file_type;                /* ascii or binary */
  float version;                /* version number of file */
  int nelems;                   /* number of elements of object */
  PlyElement **elems;           /* list of elements */
  int num_comments;             /* number of comments */
  char **comments;              /* list of comments */
  int num_obj_info;             /* number of items of object information */
  char **obj_info;              /* list of object info items */
  PlyElement *which_elem;       /* which element we're currently writing */
  PlyOtherElems *other_elems;   /* "other" elements from a PLY file */
} PlyFile;


/*** delcaration of routines ***/

extern PlyFile *ply_write(FILE *, int, char **, int);
extern PlyFile *ply_open_for_writing(const char *, int, char **, int, float *);
extern void ply_describe_element(PlyFile *, char *, int, int, PlyProperty *);
extern void ply_describe_property(PlyFile *, char *, PlyProperty *);
extern void ply_element_count(PlyFile *, char *, int);
extern void ply_header_complete(PlyFile *);
extern void ply_put_element_setup(PlyFile *, char *);
extern void ply_put_element(PlyFile *, void *);
extern void ply_put_comment(PlyFile *, char *);
extern void ply_put_obj_info(PlyFile *, char *);
extern PlyFile *ply_read(FILE *, int *, char ***);
extern PlyFile *ply_open_for_reading( const char *, int *, char ***, int *, float *);
extern PlyProperty **ply_get_element_description(PlyFile *, char *, int*, int*);
extern void ply_get_element_setup( PlyFile *, char *, int, PlyProperty *);
extern void ply_get_property(PlyFile *, char *, PlyProperty *);
extern PlyOtherProp *ply_get_other_properties(PlyFile *, char *, int);
extern void ply_get_element(PlyFile *, void *);
extern char **ply_get_comments(PlyFile *, int *);
extern char **ply_get_obj_info(PlyFile *, int *);
extern void ply_close(PlyFile *);
extern void ply_get_info(PlyFile *, float *, int *);
extern PlyOtherElems *ply_get_other_element (PlyFile *, char *, int);
extern void ply_describe_other_properties(
    PlyFile *plyfile, PlyOtherProp *other, int offset);
extern void ply_describe_other_elements ( PlyFile *, PlyOtherElems *);
extern void ply_put_other_elements (PlyFile *);
extern void ply_free_other_elements (PlyOtherElems *);
extern void copy_property(PlyProperty *, PlyProperty *);
extern int equal_strings(char *, char *);
extern int ply_is_valid_property(PlyFile *plyfile, 
				 char *elem_name, char *prop_name);
extern void ply_get_element_noalloc(PlyFile *plyfile, void *elem_ptr);


/* Define some functions/macros to convert numbers (as character strings) 
 * from BYTE_ORDER to BIG or LITTLE ENDIAN. */

/* Reverse order of 2 characters */

#define SWAP2C(str) { char c; \
    c = (str)[0]; (str)[0] = (str)[1]; (str)[1] = c; }

/* Reverse order of 4 characters */
#define SWAP4C(str) { char c; \
  c = (str)[0]; (str)[0] = (str)[3]; (str)[3] = c; \
  c = (str)[1]; (str)[1] = (str)[2]; (str)[2] = c; }

/* Reverse order of 8 characters */
#define SWAP8C(str) { char c; \
  c = (str)[0]; (str)[0] = (str)[7]; (str)[7] = c; \
  c = (str)[1]; (str)[1] = (str)[6]; (str)[6] = c; \
  c = (str)[2]; (str)[2] = (str)[5]; (str)[5] = c; \
  c = (str)[3]; (str)[3] = (str)[4]; (str)[4] = c; }

/* Macros that call the SWAPn function if necessary for this BYTE_ORDER */
#if BYTE_ORDER == BIG_ENDIAN
#define SWAP_TO_ENDIAN2(str,end) if ((end)==PLY_BINARY_LE) SWAP2C((char*) str)
#define SWAP_TO_ENDIAN4(str,end) if ((end)==PLY_BINARY_LE) SWAP4C((char*) str)
#define SWAP_TO_ENDIAN8(str,end) if ((end)==PLY_BINARY_LE) SWAP8C((char*) str)
#elif BYTE_ORDER == LITTLE_ENDIAN
#define SWAP_TO_ENDIAN2(str,end) if ((end)==PLY_BINARY_BE) SWAP2C((char*) str)
#define SWAP_TO_ENDIAN4(str,end) if ((end)==PLY_BINARY_BE) SWAP4C((char*) str)
#define SWAP_TO_ENDIAN8(str,end) if ((end)==PLY_BINARY_BE) SWAP8C((char*) str)
#endif

#ifdef __cplusplus 
} 
#endif 
#endif /* !__PLY_H__ */

