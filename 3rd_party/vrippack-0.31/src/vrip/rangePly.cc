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


#include <stdio.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>

#include "rangePly.h"
#include "vripGlobals.h"
#include "defines.h"
#include "plyio.h"

#ifndef RAD
#define RAD(x) ((x)*M_PI/180)
#endif

#ifdef linux
#include <float.h>
#endif

struct PlyVertex {
  float x,y,z;
  float confidence;
  float intensity;
  unsigned char red,grn,blu;
};

PlyProperty vert_std_props[] = {
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,z), 0, 0, 0, 0},
  {"confidence", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,confidence), 0, 0, 0,0},
  {"intensity", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,intensity), 0, 0, 0,0},
  {"diffuse_red", PLY_UCHAR, PLY_UCHAR, offsetof(PlyVertex,red), 0, 0, 0, 0},
  {"diffuse_green", PLY_UCHAR, PLY_UCHAR, offsetof(PlyVertex,grn), 0, 0, 0, 0},
  {"diffuse_blue", PLY_UCHAR, PLY_UCHAR, offsetof(PlyVertex,blu), 0, 0, 0, 0},
  {"std_dev", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,confidence), 0, 0, 0,0},
};


struct RangePnt {
  unsigned char num_pts;
  int *pts;
};

/* list of property information for a range data point */
PlyProperty range_props[] = {
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(RangePnt,pts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(RangePnt,num_pts)},
};




static float RANGE_DATA_SIGMA_FACTOR = 4;
static float RANGE_DATA_MIN_INTENSITY = 0.05;



static void trimMesh(Mesh *mesh);
static void addTriangleUseNormal(Mesh *mesh, int vin1, int vin2, int vin3, 
				 float minDot, Vec3f &viewDir, int connectAll);
static void addTriangleUseLength(Mesh *mesh, int vin1, int vin2, int vin3, 
				 float maxLength, int connectAll);


RangeGrid::~RangeGrid()
{
    delete [] coords;

    delete [] indices;

    if (num_obj_info > 0) {
	for (int i = 0; i < num_obj_info; i++)
	    delete [] obj_info[i];
	delete [] obj_info;
    }

    if (hasConfidence)
	delete [] confidence;

    if (hasColor)
	delete [] matDiff;

    if (hasIntensity)
	delete [] intensity;

}


void
set_range_data_sigma_factor(float factor)
{
    RANGE_DATA_SIGMA_FACTOR = factor;
}


float
get_range_data_sigma_factor()
{
    return RANGE_DATA_SIGMA_FACTOR;
}



void
set_range_data_min_intensity(float intensity)
{
    RANGE_DATA_MIN_INTENSITY = intensity;
}


float
get_range_data_min_intensity()
{
    return RANGE_DATA_MIN_INTENSITY;
}



int 
is_range_grid_file(const char *filename)
{
  int i;
  PlyFile *ply;
  int nelems;
  char **elist;
  int file_type;
  float version;

  ply = ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);
  if (ply == NULL)
    return 0;
  ply_close(ply);
  
  for (i = 0; i < nelems; i++) {
      if (!strcmp(elist[i], "range_grid"))
	  return 1;
  }

  return 0;
}


/******************************************************************************
Read range data from a PLY file.

Entry:
  name - name of PLY file to read from

Exit:
  returns pointer to data, or NULL if it couldn't read from file
******************************************************************************/

RangeGrid *
readRangeGrid(const char *name)
{
  int i,j,k,index, best_index;
  PlyFile *ply;
  RangeGrid *rangeGrid = NULL;
  char **obj_info;
  int num_obj_info;
  int num_elems;
  int nprops;
  int nelems;
  char **elist;
  int file_type;
  float version;
  PlyProperty **plist;
  int num_rows,num_cols;
  RangePnt range_pnt;
  PlyVertex vert;
  char *elem_name;
  int get_std_dev = 0;
  int get_confidence = 0;
  int get_intensity = 0;
  int get_color = 0;
  int has_red = 0;
  int has_green = 0;
  int has_blue = 0;
  int is_warped = 0;
  char temp[PATH_MAX];
  int isRightMirrorOpen = 1;
  int found_mirror = 0;
  float conf,std;
  float min_std,max_std,max;
  float avg_std = 0;
  float lgincr;

  if (name == NULL) {
     ply = ply_read (stdin, &nelems, &elist);
  } else {
     ply = ply_open_for_reading(name, &nelems, &elist, &file_type, &version);
  }

  if (ply == NULL) {
    return (NULL);
  }

  int isRangeGrid = 0;
  for (i = 0; i < nelems; i++) {
     if (!strcmp(elist[i], "range_grid")) {
	isRangeGrid = 1;
	break;
     }
  }

  if (!isRangeGrid)
     return NULL;

  /* parse the obj_info */

  obj_info = ply_get_obj_info (ply, &num_obj_info);
  for (i = 0; i < num_obj_info; i++) {
    if (strstr(obj_info[i], "num_cols"))
	sscanf(obj_info[i], "%s%d", temp, &num_cols);
    if (strstr(obj_info[i], "num_rows"))
	sscanf(obj_info[i], "%s%d", temp, &num_rows);
    if (strstr(obj_info[i], "is_warped"))
	sscanf(obj_info[i], "%s%d", temp, &is_warped);
    if (strstr(obj_info[i], "optimum_std_dev"))
	sscanf(obj_info[i], "%s%f", temp, &avg_std);
    if (strstr(obj_info[i], "echo_lgincr"))
	sscanf(obj_info[i], "%s%f", temp, &lgincr);
    if (strstr(obj_info[i], "is_right_mirror_open")) {
	found_mirror = TRUE;
	sscanf(obj_info[i], "%s%d", temp, &isRightMirrorOpen);
    }
  }

  EdgeLength = lgincr;

  if (is_warped && !found_mirror && Warn)
      printf("Couldn't tell which mirror was open.  Right mirror assumed.\n");

  min_std = avg_std / RANGE_DATA_SIGMA_FACTOR;
  max_std = avg_std * RANGE_DATA_SIGMA_FACTOR;

  /* set up the range data structure */
  rangeGrid = new RangeGrid;
  rangeGrid->nlg = num_rows;
  rangeGrid->nlt = num_cols;
  rangeGrid->intensity = NULL;
  rangeGrid->confidence = NULL;
  rangeGrid->matDiff = NULL;
  rangeGrid->hasColor = 0;
  rangeGrid->hasIntensity = 0;
  rangeGrid->hasConfidence = 0;
  rangeGrid->multConfidence = 0;

  if (!is_warped) {
      rangeGrid->viewDir.setValue(0, 0, -1);
  } else {
      if (isRightMirrorOpen) {
	  rangeGrid->viewDir.setValue(-sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      } else {
	  rangeGrid->viewDir.setValue(sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      }
  }

  rangeGrid->num_obj_info = num_obj_info;
  rangeGrid->obj_info = new char*[num_obj_info];
  for (i = 0; i < num_obj_info; i++) {
      rangeGrid->obj_info[i] = new char[strlen(obj_info[i])+1];
      strcpy(rangeGrid->obj_info[i], obj_info[i]);
  }


  rangeGrid->isWarped = is_warped;
  rangeGrid->isRightMirrorOpen = isRightMirrorOpen;

  /* see if we've got both vertex and range_grid data */

  plist = ply_get_element_description (ply, "vertex", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain vertex data\n");
    return (NULL);
  }
  rangeGrid->numOrigSamples = num_elems;
  rangeGrid->numSamples = num_elems;
  plist = ply_get_element_description (ply, "range_grid", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain range_grid data\n");
    return (NULL);
  }

  /* read in the range data */

  for (i = 0; i < nelems; i++) {

    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* see if the file contains intensities */
      for (j = 0; j < nprops; j++) {
        if (equal_strings ("std_dev", plist[j]->name))
          get_std_dev = 1;
        if (equal_strings ("confidence", plist[j]->name))
          get_confidence = 1;
        if (equal_strings ("intensity", plist[j]->name))
          get_intensity = 1;
        if (equal_strings ("diffuse_red", plist[j]->name))
          has_red = 1;
        if (equal_strings ("diffuse_green", plist[j]->name))
          has_green = 1;
        if (equal_strings ("diffuse_blue", plist[j]->name))
          has_blue = 1;
      }

      if (has_red && has_green && has_blue) {
        get_color = 1;
        rangeGrid->hasColor = 1;
      }

      if (get_intensity)
        rangeGrid->hasIntensity = 1;

      if (get_std_dev && is_warped) {
        rangeGrid->hasConfidence = 1;
        rangeGrid->multConfidence = 1;
      }
      else if (get_confidence)
        rangeGrid->hasConfidence = 1;

      ply_get_property (ply, "vertex", &vert_std_props[0]);
      ply_get_property (ply, "vertex", &vert_std_props[1]);
      ply_get_property (ply, "vertex", &vert_std_props[2]);
      if (get_confidence)     
        ply_get_property (ply, "vertex", &vert_std_props[3]);
      if (get_intensity)             
        ply_get_property (ply, "vertex", &vert_std_props[4]);
      if (get_color) {    
        ply_get_property (ply, "vertex", &vert_std_props[5]);
        ply_get_property (ply, "vertex", &vert_std_props[6]);
        ply_get_property (ply, "vertex", &vert_std_props[7]);
      }
      if (get_std_dev)
        ply_get_property (ply, "vertex", &vert_std_props[8]);

      rangeGrid->coords = new Vec3f[rangeGrid->numSamples];

      if (get_confidence || get_std_dev)
	  rangeGrid->confidence = new float[rangeGrid->numSamples];

      if (get_intensity)
	  rangeGrid->intensity = new float[rangeGrid->numSamples];

      if (get_color)
	  rangeGrid->matDiff = new vec3uc[rangeGrid->numSamples];

      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &vert);
        rangeGrid->coords[j].x = vert.x;
        rangeGrid->coords[j].y = vert.y;
        rangeGrid->coords[j].z = vert.z;

        if (get_intensity) {
          rangeGrid->intensity[j] = vert.intensity;
	}

        if (get_std_dev&&is_warped) {

          std = vert.confidence;

          if (std < min_std)
            conf = 0;
          else if (std < avg_std)
            conf = (std - min_std) / (avg_std - min_std);
          else if (std > max_std)
            conf = 0;
          else
	      conf = (max_std - std) / (max_std - avg_std);

/*
       Unsafe to use vertex intensity, as aperture settings may change
       between scans.  Instead, use std_dev confidence * orientation.

	  conf *= vert.intensity;
*/	    

	  if (get_intensity)
	      if (vert.intensity < RANGE_DATA_MIN_INTENSITY) conf = 0.0;

          rangeGrid->confidence[j] = conf;
        }
        else if (get_confidence) {
          rangeGrid->confidence[j] = vert.confidence;
        }

        if (get_color) {
          rangeGrid->matDiff[j][0] = vert.red;
          rangeGrid->matDiff[j][1] = vert.grn;
          rangeGrid->matDiff[j][2] = vert.blu;
        }
      }
    }

    if (equal_strings ("range_grid", elem_name)) {
      rangeGrid->indices = new int[rangeGrid->nlt * rangeGrid->nlg];
      ply_get_element_setup (ply, elem_name, 1, range_props);
      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &range_pnt);
        if (range_pnt.num_pts == 0)
          rangeGrid->indices[j] = -1;
        else {
	    max = -FLT_MAX;
	    for (k = 0; k < range_pnt.num_pts; k++) {
		index = range_pnt.pts[k];
		// There will only be more than one point per sample
		// if there are intensities
		if (get_intensity) {
		    if (rangeGrid->intensity[index] > max) {
			max = rangeGrid->intensity[index];
			best_index = index;
		    }
		}
		else {
		    best_index = index;
		}
	    }
	    index = best_index;
	    if (get_confidence || get_std_dev) {
		if (rangeGrid->confidence[index] > 0.0)  
		    rangeGrid->indices[j] = index;
		else
		    rangeGrid->indices[j] = -1;
	    } else {
		rangeGrid->indices[j] = index;
	    }

	    if (get_intensity) {
		if (rangeGrid->intensity[index] < RANGE_DATA_MIN_INTENSITY) {
		    rangeGrid->indices[j] = -1;
		}
	    }

	    free (range_pnt.pts);
	}
      }
    }
  }

  ply_close (ply);

  return (rangeGrid);
}


