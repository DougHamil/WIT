/***********************************************************************
* AUTHOR: Anthony Sherbondy
*   FILE: qVolumeViz.cpp
*   DATE: Fri Oct 24 07:53:49 2008
*  DESCR: 
***********************************************************************/
#include "WITVolumeViz.h"
#include "vtkImageData.h"
#include "vtkImageActor.h"
#include "vtkWindowLevelLookupTable.h"
#include "vtkColorTransferFunction.h"
#include "vtkMatrix4x4.h"
#include "vtkImageMapToColors.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkTextActor.h"
#include "vtkRenderer.h"
#include <util/io_utils.h>
#include "vtkCamera.h"
#include <vtkImageSlice.h>
#include <vtkRendererCollection.h>
//#include "Quench.h"
#include "vtkRenderWindow.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include <vtkAssemblyPath.h>


WITVolumeViz::WITVolumeViz()
{
	_img=0;
	_vol=NULL;
	_pdBorder=NULL;
	_nActiveImage=(DTISceneActorID)-1;

	// create a look up table to adjust the brightness/contrast of the background image
	_lutBW = vtkWindowLevelLookupTable::New();
	_lutColor = vtkColorTransferFunction::New();
	_lutColor->SetColorSpaceToRGB();
	_lutColor->AddRGBSegment  	(0,
		0,0,0,
		1,
		1,1,1);
	_lutBW->SetValueRange (0, 1);
	_lutBW->SetHueRange (0, 1);
	_lutBW->SetAlphaRange (0, 1);

	_pdBorder = vtkPolyData::New();
	vtkPoints *pts = vtkPoints::New();
	pts->SetNumberOfPoints(4);
	_pdBorder->SetPoints(pts);
	pts->Delete();

	// create a border actor used to highlight a particular image plane
	vtkCellArray *border = vtkCellArray::New();
	border->InsertNextCell(5);
	border->InsertCellPoint(0);
	border->InsertCellPoint(1);
	border->InsertCellPoint(2);
	border->InsertCellPoint(3);
	border->InsertCellPoint(0);
	_pdBorder->SetLines(border);  
	border->Delete();
}

void WITVolumeViz::RegisterRenderer(vtkRenderer *renderer)
{
	vtkRenderWindow *win = renderer->GetRenderWindow();

	ActorSet *actors = 0;

	// Is this window already registered?
	actors = this->windowToActorSet.find(win) != this->windowToActorSet.end() ? this->windowToActorSet.find(win)->second : generateNewActorSet(win);

	// If actors is zero, that means the renderer has no render window
	if(actors == 0)
		return;

	// Add the actors to the renderer
	renderer->AddActor(actors->border);
	renderer->AddActor(actors->sag);
	renderer->AddActor(actors->cor);
	renderer->AddActor(actors->axial);
	renderer->AddActor(actors->text);
}

WITVolumeViz::ActorSet *WITVolumeViz::generateNewActorSet(vtkRenderWindow *win)
{
	if(!win)
		return 0;

	ActorSet* as = new ActorSet();
	as->sag = vtkImageActor::New();
	as->axial = vtkImageActor::New();
	as->cor = vtkImageActor::New();
	
	as->text = vtkTextActor::New();

	// Build border actor/mapper
	as->border = vtkActor::New();
	as->borderMapper = vtkPolyDataMapper::New();
	as->borderMapper->SetInput(this->_pdBorder);
	as->border->SetMapper(as->borderMapper);
	as->border->GetProperty()->SetColor(1,0,0);
	as->border->GetProperty()->SetLineWidth(5);

	// used to see if we clicked on a image plane
	as->propPicker = vtkPropPicker::New();
	as->propCollection = vtkPropCollection::New();

	// add the image planes and border to the actor picker
	as->propCollection->AddItem(as->axial);
	as->propCollection->AddItem(as->sag);
	as->propCollection->AddItem(as->cor);
	as->propCollection->AddItem(as->border);

	// Store it in our hashmap
	this->windowToActorSet[win] = as;

	updateActors();

	return as;
}
void WITVolumeViz::setSliceActorsToColors(vtkImageMapToColors* sag, vtkImageMapToColors* axial, vtkImageMapToColors* cor)
{
	for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
		it != this->windowToActorSet.end(); it++)
	{
		
		ActorSet *as = it->second;
		as->sag->SetInput(sag->GetOutput());
		as->cor->SetInput(cor->GetOutput());
		as->axial->SetInput(axial->GetOutput());
	}
}

