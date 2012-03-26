/***********************************************************************
* AUTHOR: sherbond <sherbond>
*   FILE: qPathwayViz.cpp
*   DATE: Thu Nov 20 16:42:12 2008
*  DESCR: 
***********************************************************************/
#include <util/typedefs.h>

// - WIT
#include "../WITTypedefs.h"
#include "WITPathwayViz.h"

// Rendering
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

// - util
#include <util/DTIPathwayIO.h>
#include <util/DTIPathDistanceMatrix.h>
#include <util/DTIPathwayDatabase.h>
#include <util/DTIPathwaySelection.h>
#include <util/DTIPathway.h>
#include <util/DTIMath.h>
#include <util/DTIPathwayStatisticHeader.h>
#include <util/ROIManager.h>

// - VTK
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkCellArray.h"
#include "../View/VTK/vtkFilledCaptionActor2D.h"
#include "vtkMatrix4x4.h"
#include "vtkPropCollection.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include "vtkTubeFilter.h"

#include <numeric>

using namespace std;
vtkTubeFilter* tubeFilter;

WITPathwayViz::WITPathwayViz()
{
	_filterOperation = UNION;
	_lineWidthSetting = 0;
	_pdPathways = NULL;

	// allocate memory for surface intersection actor
	_pdIntersections = vtkPolyData::New();
	vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
	
	// allocate memory for fiber actor
	vtkPoints *intPoints = vtkPoints::New();
	vtkCellArray *intCells = vtkCellArray::New();
	_pdIntersections->SetPoints(intPoints);
	_pdIntersections->SetVerts (intCells);
	//_aIntersections->GetProperty()->SetPointSize (5);
	intPoints->Delete();
	intCells->Delete();

	_propPicker = vtkPropPicker::New();
	_propCollection = vtkPropCollection::New();

	//_propCollection->AddItem(_aPathways);
	//_propCollection->AddItem(_aIntersections);

	_pathwayVisibility = false;
	_pointsVisibility = false;
	tubeFilter = vtkTubeFilter::New();

	_do_per_point_color = false;
}

WITPathwayViz::~WITPathwayViz()
{
	if(_pdPathways!=NULL) 
	{
		VTK_SAFE_DELETE(_pdPathways);
		VTK_SAFE_DELETE(_pdIntersections);
	}
	VTK_SAFE_DELETE(_propPicker);
	VTK_SAFE_DELETE(_propCollection);
	VTK_SAFE_DELETE(tubeFilter);
}

void WITPathwayViz::SetPathwayVisibility(bool &b)
{
	// TODO: This should be handled per-frame, we don't keep track
	// of the actor in here
}
void WITPathwayViz::SetPointsVisibility(bool &b)
{
	// TODO: This should be handled per-frame, we don't keep track
	// of the actor in here
}