/******************************************************************************
Read range data from a PLY file.

Entry:
  name - name of PLY file to read from

Exit:
  returns pointer to data, or NULL if it couldn't read from file
******************************************************************************/

RangeGrid *
readRangeGridFillGaps(const char *name)
{
  int i,j,k,index, best_index;
  PlyFile *ply;
  RangeGrid *rangeGrid = NULL;
  char **obj_info;
  int num_obj_info;
  int num_elems;
  int nprops;
  int nelems;
  char **elist;
  int file_type;
  float version;
  PlyProperty **plist;
  int num_rows,num_cols;
  RangePnt range_pnt;
  PlyVertex vert;
  char *elem_name;
  int get_std_dev = 0;
  int get_confidence = 0;
  int get_intensity = 0;
  int get_color = 0;
  int has_red = 0;
  int has_green = 0;
  int has_blue = 0;
  int is_warped = 0;
  char temp[PATH_MAX];
  int isRightMirrorOpen = 1;
  int found_mirror = 0;
  float conf,std;
  float min_std,max_std,max;
  float avg_std = 0;
  float angle, alongSensor1, alongSensor2, x;
  float lgincr;

  ply = ply_open_for_reading(name, &nelems, &elist, &file_type, &version);
  if (ply == NULL)
    return (NULL);

  /* parse the obj_info */

  obj_info = ply_get_obj_info (ply, &num_obj_info);
  for (i = 0; i < num_obj_info; i++) {
    if (strstr(obj_info[i], "num_cols"))
	sscanf(obj_info[i], "%s%d", temp, &num_cols);
    if (strstr(obj_info[i], "num_rows"))
	sscanf(obj_info[i], "%s%d", temp, &num_rows);
    if (strstr(obj_info[i], "is_warped"))
	sscanf(obj_info[i], "%s%d", temp, &is_warped);
    if (strstr(obj_info[i], "optimum_std_dev"))
	sscanf(obj_info[i], "%s%f", temp, &avg_std);
    if (strstr(obj_info[i], "echo_lgincr"))
	sscanf(obj_info[i], "%s%f", temp, &lgincr);
    if (strstr(obj_info[i], "is_right_mirror_open")) {
	found_mirror = TRUE;
	sscanf(obj_info[i], "%s%d", temp, &isRightMirrorOpen);
    }
  }

  EdgeLength = lgincr;

  if (is_warped && !found_mirror && Warn)
      printf("Couldn't tell which mirror was open.  Right mirror assumed.\n");

  min_std = avg_std / RANGE_DATA_SIGMA_FACTOR;
  max_std = avg_std * RANGE_DATA_SIGMA_FACTOR;

  /* set up the range data structure */
  rangeGrid = new RangeGrid;
  rangeGrid->nlg = num_rows;
  rangeGrid->nlt = num_cols;
  rangeGrid->intensity = NULL;
  rangeGrid->confidence = NULL;
  rangeGrid->matDiff = NULL;
  rangeGrid->hasColor = 0;
  rangeGrid->hasIntensity = 0;
  rangeGrid->hasConfidence = 0;
  rangeGrid->multConfidence = 0;

  if (!is_warped) {
      rangeGrid->viewDir.setValue(0, 0, -1);
  } else {
      if (isRightMirrorOpen) {
	  rangeGrid->viewDir.setValue(-sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      } else {
	  rangeGrid->viewDir.setValue(sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      }
  }

  rangeGrid->num_obj_info = num_obj_info;
  rangeGrid->obj_info = new char*[num_obj_info];
  for (i = 0; i < num_obj_info; i++) {
      rangeGrid->obj_info[i] = new char[strlen(obj_info[i])+1];
      strcpy(rangeGrid->obj_info[i], obj_info[i]);
  }


  rangeGrid->isWarped = is_warped;
  rangeGrid->isRightMirrorOpen = isRightMirrorOpen;

  /* see if we've got both vertex and range_grid data */

  plist = ply_get_element_description (ply, "vertex", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain vertex data\n");
    return (NULL);
  }
  rangeGrid->numOrigSamples = num_elems;
  rangeGrid->numSamples = num_elems;
  rangeGrid->maxSamples = rangeGrid->nlt * rangeGrid->nlg;
  plist = ply_get_element_description (ply, "range_grid", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain range_grid data\n");
    return (NULL);
  }

  /* read in the range data */

  for (i = 0; i < nelems; i++) {

    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* see if the file contains intensities */
      for (j = 0; j < nprops; j++) {
        if (equal_strings ("std_dev", plist[j]->name))
          get_std_dev = 1;
        if (equal_strings ("confidence", plist[j]->name))
          get_confidence = 1;
        if (equal_strings ("intensity", plist[j]->name))
          get_intensity = 1;
        if (equal_strings ("diffuse_red", plist[j]->name))
          has_red = 1;
        if (equal_strings ("diffuse_green", plist[j]->name))
          has_green = 1;
        if (equal_strings ("diffuse_blue", plist[j]->name))
          has_blue = 1;
      }

      if (has_red && has_green && has_blue) {
        get_color = 1;
        rangeGrid->hasColor = 1;
      }

      if (get_intensity)
        rangeGrid->hasIntensity = 1;

      if (get_std_dev && is_warped) {
        rangeGrid->hasConfidence = 1;
        rangeGrid->multConfidence = 1;
      }
      else if (get_confidence)
        rangeGrid->hasConfidence = 1;

      ply_get_property (ply, "vertex", &vert_std_props[0]);
      ply_get_property (ply, "vertex", &vert_std_props[1]);
      ply_get_property (ply, "vertex", &vert_std_props[2]);
      if (get_confidence)     
        ply_get_property (ply, "vertex", &vert_std_props[3]);
      if (get_intensity)             
        ply_get_property (ply, "vertex", &vert_std_props[4]);
      if (get_color) {    
        ply_get_property (ply, "vertex", &vert_std_props[5]);
        ply_get_property (ply, "vertex", &vert_std_props[6]);
        ply_get_property (ply, "vertex", &vert_std_props[7]);
      }
      if (get_std_dev)
        ply_get_property (ply, "vertex", &vert_std_props[8]);

      rangeGrid->coords = new Vec3f[rangeGrid->maxSamples];

      if (get_confidence || get_std_dev)
	  rangeGrid->confidence = new float[rangeGrid->maxSamples];

      if (get_intensity)
	  rangeGrid->intensity = new float[rangeGrid->maxSamples];

      if (get_color)
	  rangeGrid->matDiff = new vec3uc[rangeGrid->maxSamples];

      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &vert);
        rangeGrid->coords[j].x = vert.x;
        rangeGrid->coords[j].y = vert.y;
        rangeGrid->coords[j].z = vert.z;

        if (get_intensity) {
          rangeGrid->intensity[j] = vert.intensity;
	}

        if (get_std_dev&&is_warped) {

          std = vert.confidence;

          if (std < min_std)
            conf = 0;
          else if (std < avg_std)
            conf = (std - min_std) / (avg_std - min_std);
          else if (std > max_std)
            conf = 0;
          else
	      conf = (max_std - std) / (max_std - avg_std);

/*
       Unsafe to use vertex intensity, as aperture settings may change
       between scans.  Instead, use std_dev confidence * orientation.

	  conf *= vert.intensity;
*/	    

	  if (get_intensity)
	      if (vert.intensity < RANGE_DATA_MIN_INTENSITY) conf = 0.0;

          rangeGrid->confidence[j] = conf;
        }
        else if (get_confidence) {
          rangeGrid->confidence[j] = vert.confidence;
        }

        if (get_color) {
          rangeGrid->matDiff[j][0] = vert.red;
          rangeGrid->matDiff[j][1] = vert.grn;
          rangeGrid->matDiff[j][2] = vert.blu;
        }
      }
    }

    if (equal_strings ("range_grid", elem_name)) {
      rangeGrid->indices = new int[rangeGrid->nlt * rangeGrid->nlg];
      if (rangeGrid->indices == NULL) {
	  fprintf (stderr, "could not allocate space\n");
	  return (NULL);
      }

      ply_get_element_setup (ply, elem_name, 1, range_props);
      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &range_pnt);
        if (range_pnt.num_pts == 0)
          rangeGrid->indices[j] = -1;
        else {
	    max = -FLT_MAX;
	    for (k = 0; k < range_pnt.num_pts; k++) {
		index = range_pnt.pts[k];
		// There will only be more than one point per sample
		// if there are intensities
		if (get_intensity) {
		    if (rangeGrid->intensity[index] > max) {
			max = rangeGrid->intensity[index];
			best_index = index;
		    }
		}
		else {
		    best_index = index;
		}
	    }
	    index = best_index;
	    if (get_confidence || get_std_dev) {
		if (rangeGrid->confidence[index] > 0.0)  
		    rangeGrid->indices[j] = index;
		else
		    rangeGrid->indices[j] = -1;
	    } else {
		rangeGrid->indices[j] = index;
	    }

	    if (get_intensity) {
		if (rangeGrid->intensity[index] < RANGE_DATA_MIN_INTENSITY) {
		    rangeGrid->indices[j] = -1;
		}
	    }

	    free (range_pnt.pts);
	}
      }
    }
  }

  Vec3f vlast, v, dv, vnew;
  float z;
  int xx, yy, in, lastOut, lastIn, prevIndex;
  int xlim, ylim, doFill, oldIndex;

  // The actual filling stuff begins here.
  // Bugs: 1. I don't account for interlace when interpolating in the
  //   in the y-direction
  //    2. I fill by direct connection without regard to 
  //    maximal slopes
  //    3. I check which sample is closest by z-comparison - should use
  //       the laser perspective

  // Mark the regions that are off the edge of the mesh as
  // "unfillable"