void WITVolumeViz::updateUserMatrices()
{
	if(_vol) {
		vtkMatrix4x4 *mx = vtkMatrix4x4::New();
		mx->DeepCopy (_vol->getTransformMatrix());

		// Apply user matrix to each registered actor
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
		
			ActorSet *as = it->second;
			as->sag->SetUserMatrix(mx);
			as->cor->SetUserMatrix(mx);
			as->axial->SetUserMatrix(mx);
		}

		mx->Delete();
	}
}

WITVolumeViz::~WITVolumeViz()
{
	// release memory
	//_overlays.clear();
	VTK_SAFE_DELETE(_img);
	VTK_SAFE_DELETE(_lutBW);
	VTK_SAFE_DELETE(_lutColor);
	VTK_SAFE_DELETE(_pdBorder);
}

void WITVolumeViz::AddVolume(DTIScalarVolume *vol)
{
	/*
	//vol->
	// add a corresponding overlay to the image
	Overlay *o = new Overlay(_renderer, vol, 0, 1, 0);
	POverlay overlay( o );
	_overlays.push_back(overlay);
	
	// compute the range of left and right values
	float left,right,min,max;
	o->Range(left,right,min,max);
	*/
	float _min = vol->getCalcMinVal(); float _max = vol->getCalcMaxVal();
	float _left = _min+0.0*(_max-_min); float _right = _max-0.0*(_max-_min);
	// set the volume
	SetVolume(vol, _left, _right);

}

void WITVolumeViz::updateActors()
{
	if(!_vol)
		return;

	this->updateUserMatrices();

	uint dim[4];
	double voxSize[3];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	_vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
	
	VTK_SAFE_DELETE(_img);
	_img = vtkImageData::New();
	_img->SetScalarTypeToFloat();
	_img->SetNumberOfScalarComponents(dim[3]); 
	_img->SetDimensions (dim[0], dim[1], dim[2]);
	_img->SetSpacing (1,1,1);
	if (dim[3] == 1) 
		_img->SetScalarTypeToFloat();
	else 
		_img->SetScalarTypeToUnsignedChar();
	_img->AllocateScalars();

	vtkScalarsToColors *lut = _lutBW;
	if (dim[3] == 1) // more common case
	{
		// create the sagital actor
		vtkImageMapToColors *sagittalColors = vtkImageMapToColors::New();
		sagittalColors->SetInput(_img);
		sagittalColors->SetLookupTable(lut);

		// create the axial actor
		vtkImageMapToColors *axialColors = vtkImageMapToColors::New();
		axialColors->SetInput(_img);
		axialColors->SetLookupTable(lut);

		// create the coronal actor
		vtkImageMapToColors *coronalColors = vtkImageMapToColors::New();
		coronalColors->SetInput(_img);
		coronalColors->SetLookupTable(lut);

		// Apply the color map to all registered actors
		this->setSliceActorsToColors(sagittalColors, axialColors, coronalColors);
		sagittalColors->Delete();
		axialColors->Delete();
		coronalColors->Delete();
	}
	else {
		// Apply user matrix to each registered actor
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
		
			ActorSet *as = it->second;
			as->sag->SetInput(_img);
			as->axial->SetInput(_img);
			as->cor->SetInput(_img);
		}
	}

	// Find the local pos based on world pos
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);

	sagExtent[0] = lPos[0];		sagExtent[1] = lPos[0];			sagExtent[2] = 0; 
	sagExtent[3] = dim[1]-1;	sagExtent[4] = 0;				sagExtent[5] = dim[2] - 1;

	axialExtent[0] = 0;			axialExtent[1] = dim[0] - 1;	axialExtent[2] = 0;
	axialExtent[3] = dim[1]-1;	axialExtent[4] = lPos[2];		axialExtent[5] = lPos[2];

	corExtent[0] = 0;			corExtent[1] = dim[0]-1;		corExtent[2] = lPos[1];
	corExtent[3] = lPos[1];		corExtent[4] = 0;				corExtent[5] = dim[2] - 1;

	// set the extent to which the image should be mapped
	// Apply user matrix to each registered actor
	for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
		it != this->windowToActorSet.end(); it++)
	{
		
		ActorSet *as = it->second;
		as->sag->SetDisplayExtent(sagExtent);
		as->axial->SetDisplayExtent(axialExtent);
		as->cor->SetDisplayExtent(corExtent);

		as->sag->Modified();
		as->cor->Modified();
		as->axial->Modified();

		// set the position in the as->text text actor
		as->text->GetTextProperty()->SetFontSize(16);
		as->text->GetTextProperty()->SetFontFamilyToArial();
		as->text->GetTextProperty()->SetJustificationToLeft();
		as->text->GetPositionCoordinate()->SetValue(2,0);
		//as->text->GetTextProperty()->BoldOn();
		//as->text->GetTextProperty()->ItalicOn();
		as->text->GetTextProperty()->SetColor(0,0,0);
		as->text->Modified();
	}
}


