From levoy@blueridge  Fri Nov  6 17:30:29 1992
Received: from blueridge.stanford.edu by redclay.stanford.edu (920330.SGI/inc-1.0)
	id AA20730; Fri, 6 Nov 92 17:30:29 -0800
Received: by blueridge (920330.SGI/inc-1.0)
	id AA10004; Fri, 6 Nov 92 17:23:44 -0800
Date: Fri, 6 Nov 92 17:23:44 -0800
From: levoy@blueridge.stanford.edu (Marc Levoy)
Message-Id: <9211070123.AA10004@blueridge>
To: turk@redclay.stanford.edu
Subject: My .opc file format
Status: RO

NM = 3
PIXEL is unsigned char
Shorts and longs are VAX byte ordering!
You can use my ~levoy/libs/file.c if you don't want to swap them yourself.
To convert a .shd file into .opc:

run my den program and type:
	shd
		load
			myfile (.shd is assumed)
		sopc
			myfile (.opc is assumed)
		return
	quit

I have the comb currently in cyb.shd in /kittyhawk/usr/voldata/cyb/test
I have various versions of the blocks from splatted mesh points in blocks*.shd
in /kittyhawk/usr/voldata/cyb/custom

If you want to read my .den files, which don't require a shading step,
I also enclose the format of those files.


/********** opacity - 3-d opacity map **********/

/* Copyright 1990 by Marc Levoy - all rights reserved */

/* The following declarations show the layout of the .opc file.              */
/* If changed, the version number must be incremented and code               */
/* written to handle loading of both old and current versions.               */

				/* Version for new .opc files:               */
#define	OPC_CUR_VERSION	1	/*   Initial release                         */
short opc_version;		/* Version of this .opc file                 */

short opc_len[NM];		/* Size of this opacity map                  */

long opc_length;		/* Total number of opacities in map          */
				/*   (= product of lens)                     */
PIXEL *opc_address;		/* Pointer to opacity map                    */

/* End of layout of .opc file.                                               */

				/* Subscripted access to opacity map         */
				/*   (1 char at each IX,IY,IZ)               */
#define OPC_ADDRESS(IZ,IY,IX)	(opc_address+\
				 ((IZ)*opc_len[Y]+(IY))*opc_len[X]+(IX))
#define OPC(IZ,IY,IX)		(*OPC_ADDRESS(IZ,IY,IX))

short out_opc_len[NM];		/* Size of output opacity map                */
long out_opc_length;		/* Total number of opacities in map          */
PIXEL *out_opc_address;		/* Pointer to map                            */

				/* Subscripted access to output map          */
#define OUT_OPC_ADDRESS(IZ,IY,IX)	\
				(out_opc_address+\
				 ((IZ)*out_opc_len[Y]+\
				  (IY))*out_opc_len[X]+\
				 (IX))
#define OUT_OPC(IZ,IY,IX)	(*OUT_OPC_ADDRESS(IZ,IY,IX))

short acc_opc_len[NM];		/* Size of 3-d opacity accumulator           */
long acc_opc_length;		/* Total number of opacities                 */
WPIXELSUM *acc_opc_address;	/* Pointer to accumulator                    */

				/* Subscripted access to accumulator         */
#define ACC_OPC_ADDRESS(IZ,IY,IX)	\
				(acc_opc_address+\
				 ((IZ)*acc_opc_len[Y]+\
				  (IY))*acc_opc_len[X]+\
				 (IX))
#define ACC_OPC(IZ,IY,IX)	(*ACC_OPC_ADDRESS(IZ,IY,IX))

BOOLEAN opc_histogram_exists;			/* True if histogram exists  */
long opacity_histogram[MAX_PIXEL+1];		/* Histogram of opacity map  */

short light_len[NM];		/* Size of light strength map                */
long light_length;		/* Total number of strengths in map          */
PIXEL *light_address;		/* Pointer to map                            */

				/* Subscripted access to light strength map  */
