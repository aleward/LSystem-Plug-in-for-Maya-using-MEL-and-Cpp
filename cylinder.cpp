#include "cylinder.h"
#include <maya/MMatrix.h>
#include <math.h>

MPointArray CylinderMesh::gPoints;
MVectorArray CylinderMesh::gNormals;
MIntArray CylinderMesh::gFaceCounts;
MIntArray CylinderMesh::gFaceConnects;

CylinderMesh::CylinderMesh(
   const MPoint& start, const MPoint& end, double _r) : 
    mStart(start), mEnd(end), r(_r)
{
    if (gPoints.length() == 0)
    {
        initCylinderMesh(r);
    }
}

CylinderMesh::~CylinderMesh(){}

void CylinderMesh::transform(MPointArray& points, MVectorArray& normals)
{
    MVector forward = mEnd - mStart;
    double s = forward.length();
    forward.normalize();

    MVector left = MVector(0,0,1)^forward;
    MVector up;
    if (left.length() < 0.0001)
    {
        up = forward^MVector(0,1,0);
        left = up^forward;
    }
    else
    {
        up = forward^left;
    }
	left.normalize();
	up.normalize();

    MMatrix mat;
    mat[0][0] = forward[0];  mat[0][1] = left[0]; mat[0][2] = up[0]; mat[0][3] = 0;
    mat[1][0] = forward[1];  mat[1][1] = left[1]; mat[1][2] = up[1]; mat[1][3] = 0;
    mat[2][0] = forward[2];  mat[2][1] = left[2]; mat[2][2] = up[2]; mat[2][3] = 0;
    mat[3][0] = 0;           mat[3][1] = 0;       mat[3][2] = 0;     mat[3][3] = 1;
    mat = mat.transpose();

    for (int i = 0; i < gPoints.length(); i++)
    {
        MPoint p = gPoints[i];
        p.x = p.x * s; // scale
        p = p * mat + mStart; // transform
        points.append(p);

        MVector n = gNormals[i] * mat;
        normals.append(n);
    }
}

void CylinderMesh::appendToMesh(
    MPointArray& points, 
    MIntArray& faceCounts, 
    MIntArray& faceConnects)
{
    MPointArray cpoints;
    MVectorArray cnormals; 
    transform(cpoints, cnormals);    

    int startIndex = points.length(); // offset for indexes
    for (int i = 0; i < cpoints.length(); i++)
    {
        points.append(cpoints[i]);
    }
    for (int i = 0; i < gFaceCounts.length(); i++)
    {
        faceCounts.append(gFaceCounts[i]);
    }

    for (int i = 0; i < gFaceConnects.length(); i++)
    {
        faceConnects.append(gFaceConnects[i]+startIndex);
    }
}

void CylinderMesh::getMesh(
    MPointArray& points, 
    MIntArray& faceCounts, 
    MIntArray& faceConnects)
{
    MVectorArray cnormals; 
    transform(points, cnormals);    
    faceCounts = gFaceCounts;
    faceConnects = gFaceConnects;
}

void CylinderMesh::initCylinderMesh(double r)
{
    int numslices = 10;
    double angle = M_PI*2/numslices;

    // Add points and normals
    gPoints.clear();
    gNormals.clear();
    gFaceCounts.clear();
    gFaceConnects.clear();

    for (int i = 0; i < numslices; i++)
    {
        gPoints.append(MPoint(0,r*cos(angle*i), r*sin(angle*i)));
        gNormals.append(MVector(0,r*cos(angle*i), r*sin(angle*i)));
    }
    for (int i = 0; i < numslices; i++)
    {
        gPoints.append(MPoint(1,r*cos(angle*i), r*sin(angle*i)));
        gNormals.append(MVector(0,r*cos(angle*i), r*sin(angle*i)));
    }
    // endcap 1
    gPoints.append(MPoint(0,0,0));
    gNormals.append(MVector(-1,0,0));

    // endcap 2
    gPoints.append(MPoint(1,0,0));
    gNormals.append(MVector(1,0,0));

    // Set indices for endcap 1
    for (int i = 0; i < numslices; i++)
    {
        gFaceCounts.append(3); // append triangle
        gFaceConnects.append(2*numslices);
        gFaceConnects.append((i+1)%numslices);
        gFaceConnects.append(i);
    }
    
    // Set indices for endcap 2
    for (int i = numslices; i < 2*numslices; i++)
    {
        gFaceCounts.append(3); // append triangle
        gFaceConnects.append(2*numslices+1);
        gFaceConnects.append(i);
        int next = i+1;
        if (next >= 2*numslices) next = numslices;
        gFaceConnects.append(next);
    }

    // Set indices for middle
    for (int i = 0; i < numslices; i++)
    {
        gFaceCounts.append(4); // append quad
        gFaceConnects.append(i);
        gFaceConnects.append((i+1)%numslices);
        gFaceConnects.append((i+1)%numslices+numslices);
        gFaceConnects.append(i+numslices);
    }
}

void CylinderMesh::generateGeometry(unsigned int n, LSystem* lsystem,
	MPointArray & points, MIntArray & faceCounts, MIntArray & faceConnects)
{
	std::vector<LSystem::Branch> branches = std::vector<LSystem::Branch>();
	lsystem->process(n, branches);

	// Create Mesh
	if (branches.size() > 0) {
		// Build each branch as a cylinder and append data into Arrays
		for (int i = 0; i < branches.size(); i++) {

			// Find start and end points of each branch 
			vec3 p1 = branches.at(i).first;
			vec3 p2 = branches.at(i).second;
			// Flipped Y and Z for maya
			const MPoint& start = MPoint(p1[0], p1[2], p1[1]);
			const MPoint& end = MPoint(p2[0], p2[2], p2[1]);

			CylinderMesh cylinder = CylinderMesh(start, end, 0.1);
			cylinder.appendToMesh(points, faceCounts, faceConnects);
		}
	}
}