#if 1

  // X-pass
  for (yy = 0; yy < rangeGrid->nlg; yy++) {

      xx = 0;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  xx++;
	  if (xx == rangeGrid->nlt)
	      break;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      xlim = xx;
      for (xx = 0; xx < xlim; xx++) {
	  rangeGrid->indices[xx+yy*rangeGrid->nlt] = INT_MIN;
      }

      if (xlim == rangeGrid->nlt)
	  continue;

      xx = rangeGrid->nlt-1;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  xx--;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      xlim = xx+1;
      for (xx = xlim; xx < rangeGrid->nlt; xx++) {
	  rangeGrid->indices[xx+yy*rangeGrid->nlt] = INT_MIN;
      }
  }


  // Y-pass
  for (xx = 0; xx < rangeGrid->nlt; xx++) {

      yy = 0;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  yy++;
	  if (yy == rangeGrid->nlg)
	      break;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      ylim = yy;
      for (yy = 0; yy < ylim; yy++) {
	  rangeGrid->indices[xx+yy*rangeGrid->nlt] = INT_MIN;
      }

      if (ylim == rangeGrid->nlg)
	  continue;

      yy = rangeGrid->nlg-1;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  yy--;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      ylim = yy + 1;
      for (yy = ylim; yy < rangeGrid->nlg; yy++) {
	  rangeGrid->indices[xx+yy*rangeGrid->nlt] = INT_MIN;
      }
  }
#endif

  // Fill the gaps in the x-direction