//Set the vtk volume for display
void WITVolumeViz::SetVolume(DTIScalarVolume *vol, float left, float right)
{
	//Volume is being set for the first time so display the border

	const DTIScalarVolume* oldVol = _vol;
	_vol = vol;

	// use the brightness and contrast by using the left and right variables
	_lutBW->SetTableRange (left, right);
	double window = right - left;
	double level = left + window/2.0;
	// adjust the range
	_lutBW->SetWindow(window);
	// adjust the midpoint
	_lutBW->SetLevel(level);
	_lutBW->Modified();

	this->updateActors();
	

	uint dim[4];
	double voxSize[3];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	_vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

	if (oldVol == NULL) 
	{
		// Start in center of volume
		double localPos[4]={dim[0]/2, dim[1]/2, dim[2]/2, 1}; 
		LocalToWorld(_vol->getTransformMatrix(), localPos, _vPos);
		
	}

	_img->Modified();

	if (dim[3] == 1) // more common case
	{
		// copy the background image data to a cache(_img)
		float *dataPtr = _vol->getDataPointer();
		float *destinationPtr = static_cast<float *>(_img->GetScalarPointer());  
		memcpy (destinationPtr, dataPtr, dim[0]*dim[1]*dim[2]*dim[3]*sizeof(float));
	}
	else {
		for(uint k=0;k<dim[2];k++) {
			for(uint j=0;j<dim[1];j++) {
				for(uint i=0;i<dim[0];i++){
					for (uint c = 0; c<dim[3];c++) {
						float f = _vol->getScalar(i,j,k,c)*255;
						_img->SetScalarComponentFromFloat (i,j,k,c,f);
					}
				}
			}
		}
	}

	SetPosition(_vPos);

	// check to see if this is the very first image that has been loaded. 
	// if so set the camera to a default position and orientation
	if(_nActiveImage == -1)
	{
		DTISceneActorID n = DTI_ACTOR_CORONAL_TOMO;
		SetActiveImage(n);

		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			vtkRenderWindow *win = it->first;

			vtkRendererCollection *rends = win->GetRenderers();

			rends->InitTraversal();
			while(vtkRenderer *rend = rends->GetNextItem())
			{
				this->SetCameraToDefault(rend);
			}
		}
	}
}

void WITVolumeViz::SetCameraToDefault(vtkRenderer *rend)
{
	if(this->windowToActorSet.find(rend->GetRenderWindow()) == this->windowToActorSet.end())
		return;

	ActorSet *as = this->windowToActorSet.find(rend->GetRenderWindow())->second;
	vtkCamera *camera = rend->GetActiveCamera();
	vtkMatrix4x4 *mx = as->cor->GetUserMatrix();
	unsigned int dim[3];
	_vol->getDimension(dim[0], dim[1], dim[2]);
	const double imageCenter[4] = {dim[0]/2.0, dim[1]/2.0, dim[2] / 2.0, 1};
	const double camP[4] = {dim[0]/2.0, 4*dim[1], dim[2] / 2.0, 1};
	double transformedCenter[4];
	double transformedCamP[4];

	mx->MultiplyPoint(imageCenter, transformedCenter);
	mx->MultiplyPoint(camP, transformedCamP);

	camera->SetViewUp (0, 0, 1);
	camera->SetPosition (transformedCamP[0], transformedCamP[1],  transformedCamP[2]);
	camera->SetFocalPoint (transformedCenter[0],transformedCenter[1],transformedCenter[2]);
	camera->ComputeViewPlaneNormal();
	camera->Modified();
}