/*! Generates the pathways in vtk 3d format. It also runs the other 2 pathway update functions later.
Call this function when 
*/
void WITPathwayViz::GeneratePathways(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	unsigned char rgb[3];
	bool visible = true;
	_pathwayVisibility = _pointsVisibility = true;

	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	// allocate array to hold fiber colors
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("colors");

	// cache the assignment and grouparray so that we only update the fibers which have actually changed
	_cached_assn = assn;
	_cached_group_array = groupArray;
	_is_color_cached = true;

	//Delete existing pathways
	VTK_SAFE_DELETE(_pdPathways);
	_pdPathways = vtkPolyData::New();
	_mapPathwayLuminance.clear();

	// Set up the scene pathways for the first time by recording
	// all polylines for the entire pathway database
	vtkPoints* points = vtkPoints::New();
	vtkFloatArray* pcoords = vtkFloatArray::New();	
	pcoords->SetNumberOfComponents(3);
	vtkCellArray* cells = vtkCellArray::New();

	// This takes the given database and sets the pathway actor to use these new

	// pathways.
	// Note that by default, an array has 1 component.
	// We have to change it to 3 for points
	for(int j=0,currPoint=0; j < pdb->getNumFibers(); j++) 
	{
		DTIPathway* pathway = pdb->getPathway(j);
		_mapPathwayLuminance[pathway] = DTIMath::randzeroone()*0.3 - 0.15;
		if(pathway->getNumPoints() > 0) 
		{
			// Add a cell to handle this pathway
			cells->InsertNextCell(pathway->getNumPoints());
			double *pts = new double[3*pathway->getNumPoints()];

			// jitter the pathway color so we can distinguish between fibers
			JitteredPathwayColor(pathway, groupArray[ assn[j] ].Color(), rgb);

			for(int i=0;i<pathway->getNumPoints();i++, currPoint++)
			{
				// Get a point from the pathway database
				pathway->getPoint(i,pts+3*i);
				pcoords->InsertTuple(currPoint, pts+3*i);
				cells->InsertCellPoint(currPoint);
				colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
			}
			delete []pts;
		}
	}

	// Create vtkPoints and assign pcoords as the internal data array.
	points->SetData(pcoords);
	// Create the dataset. In this case, we create a vtkPolyData
	// Assign points and cells
	_pdPathways->SetPoints(points);
	_pdPathways->SetLines(cells);
	_pdPathways->GetPointData()->SetScalars(colors);


    // release memory
	points->Delete();
	pcoords->Delete();
	cells->Delete();
	colors->Delete();

	// We have added the vertices, now add the lines
	UpdatePathwaysVisibility(helper,groupArray);

}
void WITPathwayViz::UpdatePathwaysColor(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	if (_pdPathways == NULL)
	{
		// Check added by dla 7/27/11 - was causing crash on exit with NULL pathways
		return;
	}
	unsigned char rgb[3];
	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	vtkUnsignedCharArray* colors = (vtkUnsignedCharArray* )_pdPathways->GetPointData()->GetScalars();
	int stat_idx = -1;
	if(_do_per_point_color)
	{
		/*
		TODO - Port this from Quench

		ColorMap &c = this->ColorMapPanel_()->ColorMaps()[ _per_point_info.ColorMapIndex() ];
		float stat_min = _per_point_info.Min();
		float stat_max = _per_point_info.Max();
		float scale = (c.Colors.size()-1)/(stat_max-stat_min);

		// for each fiber
		for(int j=0, point_idx = 0; j < pdb->getNumFibers(); j++) 
		{
			DTIPathway* pathway = pdb->getPathway(j);

			//Pathway is assigned to a different group so change color
			if(pathway->getNumPoints()) 
				for(int i=0;i<pathway->getNumPoints();i++)
				{
					//ComputePerPointColor(pathway, i, stat_idx, stat_min, scale, c, rgb);
					colors->SetTuple3(point_idx+i, rgb[0], rgb[1], rgb[2]);
				}
				point_idx += pathway->getNumPoints();
		}
		*/
	}
	else
	{
		// for each fiber
		for(int j=0, point_idx = 0; j < pdb->getNumFibers(); j++) 
		{
			DTIPathway* pathway = pdb->getPathway(j);

			//Pathway is assigned to a different group so change color
			if(pathway->getNumPoints() > 0 && (_is_color_cached == false || assn[j] != _cached_assn[j]) ) 
			{
				// jitter the pathway color so we can distinguish between fibers
				JitteredPathwayColor(pathway, groupArray[ assn[j] ].Color(), rgb);
				for(int i=0;i<pathway->getNumPoints();i++)
					colors->SetTuple3(point_idx+i, rgb[0],rgb[1],rgb[2]);
			}
			point_idx += pathway->getNumPoints();
		}
	}
	colors->Modified();

	//Update the visibility if either group visibility has changed
	for(int i = 0; i < (int)groupArray.size(); i++)
	{
		if( groupArray[i].Visible() ^ _cached_group_array[i].Visible())
		{
			UpdatePathwaysVisibility(helper, groupArray);
			return;
		}
	}

	// Update visibility if a fiber is assigned from a visible group to an invisible group or

	// vice versa

	for(int i = 0; i < pdb->getNumFibers(); i++)
	{
		if( groupArray[ assn[i] ].Visible()  ^  _cached_group_array[ _cached_assn[i] ].Visible())
		{
			UpdatePathwaysVisibility(helper, groupArray);
			return;
		}
	}

	_cached_assn = assn;
	_is_color_cached = true;
}
void WITPathwayViz::UpdatePathwaysVisibility(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	vtkCellArray* cells = vtkCellArray::New();
	int pt_offset	=0;
	int fiber_shown	=0;

	// for each fiber
	for(int j=0; j<pdb->getNumFibers();j++) 
	{
		DTIPathway*pathway = pdb->getPathway(j);
		int npts		   = pathway->getNumPoints();

		//If fiber group is visible add this fiber to the 3d dataset
		if(groupArray[assn[j]].Visible())
		{
			cells->InsertNextCell( npts );
			for(int i=0;i<npts;i++)
				cells->InsertCellPoint(i+pt_offset);
		}
		pt_offset+=npts;
	}
	
	_pdPathways-> SetLines(cells);
	_pdPathways-> Modified();
	 cells	   -> Delete();
		    
	// update the cache
	_cached_assn		= assn;
	_cached_group_array = groupArray;
	_is_color_cached = true;
}
/*
void WITPathwayViz::BeginPerPointColoringMode(PDBHelper& helper, PathwayGroupArray &groupArray, EventShowPerPointColorPanel &event_info)
{
	
	TODO: Port from Quench

	_per_point_info = event_info;
	_do_per_point_color = true;
	_vis_panel->SetTransparent(_do_per_point_color);
	if(_msGesture->SelectMode() == WITPathwayViz::TOUCH_MODE)
		UpdatePathwaysColor(helper, groupArray);
	else 
		SetIntersectionGeometry(helper, groupArray);
		
}
*/
void WITPathwayViz::EndPerPointColoringMode()
{
	_do_per_point_color = false;
}

