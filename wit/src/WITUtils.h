#ifndef WIT_UTILS_H
#define WIT_UTILS_H

#include <util/typedefs.h>

#include <vtkPolyData.h>

#include <QtGlobal>

static PCollModel CollModelFromPolyData(vtkPolyData *tris)
{
	PCollModel model(new CollModel);

#ifdef USE_RAPID
	model->BeginModel();
	
	vtkIdType            npts = 0;
	vtkIdType            *pts = 0;
	
	// Add triangles to the filter
	vtkCellArray* cells = tris->GetPolys();
	
	// start traversal on the polydata
	cells->InitTraversal();
	int counter=0;
	
	// for each triangle in the polydata
	while(cells->GetNextCell( npts, pts ))
	{     
		if(npts != 3)
			std::cout << "May be doing intersection incorrectly." << endl;
		double *p;
		
		// add the triangle to the rapid model.
		p = tris->GetPoint(pts[0]); double a[3] = {p[0], p[1], p[2]};
		p = tris->GetPoint(pts[1]); double b[3] = {p[0], p[1], p[2]};
		p = tris->GetPoint(pts[2]); double c[3] = {p[0], p[1], p[2]};
		model->AddTri (a,b,c,counter);
		counter++;
	}
	model->EndModel();
	  
#else
	int numVerts = tris->GetNumberOfCells()*3;
	int numTris = tris->GetNumberOfCells();
	
	IceMaths::Point *verts = new IceMaths::Point[numVerts];
	IceMaths::IndexedTriangle *optris = new IceMaths::IndexedTriangle[numTris];

	vtkCellArray *cells = tris->GetPolys();

	cells->InitTraversal();
	vtkIdType            npts = 0;
	vtkIdType            *pts = 0;
	int index = 0;
	int triIndex = 0;
	while(cells->GetNextCell(npts, pts))
	{
		double *p;
		if(npts != 3)
			qDebug("nPts is not three: %i",npts);

		for(int i = 0; i < 3; i++)
		{
			p = tris->GetPoint(pts[i]);
			//qDebug("Added Vertex at %i: (%f, %f, %f)", pts[i], p[0], p[1], p[2]);
			verts[index++] = IceMaths::Point(p[0], p[1], p[2]);
		}

		optris[triIndex++] = IceMaths::IndexedTriangle(index - 3, index - 2, index-1);
	}

	Opcode::OPCODECREATE OPCC;
	OPCC.mIMesh = new Opcode::MeshInterface();
	//OPCC.mQuantized = false;
	//OPCC.mKeepOriginal = false;
	OPCC.mNoLeaf = false;
	OPCC.mIMesh->SetNbTriangles(numTris);
	OPCC.mIMesh->SetNbVertices(numVerts);
	OPCC.mIMesh->SetPointers(optris, verts);

	if(model->Build(OPCC))
		qDebug("Successfully built ROI collision model");
	else
		qDebug("Failed to build ROI Collision model");

	//delete [] verts;
	//delete [] optris;
#endif
	return model;
}

#endif