///Display border around the selected image plane
void WITVolumeViz::DisplayBorder()
{
	if(_nActiveImage>=0 && _nActiveImage<3) 
	{
		// Position the slice
		double voxSize[3];
		uint dim[4];
		_vol->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
		_vol->getDimension (dim[0], dim[1], dim[2], dim[3]);
		DTIVoxel lPos = DTIVoxel(3);
		WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
		double p[3]={lPos[0], lPos[1], lPos[2]}, m[3] = {dim[0]-1, dim[1]-1, dim[2]-1};


		vtkPoints *pts = _pdBorder->GetPoints();

		// specify the 4 corners of the border depending on which image plane has been selected
		switch(_nActiveImage) 
		{
		case DTI_ACTOR_SAGITTAL_TOMO:
			pts->SetPoint(0, p[0],0,0);
			pts->SetPoint(1, p[0],0,m[2]);
			pts->SetPoint(2, p[0],m[1],m[2]);
			pts->SetPoint(3, p[0],m[1],0);
			break;
		case DTI_ACTOR_AXIAL_TOMO:
			pts->SetPoint(0, 0,0,p[2]);
			pts->SetPoint(1, m[0],0,p[2]);
			pts->SetPoint(2, m[0],m[1],p[2]);
			pts->SetPoint(3, 0,m[1],p[2]);
			break;
		case DTI_ACTOR_CORONAL_TOMO:
			pts->SetPoint(0, 0,p[1],0);
			pts->SetPoint(1, m[0],p[1],0);
			pts->SetPoint(2, m[0],p[1],m[2]);
			pts->SetPoint(3, 0,p[1],m[2]);
			break;
		}
		_pdBorder->Modified();
	
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->border->SetUserMatrix(as->cor->GetUserMatrix());
			as->border->SetVisibility(true);
			as->border->Modified();
		}
	} 
	else 
	{
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->border->SetUserMatrix(as->cor->GetUserMatrix());
			as->border->SetVisibility(false);
			as->border->Modified();
		}
	}
}

void WITVolumeViz::SetPosition(Vector3d &v)
{
	uint dim[4];
	double voxSize[3];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	_vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), v, dim, lPos);

	sagExtent[0] = lPos[0];		sagExtent[1] = lPos[0];			sagExtent[2] = 0; 
	sagExtent[3] = dim[1]-1;	sagExtent[4] = 0;				sagExtent[5] = dim[2] - 1;

	axialExtent[0] = 0;			axialExtent[1] = dim[0] - 1;	axialExtent[2] = 0;
	axialExtent[3] = dim[1]-1;	axialExtent[4] = lPos[2];		axialExtent[5] = lPos[2];

	corExtent[0] = 0;			corExtent[1] = dim[0]-1;		corExtent[2] = lPos[1];
	corExtent[3] = lPos[1];		corExtent[4] = 0;				corExtent[5] = dim[2] - 1;

	// do a lazy update on the image planes that are mapped to the image actors
	if(v[0] != _vPos[0]) 
	{
		_vPos[0] = v[0];
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->sag->SetDisplayExtent(sagExtent);
			as->sag->Modified();
		}
	}

	if(v[1] != _vPos[1]) 
	{
		_vPos[1] = v[1];
		// Update Coronal position
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->cor->SetDisplayExtent(corExtent);
			as->cor->Modified();
		}
	}

	if(v[2] != _vPos[2]) 
	{
		_vPos[2] = v[2];
		// Update Coronal position
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->axial->SetDisplayExtent(axialExtent);
			as->axial->Modified();
		}
	}

	double flPos[3] = {lPos[0], lPos[1], lPos[2]};
	for(int i=0; i<3; i++) { flPos[i]*=voxSize[i]; } 