#if 1
  for (yy = 0; yy < rangeGrid->nlg; yy++) {

      // Fill in gaps with simple linear interpolation
      // between depths of adjacent valid pixels
	
      in = FALSE;
      lastOut = -1;
      
      for (xx = 0; xx < rangeGrid->nlt; xx++) {
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
	  if (index >= 0 && !in) {
	      
	      lastIn = xx;
	      in = TRUE;
	      
	      if (lastOut == -1)
		  continue;

	      // Look for gaps that include boundaries that
	      // shouldn't be filled
	      doFill = TRUE;

	      for (i = lastOut+1; i <lastIn; i++) {
		  oldIndex = rangeGrid->indices[i+yy*rangeGrid->nlt];
		  if (oldIndex == INT_MIN) {
		      doFill = FALSE;
		      break;
		  }
	      }

	      if (!doFill) {
		  continue;
	      }

	      v.setValue(rangeGrid->coords[index]);
	      dv = v;
	      dv -= vlast;
	      dv /= (lastIn - lastOut);
	      vnew = vlast;
	      vnew += dv;
	      for (i = lastOut+1; i <lastIn; i++, vnew+=dv) {
		  rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);
		  
		  if (rangeGrid->hasConfidence)
		      rangeGrid->confidence[rangeGrid->numSamples] = 0;

		  // Fill with a negative number so that it
		  // can be recognized later as a hole fill
		  rangeGrid->indices[i+yy*rangeGrid->nlt] = 
		      -rangeGrid->numSamples;
		  rangeGrid->numSamples++;
	      }
	  } 
	  else if (index < 0 && in) {		
	      lastOut = xx-1;
	      prevIndex = rangeGrid->indices[lastOut+yy*rangeGrid->nlt];
	      vlast.setValue(rangeGrid->coords[prevIndex]);
	      in = FALSE;
	  }
      }
  }
#endif

  if (is_warped && isRightMirrorOpen) {
      angle = 30;
  } else if (is_warped && !isRightMirrorOpen) {
      angle = -30;
  } else {
      angle = 0;
  }

  Vec3f dir(sin(RAD(angle)), 0, cos(RAD(angle)));

#if 1

  // Fill the gaps in the y-direction

  for (xx = 0; xx < rangeGrid->nlt; xx++) {

      // Fill in gaps with simple linear interpolation
      // between depths of adjacent valid pixels
	
      in = FALSE;
      lastOut = -1;
      
      for (yy = 0; yy < rangeGrid->nlg; yy++) {
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
	  if (index >= 0 && !in) {
	      
	      lastIn = yy;
	      in = TRUE;
	      
	      if (lastOut == -1)
		  continue;

	      doFill = TRUE;
	      for (i = lastOut+1; i <lastIn; i++, vnew+=dv) {
		  oldIndex = rangeGrid->indices[xx+i*rangeGrid->nlt];
		  if (oldIndex == INT_MIN) {
		      doFill = FALSE;
		      break;
		  }
	      }

	      if (!doFill)
		  continue;

	      v.setValue(rangeGrid->coords[index]);
	      dv = v;
	      dv -= vlast;
	      dv /= (lastIn - lastOut);
	      vnew = vlast;
	      vnew += dv;
	      for (i = lastOut+1; i <lastIn; i++, vnew+=dv) {
		  oldIndex =  rangeGrid->indices[xx+i*rangeGrid->nlt];
		  if (oldIndex == -1) {
		      rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);

		      if (rangeGrid->hasConfidence)
			  rangeGrid->confidence[rangeGrid->numSamples] = 0;

		      rangeGrid->indices[xx+i*rangeGrid->nlt] = 
			  rangeGrid->numSamples;
		      rangeGrid->numSamples++;
		  } else {
		      oldIndex = -oldIndex;
		      z = rangeGrid->coords[oldIndex].z;

		      // Should probably check along line of sight to
		      // laser, rather than along a simple orthographic
		      // projection in the z-direction...

		      x = rangeGrid->coords[oldIndex].x;

		      alongSensor1 = x*dir.x + z*dir.z;
		      alongSensor2 = vnew.x*dir.x + vnew.z*dir.z;

		      if (alongSensor1 < alongSensor2) {
			  rangeGrid->coords[oldIndex].setValue(vnew);

/*
		      if (z < vnew.z) {
			  rangeGrid->coords[oldIndex].setValue(vnew);
			  */

		      }
		  }
	      }
	  } 
	  else if (index < 0 && in) {		
	      lastOut = yy-1;
	      prevIndex = rangeGrid->indices[xx+lastOut*rangeGrid->nlt];
	      vlast.setValue(rangeGrid->coords[prevIndex]);
	      in = FALSE;
	  }
      }
  }

#endif

#if 1
  // Clean up indices to be -1 or valid (i.e., index >= 0 )

  for (yy = 0; yy < rangeGrid->nlg; yy++) {
      for (xx = 0; xx < rangeGrid->nlt; xx++) {
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
	  if (index == INT_MIN) {
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = -1;
	  } else if (index < -1) {
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = -index;
	  }
      }
  }
#endif

  ply_close (ply);

  return (rangeGrid);
}


/******************************************************************************
Read range data from a PLY file.

Entry:
  name - name of PLY file to read from

Exit:
  returns pointer to data, or NULL if it couldn't read from file
******************************************************************************/

