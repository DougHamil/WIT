/** Copyright (c) 2005-2007, The Board of Trustees of Leland Stanford
Junior University. All rights reserved. **/

#include "vtkInteractorStyleViewFrame.h"

#include <wx/filedlg.h>

#ifndef _MSC_VER
#include <GL/gl.h>
#endif

//
#include "vtkTextActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCellPicker.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkMatrix4x4.h"
#include "vtkVersion.h"
#include <vtkPropPicker.h>
#include <vtkCullerCollection.h>
#include <vtkCuller.h>
#include <vtkFrustumCoverageCuller.h>
#include <vtkTransform.h>
#include <vtkCellData.h>
#include <vtkPlane.h>
#include <vtkAppendPolyData.h>

#include "query_typedefs.h"
#include "StreamUtils.h"

#include <string.h>
#include <util/DTIPathway.h>
#include <util/DTIFilterTree.h>
#include <util/PathwayIntersectionVolume.h>
#include <util/DTIPathwaySelection.h>

#include <math.h>
#ifdef _MSC_VER
#include <hash_map>
#else
#include <ext/hash_map>
#endif
#include <limits>

#include "vtkStripper.h"
#include "vtkCellLocator.h"
#include "vtkCellLocator.h"
#include "VOICube.h"
#include "VOIMesh.h"
#include "VOISphere.h"
#include "VOILoaded.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkWorldPointPicker.h"
#include "vtkPointPicker.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "ViewCone.h"
#include <util/DTIPathwayDatabase.h>
#include <util/DTIFilterPathwayScalar.h>
#include <util/DTIPathwayIO.h>
#include <util/DTIFilterAlgorithm.h>
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCubeSource.h"
#include "vtkProp3DCollection.h"
#include "vtkAbstractPropPicker.h"
#include <util/DTIFilterCube.h>
#include "InteractorListener.h"
#include "vtkImageActor.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkColorTransferFunction.h"
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include "CameraControls.h"
#include "DTIViewFrame.h"
#include <util/DTIQueryProcessor.h>
#include <util/DTIVolumeIO.h>
#include <util/ScalarVolume.h>
#include "SystemInfoDialog.h"
#include "ButtonChoiceDialog.h"
#include "StreamUtils.h"
#include "DataStore.h"
#include "DTIPathwayVisualization.h"
#include "DTITomograms.h"
#include "SurfaceVisualization.h"

#include "wx/dir.h"

static const double __initial_VOI_size = 20.0;

vtkCxxRevisionMacro(vtkInteractorStyleViewFrame, "$Revision: 1.28 $");
vtkStandardNewMacro(vtkInteractorStyleViewFrame);

using std::cout;
using std::endl;
#ifndef _MSC_VER
using namespace __gnu_cxx;
#endif

//----------------------------------------------------------------------------
vtkInteractorStyleViewFrame::vtkInteractorStyleViewFrame()
  :vtkInteractorStyleDTI()
{
  this->_lut_level = 0.5;
  this->_lut_window = 1.0;

  this->surfaceRepresentation = 2;
  this->sceneInfo = vtkTextActor::New();
  this->pathwaysActor = NULL;
  // Setup cursor text
  //this->sceneInfo->ScaledTextOn();
  this->sceneInfo->SetDisplayPosition(2, 32);
  this->sceneInfo->GetTextProperty()->SetFontSize(18);
  this->sceneInfo->GetTextProperty()->SetFontFamilyToArial();
  this->sceneInfo->GetTextProperty()->SetJustificationToLeft();
  this->sceneInfo->GetTextProperty()->BoldOn();
  this->sceneInfo->GetTextProperty()->ItalicOn();
  this->sceneInfo->SetTextScaleModeToNone();
  // On Ubuntu, the text shadowing looks very bad...
#if __LINUX__
#else
  //this->sceneInfo->GetTextProperty()->ShadowOn();
#endif
  this->sceneInfo->GetTextProperty()->SetColor(1,1,1);

  this->current_background_id = 0;
  this->voiLabelsToggle = true;
  this->voiVisibilityToggle = true;
  this->pathwayVisibilityToggle = true;
  this->arcsVisible = false;

  this->TomoBorderActor = NULL;
  this->TomoBorderMapper = NULL;
  this->TomoBorderPolyData = NULL;
  this->selectedTomogram = AXIAL;

  this->activeSurface = NULL;

  this->currentPV = NULL;
  this->oldCamera = NULL;

  this->bSPosSlice = true;
  this->bAPosSlice = true;
  this->lineWidthSetting = 0;
  this->MotionFactor   = 10.0;
  this->activeVOI = NULL;
  this->minPathwayLength = 0;
  this->camera_controls = new CameraControls();
}

//----------------------------------------------------------------------------
vtkInteractorStyleViewFrame::~vtkInteractorStyleViewFrame()
{
  if(datastore)
	  datastore->RemoveListener(this);
  if(pathwaysActor)
    pathwaysActor->Delete();
  if(TomoBorderPolyData)
    TomoBorderPolyData->Delete();
  if(TomoBorderMapper)
    TomoBorderMapper->Delete();
  if(TomoBorderActor)
    TomoBorderActor->Delete();
}

