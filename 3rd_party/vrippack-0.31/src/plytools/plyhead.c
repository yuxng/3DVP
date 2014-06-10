#include <stdio.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>

#ifdef linux
#include <string.h>
#endif

#define MAX_HEADER_LENGTH 10000

void usage(char *progname);


int
main(int argc, char **argv)
{
    int nelems;
    char **elist;
    int file_type;
    float version;
    char header[MAX_HEADER_LENGTH];
    PlyFile *ply;
    char *end;
    char *progname;
    char *inName = NULL;
    FILE *inFile = stdin;

    progname = argv[0];
    argc--; 
    argv++;

    /* Print usage? */
    if (argc > 0 && !strcmp(argv[0], "-h")) {
      usage(progname);
      exit(-1);
    }

   /* optional input file (if not, read stdin ) */
   if (argc > 0 && *argv[0] != '-') {
       inName = argv[0];
       inFile = fopen(inName, "r");
       if (inFile == NULL) {
           fprintf(stderr, "Error: Couldn't open input file %s\n", inName);
           usage(progname);
	   exit(-1);
       }
       argc --;
       argv ++;
   } 

   /* Check no extra args */
   if (argc > 0) {
     fprintf(stderr, "Error: Unhandled arg: %s\n", argv[0]);
     usage(progname);
     exit(-1);
   }
   
   /* If it's a file, open it twice (first time checks it's a valid
    * ply file).  If it's on stdin, just assume valid ply file, and
    * get the header in a single pass.
    */
   if (inFile != stdin) {
     ply = ply_read(inFile, &nelems, &elist);

     if (!ply) {
       fprintf(stderr, "Not a Ply file.\n");
       exit(1);
     }

     ply_close(ply);
    
     inFile = fopen(inName, "r");
   }

    fread(header, 1, MAX_HEADER_LENGTH, inFile);
    header[MAX_HEADER_LENGTH-1] = 0;

    end = strstr(header, "end_header");

    if (end == NULL) {
	fprintf(stderr, "Did not find end of header within the first ");
	fprintf(stderr, "%d bytes of the file.\n",  MAX_HEADER_LENGTH);
	exit(1);
    }

    *(end+strlen("end_header")+1) = 0;

    printf("\n%s\n", header);
}


void
usage(char *progname)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [ply-file]\n", progname);
    fprintf(stderr, "   or: %s < ply-file\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s prints the header information from a Ply file.\n\n",
	    progname);
    fprintf(stderr, "Note:  When reading on stdin, it makes the assumption\n");
    fprintf(stderr, "that it is a valid ply file.  If you're not sure,\n");
    fprintf(stderr, "pass the filename as an argument.\n");
	    
    exit(-1);
}

