/********** shading - 3-d shading map **********/

/* Copyright 1990 by Marc Levoy - all rights reserved */

/* The following declarations show the layout of the .shd file.              */
/* If changed, the version number must be incremented and code               */
/* written to handle loading of both old and current versions.               */

				/* Version for new .shd files:               */
#define	SHD_CUR_VERSION	1	/*   Initial release                         */
short shd_version;		/* Version of this .shd file                 */

short shd_len[NM];		/* Size of this shading map                  */

long shd_length;		/* Total number of shades & opacities in map */
				/*   (= (ICSIZE+1) * product of lens)        */
PIXEL *shd_address;		/* Pointer to shading map                    */

/* End of layout of .shd file.                                               */

				/* Subscripted access to shading map         */
				/*   (ICSIZE+1 chars at each IX,IY,IZ)       */
#define SHD_ADDRESS(IZ,IY,IX,C)	(shd_address+\
				 (((IZ)*shd_len[Y]+(IY))*shd_len[X]+(IX))*\
				 (ICSIZE+1)+(C))
#define SHD(IZ,IY,IX,C)		(*SHD_ADDRESS(IZ,IY,IX,C))

short out_shd_len[NM];		/* Size of output shading map                */
long out_shd_length;		/* Total number of shades & opacities in map */
PIXEL *out_shd_address;		/* Pointer to map                            */

				/* Subscripted access to output map          */
#define OUT_SHD_ADDRESS(IZ,IY,IX,C)	\
				(out_shd_address+\
				 (((IZ)*out_shd_len[Y]+\
				   (IY))*out_shd_len[X]+\
				  (IX))*(ICSIZE+1)+(C))
#define OUT_SHD(IZ,IY,IX,C)	(*OUT_SHD_ADDRESS(IZ,IY,IX,C))

short acc_shd_len[NM];		/* Size of 3-d shading accumulator           */
long acc_shd_length;		/* Total number of shades & opacities        */
WPIXELSUM *acc_shd_address;	/* Pointer to accumulator                    */

				/* Subscripted access to accumulator         */
#define ACC_SHD_ADDRESS(IZ,IY,IX,C)	\
				(acc_shd_address+\
				 (((IZ)*acc_shd_len[Y]+\
				   (IY))*acc_shd_len[X]+\
				  (IX))*(ICSIZE+1)+(C))
#define ACC_SHD(IZ,IY,IX,C)	(*ACC_SHD_ADDRESS(IZ,IY,IX,C))

BOOLEAN shd_histogram_exists;			/* True if histogram exists  */
long shading_color_histogram[MAX_PIXEL+1];	/* Histogram of shading map  */
long shading_opacity_histogram[MAX_PIXEL+1];	/*                           */