void WITPathwayViz::SetActiveImageExtents(double pts[4][3], double normal[4])
{
	memcpy(_planePts, pts, sizeof(double)*4*3);
	memcpy(_planeNormal, normal, sizeof(double)*4);
}
vtkActor *WITPathwayViz::getPathwayActor()
{
	//return this->_aPathways;
	// Create the mapper 
	vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
	mapper->SetInput(_pdPathways);
	vtkActor *act = vtkActor::New();
	//act->GetProperty()->SetLineWidth(1.0);
	act->SetMapper(mapper);
	return act;
}
vtkActor *WITPathwayViz::getIntersectionActor(){
	return 0;
}
void WITPathwayViz::SetIntersectionGeometry(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	/*
	TODO: Port this to use OPCODE

	// Create a RAPID model of the image slice currently selected.
	// The model is simply a quad
	RAPID_model *planeModel = new RAPID_model;
	planeModel->BeginModel();
		planeModel->AddTri (_planePts[0], _planePts[1], _planePts[2], 0);
		planeModel->AddTri (_planePts[0], _planePts[2], _planePts[3], 1);
	planeModel->EndModel();

	// color array to store the color of the pathway intersections
	vtkPoints *newPts = vtkPoints::New();
	vtkCellArray *newCells = vtkCellArray::New();
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetName("colors");
	colors->SetNumberOfComponents(3);
	unsigned char rgb[3];

	int stat_idx = helper.PDB()->getStatisticIndex(_per_point_info.StatisticName());
	ColorMap &c = this->ColorMapPanel_()->ColorMaps()[ _per_point_info.ColorMapIndex() ];
	float stat_min = _per_point_info.Min();
	float stat_max = _per_point_info.Max();
	float scale = (c.Colors.size()-1)/(stat_max-stat_min);

	// Now intersect the RAPID model with each pathway in the scene, and record any intersection points.
	for (int i = 0; i < helper.PDB()->getNumFibers(); i++) 
	{
		DTIPathway *pathway = helper.PDB()->getPathway(i);
		bool selected = helper.Assignment()[i] == helper.Assignment().SelectedGroup();
		if (!groupArray[ helper.Assignment()[i] ].Visible())
			continue;

		RAPID_Collide (ZERO_ROTATION, ZERO_TRANSLATION, planeModel,
			ZERO_ROTATION, ZERO_TRANSLATION, pathway->getRAPIDModel(), RAPID_ALL_CONTACTS);

		for(int contact=0; contact<RAPID_num_contacts; contact++)
		{
			Vector3<double> p0;
			Vector3<double> p1;
			int index1 = RAPID_contact[contact].id2-1;
			int index2 = index1+1;
			pathway->getPoint (index1, p0);
			pathway->getPoint (index2, p1);
			double s1 = DTIMath::computeLinePlaneIntersection (p0, p1, _planeNormal, _planePts[0]);
			// Now use s1 to figure out the actual point, and create a point cell.
			Vector3<double> intersectionPoint = (p1-p0)*s1 + p0;

			newPts->InsertNextPoint (intersectionPoint.v);
			newCells->InsertNextCell (1);
			newCells->InsertCellPoint (newPts->GetNumberOfPoints()-1);

			if(_do_per_point_color && -1 != stat_idx )
				ComputePerPointColor(pathway, i, stat_idx, stat_min, scale, c, rgb);
			else// jitter the pathway color so we can distinguish between fibers
				JitteredPathwayColor(pathway, groupArray[ helper.Assignment()[i] ].Color(), rgb);

			colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
		}
	}

	// update data and release memory
	_pdIntersections->SetPoints (newPts);
	newPts->Delete();
	_pdIntersections->SetVerts (newCells);
	newCells->Delete();
	_pdIntersections->GetPointData()->SetScalars (colors);
	_pdIntersections->Modified();
	colors->Delete();
	//	_vis_panel->Update(groupArray, helper.Assignment());
	NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));

	_renderer->GetRenderWindow()->Render();
	delete planeModel;

	*/
}
void WITPathwayViz::IncreaseLineWidth(int amount)
{
	// TODO: This should be implemented per frame
}