//	for(int i = 0; i < (int)_overlays.size(); i++)
//		_overlays[i]->SetPosition(_vPos, flPos);

	DisplayBorder();  
	
	// update the text of the position actor to show the new position in ACPC space
	char spos[100]; sprintf(spos, "Position: %.1f, %.1f, %.1f",_vPos[0],_vPos[1],_vPos[2]);

	for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
		it != this->windowToActorSet.end(); it++)
	{
		ActorSet *as = it->second;
		as->text->SetInput(spos);
	}

	//for(unsigned i = 0; i < _overlays.size(); i++)
	//	_overlays[i]->SetPosition(v, lPos);
}

DTIScalarVolume* WITVolumeViz::GetActiveVolume()
{
  return _vol;
}

//Move the active image slice by the specified amount
void WITVolumeViz::MoveActiveImage(int amount)
{
	if(!_vol)
		return;

	uint dim[4];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);

	DTISceneActorID i = ActiveImage();
	if(i >=0 && i < 3) 
	{
		lPos[i] += amount;
		// see if the new position exceeds the dimensions
		// since both are unsigned, a value of -1 will be mapped to 0xfffffff which is greater than dim[i]
		if(lPos[i] >= dim[i])
			lPos[i] = (amount > 0) ? dim[i]-1 : 0;

		Vector3d vPos;
		LocalToWorld(_vol->getTransformMatrix(), lPos, vPos.v);
		SetPosition(vPos);
	}
}
void WITVolumeViz::SetActiveImage(DTISceneActorID &nType)
{
	_nActiveImage = nType;
	// Make sure active image's actor is visible
	switch(_nActiveImage) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->sag->SetVisibility(true);
			as->sag->Modified();
		}
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->axial->SetVisibility(true);
			as->axial->Modified();
		}
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		for(std::map<vtkRenderWindow*, ActorSet*>::iterator it = this->windowToActorSet.begin(); 
			it != this->windowToActorSet.end(); it++)
		{
			ActorSet *as = it->second;
			as->cor->SetVisibility(true);
			as->cor->Modified();
		}
		break;
	};
	// show the overlays if they were previously hidden
//	for(unsigned i = 0; i < _overlays.size(); i++)
//		_overlays[i]->SetVisible(_nActiveImage,true);
	// Show border around active image
	DisplayBorder();
}
void WITVolumeViz::GetWindowLevelParams(double &dW, double &dWMin, double &dWMax, double &dL, double &dLMin, double &dLMax)
{
	// unused, will be removed
	dL = _lutBW->GetLevel();
	dLMin = _vol->getCalcMinVal();
	dLMax = _vol->getCalcMaxVal();
	dW = _lutBW->GetWindow();
	dWMin = 0;
	dWMax = dLMax - dLMin;
}


void WITVolumeViz::GetPlaneEquation (DTISceneActorID id, double pt[4], double normal[4]) 
{
	// unused, will be removed
	double untransformedNormal[4]={0,0,0,0};
	double untransformedPoint[4]={0,0,0,1};

	untransformedPoint[id] = _vPos[0];
	untransformedNormal[id] = 1.0;
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (_vol->getTransformMatrix());
	mx->MultiplyPoint (untransformedPoint, pt);

	vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
	mx->Invert (mx, invertedMx);
	vtkMatrix4x4 *transposedMx = vtkMatrix4x4::New();
	mx->Transpose (invertedMx, transposedMx);
	mx->MultiplyPoint (untransformedNormal, normal);
	transposedMx->Delete();
	invertedMx->Delete();
	mx->Delete();
}

