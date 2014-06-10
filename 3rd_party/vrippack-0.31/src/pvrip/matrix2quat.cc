// matrix2quat.C by Sean Eron Anderson, 1998
// Compile with: CC matrix2quat.C -o matrix2quat -lInventor

#include <Inventor/SbLinear.h>
#include <iostream.h>

int
main()
{
     SbMatrix mat;
     for (int i = 0; i < 4; i++)
     {
	  for (int j = 0; j < 4; j++)
	  {
	       cin >> mat[i][j];
	  }
     }
     if (!cin)
     {
	  cerr << "Error: premature end of input.\n"
	       << "Input: 4x4 matrix; output: tx ty tz qi qj qk ql\n";
	  return 1;
     }
     
     mat = mat.transpose();
     
     SbVec3f t, s;
     SbRotation r, so;
     mat.getTransform(t, r, s, so);
     
     cout << t[0] << " " << t[1] << " " << t[2] << " ";
     cout << -r.getValue()[0] << " " << -r.getValue()[1] << " "
	  << -r.getValue()[2] << " " << r.getValue()[3] << endl;
     
     return 0;
}