/*

TODO: Portfrom Quench
void WITPathwayViz::ComputePerPointColor(DTIPathway *pathway, int idx, int stat_idx, float stat_min, float inv_range, ColorMap &c, unsigned char rgb[3])
{
	float stat = pathway->getPointStatistic(idx,stat_idx);
					
	// get the color index
	int k = (stat-stat_min)*inv_range;
					
	//check if its in range
	if (k <= 0) k = 1;
	if (k >= (int)c.Colors.size()) k = (int)c.Colors.size()-1;

	Coloruc col = c.Colors[k];
	rgb[0]=col.r; rgb[1]=col.g; rgb[2]=col.b; 
}
*/
void WITPathwayViz::JitteredPathwayColor(DTIPathway* t, Colord &col, unsigned char rgb[3])
{
	// See if the pathway is within any of the groups
	double vRand = 0;
	// 0 is the default pathway group
	if(!_mapPathwayLuminance.empty()) 
	{
		map<DTIPathway*, double>::iterator fiber  = _mapPathwayLuminance.find(t);
		if(fiber != _mapPathwayLuminance.end())
			vRand = (*fiber).second;
	}

	// jitter the hue
	double hsv[3],drgb[3] = {col.r, col.g, col.b};
	vtkMath::RGBToHSV(drgb,hsv);
	hsv[2]+=vRand; hsv[2] = max(.0,min(1.,hsv[2]));
	vtkMath::HSVToRGB(hsv,drgb);	

	for(int i = 0; i < 3; i++)
		rgb[i]= (unsigned char) floor(255 * drgb[i]);
}

void WITPathwayViz::LoadAndReplaceDistanceMatrix(istream &matrixStream)
{
#if 0
	delete _DistanceMatrix;
	_DistanceMatrix = NULL;
	if (matrixStream) 
	{
		cerr << "Loading distance matrix from file..." << endl;
		_DistanceMatrix = DTIPathDistanceMatrix::loadDistanceMatrix (matrixStream);
	}
#endif
}