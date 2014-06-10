#include "Linear.h"
#include <iostream>
#include <unistd.h>

int
main()
{
    Matrix4f R, T, S, a, b, c, d, e, f;

    S.makeIdentity();
    R.makeIdentity();
    T.makeIdentity();

    Vec3f scale(2, 3, 4);
    Vec3f trans(5, 6, 7);
    float thetaY = M_PI/180*45;

    S.setScale(scale);
    R.setRotateY(thetaY);
    T.setTranslate(trans);

    a.makeIdentity();
    a.multLeft(S);
    a.multLeft(R);
    a.multLeft(T);

    b.setValue(a.inverse());
    b.print();
    printf("\n");

    d.makeIdentity();
    d.scale(scale);
    d.rotateY(thetaY);
    d.translate(trans);

    e.setValue(d.inverse());
    e.print();
    printf("\n");

    scale.setValue(1/scale[0], 1/scale[1], 1/scale[2]);
    trans.negate();
    thetaY = -thetaY;
    
    S.setScale(scale);
    R.setRotateY(thetaY);
    T.setTranslate(trans);

    c.makeIdentity();
    c.multLeft(T);
    c.multLeft(R);
    c.multLeft(S);
    c.print();
    printf("\n");
    

    f.makeIdentity();
    f.translate(trans);
    f.rotateY(thetaY);
    f.scale(scale);
    f.print();
    printf("\n");


#if 0
    if (b == c && c == e && e == f)
       cout << "Success.\n";
    else
       cout << "Failure.\n";
#endif
 
/*
    for (int i = 0; i < 100000; i++)
	R.setValue(T.inverse());

    cerr << "Done inverting.\n";
*/
    
    /*

    float **verts;
    verts = new float*[3];
    verts[0] = new float[3];
    verts[1] = new float[3];
    verts[2] = new float[3];

    verts[0][0] = 1;
    verts[0][1] = 3;
    verts[0][2] = 6;

    verts[1][0] = 1;
    verts[1][1] = 4;
    verts[1][2] = 7;

    verts[2][0] = -1;
    verts[2][1] = 1;
    verts[2][2] = 3;

    float *norm;
    norm = new float[3];

    fitplane(verts, 3, 3, norm);
    float dist = 1/sqrt(norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2]);
    norm[0] *= dist;
    norm[1] *= dist;
    norm[2] *= dist;

    float diff, err = 0;
    for (int i = 0; i < 3; i++) {
	diff = norm[0]*verts[i][0] + norm[1]*verts[i][1] 
	    + norm[2]*verts[i][2] - dist;
	err += fabs(diff);
    }

    cerr << "Average distance from plane = " << err/3 << endl;
    */

    return 0;
}