void WITVolumeViz::ActiveImageExtents(double transPts[4][3], double transNormal[4])
{
	// get the active image extents

	double pts[4][3];
	int displayExtent[6];
	double planeNormal[4]={0,0,0,0};
	uint dim[4];
	// get the image dimensions
	_vol->getDimension (dim[0], dim[1], dim[2], dim[3]);
	DTIVoxel lPos = DTIVoxel(3);
	// map position from ACPC to 3d world space
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
	
	memset(pts, 0, sizeof(double)*4*3);

	int i = ActiveImage();
	GetDisplayExtent ((DTISceneActorID)i,displayExtent);
	for (int j = 0; j < 4; j++)
		pts[j][i] = lPos[i];
	planeNormal[i] = 1;
	
	// get the 4 corners depending on which image plane is selected
	switch (ActiveImage()) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		pts[1][1] = displayExtent[3];
		pts[2][1] = displayExtent[3];
		pts[2][2] = displayExtent[5];
		pts[3][2] = displayExtent[5];
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		pts[1][0] = displayExtent[1];
		pts[2][0] = displayExtent[1];
		pts[2][2] = displayExtent[5];
		pts[3][2] = displayExtent[5];
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		pts[1][0] = displayExtent[1];
		pts[2][0] = displayExtent[1];
		pts[2][1] = displayExtent[3];
		pts[3][1] = displayExtent[3];
		break;
	default:
		break;
	};
	vtkMatrix4x4 *mx = GetUserMatrix();
	// map the points from 3d world space to ACPC space
	for (int i = 0; i < 4; i++) 
	{
		double fooIn[4] = {pts[i][0], pts[i][1], pts[i][2], 1};
		double fooOut[4];
		mx->MultiplyPoint (fooIn, fooOut);
		transPts[i][0] = fooOut[0];
		transPts[i][1] = fooOut[1];
		transPts[i][2] = fooOut[2];
	}
	// inverse of transpose:
	vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
	vtkMatrix4x4 *transposedMx = vtkMatrix4x4::New();
	mx->Invert (mx, invertedMx);
	mx->Transpose (invertedMx, transposedMx);
	mx->MultiplyPoint (planeNormal, transNormal);
	transposedMx->Delete();
	invertedMx->Delete();
}
void WITVolumeViz::GetDisplayExtent (DTISceneActorID id, int displayExtent[6])
{
	// get the display extent of a specific image plane
	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		memcpy(displayExtent, sagExtent ,sizeof(int)*6);
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		memcpy(displayExtent, corExtent ,sizeof(int)*6);
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		memcpy(displayExtent, axialExtent ,sizeof(int)*6);
		break;
	default:
		break;
	};
}

WITVolumeViz::ActorSet *WITVolumeViz::getActorSet(vtkRenderer *rend)
{
	vtkRenderWindow *win = rend->GetRenderWindow();
	
	return win ? this->windowToActorSet[win] : 0;
}

bool WITVolumeViz::GetVisibility(vtkRenderer *rend, DTISceneActorID id)
{
	// get the visibility of the selected image plane
	bool bVis=false;

	ActorSet *as = this->getActorSet(rend);

	if(!as)
		return false;

	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		bVis = as->sag->GetVisibility()?true:false;
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		bVis = as->cor->GetVisibility()?true:false;
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		bVis = as->axial->GetVisibility()?true:false;
		break;
	default:
		break;
	};
	return bVis;
}

void WITVolumeViz::SetVisibility(vtkRenderer *rend, DTISceneActorID id, bool bVis)
{
	// set the visibility of the specified image plane
	ActorSet *as = this->getActorSet(rend);
	if(!as)
		return;

	vtkProp *actor = 0;

	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		actor = as->sag;
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		actor = as->axial;
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		actor = as->cor;
		break;
	default:
		break;
	};

	if(!actor) return;

	if(bVis && !rend->HasViewProp(actor))
		rend->AddActor(actor);
	else if(!bVis && rend->HasViewProp(actor))
		rend->RemoveActor(actor);

	// update the visibility of the overlay too
//	for(unsigned i = 0; i < _overlays.size(); i++)
//		_overlays[i]->SetVisible(id,bVis);
	if((int)id == _nActiveImage)
	{
		if(bVis && !rend->HasViewProp(as->border))
			rend->AddActor(as->border);
		else if(!bVis && rend->HasViewProp(as->border))
			rend->RemoveActor(as->border);
	}
}

vtkMatrix4x4* WITVolumeViz::GetUserMatrix()
{
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy(_vol->getTransformMatrix());
	return mx;
}