#define LIGHT_ADDRESS(IZ,IY,IX)	\
				(light_address+\
				 ((IZ)*light_len[Y]+\
				  (IY))*light_len[X]+\
				 (IX))
#define LIGHT(IZ,IY,IX)		(*LIGHT_ADDRESS(IZ,IY,IX))



-------------------------------------------------------




Ignore orig and extr lens, just look at map_len[NM].
DENSITY is unsigned char.


/********** map - 3-d density map **********/

/* Copyright 1990 by Marc Levoy - all rights reserved */

/* The following declarations show the layout of the .den file.              */
/* If changed, the version number must be incremented and code               */
/* written to handle loading of both old and current versions.               */

				/* Version for new .den files:               */
#define	MAP_CUR_VERSION	1	/*   Initial release                         */
short map_version;		/* Version of this .den file                 */

short orig_min[NM],		/* Dimensions of original data file          */
      orig_max[NM],		/*   (CT:  from <file>.header file           */
      orig_len[NM];		/*    ED:  from <file>.mi file)              */

short extr_min[NM],		/* Portion of file extracted for this map    */
      extr_max[NM],		/*   (mins and maxes will be subset of       */
      extr_len[NM];		/*    orig and lengths will be <= orig)      */

short map_min[NM],		/* Dimensions of this map                    */
      map_max[NM],		/*   (mins will be 0 in this program and     */
      map_len[NM];		/*    lens may be != extr if warps > 0)      */

short map_warps;		/* Number of warps since extraction          */
				/*   (0 = none)                              */

long map_length;		/* Total number of densities in map          */
				/*   (= product of lens)                     */
DENSITY *map_address;		/* Pointer to map                            */

/* End of layout of .den file.                                               */

				/* Slow subscripted access to map            */
				/*   (assumes nothing about mins or sizeof)  */
#define SLOW_MAP_ADDRESS(IZ,IY,IX)	\
				(map_address+\
				 ((((IZ)-map_min[Z])*map_len[Y]+\
				   ((IY)-map_min[Y]))*map_len[X]+\
				  ((IX)-map_min[X]))*\
				 sizeof(DENSITY))
#define SLOW_MAP(IZ,IY,IX)	(*SLOW_MAP_ADDRESS(IZ,IY,IX))

				/* Fast subscripted access to map            */
				/*   (assumes mins = 0 and chars)            */
#define MAP_ADDRESS(IZ,IY,IX)	(map_address+\
				 ((IZ)*map_len[Y]+(IY))*map_len[X]+(IX))
#define MAP(IZ,IY,IX)		(*MAP_ADDRESS(IZ,IY,IX))

short out_map_len[NM];		/* Size of output density map                */
long out_map_length;		/* Total number of densities in map          */
				/*   (= product of lens)                     */
DENSITY *out_map_address;	/* Pointer to map                            */

				/* Fast subscripted access to output map     */
#define OUT_MAP_ADDRESS(IZ,IY,IX)	\
				(out_map_address+\
				 ((IZ)*out_map_len[Y]+\
				  (IY))*out_map_len[X]+\
				 (IX))
#define OUT_MAP(IZ,IY,IX)	(*OUT_MAP_ADDRESS(IZ,IY,IX))

short acc_map_len[NM];		/* Size of density accumulator               */
long acc_map_length;		/* Total number of densities                 */
WDENSITYSUM *acc_map_address;	/* Pointer to accumulator                    */

				/* Fast subscripted access to accumulator    */
#define ACC_MAP_ADDRESS(IZ,IY,IX)	\
				(acc_map_address+\
				 ((IZ)*acc_map_len[Y]+\
				  (IY))*acc_map_len[X]+\
				 (IX))
#define ACC_MAP(IZ,IY,IX)	(*ACC_MAP_ADDRESS(IZ,IY,IX))

				/* Cyberware point splatting code            */
char splat_code;		/* One of {'A':add}                          */

BOOLEAN map_histogram_exists;			/* True if histogram exists  */
long density_histogram[MAX_DENSITY+1];		/* Histogram of density map  */