RangeGrid *
readRangeGridExtendEdges(const char *name)
{
  int i,j,k,index, best_index;
  PlyFile *ply;
  RangeGrid *rangeGrid = NULL;
  char **obj_info;
  int num_obj_info;
  int num_elems;
  int nprops;
  int nelems;
  char **elist;
  int file_type;
  float version;
  PlyProperty **plist;
  int num_rows,num_cols;
  RangePnt range_pnt;
  PlyVertex vert;
  char *elem_name;
  int get_std_dev = 0;
  int get_confidence = 0;
  int get_intensity = 0;
  int get_color = 0;
  int has_red = 0;
  int has_green = 0;
  int has_blue = 0;
  int is_warped = 0;
  char temp[PATH_MAX];
  int isRightMirrorOpen = 1;
  int found_mirror = 0;
  float conf,std;
  float min_std,max_std,max;
  float avg_std = 0;
  float angle, alongSensor1, alongSensor2, x;
  float lgincr;

  ply = ply_open_for_reading(name, &nelems, &elist, &file_type, &version);
  if (ply == NULL)
    return (NULL);

  /* parse the obj_info */

  obj_info = ply_get_obj_info (ply, &num_obj_info);
  for (i = 0; i < num_obj_info; i++) {
    if (strstr(obj_info[i], "num_cols"))
	sscanf(obj_info[i], "%s%d", temp, &num_cols);
    if (strstr(obj_info[i], "num_rows"))
	sscanf(obj_info[i], "%s%d", temp, &num_rows);
    if (strstr(obj_info[i], "is_warped"))
	sscanf(obj_info[i], "%s%d", temp, &is_warped);
    if (strstr(obj_info[i], "optimum_std_dev"))
	sscanf(obj_info[i], "%s%f", temp, &avg_std);
    if (strstr(obj_info[i], "echo_lgincr"))
	sscanf(obj_info[i], "%s%f", temp, &lgincr);
    if (strstr(obj_info[i], "is_right_mirror_open")) {
	found_mirror = TRUE;
	sscanf(obj_info[i], "%s%d", temp, &isRightMirrorOpen);
    }
  }

  EdgeLength = lgincr;

  if (is_warped && !found_mirror && Warn)
      printf("Couldn't tell which mirror was open.  Right mirror assumed.\n");

  min_std = avg_std / RANGE_DATA_SIGMA_FACTOR;
  max_std = avg_std * RANGE_DATA_SIGMA_FACTOR;

  /* set up the range data structure */
  rangeGrid = new RangeGrid;
  rangeGrid->nlg = num_rows;
  rangeGrid->nlt = num_cols;
  rangeGrid->intensity = NULL;
  rangeGrid->confidence = NULL;
  rangeGrid->matDiff = NULL;
  rangeGrid->hasColor = 0;
  rangeGrid->hasIntensity = 0;
  rangeGrid->hasConfidence = 0;
  rangeGrid->multConfidence = 0;

  if (!is_warped) {
      rangeGrid->viewDir.setValue(0, 0, -1);
  } else {
      if (isRightMirrorOpen) {
	  rangeGrid->viewDir.setValue(-sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      } else {
	  rangeGrid->viewDir.setValue(sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      }
  }

  rangeGrid->num_obj_info = num_obj_info;
  rangeGrid->obj_info = new char*[num_obj_info];
  for (i = 0; i < num_obj_info; i++) {
      rangeGrid->obj_info[i] = new char[strlen(obj_info[i])+1];
      strcpy(rangeGrid->obj_info[i], obj_info[i]);
  }


  rangeGrid->isWarped = is_warped;
  rangeGrid->isRightMirrorOpen = isRightMirrorOpen;

  /* see if we've got both vertex and range_grid data */

  plist = ply_get_element_description (ply, "vertex", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain vertex data\n");
    return (NULL);
  }
  rangeGrid->numOrigSamples = num_elems;
  rangeGrid->numSamples = num_elems;
  rangeGrid->maxSamples = rangeGrid->nlt * rangeGrid->nlg;
  plist = ply_get_element_description (ply, "range_grid", &num_elems, &nprops);
  if (plist == NULL) {
    fprintf (stderr, "file doesn't contain range_grid data\n");
    return (NULL);
  }

  /* read in the range data */

  for (i = 0; i < nelems; i++) {

    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* see if the file contains intensities */
      for (j = 0; j < nprops; j++) {
        if (equal_strings ("std_dev", plist[j]->name))
          get_std_dev = 1;
        if (equal_strings ("confidence", plist[j]->name))
          get_confidence = 1;
        if (equal_strings ("intensity", plist[j]->name))
          get_intensity = 1;
        if (equal_strings ("diffuse_red", plist[j]->name))
          has_red = 1;
        if (equal_strings ("diffuse_green", plist[j]->name))
          has_green = 1;
        if (equal_strings ("diffuse_blue", plist[j]->name))
          has_blue = 1;
      }

      if (has_red && has_green && has_blue) {
        get_color = 1;
        rangeGrid->hasColor = 1;
      }

      if (get_intensity)
        rangeGrid->hasIntensity = 1;

      if (get_std_dev && is_warped) {
        rangeGrid->hasConfidence = 1;
        rangeGrid->multConfidence = 1;
      }
      else if (get_confidence)
        rangeGrid->hasConfidence = 1;

      ply_get_property (ply, "vertex", &vert_std_props[0]);
      ply_get_property (ply, "vertex", &vert_std_props[1]);
      ply_get_property (ply, "vertex", &vert_std_props[2]);
      if (get_confidence)     
        ply_get_property (ply, "vertex", &vert_std_props[3]);
      if (get_intensity)             
        ply_get_property (ply, "vertex", &vert_std_props[4]);
      if (get_color) {    
        ply_get_property (ply, "vertex", &vert_std_props[5]);
        ply_get_property (ply, "vertex", &vert_std_props[6]);
        ply_get_property (ply, "vertex", &vert_std_props[7]);
      }
      if (get_std_dev)
        ply_get_property (ply, "vertex", &vert_std_props[8]);

      rangeGrid->coords = new Vec3f[rangeGrid->maxSamples];

      if (get_confidence || get_std_dev)
	  rangeGrid->confidence = new float[rangeGrid->maxSamples];

      if (get_intensity)
	  rangeGrid->intensity = new float[rangeGrid->maxSamples];

      if (get_color)
	  rangeGrid->matDiff = new vec3uc[rangeGrid->maxSamples];

      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &vert);
        rangeGrid->coords[j].x = vert.x;
        rangeGrid->coords[j].y = vert.y;
        rangeGrid->coords[j].z = vert.z;

        if (get_intensity) {
          rangeGrid->intensity[j] = vert.intensity;
	}

        if (get_std_dev&&is_warped) {

          std = vert.confidence;

          if (std < min_std)
            conf = 0;
          else if (std < avg_std)
            conf = (std - min_std) / (avg_std - min_std);
          else if (std > max_std)
            conf = 0;
          else
	      conf = (max_std - std) / (max_std - avg_std);

/*
       Unsafe to use vertex intensity, as aperture settings may change
       between scans.  Instead, use std_dev confidence * orientation.

	  conf *= vert.intensity;
*/	    

	  if (get_intensity)
	      if (vert.intensity < RANGE_DATA_MIN_INTENSITY) conf = 0.0;

          rangeGrid->confidence[j] = conf;
        }
        else if (get_confidence) {
          rangeGrid->confidence[j] = vert.confidence;
        }

        if (get_color) {
          rangeGrid->matDiff[j][0] = vert.red;
          rangeGrid->matDiff[j][1] = vert.grn;
          rangeGrid->matDiff[j][2] = vert.blu;
        }
      }
    }

    if (equal_strings ("range_grid", elem_name)) {
      rangeGrid->indices = new int[rangeGrid->nlt * rangeGrid->nlg];
      ply_get_element_setup (ply, elem_name, 1, range_props);
      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) &range_pnt);
        if (range_pnt.num_pts == 0)
          rangeGrid->indices[j] = -1;
        else {
	    max = -FLT_MAX;
	    for (k = 0; k < range_pnt.num_pts; k++) {
		index = range_pnt.pts[k];
		// There will only be more than one point per sample
		// if there are intensities
		if (get_intensity) {
		    if (rangeGrid->intensity[index] > max) {
			max = rangeGrid->intensity[index];
			best_index = index;
		    }
		}
		else {
		    best_index = index;
		}
	    }
	    index = best_index;
	    if (get_confidence || get_std_dev) {
		if (rangeGrid->confidence[index] > 0.0)  
		    rangeGrid->indices[j] = index;
		else
		    rangeGrid->indices[j] = -1;
	    } else {
		rangeGrid->indices[j] = index;
	    }

	    if (get_intensity) {
		if (rangeGrid->intensity[index] < RANGE_DATA_MIN_INTENSITY) {
		    rangeGrid->indices[j] = -1;
		}
	    }

	    free (range_pnt.pts);
	}
      }
    }
  }


  Vec3f vlast, v, dv, vnew;
  float z;
  int xx, yy, in, lastOut, lastIn, prevIndex;
  int xlim, ylim, doFill, oldIndex;


  // X-pass
  for (yy = 0; yy < rangeGrid->nlg; yy++) {

      xx = 0;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  xx++;
	  if (xx == rangeGrid->nlt)
	      break;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      xlim = xx;

      if (xlim == rangeGrid->nlt)
	  continue;

      if (xlim > EdgeExtensionSamples - 1) {
	  v.setValue(rangeGrid->coords[index]);
	  dv.setValue(-lgincr, 0, -lgincr*cos(M_PI*EdgeExtensionAngle/180));
	  vnew.setValue(v);
	  vnew += dv;
	  for (xx = xlim-1; xx > xlim-EdgeExtensionSamples-1; xx--) {
	      rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);

	      if (rangeGrid->hasConfidence)
		  rangeGrid->confidence[rangeGrid->numSamples] = 0;

	      // Fill with a negative number so that it
	      // can be recognized later as a hole fill
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = 
		  -rangeGrid->numSamples;
	      rangeGrid->numSamples++;

	      vnew += dv;
	  }
      }

      xx = rangeGrid->nlt-1;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  xx--;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }


      xlim = xx+1;
      if (xlim < rangeGrid->nlt-EdgeExtensionSamples) {
	  v.setValue(rangeGrid->coords[index]);
	  dv.setValue(lgincr, 0, -lgincr*tan(M_PI*EdgeExtensionAngle/180));

	  vnew.setValue(v);
	  vnew += dv;
	  for (xx = xlim; xx < xlim+EdgeExtensionSamples; xx++) {
	      rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);

	      if (rangeGrid->hasConfidence)
		  rangeGrid->confidence[rangeGrid->numSamples] = 0;

	      // Fill with a negative number so that it
	      // can be recognized later as a hole fill
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = 
		  -rangeGrid->numSamples;
	      rangeGrid->numSamples++;

	      vnew += dv;
	  }
      }
  }


  if (is_warped && isRightMirrorOpen) {
      angle = 30;
  } else if (is_warped && !isRightMirrorOpen) {
      angle = -30;
  } else {
      angle = 0;
  }

  Vec3f dir(sin(RAD(angle)), 0, cos(RAD(angle)));

  // Y-pass
  for (xx = 0; xx < rangeGrid->nlt; xx++) {

      yy = 0;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  yy++;
	  if (yy == rangeGrid->nlg)
	      break;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }

      ylim = yy;

      if (ylim == rangeGrid->nlg)
	  continue;

      if (ylim > EdgeExtensionSamples - 1) {
	  v.setValue(rangeGrid->coords[index]);
	  dv.setValue(0, -lgincr, -lgincr*tan(M_PI*EdgeExtensionAngle/180));
	  vnew.setValue(v);
	  vnew += dv;
	  for (yy = ylim-1; yy > ylim-EdgeExtensionSamples-1; yy--) {
	      oldIndex =  rangeGrid->indices[xx+yy*rangeGrid->nlt];
	      if (oldIndex == -1) {
		  rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);

		  if (rangeGrid->hasConfidence)
		      rangeGrid->confidence[rangeGrid->numSamples] = 0;

		  rangeGrid->indices[xx+yy*rangeGrid->nlt] = 
		      rangeGrid->numSamples;
		  rangeGrid->numSamples++;
	      } else {
		  oldIndex = -oldIndex;
		  z = rangeGrid->coords[oldIndex].z;

		  // Should probably check along line of sight to
		  // laser, rather than along a simple orthographic
		  // projection in the z-direction...

		  x = rangeGrid->coords[oldIndex].x;

		  alongSensor1 = x*dir.x + z*dir.z;
		  alongSensor2 = vnew.x*dir.x + vnew.z*dir.z;

		  if (alongSensor1 < alongSensor2) {
		      rangeGrid->coords[oldIndex].setValue(vnew);
		  }
	      }

	      vnew += dv;
	  }
      }

      yy = rangeGrid->nlg-1;
      index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      while (index < 0) {
	  yy--;
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
      }


      ylim = yy+1;
      if (ylim < rangeGrid->nlg-EdgeExtensionSamples-1) {
	  v.setValue(rangeGrid->coords[index]);
	  dv.setValue(0, lgincr, -lgincr*cos(M_PI*EdgeExtensionAngle/180));
	  vnew.setValue(v);
	  vnew += dv;
	  for (yy = ylim; yy < ylim+EdgeExtensionSamples-1; yy++) {
	      oldIndex =  rangeGrid->indices[xx+yy*rangeGrid->nlt];
	      if (oldIndex == -1) {
		  rangeGrid->coords[rangeGrid->numSamples].setValue(vnew);

		  if (rangeGrid->hasConfidence)
		      rangeGrid->confidence[rangeGrid->numSamples] = 0;

		  rangeGrid->indices[xx+yy*rangeGrid->nlt] = 
		      rangeGrid->numSamples;
		  rangeGrid->numSamples++;
	      } else {
		  oldIndex = -oldIndex;
		  z = rangeGrid->coords[oldIndex].z;

		  // Should probably check along line of sight to
		  // laser, rather than along a simple orthographic
		  // projection in the z-direction...

		  x = rangeGrid->coords[oldIndex].x;

		  alongSensor1 = x*dir.x + z*dir.z;
		  alongSensor2 = vnew.x*dir.x + vnew.z*dir.z;

		  if (alongSensor1 < alongSensor2) {
		      rangeGrid->coords[oldIndex].setValue(vnew);
		  }
	      }

	      vnew += dv;
	  }
      }
  }


  for (yy = 0; yy < rangeGrid->nlg; yy++) {
      for (xx = 0; xx < rangeGrid->nlt; xx++) {
	  index = rangeGrid->indices[xx+yy*rangeGrid->nlt];
	  if (index == INT_MIN) {
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = -1;
	  } else if (index < -1) {
	      rangeGrid->indices[xx+yy*rangeGrid->nlt] = -index;
	  }
      }
  }

  ply_close (ply);

  return (rangeGrid);
}