bool WITVolumeViz::OnRightButtonUp(int x, int y)
{
	// show the overlay panel if we clicked on one of the image planes...

	/*

	TODO: Port from Quench

	_propPicker->PickProp(x, y, _renderer, _propCollection);
	vtkProp *prop = _propPicker->GetViewProp();

	 // There could be other props assigned to this picker, so 

    // make sure we picked the image actor

    vtkAssemblyPath* path = _propPicker->GetPath();

    bool validPick = false;

	if (path)

	{

		vtkCollectionSimpleIterator sit;

		path->InitTraversal(sit);

		vtkAssemblyNode *node;

		for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)

		{

			node = path->GetNextNode(sit);

			if( prop == _aAxial || prop == _aCor || prop == _aSag || prop == _aBorder)
			{
				//NotifyAllListeners( PEvent (new Event(SHOW_OVERLAY_PANEL)) );
				return true;
			}

		}

	}


	// this is for backup if the first one didnt work
	if( prop == _aAxial || prop == _aCor || prop == _aSag || prop == _aBorder)
	{
		//NotifyAllListeners( PEvent (new Event(SHOW_OVERLAY_PANEL)) );
		return true;
	}
	*/
	return false;
}


void WITVolumeViz::ComputeCurrentVolumeBounds(Vector3d &vmin, Vector3d &vmax)
{
	if(!_vol)return;
	double *transform=_vol->getTransformMatrix();
	unsigned dim[3];
	_vol->getDimension(dim[0], dim[1], dim[2]);
	for(int i = 0; i < 3; i++)
	{
		vmax[i] = transform[i*4+0]*dim[0] + transform[i*4+1]*dim[1] + transform[i*4+2]*dim[2] + transform[i*4+3];
		vmin[i] = transform[i*4+3];
		if(vmin[i] > vmax[i])
		{
			float temp = vmin[i]; vmin[i]=vmax[i], vmax[i]=temp;
		}
	}
}
bool WITVolumeViz::WorldToLocal(double *mat, double *wPos, uint *dim, DTIVoxel &lPos, bool round)
{
  bool clamped = false;
  vtkMatrix4x4 *mx = vtkMatrix4x4::New();
  mx->DeepCopy (mat);

  double worldPos[4]={wPos[0], wPos[1], wPos[2], 1}; 
  mx->Invert();
  double *localPos = mx->MultiplyDoublePoint(worldPos);
  for(int ii = 0; ii < 3; ii++)
  {
	  // Must add because of floating point errors that we might get that could be exaggerated by float
	  double very_small_value = 0.0001;
	  if (round) {
	    very_small_value += 0.5;
	  }
	  int pos = floor(localPos[ii]+very_small_value);
	  if (pos < 0) {
	    clamped = true;
	    pos = 0;
	  }
	  lPos[ii]=(uint)pos; 
	  if (lPos[ii] > dim[ii]-1) {
	    clamped = true;
	    lPos[ii] = dim[ii]-1;
	  }
  }
  mx->Delete();
  return clamped;
}
void WITVolumeViz::WorldToLocal(double *mat, double *wPos, uint *dim, double *flPos)
{
  vtkMatrix4x4 *mx = vtkMatrix4x4::New();
  mx->DeepCopy (mat);

  double worldPos[4]={wPos[0], wPos[1], wPos[2], 1}; 
  mx->Invert();
  double *localPos = mx->MultiplyDoublePoint(worldPos);
  for(int ii = 0; ii < 3; ii++)
	  flPos[ii]=localPos[ii];
	//  if (flPos[ii] > dim[ii]-1) flPos[ii] = dim[ii]-1;
  mx->Delete();
}

void WITVolumeViz::LocalToWorld(double *mat, DTIVoxel &lPos, double *wPos)
{
	double localPos[4]={lPos[0], lPos[1], lPos[2], 1}; 
	LocalToWorld(mat, localPos, wPos);
}
void WITVolumeViz::LocalToWorld(double *mat, double *lPos, double *wPos)
{
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (mat);

	double localPos[4]={lPos[0], lPos[1], lPos[2], 1}; 
	double *worldPos = mx->MultiplyDoublePoint(localPos);
	for(int ii = 0; ii < 3; ii++)
		wPos[ii]=worldPos[ii]; 
	mx->Delete();
}

void WITVolumeViz::GetCursorPosition(DTIVoxel &lPos)
{
  uint dim[4];
  _vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
}
