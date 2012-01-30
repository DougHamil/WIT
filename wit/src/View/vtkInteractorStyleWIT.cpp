/** Copyright (c) 2005-2007, The Board of Trustees of Leland Stanford 
Junior University. All rights reserved. **/

#include "vtkInteractorStyleWIT.h"

#ifndef _MSC_VER
#include <GL/gl.h>
#else
#include <Windows.h>
#include <gl/GL.h>
#endif

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

#include <string.h>
#include <util/DTIPathway.h>
#include <util/DTIFilterTree.h>
#include <util/PathwayIntersectionVolume.h>
#include <util/DTIPathwaySelection.h>

#include <math.h>

#include <limits>

#include "vtkStripper.h"
#include "vtkCellLocator.h"
#include "vtkCellLocator.h"
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

#include "CameraControls.h"
#include <util/DTIQueryProcessor.h>
#include <util/DTIVolumeIO.h>
#include <util/ScalarVolume.h>

vtkCxxRevisionMacro(vtkInteractorStyleWIT, "$Revision: 1.28 $");
vtkStandardNewMacro(vtkInteractorStyleWIT);

using std::cout;
using std::endl;

#ifndef _MSC_VER
using namespace __gnu_cxx;
#endif

//----------------------------------------------------------------------------
vtkInteractorStyleWIT::vtkInteractorStyleWIT() 
{
  this->camera_controls = NULL; 
}

//----------------------------------------------------------------------------
vtkInteractorStyleWIT::~vtkInteractorStyleWIT() 
{
}


//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnMouseMove() 
{
  
#if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;

  if(rwi->GetRenderWindow()->GetAAFrames() > 0) {
    rwi->GetRenderWindow()->SetAAFrames(0);
  }
#endif

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->State) 
  {

  case VTKIS_ROTATE:
    this->FindPokedRenderer(x, y);
    this->Rotate();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;

  case VTKIS_PAN:
    this->FindPokedRenderer(x, y);
    this->Pan();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;

  case VTKIS_DOLLY:
    this->FindPokedRenderer(x, y);
    this->Dolly();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;

  case VTKIS_SPIN:
    this->FindPokedRenderer(x, y);
    this->Spin();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    break;

  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnLeftButtonDown() 
{ 
  glEnable (GL_DEPTH_TEST);
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
  {
    return;
  }

  if (this->Interactor->GetShiftKey()) 
  {
    if (this->Interactor->GetControlKey()) 
    {
      this->StartDolly();
    }
    else 
    {
      this->StartPan();
    }
  } 
  else 
  {
    if (this->Interactor->GetControlKey()) 
    {
    }
    else 
    {
      this->StartRotate();
    }
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnLeftButtonUp()
{
  #if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render

  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
  }
#endif
  
  switch (this->State) 
  {
  case VTKIS_DOLLY:
    this->EndDolly();
    break;

  case VTKIS_PAN:
    this->EndPan();
    break;

  case VTKIS_SPIN:
    this->EndSpin();
    break;

  case VTKIS_ROTATE:
    this->EndRotate();
    break;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnMiddleButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    return
  
  this->StartPan();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnMiddleButtonUp()
{
#if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render
  
  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
    rwi->Render();
  }
#endif

  switch (this->State) 
  {
  case VTKIS_PAN:
    this->EndPan();
    break;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnRightButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
  {
    return;
  }
  
  this->StartDolly();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnRightButtonUp()
{
#if 0
   vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render

  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
    rwi->Render();
  }
#endif
  switch (this->State) 
  {
  case VTKIS_DOLLY:
    this->EndDolly();
    break;
  }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::Rotate()
{
  if(this->CurrentRenderer == NULL)
    return;

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
}


//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::Spin()
{
  if (this->CurrentRenderer == NULL)
    return;

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double *center = this->CurrentRenderer->GetCenter();

  double newAngle =
    atan2((double)rwi->GetEventPosition()[1] - (double)center[1],
          (double)rwi->GetEventPosition()[0] - (double)center[0]);

  double oldAngle =
    atan2((double)rwi->GetLastEventPosition()[1] - (double)center[1],
          (double)rwi->GetLastEventPosition()[0] - (double)center[0]);

  newAngle = vtkMath::DegreesFromRadians(newAngle);
  oldAngle = vtkMath::DegreesFromRadians(oldAngle);
 
  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::Pan()
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
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::Dolly()
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
void vtkInteractorStyleWIT::Dolly(double factor)
{
  if(this->CurrentRenderer == NULL)
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
}


//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnChar(){}

//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnMouseWheelForward(){}
//----------------------------------------------------------------------------
void vtkInteractorStyleWIT::OnMouseWheelBackward(){}

void vtkInteractorStyleWIT::ResetView(){}