void vtkInteractorStyleViewFrame::LoadFromXML(TiXmlElement *elem)
{
  TiXmlElement *vis = elem->FirstChildElement("Visibility");

  if(elem->Attribute("ActiveSurface"))
  {
    int index = StringToNumber<int>(elem->Attribute("ActiveSurface"));
    std::vector<SurfaceVisualization*>::const_iterator iter = datastore->GetSurfaceVisualizations()->begin();
    iter += index;
    this->SetActiveSurface((*iter));
  }

  if(elem->Attribute("ActiveVOI"))
  {
    int voiID = StringToNumber<int>(elem->Attribute("ActiveVOI"));
    std::vector<VOI*>::const_iterator iter = datastore->GetVOIList()->begin();
    for(;iter != datastore->GetVOIList()->end(); ++iter)
    {
      if((*iter)->getID() == voiID)
      {
        this->SetActiveVOI((*iter));
        break;
      }
    }
  }

  if(this->tomograms)
  {
    if(elem->Attribute("SelectedTomogram"))
      this->selectedTomogram = (AnatomicalAxis)StringToNumber<int>(elem->Attribute("SelectedTomogram"));

    // Display border on selected tomogram
    DisplayTomoBorder(this->tomograms->GetTomogramActor(selectedTomogram)->GetVisibility());
  }

  if(elem->Attribute("SurfaceRepresenation"))
  {
    this->SetSurfaceRepresentation(HEMISPHERE_BOTH, StringToNumber<int>(elem->Attribute("SurfaceRepresentation")));
  }

  TiXmlElement *cam = elem->FirstChildElement("Camera");
  if(cam)
  {
    double camPos[3] = {0,0,0};
    if(cam->Attribute("PositionX"))
      camPos[0] = StringToNumber<double>(cam->Attribute("PositionX"));
    if(cam->Attribute("PositionY"))
      camPos[1] = StringToNumber<double>(cam->Attribute("PositionY"));
    if(cam->Attribute("PositionZ"))
      camPos[2] = StringToNumber<double>(cam->Attribute("PositionZ"));
    double camFoc[3] = {0,0,0};
    if(cam->Attribute("FocusX"))
      camFoc[0] = StringToNumber<double>(cam->Attribute("FocusX"));
    if(cam->Attribute("FocusY"))
      camFoc[1] = StringToNumber<double>(cam->Attribute("FocusY"));
    if(cam->Attribute("FocusZ"))
      camFoc[2] = StringToNumber<double>(cam->Attribute("FocusZ"));
    if(cam->Attribute("Roll"))
      this->GetCurrentRenderer()->GetActiveCamera()->Roll(StringToNumber<double>(cam->Attribute("Roll")));

    this->GetCurrentRenderer()->GetActiveCamera()->SetPosition(&camPos[0]);
    this->GetCurrentRenderer()->GetActiveCamera()->SetFocalPoint(&camFoc[0]);
  }


  if(vis)
  {
    if(this->tomograms)
    {
      if(vis->Attribute("SPlane"))
        if(this->tomograms->GetTomogramActor(SAGITTAL)->GetVisibility() != StringToBool(vis->Attribute("SPlane")))
          this->ToggleTomogramVisibility(SAGITTAL);
      if(vis->Attribute("APlane"))
        if(this->tomograms->GetTomogramActor(AXIAL)->GetVisibility() != StringToBool(vis->Attribute("APlane")))
          this->ToggleTomogramVisibility(AXIAL);
      if(vis->Attribute("CPlane"))
        if(this->tomograms->GetTomogramActor(CORONAL)->GetVisibility() != StringToBool(vis->Attribute("CPlane")))
          this->ToggleTomogramVisibility(CORONAL);
    }

    if(vis->Attribute("VOI"))
      if(StringToBool(vis->Attribute("VOI")) != this->voiVisibilityToggle)
        this->ToggleVOIVisibility();

    if(vis->Attribute("Pathways"))
      if(StringToBool(vis->Attribute("Pathways")) != this->pathwayVisibilityToggle)
        this->TogglePathwaysVisibility();

    if(vis->Attribute("Arcs"))
      if(StringToBool(vis->Attribute("Arcs")) != this->arcsVisible)
        this->ToggleArcsVisibility();

    if(this->activeSurface)
    {
      if(vis->Attribute("LHemisphere"))
        if(StringToBool(vis->Attribute("LHemisphere")) != this->surfaceMapping[activeSurface]->GetVisibility(HEMISPHERE_LEFT))
          this->ToggleSurfaceVisibility(HEMISPHERE_LEFT);
      if(vis->Attribute("RHemisphere"))
        if(StringToBool(vis->Attribute("RHemisphere")) != this->surfaceMapping[activeSurface]->GetVisibility(HEMISPHERE_RIGHT))
          this->ToggleSurfaceVisibility(HEMISPHERE_RIGHT);
    }
  }

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

void vtkInteractorStyleViewFrame::SaveToXML(TiXmlElement *elem)
{
  // Save visibility flags
  TiXmlElement *vis = new TiXmlElement("Visibility");

  if(this->tomograms)
  {
    vis->SetAttribute("SPlane",
        BoolToString(this->tomograms->GetTomogramActor(SAGITTAL)->GetVisibility()).c_str());
    vis->SetAttribute("APlane",
        BoolToString(this->tomograms->GetTomogramActor(AXIAL)->GetVisibility()).c_str());
    vis->SetAttribute("CPlane",
        BoolToString(this->tomograms->GetTomogramActor(CORONAL)->GetVisibility()).c_str());
  }
  vis->SetAttribute("VOI",BoolToString(this->voiVisibilityToggle).c_str());
  vis->SetAttribute("Pathways",BoolToString(this->pathwayVisibilityToggle).c_str());
  vis->SetAttribute("Arcs",BoolToString(this->arcsVisible).c_str());

  if(this->activeSurface)
  {
    vis->SetAttribute("LHemisphere",
        BoolToString(surfaceMapping[activeSurface]->GetVisibility(HEMISPHERE_LEFT)).c_str());
    vis->SetAttribute("RHemisphere",
        BoolToString(surfaceMapping[activeSurface]->GetVisibility(HEMISPHERE_RIGHT)).c_str());
  }

  if(this->activeSurface)
  {
    int index = 0;
    std::vector<SurfaceVisualization*>::const_iterator iter;
    for(iter = datastore->GetSurfaceVisualizations()->begin(); iter != datastore->GetSurfaceVisualizations()->end();++iter)
    {
      if((*iter) == this->activeSurface)
        break;
      index++;
    }
    elem->SetAttribute("ActiveSurface",NumberToString<int>(index).c_str());
  }

  if(this->activeVOI)
  {
    elem->SetAttribute("ActiveVOI",NumberToString<int>(this->activeVOI->getID()).c_str());
  }

  elem->SetAttribute("SurfaceRepresentation",NumberToString<int>(this->surfaceRepresentation).c_str());

  elem->SetAttribute("SelectedTomogram", NumberToString<int>(this->selectedTomogram).c_str());

  TiXmlElement *cam = new TiXmlElement("Camera");
  double *camPos = this->GetCurrentRenderer()->GetActiveCamera()->GetPosition();
  vtkCamera *camera = this->GetCurrentRenderer()->GetActiveCamera();
  cam->SetAttribute("PositionX",NumberToString<double>(camPos[0]).c_str());
  cam->SetAttribute("PositionY",NumberToString<double>(camPos[1]).c_str());
  cam->SetAttribute("PositionZ",NumberToString<double>(camPos[2]).c_str());
  double *camFocus = camera->GetFocalPoint();
  cam->SetAttribute("FocusX",NumberToString<double>(camFocus[0]).c_str());
  cam->SetAttribute("FocusY",NumberToString<double>(camFocus[1]).c_str());
  cam->SetAttribute("FocusZ",NumberToString<double>(camFocus[2]).c_str());
  cam->SetAttribute("Roll",NumberToString<double>(camera->GetRoll()).c_str());

  elem->LinkEndChild(cam);

  elem->LinkEndChild(vis);
}

void vtkInteractorStyleViewFrame::SetCellLocatorFromVisibility(unsigned int vis)
{
  if(!activeSurface)
  {
    activeSurfaceCellLocator.second = NULL;
    return;
  }
  // SurfaceInstance visibility is a bitmask
  // 1 = left on
  // 2 = right on
  if(vis == 3 || vis == 0)
  {
    // Pick BOTH if you can, otherwise pick whatever is loaded
    if(activeSurface->IsHemisphereLoaded(HEMISPHERE_BOTH))
      activeSurfaceCellLocator = std::pair<Hemisphere,vtkCellLocator*>(HEMISPHERE_BOTH,activeSurface->GetCellLocator(HEMISPHERE_BOTH));
    else if(activeSurface->IsHemisphereLoaded(HEMISPHERE_LEFT))
      activeSurfaceCellLocator = std::pair<Hemisphere,vtkCellLocator*>(HEMISPHERE_LEFT,activeSurface->GetCellLocator(HEMISPHERE_LEFT));
    else if(activeSurface->IsHemisphereLoaded(HEMISPHERE_RIGHT))
      activeSurfaceCellLocator = std::pair<Hemisphere,vtkCellLocator*>(HEMISPHERE_RIGHT,activeSurface->GetCellLocator(HEMISPHERE_RIGHT));
    else
      activeSurfaceCellLocator = std::pair<Hemisphere, vtkCellLocator*>(HEMISPHERE_BOTH, (vtkCellLocator*)NULL);
  }
  else if(vis == 1)
    activeSurfaceCellLocator = std::pair<Hemisphere,vtkCellLocator*>(HEMISPHERE_LEFT,activeSurface->GetCellLocator(HEMISPHERE_LEFT));
  else if(vis == 2)
    activeSurfaceCellLocator = std::pair<Hemisphere,vtkCellLocator*>(HEMISPHERE_RIGHT,activeSurface->GetCellLocator(HEMISPHERE_RIGHT));
}

void vtkInteractorStyleViewFrame::UpdateSceneInfo()
{
  std::stringstream info;

  if(!datastore)
  {
    info << "No Datastore" << endl;
    goto end;
  }

  if(datastore->GetDatabase() && datastore->GetPathwayVisualization())
  {
    int pathShown = datastore->GetPathwayVisualization()->GetShownPathwaysCount();
    int total = datastore->GetDatabase()->getNumFibers();
    double percent = ((double)pathShown/(double)total) * 100.0;
    info << pathShown << "/" << total << " (" << percent << "%)" << endl;
  }

  if(datastore->IsBackgroundLoaded())
  {
    ScalarVolume *scalarVol = this->datastore->GetCurrentBackground();
    if(!scalarVol)
      goto end;

    int matrixCode = scalarVol->getSformCode() ? scalarVol->getSformCode() : scalarVol->getQformCode();

    switch(matrixCode)
    {
    case 1:
      info << "Coords: Scanner" << endl;
      break;
    case 2:
      info << "Coords: Anat" << endl;
      break;
    case 3:
      info << "Coords: ACPC/Talairach" << endl;
      break;
    default:
      info << "Coords: Unknown" << endl;
      break;
    }

    double transformedPos[4] = {0,0,0,1};
    double cursorPos[4] = { this->tomograms->GetSlice(SAGITTAL),
                            this->tomograms->GetSlice(CORONAL),
                            this->tomograms->GetSlice(AXIAL),
                            1};

    vtkMatrix4x4 *transformMx = tomograms->GetTomogramActor(SAGITTAL)->GetUserMatrix();
    if (transformMx)
      transformMx->MultiplyPoint(cursorPos, transformedPos);

    info << "(" << transformedPos[0] << " , " << transformedPos[1] << " , " << transformedPos[2] << ")" << endl;

    info << datastore->GetCurrentBackgroundPath();
  }

  end:
  this->sceneInfo->SetInput(info.str().c_str());
  this->sceneInfo->Modified();
  return;
}


//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnMouseMove()
{
  if(!this->frame->HasFocus())
    return;
#if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;

  if(rwi->GetRenderWindow()->GetAAFrames() > 0) {
    rwi->GetRenderWindow()->SetAAFrames(0);
  }
#endif

  vtkInteractorStyleDTI::OnMouseMove();

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->State)
  {
  case VTKIS_MOVEVOI:
    this->FindPokedRenderer(x, y);
    this->MoveVOI();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;
  case VTKIS_PAINTSURFACE:
    this->FindPokedRenderer(x, y);
    //this->PaintSurface();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;
  case VTKIS_PICKCELL:
    break;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnLeftButtonDown()
{
  vtkInteractorStyleDTI::OnLeftButtonDown();

this->FindPokedRenderer(0,0);
  if(this->GetCurrentRenderer() == NULL)
    return;

  if (!this->Interactor->GetShiftKey() && this->Interactor->GetControlKey())
  {
      if (this->datastore->GetDatabase() == NULL)
        return;

      if(this->datastore->GetActiveVOI())
        this->State = VTKIS_MOVEVOI;

  }
  else if(this->Interactor->GetShiftKey() && this->Interactor->GetControlKey())
  {
    if(!this->datastore->GetDatabase())
      return;

    // There must be an active surface and it must be visible
    if(this->activeSurface && this->surfaceMapping[activeSurface]->GetVisibility())
    {
      //this->State = VTKIS_PAINTSURFACE;
      this->State = VTKIS_PICKCELL;
    }
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnLeftButtonUp()
{
  #if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render

  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
  }
#endif

  vtkInteractorStyleDTI::OnLeftButtonUp();

  switch (this->State)
  {
  case VTKIS_MOVEVOI:
    this->State = VTKIS_NONE;
    break;
  case VTKIS_PAINTSURFACE:
    this->State = VTKIS_NONE;
    break;
  case VTKIS_PICKCELL:
    this->State = VTKIS_NONE;
    break;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnMiddleButtonDown()
{
  vtkInteractorStyleDTI::OnMiddleButtonDown();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnMiddleButtonUp()
{
  vtkInteractorStyleDTI::OnMiddleButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnRightButtonDown()
{
  vtkInteractorStyleDTI::OnRightButtonDown();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnRightButtonUp()
{
  vtkInteractorStyleDTI::OnRightButtonUp();
}



void vtkInteractorStyleViewFrame::SetTemporaryCamera(vtkCamera *cam)
{
  if(cam)
  {
    // Store the old camera for later reference
    oldCamera = this->CurrentRenderer->GetActiveCamera();
    this->CurrentRenderer->SetActiveCamera(cam);
  }
  else
  {
    if(oldCamera != NULL && this->CurrentRenderer->GetActiveCamera() != oldCamera)
    {
      oldCamera->Azimuth(90);
      this->CurrentRenderer->SetActiveCamera(oldCamera);
    }
    oldCamera = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::Rotate()
{
  if (this->CurrentRenderer == NULL)
  {
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = (double)dx * delta_azimuth * this->MotionFactor;
  double ryf = (double)dy * delta_elevation * this->MotionFactor;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Azimuth(rxf);
  camera->Elevation(ryf);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  if (rwi->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }

  if(oldCamera)
    datastore->RefreshAll(this->frame);
  else
    this->frame->OnRefresh(this->frame);
}


//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::Spin()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double *center = this->CurrentRenderer->GetCenter();

  double newAngle =
    atan2((double)rwi->GetEventPosition()[1] - (double)center[1],
          (double)rwi->GetEventPosition()[0] - (double)center[0]);

  double oldAngle =
    atan2((double)rwi->GetLastEventPosition()[1] - (double)center[1],
          (double)rwi->GetLastEventPosition()[0] - (double)center[0]);

  newAngle *= vtkMath::RadiansToDegrees();
  oldAngle *= vtkMath::RadiansToDegrees();

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();



  if(oldCamera)
    datastore->RefreshAll(this->frame);
  else
    this->frame->OnRefresh(this->frame);

}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::Pan()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];

  // Calculate the focal depth since we'll be using it a lot

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetFocalPoint(viewFocus);
  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2],
                              viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld((double)rwi->GetEventPosition()[0],
                              (double)rwi->GetEventPosition()[1],
                              focalDepth,
                              newPickPoint);

  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  this->ComputeDisplayToWorld((double)rwi->GetLastEventPosition()[0],
                              (double)rwi->GetLastEventPosition()[1],
                              focalDepth,
                              oldPickPoint);

  // Camera motion is reversed

  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];

  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->SetFocalPoint(motionVector[0] + viewFocus[0],
                        motionVector[1] + viewFocus[1],
                        motionVector[2] + viewFocus[2]);

  camera->SetPosition(motionVector[0] + viewPoint[0],
                      motionVector[1] + viewPoint[1],
                      motionVector[2] + viewPoint[2]);

  if (rwi->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }

  if(oldCamera)
    datastore->RefreshAll(this->frame);
  else
    this->frame->OnRefresh(this->frame);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::Dolly()
{
  if (this->CurrentRenderer == NULL)
    return;

  vtkRenderWindowInteractor *rwi = this->Interactor;
  double *center = this->CurrentRenderer->GetCenter();
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
  double dyf = this->MotionFactor * (double)(dy) / (double)(center[1]);
  this->Dolly(pow((double)1.1, dyf));
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::Dolly(double factor)
{
  if (this->CurrentRenderer == NULL)
    return;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  if (camera->GetParallelProjection())
  {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
  }
  else
  {
    camera->Dolly(factor);
    if(this->AutoAdjustCameraClippingRange)
    {
      this->CurrentRenderer->ResetCameraClippingRange();
    }
  }

  if (this->Interactor->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }



  if(oldCamera)
    datastore->RefreshAll(this->frame);
  else
    this->frame->OnRefresh(this->frame);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}

void
vtkInteractorStyleViewFrame::MoveCurrentTomogram(int amount)
{
  if(!datastore)
    return;

  tomograms->MoveTomogramByAmount(this->selectedTomogram, amount);

  /*
  DisplayTomoBorder(true);

  if(this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
  */

  this->UpdateSceneInfo();
}

void vtkInteractorStyleViewFrame::OnKeyPress()
{

  char *keysym = this->Interactor->GetKeySym();

}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnChar()
{

  vtkRenderWindowInteractor *rwi = this->Interactor;

  std::vector<VOI*>::const_iterator iter;
  float bg[3];

  this->FindPokedRenderer(0,0);

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  double surfPos[3] = {0,0,0};
  double move = 1;

  /*
  if(this->surfaceActor)
    this->datastore->GetSurfaceVisualization()->GetTransform()->GetPosition(surfPos);
    */

  switch (this->Interactor->GetKeyCode())
  {
    case 'o':
      if(this->GetActiveSurface())
      {

      }
    break;
    case ',':
      MoveCurrentTomogram (-1);
      break;
    case '.':
      MoveCurrentTomogram (+1);
      break;

    case 'a':
    case 'A':
      if (datastore->IsBackgroundLoaded()) {
        if (Interactor->GetShiftKey()) {
          ToggleTomogramVisibility(AXIAL);
        }
        else {
          SelectTomogram(AXIAL);
        }
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 's':
    case 'S':
      if (datastore->IsBackgroundLoaded()) {
        if (Interactor->GetShiftKey()) {
          ToggleTomogramVisibility (SAGITTAL);
        }
        else {
          SelectTomogram(SAGITTAL);
        }
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'c':
    case 'C':
      if (datastore->IsBackgroundLoaded()) {
        if (Interactor->GetShiftKey()) {
          ToggleTomogramVisibility(CORONAL);
        }
        else {
          SelectTomogram(CORONAL);
        }
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'v':
    case 'V':
      if (this->datastore->GetDatabase()) {
        if (Interactor->GetShiftKey()) {
          ToggleVOIVisibility();
        }
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'p':
    case 'P':
      if (this->datastore->GetDatabase()) {
        if (Interactor->GetShiftKey()) {
          TogglePathwaysVisibility();
        }
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
#if 0
    case 'm':
      this->ActorSelected = DTI_ACTOR_SURFACE;
      DisplayTomoBorder(false);
      this->EventCallbackCommand->SetAbortFlag(1);
      this->frame->OnRefresh(this->frame);
      break;
    case 'g':
    case 'G':
      // ghosting mode
      if (!GetCurrentSurfaceActor())
	  {
		m_ghosting = !m_ghosting;
		break;
      }
      if (m_ghosting)
	  {
		GetCurrentSurfaceActor()->SetVisibility (true);
		SetGhostVisibility(false);
      }
      else {
  // turning ON ghosting
  GetCurrentSurfaceActor()->SetVisibility (false);
  SetGhostVisibility(true);
      }

      m_ghosting = !(m_ghosting);
      this->EventCallbackCommand->SetAbortFlag(1);
      this->StopState();
      break;
#endif
    case 't':
    case 'T':
      // Toggle the VOI labels on/off

      // We have to remove the actor from the renderer, as the actor is shared between
      // all views
      for(iter=datastore->GetVOIList()->begin(); iter!=datastore->GetVOIList()->end();iter++)
      {
        VOI *voi = *iter;

        if(voiLabelsToggle)
          this->CurrentRenderer->RemoveActor2D(voi->getTextActor());
        else if(voiVisibilityToggle)
          this->CurrentRenderer->AddActor2D((vtkProp*)voi->getTextActor());

      }

      voiLabelsToggle = !(voiLabelsToggle);

      this->frame->OnRefresh(this->frame);
      break;
    case 'b':
    case 'B':
      //this->CurrentRenderer->SetBackground(bg[0],bg[1],bg[2]);
      //this->frame->OnRefresh(this->frame);
      break;
    case 'k':
    case 'K':
      if(rwi->GetRenderWindow()->GetAAFrames() > 0)
  rwi->GetRenderWindow()->SetAAFrames(0);
      else
  rwi->GetRenderWindow()->SetAAFrames(10);
      this->frame->OnRefresh(this->frame);
      break;
    case '[':
      if (datastore->GetCurrentBackground())
      {
        unsigned int dim[3];
        unsigned int dummy;
        double voxSize[3];
        datastore->GetCurrentBackground()->getDimension (dim[0], dim[1], dim[2], dummy);
        datastore->GetCurrentBackground()->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
        vtkMatrix4x4 *mx = this->tomograms->GetTomogramActor(CORONAL)->GetUserMatrix();
        this->camera_controls->NextPosition();
        this->camera_controls->Update(camera, dim, voxSize, mx);
        this->CurrentRenderer->ResetCameraClippingRange();
      }
      this->frame->OnRefresh(this->frame);
      break;
    case 'w':
    case 'W':
      this->NextLineWidth();
      this->frame->OnRefresh(this->frame);
      break;
    default:
      break;
    }
}

void vtkInteractorStyleViewFrame::DisplayTomoBorder(bool on)
{
  if (!datastore->GetCurrentBackground()) {
    return;
  }

  if (!this->tomograms->GetTomogramActor(selectedTomogram)->GetVisibility()) {
    on = false;
  }
  //on = false;
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
        this->Interactor->GetEventPosition()[1]);

  if(this->TomoBorderActor == NULL && this->CurrentRenderer!=NULL )
  {
    this->TomoBorderPolyData = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(4);
    this->TomoBorderPolyData->SetPoints(pts);
    pts->Delete();
    vtkCellArray *border = vtkCellArray::New();
    border->InsertNextCell(5);
    border->InsertCellPoint(0);
    border->InsertCellPoint(1);
    border->InsertCellPoint(2);
    border->InsertCellPoint(3);
    border->InsertCellPoint(0);
    this->TomoBorderPolyData->SetLines(border);
    border->Delete();
    this->TomoBorderMapper = vtkPolyDataMapper::New();
    this->TomoBorderMapper->SetInput(this->TomoBorderPolyData);
    this->TomoBorderActor = vtkActor::New();
    this->TomoBorderActor->SetMapper(this->TomoBorderMapper);
    this->CurrentRenderer->AddActor(this->TomoBorderActor);
    this->TomoBorderActor->GetProperty()->SetColor(1,0,0);
    this->TomoBorderActor->GetProperty()->SetLineWidth(5);
  }

  // Position the slice
  double p[3], m[3];
  double voxSize[3];
  datastore->GetCurrentBackground()->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
  unsigned int dim[4];
  datastore->GetCurrentBackground()->getDimension (dim[0], dim[1], dim[2], dim[3]);
  p[0] = this->tomograms->GetSlice(SAGITTAL);
  p[1] = this->tomograms->GetSlice(CORONAL);
  p[2] = this->tomograms->GetSlice(AXIAL);
  m[0] = (dim[0]-1);
  m[1] = (dim[1]-1);
  m[2] = (dim[2]-1);

  this->TomoBorderActor->SetUserMatrix(tomograms->GetTomogramActor(CORONAL)->GetUserMatrix());
  vtkPoints *pts = this->TomoBorderPolyData->GetPoints();

  // Axial Plane
  switch(this->selectedTomogram) {
  case SAGITTAL:
    pts->SetPoint(0, p[0],0,0);
    pts->SetPoint(1, p[0],0,m[2]);
    pts->SetPoint(2, p[0],m[1],m[2]);
    pts->SetPoint(3, p[0],m[1],0);
    break;
  case CORONAL:
    pts->SetPoint(0, 0,p[1],0);
    pts->SetPoint(1, m[0],p[1],0);
    pts->SetPoint(2, m[0],p[1],m[2]);
    pts->SetPoint(3, 0,p[1],m[2]);
    break;
  case AXIAL:
    pts->SetPoint(0, 0,0,p[2]);
    pts->SetPoint(1, m[0],0,p[2]);
    pts->SetPoint(2, m[0],m[1],p[2]);
    pts->SetPoint(3, 0,m[1],p[2]);
    break;
  default:
    break;
  }
  this->TomoBorderPolyData->Modified();
  this->TomoBorderActor->SetVisibility(on);

}

void vtkInteractorStyleViewFrame::OnBackgroundsLoaded(bool clear)
{
  DisplayTomoBorder(true);
}

void vtkInteractorStyleViewFrame::SetTomogramSlices(unsigned int sSlice, unsigned int cSlice, unsigned int aSlice)
{
  tomograms->SetSlices(sSlice, cSlice, aSlice);

  // Update tomo border position
  DisplayTomoBorder(true);

  if(tomograms->GetTomogramActor(SAGITTAL)->GetVisibility()
      || tomograms->GetTomogramActor(AXIAL)->GetVisibility()
      || tomograms->GetTomogramActor(CORONAL)->GetVisibility())
    this->frame->OnRefresh(this->frame);
}

void
vtkInteractorStyleViewFrame::OnSelectedBackgroundChanged(ScalarVolume *oldVol)
{
  ScalarVolume *scalarVol = (ScalarVolume*)datastore->GetCurrentBackground();

  if(!scalarVol)
    return;

  tomograms->SetTomograms(scalarVol, oldVol);

  unsigned int dim[4];
  scalarVol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  double voxSize[3];
  scalarVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  if (!oldVol)
  {
    ResetView();

    vtkCamera *camera = this->GetCurrentRenderer()->GetActiveCamera();
    vtkMatrix4x4 *mx = this->tomograms->GetTomogramActor(CORONAL)->GetUserMatrix();
    this->camera_controls->Update(camera, dim, voxSize, mx);
    this->GetCurrentRenderer()->ResetCameraClippingRange();

  }

  // TODO: Actually put background file in scene
//  strcpy (scene->backgroundFilename, "Unknown");

 // scene->ReportPositionFromTomos (scene->sagittal->GetUserMatrix());

  DisplayTomoBorder(true);

  this->frame->OnRefresh(this->frame);
}

void
vtkInteractorStyleViewFrame::OnPathwaysLoaded()
{
  this->FindPokedRenderer(0,0);

  if(this->datastore->GetPathwayVisualization())
  {
    if(pathwaysActor)
    {
      this->GetCurrentRenderer()->RemoveActor(pathwaysActor);
      pathwaysActor->Delete();
      pathwaysActor = NULL;
    }
    pathwaysActor = this->datastore->GetPathwayVisualization()->GenerateActor();
    this->GetCurrentRenderer()->AddActor(pathwaysActor);

    currentPV = this->datastore->GetPathwayVisualization();

    // Add pathways, if visibility is on
    pathwaysActor->SetVisibility(this->pathwayVisibilityToggle);

    // Add arcs, if visibility is on
    if(this->activeSurface && this->datastore->GetPathwayVisualization()->GetArcsActor(this->activeSurface) && arcsVisible)
      this->GetCurrentRenderer()->AddActor(this->datastore->GetPathwayVisualization()->GetArcsActor(this->activeSurface));
  }

  this->UpdateSceneInfo();
}
void vtkInteractorStyleViewFrame::OnSetSurfaceColor(unsigned char r, unsigned char g, unsigned char b)
{
  std::map<SurfaceVisualization*, SurfaceInstance*>::iterator iter;

  for(iter = surfaceMapping.begin(); iter != surfaceMapping.end(); ++iter)
  {
    iter->second->GetActor(HEMISPHERE_LEFT)->GetProperty()->SetColor((double)r/256.0, (double)g/256.0, (double)b/256.0);
    iter->second->GetActor(HEMISPHERE_RIGHT)->GetProperty()->SetColor((double)r/256.0, (double)g/256.0, (double)b/256.0);
  }
}
void vtkInteractorStyleViewFrame::OnSurfaceLoaded(SurfaceVisualization *surf)
{
  // If the old surface was hidden, so should the new one
  int oldVisibility = 1;

  if(activeSurface)
    oldVisibility = surfaceMapping[activeSurface]->GetVisibility();

  // Generate an actor from the new visualization
  SurfaceInstance *surfaceInst = surf->GenerateInstance();

  // Associate surface with actor
  surfaceMapping[surf] = surfaceInst;

  cerr << "Loaded left actor: " << surfaceInst->GetActor(HEMISPHERE_LEFT) << endl;

  // Add it to be rendered
  if(surfaceInst->GetActor(HEMISPHERE_LEFT))
    this->CurrentRenderer->AddActor(surfaceInst->GetActor(HEMISPHERE_LEFT));
  if(surfaceInst->GetActor(HEMISPHERE_RIGHT))
    this->CurrentRenderer->AddActor(surfaceInst->GetActor(HEMISPHERE_RIGHT));

  // Hide the actor initially
  surfaceInst->SetVisibility(HEMISPHERE_BOTH, false);

  // If there wasn't a previous surface, set this one to be visible
  if(!activeSurface)
  {
    this->SetActiveSurface(surf);
  }
}

void
vtkInteractorStyleViewFrame::OnDataStoreSet()
{
  this->FindPokedRenderer(0,0);

  if(!datastore)
  {
    delete this->tomograms;
    return;
  }

  // Create our tomograms object (which communicates with the datastore)
  this->tomograms = new DTITomograms(this->datastore);

  // Hook up pathway actors to our renderer
  if(datastore->GetPathwayVisualization())
  {
    if(pathwaysActor)
    {
      this->GetCurrentRenderer()->RemoveActor(pathwaysActor);
      pathwaysActor->Delete();
      pathwaysActor = NULL;
    }
    pathwaysActor = this->datastore->GetPathwayVisualization()->GenerateActor();
    this->GetCurrentRenderer()->AddActor(pathwaysActor);

    currentPV = this->datastore->GetPathwayVisualization();

    // Add pathways, if visibility is on
    pathwaysActor->SetVisibility(this->pathwayVisibilityToggle);

    // Add arcs, if visibility is on
    if(this->activeSurface && this->datastore->GetPathwayVisualization()->GetArcsActor(this->activeSurface) && arcsVisible)
      this->GetCurrentRenderer()->AddActor(
          this->datastore->GetPathwayVisualization()->GetArcsActor(this->activeSurface));
  }

  // Hook up tomogram actors to our renderer
  this->CurrentRenderer->AddActor(tomograms->GetTomogramActor(SAGITTAL));
  this->CurrentRenderer->AddActor(tomograms->GetTomogramActor(AXIAL));
  this->CurrentRenderer->AddActor(tomograms->GetTomogramActor(CORONAL));

  // Add any existing surface models
  // Delete the old actor if it existed
  int oldVisibility = 3;
  if(this->activeSurface)
  {
    SurfaceInstance *surfaceInst = surfaceMapping[activeSurface];

    if(surfaceInst->GetActor(HEMISPHERE_LEFT))
      this->CurrentRenderer->RemoveActor(surfaceInst->GetActor(HEMISPHERE_LEFT));
    if(surfaceInst->GetActor(HEMISPHERE_RIGHT))
      this->CurrentRenderer->RemoveActor(surfaceInst->GetActor(HEMISPHERE_RIGHT));
    oldVisibility = surfaceInst->GetVisibility();
    surfaceMapping.erase(activeSurface);
    delete surfaceInst;
  }

  // Generate all of the actors from the surfaces in the datastore
  const std::vector<SurfaceVisualization*> surfaces = *datastore->GetSurfaceVisualizations();
  std::vector<SurfaceVisualization*>::const_iterator surfiter;

  for(surfiter = surfaces.begin(); surfiter != surfaces.end(); ++surfiter)
  {
    SurfaceVisualization *surf = *surfiter;
    SurfaceInstance *surfInst = surf->GenerateInstance();

    if(surfInst->GetActor(HEMISPHERE_LEFT))
      this->CurrentRenderer->AddActor(surfInst->GetActor(HEMISPHERE_LEFT));
    if(surfInst->GetActor(HEMISPHERE_RIGHT))
      this->CurrentRenderer->AddActor(surfInst->GetActor(HEMISPHERE_RIGHT));

    surfInst->SetVisibility(false);

    surfaceMapping[surf] = surfInst;
    if(!activeSurface)
    {
      this->SetActiveSurface(surf);
    }
  }

  // Hook up any existing VOI actors to our renderer
  std::vector<VOI*>::const_iterator iter;

  for(iter = datastore->GetVOIList()->begin(); iter != datastore->GetVOIList()->end(); iter++)
  {
    (*iter)->SetIStyleVisibility(this, this->voiVisibilityToggle);
  }

  if(datastore->IsBackgroundLoaded())
  {
    this->DisplayTomoBorder(true);
  }

  // Set the active voi to the datastore's currently active VOI (if it has one)
  SetActiveVOI(datastore->GetActiveVOI());

  // Add the scene info to our renderer
  this->UpdateSceneInfo();
  this->GetCurrentRenderer()->AddActor2D(this->sceneInfo);

  this->ResetView();
}

void
vtkInteractorStyleViewFrame::SetVOI (int index)
{
  // TODO: Select VOI in frame by clicking

  std::vector<VOI*> voiList = *datastore->GetVOIList();
  std::vector<VOI*>::const_iterator iter = voiList.begin();

  if(index >= voiList.size() || index < 0)
    return;

  datastore->SetActiveVOI(*(iter+index));
}

void vtkInteractorStyleViewFrame::CycleVOI()
{
  int count = 0;
  std::vector<VOI*> voiList = *datastore->GetVOIList();
  std::vector<VOI*>::const_iterator iter;
  for (iter = voiList.begin(); iter != voiList.end(); iter++) {
    VOI *voi = *iter;
    if (voi == datastore->GetActiveVOI()) {
      SetVOI ((count + 1) % voiList.size());
      return;
    }
    count++;
  }
}

void vtkInteractorStyleViewFrame::InsertVOIFromFile(){}

void vtkInteractorStyleViewFrame::OnActiveVOIChanged(VOI *oldVOI)
{
  if(this->frame->HasFocus() || activeVOI == NULL)
  {
    SetActiveVOI(datastore->GetActiveVOI());
  }

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::OnVOIInserted(VOI *newVOI)
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
    this->Interactor->GetEventPosition()[1]);

  // Add the VOI's actors to the renderer
  if(this->GetCurrentRenderer() != NULL)
  {
    // Hookup the VOI to this istyle's renderer
    newVOI->SetIStyleVisibility(this, this->voiVisibilityToggle);
  }

  // Make newly inserted VOIs the active one
  if(this->frame->HasFocus())
    datastore->SetActiveVOI(newVOI);

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::SetActiveVOI(VOI* voi)
{
  VOI *oldVOI = this->activeVOI;

  this->activeVOI = voi;

  if(voi)
    voi->SetIStyleVisibility(this, true);

  if(oldVOI)
    oldVOI->SetIStyleVisibility(this,true);
}

void vtkInteractorStyleViewFrame::OnVOIDeleted(VOI* voi)
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
    this->Interactor->GetEventPosition()[1]);

  cerr << "Removing VOI from istyle " << this << endl;
  voi->SetIStyleVisibility(this, false);

  // If it was our active VOI, set the active VOI to the datastore's currently active VOI
  if(activeVOI == voi && voi != datastore->GetActiveVOI())
  {
    this->activeVOI = NULL;
    this->SetActiveVOI(datastore->GetActiveVOI());
  }

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::SelectSlicePoint(AnatomicalAxis slice, int selectionX, int selectionY, double pick[4])
{
  unsigned int dim[4];
  datastore->GetCurrentBackground()->getDimension(dim[0], dim[1], dim[2], dim[3]);

  //fprintf(stderr , "Dimensions: %i , %i , %i \n", dim[0], dim[1], dim[2], dim[3]);

  vtkMatrix4x4 *mx = tomograms->GetTomogramActor(SAGITTAL)->GetUserMatrix();
  vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
  vtkMatrix4x4::Invert (mx, invertedMx);

  double cameraP[4] = {0, 0, 0, 1};
  double cameraD[4] = {0,0,0,0};
  double display[4] = {0, 0, 0, 0};
  double* dPickPoint;

  this->FindPokedRenderer(selectionX, selectionY);
  vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();

  // Store camera position in world coordinates
  camera->GetPosition(cameraP);

  // Translate camera position to voxel space
  double cameraPVoxSpace [4] = {0, 0, 0, 1};
  invertedMx->MultiplyPoint(cameraP, cameraPVoxSpace);

  // Determine "look at" position (based on mouse screen position)
  display[0]=selectionX;
  display[1]=selectionY;
  display[2]=0;//camera->GetDistance();
  this->CurrentRenderer->SetDisplayPoint(display);
  //this->CurrentRenderer->SetViewPoint(display);
  this->CurrentRenderer->DisplayToWorld();
  //this->CurrentRenderer->ViewToWorld();
  dPickPoint = this->CurrentRenderer->GetWorldPoint();

  // Create a direction vector pointing towards the "look at" from the camera position
  for(int i=0;i<3;i++)
    cameraD[i] = dPickPoint[i]-cameraP[i];

  // Normalize our direction vector
  vtkMath::Normalize(cameraD);

  double cameraDVoxSpace[4];

  // Translate our direction vector into voxel space
  invertedMx->MultiplyPoint(cameraD, cameraDVoxSpace);

  double d, s, P, D;
  switch(slice)
  {
  case SAGITTAL:
    // Check for intersection with sagittal plane
    s = this->datastore->GetTomogramSlice(SAGITTAL);
    P = cameraPVoxSpace[0];
    D = cameraDVoxSpace[0];
    break;
    case CORONAL:
    // Check for intersection with sagittal plane
    s = this->datastore->GetTomogramSlice(CORONAL);
    P = cameraPVoxSpace[1];
    D = cameraDVoxSpace[1];
    break;
    case AXIAL:
    // Check for intersection with sagittal plane
    s = this->datastore->GetTomogramSlice(AXIAL);
    P = cameraPVoxSpace[2];
    D = cameraDVoxSpace[2];
    break;
  }

  // Intersect our line with plane
  d = (s - P) / D;

  double pickObj[4] = {0,0,0,1};

  // Determine point of intersection (voxel space) and do bounds check to remain on tomograms
  for (int i=0; i < 3; i++)
  {
    pickObj[i] = d*cameraDVoxSpace[i] + cameraPVoxSpace[i];

    // bounds checking
    if(pickObj[i] > dim[i])
    {
      //fprintf(stderr, "%f is out of bounds of limit: %i\n", pickObj[i], dim[i]);
      pickObj[i] = dim[i];
    }
    if(pickObj[i] < 0)
    {
        pickObj[i] = 0;
    }
  }

  // Convert intersection point to world coordinates
  mx->MultiplyPoint (pickObj, pick);
}

void vtkInteractorStyleViewFrame::ToggleArcsVisibility()
{
  this->arcsVisible =!this->arcsVisible;

  this->SetArcsVisibility(this->arcsVisible);

  ((DTIViewFrame*)this->frame)->SetArcVisibilityCheck(arcsVisible);

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::TogglePathwaysVisibility()
{
  this->FindPokedRenderer(0,0);

  pathwayVisibilityToggle = !pathwayVisibilityToggle;

  if(pathwaysActor)
    pathwaysActor->SetVisibility(pathwayVisibilityToggle);

  ((DTIViewFrame*)this->frame)->SetPathwayVisibilityCheck(pathwayVisibilityToggle);
  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::ToggleTomogramVisibility(AnatomicalAxis axis)
{
  vtkActor *tomo = (vtkActor*)tomograms->GetTomogramActor(axis);

  if(tomo)
  {
    tomo->SetVisibility(!tomo->GetVisibility());

    // If this was our selected tomogram, hide the border
    // and attempt to select the next visible one
    if(selectedTomogram == axis)
    {
      DisplayTomoBorder(tomo->GetVisibility());

      if(!tomo->GetVisibility())
        SelectNextVisibleTomogram();
    }

    ((DTIViewFrame*)frame)->SetTomogramVisibilityCheck(axis, tomo->GetVisibility());
  }

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::SetActiveSurface(SurfaceVisualization* surf)
{
  unsigned int oldVisibility = 3;
  if(activeSurface)
  {
    oldVisibility = surfaceMapping[activeSurface]->GetVisibility();

    // Hide all of the old surface
    surfaceMapping[activeSurface]->SetVisibility(0);
  }

  activeSurface = surf;
  surfaceMapping[activeSurface]->SetVisibility(oldVisibility);

  this->SetCellLocatorFromVisibility(oldVisibility);

  std::vector<VOI*>::const_iterator voiIter = datastore->GetVOIList()->begin();

  for(; voiIter != datastore->GetVOIList()->end(); voiIter++)
  {
    if((*voiIter)->GetVOIType() == VOI_MESH)
      (*voiIter)->SetIStyleVisibility(this, true);
  }

  this->SetArcsVisibility(this->arcsVisible);

  if(this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

void vtkInteractorStyleViewFrame::SetArcsVisibility(bool pred)
{
  if(!datastore || !datastore->GetPathwayVisualization() || !activeSurface)
    return;

  std::vector<SurfaceVisualization*>::const_iterator surfIter;

  // Remove all arcs actors and add the current one only if pred is true
  for(surfIter = datastore->GetSurfaceVisualizations()->begin(); surfIter != datastore->GetSurfaceVisualizations()->end(); ++surfIter)
  {
    if(this->GetCurrentRenderer()->HasViewProp(datastore->GetPathwayVisualization()->GetArcsActor((*surfIter))))
        this->GetCurrentRenderer()->RemoveActor(datastore->GetPathwayVisualization()->GetArcsActor((*surfIter)));

    if(*surfIter == this->activeSurface && pred)
      this->GetCurrentRenderer()->AddActor(datastore->GetPathwayVisualization()->GetArcsActor((*surfIter)));
  }
}

void vtkInteractorStyleViewFrame::CycleSurface()
{
  if(!activeSurface)
    return;

  const std::vector<SurfaceVisualization*> surfaces = *datastore->GetSurfaceVisualizations();
  std::vector<SurfaceVisualization*>::const_iterator iter;

  SurfaceVisualization *previousSurface = NULL;
  SurfaceVisualization *oldSurface = activeSurface;
  for(iter = surfaces.begin(); iter != surfaces.end(); ++iter)
  {
    SurfaceVisualization *surface = *iter;

    if(previousSurface == activeSurface)
    {
      this->SetActiveSurface(surface);
      break;
    }
    previousSurface = surface;
  }

  // Was there no next surface? If so, set it to the first surface
  if(oldSurface == activeSurface)
    this->SetActiveSurface(*surfaces.begin());

  this->frame->OnRefresh(this->frame);
}

void
vtkInteractorStyleViewFrame::ToggleVisibility(vtkProp *actor) {
  actor->SetVisibility(!actor->GetVisibility());
}

void
vtkInteractorStyleViewFrame::SetSurfaceRepresentation(Hemisphere hemi, int rep)
{
  if(!this->activeSurface || !surfaceMapping[activeSurface])
    return;

  surfaceRepresentation = rep;

  // TODO: Clean this up and make it work for either hemisphere individually
  if(hemi == HEMISPHERE_BOTH)
  {
    vtkActor *actor = surfaceMapping[activeSurface]->GetActor(HEMISPHERE_LEFT);
    actor->GetProperty()->SetRepresentation(rep);
    actor->Modified();

    actor = surfaceMapping[activeSurface]->GetActor(HEMISPHERE_RIGHT);
    actor->GetProperty()->SetRepresentation(rep);
    actor->Modified();
  }

  return;
}

void
vtkInteractorStyleViewFrame::ToggleSurfaceVisibility(Hemisphere hemi)
{
  // Just hide our surface actor
  if(this->activeSurface)
  {
    surfaceMapping[activeSurface]->SetVisibility(hemi, !surfaceMapping[activeSurface]->GetVisibility(hemi));
    unsigned int oldVisibility = surfaceMapping[activeSurface]->GetVisibility();

    this->SetCellLocatorFromVisibility(oldVisibility);

    ((DTIViewFrame*)this->frame)->SetSurfaceVisibilityCheck(hemi,surfaceMapping[activeSurface]->GetVisibility(hemi));

    this->frame->OnRefresh(this->frame);
  }
}

void vtkInteractorStyleViewFrame::OnRefresh(DTIFrame *frame)
{
  this->UpdateSceneInfo();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnMouseWheelForward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);

  if(!Interactor->GetControlKey())
  {
    MoveCurrentTomogram (+1);
  }
  else if(this->activeVOI)
  {
    double scale[3];
    this->activeVOI->GetDimensions(&scale[0]);

    for(int i = 0; i < 3; i++)
      scale[i] += 1;

    this->activeVOI->SetScale(&scale[0]);

    this->datastore->GetPathwayVisualization()->UpdatePathways();
    this->datastore->RefreshAll(this->frame);
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;
}

extern double sqr (double x);

struct eqint {
  bool operator() (int i1, int i2) {
    return i1 == i2;
  }
};

// XXX This whole function should be in the meshes class
vector<int> *vtkInteractorStyleViewFrame::constructMeshPatch (vtkPolyData *inputData, int initialID, const double patchDim[3]) {
  #ifndef _MSC_VER
	hash_map<int, bool, hash<int>, eqint> visitedCells;
  #else
	hash_map<int, bool> visitedCells;
  #endif

  visitedCells[initialID] = true;
  vector<int> *cellIDs = new vector<int>;
  list<int> priorityQueue;
  cerr << "pushing cellID " << initialID << " to start." << endl;
  priorityQueue.push_front (initialID);

  double seedPoint[3] = {0,0,0};
  bool firstTime = true;

  while (!priorityQueue.empty()) {
    bool cellMatches = false;
    // take the first thing off the queue:
    int cellID = *(priorityQueue.begin());
    priorityQueue.erase (priorityQueue.begin());
    vtkCell *cell = inputData->GetCell (cellID);
    vtkPoints *pts = inputData->GetPoints();
    double pt[3];
    bool distanceFail = false;
    for (int vertex = 0; vertex < 3; vertex++) {

      pts->GetPoint(cell->GetPointId(vertex), pt);
      if (firstTime) {
  cellMatches = true;
  // xxx hack Why?
  if (vertex == 2) firstTime = false;
  for (int i = 0; i < 3; i++) {
    seedPoint[i] += 1.0/3.0*pt[i];
  }
      }
      else {
        double sqrDistances[3] = {sqr(pt[0]-seedPoint[0]),
            sqr(pt[1]-seedPoint[1]),
            sqr(pt[2]-seedPoint[2])};
  if (sqrDistances[0]/sqr(patchDim[0]/2.0) +
      sqrDistances[1]/sqr(patchDim[1]/2.0) +
      sqrDistances[2]/sqr(patchDim[2]/2.0) > 1) {
    distanceFail = true;
  }
      }
    }
    if (!distanceFail) {
      cellMatches = true;
      cellIDs->push_back (cellID);
    }
    // add all neighbors to queue.
    if (cellMatches) {
      vtkIdList *neighboringVerts = vtkIdList::New();
      vtkIdList *neighboringCells = vtkIdList::New();
      for (int vert = 0; vert < 3; vert++) {
  neighboringVerts->InsertId(0,inputData->GetCell(cellID)->GetPointId(vert));
  inputData->GetCellNeighbors (cellID, neighboringVerts, neighboringCells);
  for (int c = 0; c < neighboringCells->GetNumberOfIds(); c++) {
    int cellIDFound = neighboringCells->GetId(c);
    if (visitedCells.find(cellIDFound) == visitedCells.end()) {
      visitedCells[cellIDFound] = true;
      priorityQueue.push_front (neighboringCells->GetId(c));
    }
  }
      }
    }
  }
  return cellIDs;

}

//----------------------------------------------------------------------------
void vtkInteractorStyleViewFrame::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                          this->Interactor->GetEventPosition()[1]);
  if(!Interactor->GetControlKey())
  {
    MoveCurrentTomogram (-1);
  }
  else if(this->activeVOI)
  {
    double scale[3];
    this->activeVOI->GetDimensions(&scale[0]);
    for(int i = 0; i < 3; i++)
      scale[i] -= 1;
    this->activeVOI->SetScale(&scale[0]);

    this->datastore->GetPathwayVisualization()->UpdatePathways();
    this->datastore->RefreshAll(this->frame);
  }
}

int vtkInteractorStyleViewFrame::PickCell(
             vtkCellLocator *locator,
             int mouseX,
             int mouseY,
             double pickedPoint[3]) {

  double cameraP[3];
  double cameraD[3];
  double display[3];
  double lineStart[3];
  double lineEnd[3];
  double* dPickPoint;

  this->FindPokedRenderer(mouseX, mouseY);
  vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetPosition(cameraP);

  display[0]=mouseX;
  display[1]=mouseY;
  display[2]=0;

  this->CurrentRenderer->SetDisplayPoint(display);
  //this->CurrentRenderer->SetViewPoint(display);
  this->CurrentRenderer->DisplayToWorld();
  //this->CurrentRenderer->ViewToWorld();
  dPickPoint = this->CurrentRenderer->GetWorldPoint();

  double lineDistance = 5000;
  for(int i=0;i<3;i++) {
    cameraD[i] = lineDistance * (dPickPoint[i]-cameraP[i]);
    lineStart[i] = cameraP[i] - lineDistance * cameraD[i];
    lineEnd[i] = lineDistance * cameraD[i]+cameraP[i];
  }

  double t;
  int cellID = -1;
  double b[3];
  int subID;

  locator->IntersectWithLine (lineStart, lineEnd, 0.0001, t, pickedPoint, b, subID, (vtkIdType&)cellID);

  //fprintf(stderr, "Cell %i Position:  %f , %f , %f\n",cellID, pickedPoint[0], pickedPoint[1], pickedPoint[2]);
  return cellID;
}

void vtkInteractorStyleViewFrame::MoveVOI()
{
	double pick[4] = {0,0,0,0};
  int mx, my;

  this->GetInteractor()->GetEventPosition(mx, my);

  // If mouse is on the surface actor
  if(activeSurface && surfaceMapping[activeSurface]->GetVisibility())
  {
    if(!activeSurfaceCellLocator.second)
    {
      cerr << "Invalid cell locator." << endl;
      return;
    }

    int cellID = this->PickCell(activeSurfaceCellLocator.second, mx, my, pick);

    if(cellID != -1)
    {

#ifdef TIMING
      double duration;
      clock_t start, finish;
      start = clock();
#endif

      datastore->GetActiveVOI()->SetSurfaceCellID(cellID);

#ifdef TIMING
      finish = clock();
      duration = (double)(finish - start) / CLOCKS_PER_SEC;
      cerr << "Setting surface cell ID took " << duration << " seconds." << endl;
      start = clock();
#endif

      datastore->SetVOISurfaceTranslation(datastore->GetActiveVOI(), datastore->GetFilterSurface(),
          activeSurfaceCellLocator.first,
          cellID, pick[0], pick[1], pick[2]);

      datastore->RefreshAll(NULL);
#ifdef TIMING
      finish = clock();
      duration = (double)(finish - start) / CLOCKS_PER_SEC;
      cerr << "Translating surface took " << duration << " seconds." << endl;
#endif
    }
  }
  else if(tomograms->GetTomogramActor(selectedTomogram)->GetVisibility())
  {
    this->SelectSlicePoint(selectedTomogram, mx, my, pick);

    datastore->SetVOITranslation(datastore->GetActiveVOI(), pick[0], pick[1], pick[2]);
    datastore->RefreshAll(NULL);
  }

}


//----------------------------------------------------------------------------

VOI *
vtkInteractorStyleViewFrame::findVOIByID(int id)
{
  std::vector<VOI*>::const_iterator iter;
  for (iter = datastore->GetVOIList()->begin(); iter != datastore->GetVOIList()->end(); iter++) {
    VOI *v = *iter;
    if (v->getID() == id) {
      return v;
    }
  }
//  assert (false);
  return NULL;
}

//----------------------------------------------------------------------------
void
vtkInteractorStyleViewFrame::SetVOIMotion (VOIMotionType type, int id)
{
  /*
  if (!activeVOI) return;
  VOI *other = findVOIByID(id);
  if (id == activeVOI->getID()) {
    if (activeVOI->getSymmetry()) {
      activeVOI->getSymmetry()->setSymmetry(NULL);
    }
    activeVOI->setSymmetry(NULL);
    return;
  }
  else {
    activeVOI->setSymmetry (findVOIByID(id));
    other->setSymmetry (activeVOI);
  }
  // make me like the other guy
  MoveSymmetric (other);
  this->broadcastActiveVOI();

  // TODO: SetScenePathways
   * */

}

void
vtkInteractorStyleViewFrame::NextLineWidth()
{
  /* TODO: Send to datastore's pathway visualization
  this->lineWidthSetting++;
  if (this->lineWidthSetting > 1) {
    this->lineWidthSetting = 0;
  }
  double lineWidth;
  switch (this->lineWidthSetting) {
  case 0:
    lineWidth = 1.0;
    break;
  case 1:
    lineWidth = 2.0;
    break;
  };

  this->pathwaysActor->GetProperty()->SetLineWidth(lineWidth);

  this->pathwaysActor->Modified();
  */
}

void vtkInteractorStyleViewFrame::StainPathways()
{
  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::PickPoint()
{
  #if 0
    this->picker = vtkCellPicker::New();
  this->picker->SetTolerance(0.001);
  this->picker->PickFromListOn();
  //  this->picker->AddPickList (this->GetMeshActor());

  //this->Interactor->SetPicker (picker);
    int id = this->picker->Pick (this->Interactor->GetEventPosition()[0],
             this->Interactor->GetEventPosition()[1],
             0.0,
             this->CurrentRenderer);
  //int id = this->picker->Pick (0,0,0,this->CurrentRenderer);

    double p[3];
  this->picker->GetPickPosition (p);
  cerr << "pick position: " << p[0] << ", " << p[1] << ", " << p[2] << endl;

  //  this->sphereActor->SetPosition (p);
  cerr << "id is: " << this->picker->GetCellId() << endl;
  //  cerr << "foo" << endl;
  #endif
}


// XXX communicate with the meshes class for this
vtkActor *
vtkInteractorStyleViewFrame::BuildActorFromMesh (vtkPolyData *data) {
  vtkPolyDataMapper *meshMapper = vtkPolyDataMapper::New();
  meshMapper->SetInput(data);
  vtkActor *meshActor = vtkActor::New();
  meshActor->SetMapper(meshMapper);
  return meshActor;
}


// XXX communicate with the meshes class for this
vtkPolyData *
vtkInteractorStyleViewFrame::StripTriangles (vtkPolyData *meshPD) {

  vtkStripper *daveStripper = vtkStripper::New();
  daveStripper->SetInput(meshPD);
  vtkPolyData *output = daveStripper->GetOutput();
  output->SetReferenceCount(555);
  cerr << "meshing triangles..." << endl;
  daveStripper->Update();
  cerr << "done meshing triangles!" << endl;
  daveStripper->Delete();
  meshPD->Delete();
  return output;
}

void vtkInteractorStyleViewFrame::SavePathwayState(ostream &os)
{
  /* DISABLED
   *
  // Save out the color group for each pathway
  for(int j=0;j<this->whole_database->getNumFibers();j++)
  {
    DTIPathway* pathway;
    pathway = this->whole_database->getPathway(j);
    short group = GetPathwayGroup(pathway); // Get the color group this pathway is in
    os << group << " ";
  }
  */
}

void vtkInteractorStyleViewFrame::RestorePathwayState(istream &is)
{
  /* DISABLED
  // Restore the color group for each pathway

  DTIPathway* pathway;
  vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
  colors->SetNumberOfComponents(3);

  for(int j=0;j<this->whole_database->getNumFibers();j++)
  {
    pathway = this->whole_database->getPathway(j);
    short g;
    is >> g;
    this->pathwayVOImap[pathway] = g % this->pathway_colormap_class.size();

    // figure out what color we are going to give the next pathway
    // random jitter
    double vRand = this->GetPathwayJitter(pathway);
    double drgb[3],hsv[3];
    unsigned char rgb[3];
    std::vector<color>::iterator c;
    if(this->scene->curPathwayColoring==COLOR_STAIN)
    {
      c=this->pathway_colormap_class.begin();
      int group = this->GetPathwayGroup(pathway);
      c= c + group;
      drgb[0]=(*c).r/255.;
      drgb[1]=(*c).g/255.;
      drgb[2]=(*c).b/255.;
      vtkMath::RGBToHSV(drgb,hsv);
      //if(g==0)
      hsv[2]+=vRand;
      if( hsv[2] > 1)
        hsv[2]=1;
      if( hsv[2] < 0)
        hsv[2]=0;
      vtkMath::HSVToRGB(hsv,drgb);
      rgb[0]=(unsigned char) floor(255 * drgb[0]);
      rgb[1]=(unsigned char) floor(255 * drgb[1]);
      rgb[2]=(unsigned char) floor(255 * drgb[2]);
    }
    for(int i=0;i<pathway->getNumPoints();i++)
      colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
  }
  colors->SetName("colors");

  pathwaysPolyData->GetPointData()->SetScalars(colors);
  colors->Delete();

  this->SetScenePathways( );
  */
}

void vtkInteractorStyleViewFrame::SaveState()
{
  /* DISABLED

  // Bogus renderer poke just to get the current renderer
  this->FindPokedRenderer(0,0);

  // Save the entire state of the system.

  // Use a file dialog to get filename

#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
  //char defaultDir[255];
  //sprintf (defaultDir, "%s", _data_directory_path);
  wxFileDialog fd(this->frame,_T("Choose state filename."),defaultDir,_T(""),_T("Query State (*.qst)|*.qst"),wxSAVE | wxOVERWRITE_PROMPT, wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;
  char pathToUse[255];
  const char *pathReturned = fd.GetPath().mb_str();
  if (!strstr (pathReturned, ".qst")) {
    sprintf (pathToUse, "%s.qst", pathReturned);
  }
  else {
    strcpy (pathToUse, pathReturned);
  }
  std::ofstream stream(pathToUse);
  stream << "%% dtivis software version: " << _version << std::endl;

  // Save Camera
  double pos[3];
  double viewup[3];
  double focalpt[3];

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  camera->GetPosition(pos);
  camera->GetViewUp(viewup);
  camera->GetFocalPoint(focalpt);

  stream << "Camera Position: " << pos[0] << "," << pos[1] << "," << pos[2] << std::endl;
  stream << "Camera View Up: " << viewup[0] << "," << viewup[1] << "," << viewup[2] << std::endl;
  stream << "Camera Focal Point: " << focalpt[0] << "," << focalpt[1] << "," << focalpt[2] << std::endl;

  // Query operation string
  stream << "Query Mode: " << _filter_op << std::endl;
  stream << "Query String:" << this->_query_processor->getVOIFilter() << std::endl;

  // Save Units
  stream << "Units: " << scene->curUnits << std::endl;
  // Tomo Position
  stream << "Tomo. Position: " << scene->curSSlice << "," << scene->curCSlice << "," << scene->curASlice << std::endl;

  stream << "Tomo. Visibility: " << this->scene->sagittal->GetVisibility() << "," << this->scene->coronal->GetVisibility() <<"," << this->scene->axial->GetVisibility() << std::endl;


  // VOI information
  stream << "No. VOIs: " << this->voiList.size() << std::endl;
  for(voiIter i = voiList.begin(); i != voiList.end(); i++)
    {
      double scale[3];
      double center[3];
      int id;
      (*i)->GetDimensions(scale);
      (*i)->GetCenter(center);
      id = (*i)->getID();
      stream << '\t' << "VOI Scale: " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl;
      stream << '\t' << "VOI Center: " << center[0] << "," << center[1] << "," << center[2] << std::endl;
      stream << '\t' << "VOI ID: " << id << std::endl;
      stream << std::endl;
    }


  stream << "Pathway Min Length: " << this->lengthFilter->getMinValue() << endl;
  stream << "Pathway Max Length: " << this->lengthFilter->getMaxValue() << endl;
  stream << "Pathway Min FA: " << this->medianFAFilter->getMinValue() << endl;
  stream << "Pathway Max FA: " << this->medianFAFilter->getMaxValue() << endl;
  stream << "Pathway Min Curvature: " << this->meanCurvatureFilter->getMinValue() << endl;
  stream << "Pathway Max Curvature: " << this->meanCurvatureFilter->getMaxValue() << endl;

  stream << "Algorithm: " << this->algoFilter->getFilter() << endl;


  this->SavePathwayState(stream);
  stream << std::endl;

  stream.close();

  */
}

void vtkInteractorStyleViewFrame::ResetState()
{
  /* DISABLED
   *

  // Reset the state of the filtering system to nothing.
  voiIter iter;

  if(!voiList.empty()) {
    // First remove all the scene components of the VOIs
    for(iter=this->voiList.begin();iter!=this->voiList.end() && activeVOI != NULL;iter++)
      {
  if(this->CurrentRenderer != NULL)
    {
      this->CurrentRenderer->RemoveActor((*iter)->getActor());
      this->CurrentRenderer->RemoveActor((vtkProp*)(*iter)->getTextActor());
    }
  delete (*iter);
      }
    activeVOI = NULL;
    //    this->voiList.erase(this->voiList.begin(),this->voiList.end());
    this->voiList.clear();


    // Notify the GUI that the filters are gone
    broadcastActiveVOI();
    this->_query_processor->resetVOIFilter(this);

    // Refilter the pathways
    if(this->whole_database != NULL)
      {
  if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
    rebuildFilters();
      }
  //this->SetScenePathways( );
      }

    // Remove previous highlights
    this->HighlightProp(NULL);
    // Automatically go into axial slice move mode
    this->ActorSelected = DTI_ACTOR_AXIAL_TOMO;
  }

  */
}

void vtkInteractorStyleViewFrame::RestoreState()
{

  /* DISABLED
   *
   *

  // Restore the entire state of the system.

  // First lets do an artificial state restore without looking at
  // the saved file
  unsigned int ver;
  double pos[3];
  double viewup[3];
  double focalpt[3];
  unsigned int tomoPos[3];
  bool tomoVis[3];
  unsigned int nVOIs;
  int units;

  char filterString[255];
  int filterOp;

  double minPathwayLength;
  double maxPathwayLength;

#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
 // char defaultDir[255];
 // sprintf (defaultDir, "%s", _data_directory_path);

  // Use a file dialog to get filename
  wxFileDialog fd(this->frame,_T("Choose a program-state file to load:"),defaultDir,_T(""),_T("Query State (*.qst)|*.qst"),0,wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;

  std::ifstream stream(fd.GetPath().mb_str());
  getValue(stream,ver);   // Version
  getValue(stream,pos);   // Camera Position
  getValue(stream,viewup);  // Camera View Up
  getValue(stream,focalpt); // Camera Focal Pt.

  getValue(stream,filterOp);    // Query filter op
  getValue(stream,filterString); // Query string

  getValue(stream,units); // Current Unit System
  getValue(stream,tomoPos); // Tomogram positions
  getValue(stream,tomoVis);     // Tomogram visibility
  getValue(stream,nVOIs); // Number of VOIS


  // Restore Camera

  // Bogus renderer poke just to get the current renderer
  this->FindPokedRenderer(0,0);

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  camera->SetPosition(pos);
  camera->SetViewUp(viewup);
  camera->SetFocalPoint(focalpt);
  camera->ComputeViewPlaneNormal();

  // Restore Units
  while(units != scene->curUnits) {
    this->scene->NextUnits();
  }
  for (std::list<InteractorListener *>::iterator iiter = this->listeners.begin(); iiter != this->listeners.end(); iiter++) {
    InteractorListener *listener = *iiter;
    listener->interactorWindowUnitsChanged (this->scene->curUnits);
  }

  if (datastore->GetCurrentBackground()) {
    unsigned int dim[4];
    datastore->GetCurrentBackground()->getDimension(dim[0], dim[1], dim[2], dim[3]);

    /* DISABLED
    // Restore Tomos
    scene->curSSlice = tomoPos[0];
    if(scene->curSSlice >= dim[0])
      scene->curSSlice = dim[0]-1;
    scene->sagittal->SetDisplayExtent(scene->curSSlice,scene->curSSlice, 0,dim[1]-1, 0,dim[2]-1);
    scene->sagittal->Modified();

    scene->curCSlice = tomoPos[1];
    if(scene->curCSlice >= dim[0])
      scene->curCSlice = dim[0]-1;
    scene->coronal->SetDisplayExtent(0,dim[0]-1,scene->curCSlice,scene->curCSlice, 0,dim[2]-1);
    scene->coronal->Modified();

    scene->curASlice = tomoPos[2];
    if(scene->curASlice >= dim[0])
      scene->curASlice = dim[0]-1;
    scene->axial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1,scene->curASlice,scene->curASlice);
    scene->axial->Modified();
    this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());

    scene->sagittal->SetVisibility(tomoVis[0]);
    scene->coronal->SetVisibility(tomoVis[1]);
    scene->axial->SetVisibility(tomoVis[2]);

  }

  //  cerr << "about to reset state." << endl;

  // Now remove the previous VOIs
  this->ResetState();

  //  cerr << "state reset." << endl;

  // This call just refreshes the screen
  this->StopState();
  // Insert the new VOIs

  int maxVOIID = 0;

  for(int i=0; i<nVOIs; i++) {
      double scale[3];
      double center[3];
      int id;
      getValue(stream,scale); // VOI Length
      getValue(stream,center);  // VOI Position
      getValue(stream,id);      // VOI ID
      maxVOIID = id > maxVOIID ? id : maxVOIID;
      ignoreLine(stream);
      this->InsertVOI(center,scale,id);
      //      this->InsertVOI(center,scale,this->voiCounter);

      if(this->CurrentRenderer != NULL && activeVOI != NULL)
  {
    this->CurrentRenderer->AddActor(activeVOI->getActor());
    this->CurrentRenderer->AddActor2D((vtkProp*)activeVOI->getTextActor());
  }
      // Automatically go int VOI size adjusting mode
      //      this->ActorSelected = DTI_ACTOR_VOI;
  }
  this->voiCounter = maxVOIID+1;

  getValue(stream,minPathwayLength); // minimum length of pathways
  getValue(stream,maxPathwayLength); // maximum length of pathways

  double minFA, maxFA, minCurv, maxCurv;

  getValue(stream,minFA);
  getValue(stream,maxFA);

  getValue(stream,minCurv);
  getValue(stream,maxCurv);

  int algoChoice;
  getValue (stream, algoChoice);

  // Must do this in case the border was being displayed in the scene before we restore
  // has nothing to do with the saved border condition
  if(this->TomoBorderActor!=NULL)
    DisplayTomoBorder(this->TomoBorderActor->GetVisibility());

  RestorePathwayState(stream);

  // Restore query filter op


  setFilterOp ((DTIQueryOperation) filterOp);
  this->_query_processor->setVOIFilter (filterString);
  broadcastFilterString (filterString);


  this->lengthFilter->setMinValue(minPathwayLength);
  this->lengthFilter->setMaxValue(maxPathwayLength);
  this->medianFAFilter->setMinValue(minFA);
  this->medianFAFilter->setMaxValue(maxFA);
  this->meanCurvatureFilter->setMinValue(minCurv);
  this->meanCurvatureFilter->setMaxValue(maxCurv);
  this->algoFilter->setFilter(algoChoice);



  this->broadcastSliderValues();
  this->broadcastAlgoChoice();
  //  SetMinLength (minPathwayLength);
  //  SetMaxLength (maxPathwayLength);
  this->SetScenePathways();
  stream.close();

  this->CurrentRenderer->ResetCameraClippingRange();
  this->frame->RefreshVTK ();
  */
}

void vtkInteractorStyleViewFrame::SavePathwaysPDBInternal(const char *path)
{
  /* DISABLED

  DTIPathwaySelection *selection = new DTIPathwaySelection (this->whole_database->getNumFibers(), false);

  // Find the visible fibers
  int numSaved = 0;
  for(int j=0;j<this->whole_database->getNumFibers();j++) {
    DTIPathway* pathway = this->whole_database->getPathway(j);
    if (this->_query_processor->doesPathwayMatch(pathway)) {
      selection->setValue (j, true);
      numSaved++;
    }
    else {
      selection->setValue (j, false);
    }
  }
  char pathToUse[255];
  const char *pathReturned = path;
  if (!strstr (pathReturned, ".pdb")) {
    sprintf (pathToUse, "%s.pdb", pathReturned);
  }
  else {
    strcpy (pathToUse, pathReturned);
  }

  DTIPathwayIO::openDatabase (this->whole_database, pathToUse);
  unsigned long *offsets = DTIPathwayIO::appendDatabaseFile (this->whole_database, pathToUse, selection);
  DTIPathwayIO::appendFileOffsetsToDatabaseFile (numSaved, offsets, pathToUse);
  */
}

void vtkInteractorStyleViewFrame::SavePathwaysImageInternal(const char* path)
{
  /* DISABLED
   *

  PathwayIntersectionVolume *vol = new PathwayIntersectionVolume ((ScalarVolume*)datastore->GetCurrentBackground());

  // Find the visible fibers
  std::vector< DTIPathway* > pathsToSave;
  for(int j=0;j<this->whole_database->getNumFibers();j++) {
    DTIPathway* pathway = this->whole_database->getPathway(j);
    if (this->_query_processor->doesPathwayMatch(pathway)) {
      vol->munchPathway(pathway);
    }
  }

  vol->normalize();

  DTIVolumeIO::saveScalarVolumeNifti (vol, path);
  */
}

void vtkInteractorStyleViewFrame::SetScreenShotMode(bool pred)
{
  this->sceneInfo->SetVisibility(!pred);
  this->frame->OnRefresh(this->frame);
  //this->sceneInfo->SetVisibility(vis);
}

void
vtkInteractorStyleViewFrame::ResetView()
{
  vtkInteractorStyleDTI::ResetView();

  const ScalarVolume *scalarVol = datastore->GetCurrentBackground();

  if(!scalarVol)
    return;

  unsigned int dim[4];
  scalarVol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  double voxSize[3];
  scalarVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  vtkCamera *camera = this->GetCurrentRenderer()->GetActiveCamera();
  vtkMatrix4x4 *mx = this->tomograms->GetTomogramActor(CORONAL)->GetUserMatrix();

  this->camera_controls->ResetPosition();
  this->camera_controls->Update(camera, dim, voxSize, mx);
  if(this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  this->frame->OnRefresh(this->frame);
}

void
vtkInteractorStyleViewFrame::ToggleVOIVisibility()
{
  // We have to remove actors from the renderer instead of just hiding actors

  std::vector<VOI*>::const_iterator iter;

  for(iter = datastore->GetVOIList()->begin(); iter != datastore->GetVOIList()->end(); iter++)
  {
    VOI *voi = *iter;

    // Hide/Show VOI actor
    if(this->voiVisibilityToggle)
    {
      // Always remove the label actor when hiding VOIs
      this->CurrentRenderer->RemoveActor2D(voi->getTextActor());
      this->CurrentRenderer->RemoveActor(voi->getActor());
    }
    else
    {
      this->CurrentRenderer->AddActor(voi->getActor());

      // If labels aren't hidden, add them to
      if(this->voiLabelsToggle)
        this->CurrentRenderer->AddActor2D(voi->getTextActor());
    }
    //(*iter)->getActor()->SetVisibility(voiVisibilityToggle);
  }

  this->voiVisibilityToggle = !(this->voiVisibilityToggle);

  this->frame->OnRefresh(this->frame);;
}

void
vtkInteractorStyleViewFrame::SetSubjectDatapathInternal(const char *pathName)
{
  // TODO: Move this to datastore
  this->current_background_id = 0;
  DisplayTomoBorder (true);
  this->frame->OnRefresh(this->frame);
}

void
vtkInteractorStyleViewFrame::SelectTomogram(AnatomicalAxis axis)
{
  this->selectedTomogram = axis;

  if (!tomograms->GetTomogramActor(axis)->GetVisibility())
  {
    this->ToggleTomogramVisibility(axis);
  }

  DisplayTomoBorder(true);

  this->frame->OnRefresh(this->frame);
}

void vtkInteractorStyleViewFrame::SelectNextVisibleTomogram()
{
  if(tomograms->GetTomogramActor(SAGITTAL)->GetVisibility())
    SelectTomogram(SAGITTAL);
  else if(tomograms->GetTomogramActor(AXIAL)->GetVisibility())
    SelectTomogram(AXIAL);
  else if(tomograms->GetTomogramActor(CORONAL)->GetVisibility())
    SelectTomogram(CORONAL);
}

vtkActor *vtkInteractorStyleViewFrame::CreatePathwayArcs()
{
  if(!this->datastore || !this->datastore->GetDatabase() || !this->datastore->GetSurfaceVisualizations()->size() > 0)
    return NULL;

  // For each pathway in database, determine if it intersects the cortical surface at both ends
  DTIPathwayDatabase *db = this->datastore->GetDatabase();

  vtkFloatArray *coords = vtkFloatArray::New();
  coords->SetNumberOfComponents(3);
  vtkPoints *points = vtkPoints::New();
  vtkPolyData *polyData = vtkPolyData::New();
  vtkCellArray *cells = vtkCellArray::New();

  int cellOffset = 0;

  vtkAppendPolyData *append = vtkAppendPolyData::New();
  for(int i = 0; i < db->getNumFibers(); ++i)
  {
    DTIPathway *pathway = db->getPathway(i);
    double o_1[3];  // Position of first point in fiber "outside of segment 1"
    double i_1[3];  // Position of the second point in fiber "inside of segment 1"
    double o_2[3];  // Position of the last point in the fiber "outside of segment 2"
    double i_2[3];  // Position of the second to last point in the fiber "inside of segment 2"

    pathway->getPoint(0,&o_1[0]);
    pathway->getPoint(1,&i_1[0]);
    pathway->getPoint(pathway->getNumPoints()-1, &o_2[0]);
    pathway->getPoint(pathway->getNumPoints()-2, &i_2[0]);

    // Use the cell locator to find the intersection and the position of the intersection

    double t;
    double intersect1[3];
    double pcoords1[3];
    double normal1[3] = {0,0,0};
    double intersect2[3];
    double pcoords2[3];
    double normal2[3] = {0,0,0};

    vtkIdType cellID1;
    vtkIdType cellID2;
    int subID;

    double dist = 50;

    vtkDataArray *normals = this->activeSurface->GetPolyData(HEMISPHERE_BOTH)->GetPointData()->GetNormals();

    this->activeSurface->GetPolyData(HEMISPHERE_BOTH)->GetPoints();
    if(this->activeSurfaceCellLocator.second->IntersectWithLine(o_1, i_1, 0.0001, t, intersect1, pcoords1, subID, cellID1)
        && this->activeSurfaceCellLocator.second->IntersectWithLine(o_2, i_2, 0.0001, t, intersect2, pcoords2, subID, cellID2))
    {
      vtkCell *cell1 = this->activeSurface->GetPolyData(HEMISPHERE_BOTH)->GetCell(cellID1);
      vtkCell *cell2 = this->activeSurface->GetPolyData(HEMISPHERE_BOTH)->GetCell(cellID2);

      double *temp1;
      double *temp2;

      for(int j = 0; j < 3; ++j)
      {
        temp1 = normals->GetTuple3(cell1->GetPointId(j));
        temp2 = normals->GetTuple3(cell2->GetPointId(j));
        for(int k = 0; k < 3; ++k)
        {
          normal1[k] += temp1[k];
          normal2[k] += temp2[k];
        }
      }

      // Average
      for(int j = 0; j < 3; ++j)
      {
        normal1[j] /= 3.0;
        normal2[j] /= 3.0;
      }

      /*
      fprintf(stderr, "Pathway %d intersects with Normal1 ( %f, %f, %f )\n\tNormal2: ( %f, %f, %f)\n\t%f\n",
          i, normal2[0], normal2[1], normal2[2],
          normal1[0], normal1[1], normal1[2],t);
          */

      for(int j = 0; j < 3; ++j)
      {
        normal1[j] *= dist;
        normal1[j] = intersect1[j] + normal1[j];
        normal2[j] *= dist;
        normal2[j] = intersect2[j] + normal2[j];
      }

      fprintf(stderr, "Control Points: ( %f, %f, %f )  ( %f, %f, %f )\n",normal1[0],normal1[1], normal1[2],
          normal2[0], normal2[1], normal2[2]);


      int cCount = 0;
      for(double t = 0.0; t <= 1.0; t += 0.05)
      {
        cCount++;
        double t1 = pow((1-t),3);
        double t2 = 3*pow((1-t),2)*t;
        double t3 = 3 * (1-t) * pow(t,2);
        double t4 = pow(t,3);
        double p1[3];
        double p2[3];
        double p3[3];
        double p4[3];
        for(int j = 0; j < 3; ++j)
        {
          p1[j] = intersect1[j];
          p2[j] = normal1[j];
          p3[j] = normal2[j];
          p4[j] = intersect2[j];
        }

        for(int j = 0; j < 3; ++j)
        {
          p1[j] *= t1;
          p2[j] *= t2;
          p3[j] *= t3;
          p4[j] *= t4;
        }

        for(int j = 0; j < 3;++j)
        {
          p1[j] = p1[j] + p2[j] + p3[j] + p4[j];
        }

        //fprintf(stderr, "POINT: ( %f, %f, %f )\n",p1[0], p1[1],p1[2]);
        coords->InsertNextTuple3(p1[0], p1[1], p1[2]);

      }

      cells->InsertNextCell(cCount);
      for(int i = 0; i < cCount; i++)
      {
        cells->InsertCellPoint(cellOffset++);
      }


      //append->AddInput(polyData);
    }
    // If there is an intersection, create an arc!
  }

  points->SetData(coords);

  polyData->SetPoints(points);
  polyData->SetLines(cells);
  //append->Modified();

  vtkPolyDataMapper *meshMapper = vtkPolyDataMapper::New();
  meshMapper->SetInput(polyData);

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(meshMapper);

  return actor;

  // For each pathway that intersects, create a new arc from the intersection points using the normals
}