/******************************************************************************
Create a triangle mesh from scan data.

Entry:


Exit:
  returns pointer to newly-created mesh
******************************************************************************/

Mesh *
meshFromGrid(RangeGrid *rangeGrid, int subSamp, int connectAll)
{
  int i,j;
  Vec3f v1, v2, vect1, vect2, vect3, vect4;
  int ii,jj;
  int in1,in2,in3,in4,vin1,vin2,vin3,vin4;
  Vertex *vt1,*vt2,*vt3,*vt4;
  int count;
  int max_lt,max_lg;
  Mesh *mesh;
  int nlt,nlg,max_verts,max_tris;
  int *vert_index;

  mesh = new Mesh;

  if (Warn)
      printf("meshFromGrid(): Not computing useful max_length!!\n");

  mesh->isWarped = rangeGrid->isWarped;
  mesh->isRightMirrorOpen = rangeGrid->isRightMirrorOpen;

  /* allocate space for new triangles and vertices */

  mesh->numVerts = 0;
  nlt = rangeGrid->nlt;
  nlg = rangeGrid->nlg;
  // not used: // max_lt = (nlt - 1) / subSamp + 1;
  // not used: // max_lg = (nlg - 1) / subSamp + 1;
  // not used: // max_verts = max_lt * max_lg;

  /* create a list saying whether a vertex is going to be used */

  vert_index = new int[rangeGrid->numSamples];
  for (i = 0; i < rangeGrid->numSamples; i++)
    vert_index[i] = -1;

  /* see which vertices will be used in a triangle */

  count = 0;
  for (i = 0; i <= nlt - subSamp; i += subSamp)
    for (j = 0; j <= nlg - subSamp; j += subSamp) {
      in1 = rangeGrid->indices[i + j * nlt];
      if (in1 >= 0) {
	  vert_index[in1] = 1;
	  count++;
      }
    }

  mesh->numVerts = count;
  mesh->verts = new Vertex[mesh->numVerts];

  mesh->numTris = 0;
  max_tris = mesh->numVerts * 2;
  mesh->tris = new Triangle[max_tris];


  /* create the vertices */

  count = 0;
  for (i = 0; i < rangeGrid->numSamples; i++) {
    if (vert_index[i] == -1)
      continue;

    vert_index[i] = count;
    mesh->verts[count].coord = rangeGrid->coords[i];

    mesh->verts[count].maxVerts = 8;
    mesh->verts[count].verts = new Vertex*[8];
    mesh->verts[count].edgeLengths = new float[8];
    mesh->verts[count].numVerts = 0;

    mesh->verts[count].maxTris = 8;
    mesh->verts[count].tris = new Triangle*[8];
    mesh->verts[count].numTris = 0;

    if (rangeGrid->hasConfidence)
	mesh->verts[count].confidence = rangeGrid->confidence[i];
    else 
	mesh->verts[count].confidence = 1;

    count++;
  }
  mesh->numVerts = count;

  /* create the triangles */

  for (i = 0; i < nlt - subSamp; i += subSamp)
    for (j = 0; j < nlg - subSamp; j += subSamp) {

      ii = (i + subSamp) % nlt;
      jj = (j + subSamp) % nlg;

      /* count the number of good vertices */
      /*
      in1 = rangeGrid->indices[ i +  j * nlt];
      in2 = rangeGrid->indices[ i + jj * nlt];
      in3 = rangeGrid->indices[ii + jj * nlt];
      in4 = rangeGrid->indices[ii +  j * nlt];
      */

      in1 = rangeGrid->indices[ i +  j * nlt];
      in2 = rangeGrid->indices[ii +  j * nlt];
      in3 = rangeGrid->indices[ii + jj * nlt];
      in4 = rangeGrid->indices[ i + jj * nlt];

      count = (in1 >= 0) + (in2 >= 0) + (in3 >= 0) + (in4 >=0);
      if (in1 >= 0) {
	  vin1 = vert_index[in1];
	  vt1 = &mesh->verts[vin1];
	  if (in1 > rangeGrid->numOrigSamples) {
	      vt1->holeFill = TRUE;
	      vt1->stepsToEdge = -1;
	  } else {
	      vt1->holeFill = FALSE;
	      vt1->stepsToEdge = 0;
	  }
      }
      if (in2 >= 0) {
	  vin2 = vert_index[in2];
	  vt2 = &mesh->verts[vin2];
	  if (in2 > rangeGrid->numOrigSamples) {
	      vt2->holeFill = TRUE;
	      vt2->stepsToEdge = -1;
	  } else {
	      vt2->holeFill = FALSE;
	      vt2->stepsToEdge = 0;
	  }
      }
      if (in3 >= 0) {
	  vin3 = vert_index[in3];
	  vt3 = &mesh->verts[vin3];
	  if (in3 > rangeGrid->numOrigSamples) {
	      vt3->holeFill = TRUE;
	      vt3->stepsToEdge = -1;
	  } else {
	      vt3->holeFill = FALSE;
	      vt3->stepsToEdge = 0;
	  }
      }
      if (in4 >= 0) {
	  vin4 = vert_index[in4];
	  vt4 = &mesh->verts[vin4];
	  if (in4 > rangeGrid->numOrigSamples) {
	      vt4->holeFill = TRUE;
	      vt4->stepsToEdge = -1;
	  } else {
	      vt4->holeFill = FALSE;
	      vt4->stepsToEdge = 0;
	  }
      }

      if (UseEdgeLength) {
	  if (count == 4) {    /* all 4 vertices okay, so make 2 tris */
	      float len1,len2;

	      /* compute lengths of cross-edges */
	      v1 = vt1->coord - vt3->coord;
	      v2 = vt2->coord - vt4->coord;
	      len1 = v1.length();
	      len2 = v2.length();

	      /* make triangles that minimize the cross-length */

	      if (len1 < len2) {
		  addTriangleUseLength(mesh, vin1, vin2, vin3, 
				       MaxEdgeLength, connectAll);
		  addTriangleUseLength(mesh, vin1, vin3, vin4, 
				       MaxEdgeLength, connectAll);
	      } else {
		  addTriangleUseLength(mesh, vin2, vin3, vin4, 
				       MaxEdgeLength, connectAll);
		  addTriangleUseLength(mesh, vin2, vin4, vin1, 
				       MaxEdgeLength, connectAll);
	      }
	  }
	  else if (count == 3) {   /* only 3 vertices okay, so make 1 tri */
	      if (in1 == -1) {
		  addTriangleUseLength(mesh, vin2, vin3, vin4, 
				       MaxEdgeLength, connectAll);
	      }
	      else if (in2 == -1) {
		  addTriangleUseLength(mesh, vin1, vin3, vin4, 
				       MaxEdgeLength, connectAll);
	      }
	      else if (in3 == -1) {
		  addTriangleUseLength(mesh, vin1, vin2, vin4, 
				       MaxEdgeLength, connectAll);
	      }
	      else {	/* in4 == -1 */
		  addTriangleUseLength(mesh, vin1, vin2, vin3, 
				       MaxEdgeLength, connectAll);
	      }
	  }
      }
      else {
	  if (count == 4) {    /* all 4 vertices okay, so make 2 tris */
	      float len1,len2;

	      /* compute lengths of cross-edges */
	      v1 = vt1->coord - vt3->coord;
	      v2 = vt2->coord - vt4->coord;
	      len1 = v1.length();
	      len2 = v2.length();

	      /* make triangles that minimize the cross-length */

	      if (len1 < len2) {
		  addTriangleUseNormal(mesh, vin1, vin2, vin3, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
		  addTriangleUseNormal(mesh, vin1, vin3, vin4, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      } else {
		  addTriangleUseNormal(mesh, vin2, vin3, vin4, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
		  addTriangleUseNormal(mesh, vin2, vin4, vin1, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      }
	  }
	  else if (count == 3) {   /* only 3 vertices okay, so make 1 tri */
	      if (in1 == -1) {
		  addTriangleUseNormal(mesh, vin2, vin3, vin4, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      }
	      else if (in2 == -1) {
		  addTriangleUseNormal(mesh, vin1, vin3, vin4, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      }
	      else if (in3 == -1) {
		  addTriangleUseNormal(mesh, vin1, vin2, vin4, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      }
	      else {	/* in4 == -1 */
		  addTriangleUseNormal(mesh, vin1, vin2, vin3, 
				       MinViewDot, rangeGrid->viewDir, 
				       connectAll);
	      }
	  }
      }
    }

  /* free up the vertex index list */
  delete [] vert_index;

  mesh->initNormals();  

  find_mesh_edges(mesh);

  return (mesh);
}



/******************************************************************************
Create a triangle mesh from scan data.

Entry:


Exit:
  returns pointer to newly-created mesh
******************************************************************************/

Mesh *
cleanMeshFromRangeMesh(Mesh *inMesh)
{
  int i,j;
  Vec3f v1, v2, vect1, vect2, vect3, vect4;
  int ii,jj;
  int in1,in2,in3,in4,vin1,vin2,vin3,vin4;
  Vertex *vt1,*vt2,*vt3,*vt4;
  int count;
  int max_lt,max_lg;
  Mesh *mesh;
  int nlt,nlg,max_verts,max_tris;
  mesh = new Mesh;
  Vec3f viewDir;

  uchar colorThresh = MinColor;

  if (Warn)
      printf("meshFromGrid(): Not computing useful max_length!!\n");

  mesh->numVerts = inMesh->numVerts;
  mesh->verts = new Vertex[mesh->numVerts];

  mesh->numTris = 0;
  max_tris = inMesh->numTris;
  mesh->tris = new Triangle[max_tris];

  mesh->isWarped = inMesh->isWarped;
  mesh->isRightMirrorOpen = inMesh->isRightMirrorOpen;
  mesh->hasConfidence = inMesh->hasConfidence;
  mesh->hasColor = inMesh->hasColor;

  if (!mesh->isWarped) {
      viewDir.setValue(0, 0, -1);
  } else {
      if (mesh->isRightMirrorOpen) {
	  viewDir.setValue(-sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      } else {
	  viewDir.setValue(sin(30*M_PI/180), 0, -cos(30*M_PI/180));
      }
  }

  /* create the vertices */

  for (i = 0; i < inMesh->numVerts; i++) {
    mesh->verts[i].coord = inMesh->verts[i].coord;

    mesh->verts[i].maxVerts = 8;
    mesh->verts[i].verts = new Vertex*[8];
    mesh->verts[i].edgeLengths = new float[8];
    mesh->verts[i].numVerts = 0;

    mesh->verts[i].maxTris = 8;
    mesh->verts[i].tris = new Triangle*[8];
    mesh->verts[i].numTris = 0;

    mesh->verts[i].holeFill = FALSE;
    mesh->verts[i].stepsToEdge = -1;
    
    if (inMesh->hasConfidence)
	mesh->verts[i].confidence = inMesh->verts[i].confidence;
    else 
	mesh->verts[i].confidence = 1;

    if (inMesh->hasColor)
       mesh->verts[i].red = inMesh->verts[i].red;
       mesh->verts[i].green = inMesh->verts[i].green;
       mesh->verts[i].blue = inMesh->verts[i].blue;
  }

  /* create the triangles */
  for (i = 0; i < inMesh->numTris; i++) {

     if (inMesh->hasColor) {
	if (inMesh->verts[inMesh->tris[i].vindex1].red < colorThresh ||
	    inMesh->verts[inMesh->tris[i].vindex2].red < colorThresh ||
	    inMesh->verts[inMesh->tris[i].vindex3].red < colorThresh) {
	   continue;
	}
     }

     if (UseEdgeLength) {
	addTriangleUseLength(mesh, inMesh->tris[i].vindex1, 
			     inMesh->tris[i].vindex2, 
			     inMesh->tris[i].vindex3, 
			     MaxEdgeLength, 0);
     } else {
	addTriangleUseNormal(mesh, inMesh->tris[i].vindex1, 
			     inMesh->tris[i].vindex2, 
			     inMesh->tris[i].vindex3,
			     MinViewDot, viewDir, 0);
     }
  }

  mesh->initNormals();  

  find_mesh_edges(mesh);

  return (mesh);
}


static void
addTriangleUseNormal(Mesh *mesh, int vin1, int vin2, int vin3, 
		     float minDot, Vec3f &viewDir, int connectAll)
{
    Vec3f c1, c2, c3;
    Vec3f v1, v2, v3;
    Triangle *tri;
    Vertex *vert1, *vert2, *vert3;
    int tooGrazing;
    float dot;

    c1.setValue(mesh->verts[vin1].coord);
    c2.setValue(mesh->verts[vin2].coord);
    c3.setValue(mesh->verts[vin3].coord);

    v1 = c1 - c2;
    v2 = c1 - c3;
    v3 = v2.cross(v1);
    v3.normalize();
    dot = fabs(v3.dot(viewDir));

    tooGrazing = dot < minDot;
    if (tooGrazing && !connectAll)
	return;

    // No check on limits here???
    tri = &mesh->tris[mesh->numTris];

    tri->vindex1 = vin1;
    tri->vindex2 = vin2;
    tri->vindex3 = vin3;

    vert1 = &mesh->verts[vin1];
    vert2 = &mesh->verts[vin2];
    vert3 = &mesh->verts[vin3];

    vert1->tris[vert1->numTris++] = tri;
    vert2->tris[vert2->numTris++] = tri;
    vert3->tris[vert3->numTris++] = tri;

    addNeighbors(vert1,vert2);
    addNeighbors(vert1,vert3);
    addNeighbors(vert2,vert3);

    if (tooGrazing) {
	vert1->holeFill = 1;
	vert2->holeFill = 1;
	vert3->holeFill = 1;
	vert1->stepsToEdge = -1;
	vert2->stepsToEdge = -1;
	vert3->stepsToEdge = -1;
    }

    mesh->numTris++;
}

static void
addTriangleUseLength(Mesh *mesh, int vin1, int vin2, int vin3, 
		     float maxLength, int connectAll)
{
    Vec3f c1, c2, c3;
    Vec3f v;
    Triangle *tri;
    Vertex *vert1, *vert2, *vert3;
    int tooLong;

    c1.setValue(mesh->verts[vin1].coord);
    c2.setValue(mesh->verts[vin2].coord);
    c3.setValue(mesh->verts[vin3].coord);

    v = c1 - c2;
    tooLong = v.length() > maxLength;
    if (tooLong && !connectAll)
	return;

    v = c1 - c3;
    tooLong = tooLong || v.length() > maxLength;
    if (tooLong && !connectAll)
	return;

    v = c2 - c3;
    tooLong = tooLong || v.length() > maxLength;
    if (tooLong && !connectAll)
	return;

    // No check on limits here???
    tri = &mesh->tris[mesh->numTris];

    tri->vindex1 = vin1;
    tri->vindex2 = vin2;
    tri->vindex3 = vin3;

    vert1 = &mesh->verts[vin1];
    vert2 = &mesh->verts[vin2];
    vert3 = &mesh->verts[vin3];

    vert1->tris[vert1->numTris++] = tri;
    vert2->tris[vert2->numTris++] = tri;
    vert3->tris[vert3->numTris++] = tri;

    addNeighbors(vert1,vert2);
    addNeighbors(vert1,vert3);
    addNeighbors(vert2,vert3);

    if (tooLong) {
	vert1->holeFill = 1;
	vert2->holeFill = 1;
	vert3->holeFill = 1;
	vert1->stepsToEdge = -1;
	vert2->stepsToEdge = -1;
	vert3->stepsToEdge = -1;
    }

    mesh->numTris++;
}


void
addNeighbors(Vertex *v1, Vertex *v2)
{
    float length;
    Vec3f a;
    int found = 0;

    for (int i = 0; i < v1->numVerts; i++) {
	if (v1->verts[i] == v2) {
	    found = 1;
	    break;
	}
    }

    if (!found) {
	length = 0.0005;
	a = v1->coord - v2->coord;
	length = a.length();

	if (v1->numVerts == v1->maxVerts)
	   reallocVerts(v1);
	
	if (v2->numVerts == v2->maxVerts)
	   reallocVerts(v2);
	
	v1->edgeLengths[v1->numVerts] = length;
	v2->edgeLengths[v2->numVerts] = length;
	v1->verts[v1->numVerts++] = v2;
	v2->verts[v2->numVerts++] = v1;
    }
}


/******************************************************************************
Find which vertices of a mesh are on the edge of the mesh.

Entry:
  mesh - mesh to find edges of
******************************************************************************/

void
find_mesh_edges(Mesh *mesh)
{
  int i;

  /* examine each vertex of a mesh to see if it's on the edge */

  for (i = 0; i < mesh->numVerts; i++)
    vertex_edge_test (mesh, &mesh->verts[i]);
}


/******************************************************************************
Mark a vertex to say whether it is on the edge of a mesh or not.

Entry:
  vert: vertex to mark as on edge or not

Exit:
  returns 0 if everything was okay, 1 if the mesh is build funny
******************************************************************************/

int 
vertex_edge_test(Mesh *mesh, Vertex *vert)
{
  int i;
  Triangle *tri;
  unsigned char on_edge;
  static int been_here = 0;
  int bad_mesh = 0;

  /* initialize list that counts how many times an edge has been marked */
  /* (each edge should be marked twice, once for each triangle, */
  /*  otherwise the vertex is on an edge) */

  for (i = 0; i < vert->numVerts; i++)
    vert->verts[i]->count = 0;

  on_edge = 0;  /* assume we're not on an edge */

  /* go through each triangle of vertex, counting how many times an */
  /* adjacent vertex has been used */

  for (i = 0; i < vert->numTris; i++) {
    tri = vert->tris[i];

    /* count the participation of each vertex of the triangle */
    mesh->verts[tri->vindex1].count++;
    mesh->verts[tri->vindex2].count++;
    mesh->verts[tri->vindex3].count++;


    // This prevents us from operating on them as vertices
    // internal to the mesh
    if (mesh->verts[tri->vindex1].holeFill)
	on_edge = 1;

    if (mesh->verts[tri->vindex2].holeFill)
	on_edge = 1;

    if (mesh->verts[tri->vindex3].holeFill)
	on_edge = 1;
  }

  /* examine the counts of the neighboring vertices to see if they */
  /* prove the vertex to be an edge.  also check for mesh consistancy */

  for (i = 0; i < vert->numVerts; i++) {
    if (vert->verts[i]->count == 1) {
      on_edge = 1;
    }
    else if (vert->verts[i]->count == 2) {
      /* this is okay */
    }
    else {
      /* if we're here, our mesh is built wrong */
      bad_mesh = 1;
      if (!been_here) {
	been_here = 1;
	fprintf (stderr, "vertex_edge_test: %d count on an edge\n",
		 vert->verts[i]->count);
        /*
	fprintf (stderr, "(You'll only get this message once, so beware!)\n");
        */
      }
    }
  }

  vert->on_edge = on_edge;

  /* say if the mesh is improperly built */
  return (bad_mesh);
}

