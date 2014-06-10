/*

Brian Curless

Computer Graphics Laboratory
Stanford University

---------------------------------------------------------------------

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include <tk.h>
#include <stdlib.h>
#ifdef LINUX
#include <string.h>
#endif

#include "vripInit.h"

void printNotice();

bool g_bNoUI = false;

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char **argv)
{
   if (argc < 2) {
      printNotice();
   }

   if (!strcmp (argv[argc-1], "-noui")) {
     g_bNoUI = true;
     --argc;
   }

   if (g_bNoUI)
     Tcl_Main(argc, argv, Tcl_AppInit);
   else
     Tk_Main(argc, argv, Tcl_AppInit);

   return 0;			/* Needed only to prevent compiler warning. */
}



/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(Tcl_Interp *interp)
{
    Tk_Window main;

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

    if (g_bNoUI) {
      Tcl_SetVar (interp, "noui", "1", TCL_GLOBAL_ONLY);
    } else {
      Tcl_SetVar (interp, "noui", "0", TCL_GLOBAL_ONLY);
      if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
      }
      Tcl_StaticPackage(interp, "Tk", Tk_Init, (Tcl_PackageInitProc *) NULL);
    }

    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    if (Vrip_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }


    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    /* Tcl_SetVar(interp, "tcl_rcFileName", "~/.vriprc", TCL_GLOBAL_ONLY);*/

    return TCL_OK;
}


void
printNotice()
{
   printf("\n");
   printf("VRIP - Volumetric Range Image Processor\n");
   printf("\n");
   printf("Version 0.31 - November 25, 2006\n");
   printf("\n");
   printf("Copyright 2006 - University of Washington and Stanford University\n");
   printf("\n");
   printf("Type 'vrip_copyright' for copyright details.\n");
   printf("\n");
}

