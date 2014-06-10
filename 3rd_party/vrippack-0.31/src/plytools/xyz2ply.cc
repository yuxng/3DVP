#include <iostream>
#include <stdlib.h>
#include <strings.h>

#ifdef LINUX
#include <string.h>
#endif

// xyz2ply by Sean Anderson, 1999.  
// Based loosely on cyra2ply, by Lucas Periera
// Converts a NRC xyz laser range image file to a ply file.

// Compile with: CC -o xyz2ply xyz2ply.cc

void
printUsage(const char * name)
{
   std::cerr << "Convert NRC xyz laser range scan to a ply file.\n"
	<< "Usage:\n"
	<< "  " << name
	<< " -h | [-r] [headersize [rows [colunms]]] < file.xyz > file.ply\n"
	<< "where:\n"
	<< "  -h          Shows this help\n"
	<< "  -r          Reverses the order of points in a row\n"
	<< "  headersize  Override the size of the header to be headersize\n"
	<< "  rows        Override the number of rows\n"
	<< "  columns     Override the number of columns\n";
}

int
main(int argc, char * argv[])
{
   int cols = -1;
   int rows = -1;
   int headersize = -1;
   bool reverse = false;
     
   for (int c = 1; c < argc; c++)
   {
      if (!strcmp(argv[c], "-h"))
      {
	 printUsage(argv[0]);
	 return 1;
      }
      else if (!strcmp(argv[c], "-r"))
      {
	 reverse = true;
      }
      else if (headersize == -1)
      {
	 headersize = atoi(argv[c]);
      }
      else if (cols == -1)
      {
	 cols = atoi(argv[c]);
      }
      else if (rows == -1)
      {
	 rows = atoi(argv[c]);
      }
      else 
      {
	 std::cerr << "Too many arguments!\n";
	 printUsage(argv[0]);
      }
   }
   
   // If header size not given, try to find in the file.
   if (headersize == -1)
   {
      std::cin.seekg(80, std::ios::beg);
      if (!std::cin)
      {
	 std::cerr << "Header too short\n";
	 return 1;
      }
      std::cin.read((char *) &headersize, sizeof(headersize));
      if (!std::cin)
      {
	 std::cerr << "Couldn't read header size\n";
	 return 1;
      }      
   }
   
   if (cols == -1)
   {
      std::cin.seekg(244, std::ios::beg);
      if (!std::cin)
      {
	 std::cerr << "Header too short\n";
	 return 1;
      }
      std::cin.read((char *) &cols, sizeof(cols));
      if (!std::cin)
      {
	 std::cerr << "Couldn't read number of columns\n";
	 return 1;
      }      
   }
   
   if (rows == -1)
   {
      std::cin.seekg(248, std::ios::beg);
      if (!std::cin)
      {
	 std::cerr << "Header too short\n";
	 return 1;
      }
      std::cin.read((char *) &rows, sizeof(rows));
      if (!std::cin)
      {
	 std::cerr << "Couldn't read number of rows\n";
	 return 1;
      }      
   }
   
     
   std::cin.seekg(headersize, std::ios::beg);
   if (!std::cin)
   {
      std::cerr << "File too short; couldn't seek to start of xyz data\n";
      return 1;
   }	  

   typedef float float3[3];

   float3 * pts = new float3 [rows * cols];   
   int * grid = new int[rows * cols];
   int * pgrid = grid;
   int index = 0;
   
   int row, col;
   for (row = 0; row < rows; row++)
   {
      for (col = 0; col < cols; col++, pgrid++)
      {
	 std::cin.read((char *) &pts[index], sizeof(pts[index]));	 
	 
	 if (!std::cin)
	 {
	    std::cerr << "File too short\n";
	    return 1;
	 }
	 
	 if (pts[index][0] == -1 && pts[index][1] == -1 && pts[index][2] == -1)
	 {
	    *pgrid = -1;
	 }
	 else
	 {
	    *pgrid = index;
	    index++;
	 }
      }
   }
   
   const int numVerts = index;
   
   std::cout << "ply\n"
	<< "format ascii 1.0\n"
	<< "obj_info is_cyberware_data 0\n"
	<< "obj_info is_mesh 0\n"
	<< "obj_info is_warped 0\n"
	<< "obj_info is_interlaced 0\n"
	<< "obj_info num_cols " << cols << std::endl
	<< "obj_info num_rows " << rows << std::endl
	<< "element vertex " << numVerts << std::endl
	<< "property float x\n"
	<< "property float y\n"
	<< "property float z\n"
	<< "element range_grid " << rows * cols << std::endl
	<< "property list uchar int vertex_indices\n"
	<< "end_header\n";
      
   for (index = 0; index < numVerts; index++)
   {
      std::cout << pts[index][0] << " " 
	   << pts[index][1] << " "
	   << pts[index][2] << std::endl;
   }

   pgrid = grid;
   for (row = 0; row < rows; row++)
   {
      if (reverse)
      {
	 pgrid = grid + (1 + row) * cols - 1;
      }
      
      for (col = 0; col < cols; col++)
      {
	 if (*pgrid < 0)
	 {
	    std::cout << "0\n";
	 }
	 else 
	 {
	    std::cout << "1 " << *pgrid << std::endl;
	 }
	 
	 if (reverse)
	 {
	    pgrid--;
	 }
	 else 
	 {
	    pgrid++;
	 }
      }
   }
}
