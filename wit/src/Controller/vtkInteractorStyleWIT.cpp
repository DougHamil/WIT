/***********************************************************************
* AUTHOR: Anthony Sherbondy
*   FILE: vtkInteractorStyleQuench.cpp
*   DATE: Wed Oct 22 12:48:27 2008
*  DESCR: 
***********************************************************************/
#include "vtkInteractorStyleWIT.h"
#include "vtkRenderWindowInteractor.h"
#include "../View/VTK/vtkROI.h"
#include "vtkRenderer.h"
#include "../WITApplication.h"
#include "../View/WITVolumeViz.h"
#include "../Model/WITPathwayViz.h"
#include "WITGestureController.h"

#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include <vtkRendererCollection.h>
#include <vtkWindowToImageFilter.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <util/PathwayGroup.h>
#include <util/io_utils.h>
#include "vtkPropCollection.h"
#include "vtkPropPicker.h"

#ifndef _WIN32
#define stricmp strcasecmp
#define _stricmp stricmp
#endif

#include "vtkWorldPointPicker.h"
#ifdef _MSC_VER
#include <Windows.h>
#endif
#include <GL/glu.h>
vtkWorldPointPicker  *pp ;

vtkCxxRevisionMacro(vtkInteractorStyleWIT, "$Revision: 1.13 $");
vtkStandardNewMacro(vtkInteractorStyleWIT);

/***********************************************************************
*  Method: vtkInteractorStyleWIT::PrintSelf
*  Params: ostream &os, vtkIndent indent
* Returns: void
* Effects: 
***********************************************************************/
void vtkInteractorStyleWIT::PrintSelf(ostream &os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);
}

vtkInteractorStyleWIT::vtkInteractorStyleWIT()
{
	//_GestureInteractor = new qGestureInteractor();
	MotionFactor = 10.0;
	_left_button_down=0;
	_right_button_down=0;
	//_propPicker = vtkPropPicker::New();
}


vtkInteractorStyleWIT::~vtkInteractorStyleWIT()
{
	//_propPicker->Delete();
	VTK_SAFE_DELETE(_renderer);
	delete _volViz; _volViz = 0;
	delete _pathwayViz; _pathwayViz = 0;
	delete _voiViz; _voiViz = 0;
	delete _GestureInteractor;
}
void vtkInteractorStyleWIT::Init(WITFrame *frame)
{
	this->frame = frame;

	//  _editing_voi = false;
	_interaction_mode = INTERACTION_IDLE;
	// connect the render window and wxVTK window
	frame->getRenderWindow()->GetInteractor()->SetInteractorStyle(this);

	vtkRenderWindow *pRenderWindow = frame->getRenderWindow();
	_renderer = vtkRenderer::New();
	pRenderWindow->AddRenderer(_renderer);
	//pRenderWindow->LineSmoothingOff();
	//pRenderWindow->PolygonSmoothingOff();
	//pRenderWindow->SetAAFrames(0);
	//pRenderWindow->SetSize(200,200);  

	vtkCamera *camera = vtkCamera::New();
	_renderer->SetActiveCamera(camera);
	camera->Delete();
	FindPokedRenderer(0,0);
	CurrentRenderer->SetBackground(0.74, 0.74, 0.66);
	_volViz = new WITVolumeViz(_renderer);
	_pathwayViz = &WITApplication::getInstance().getPathwayController()->getPathwayViz();
	//_voiViz = new WITROIViz(_renderer);
	_GestureInteractor = new WITGestureController(_renderer);
	//_voiEditor = new qROIEditor(_renderer, _volViz);
	pp = vtkWorldPointPicker :: New();
	//_voiEditor->SetEditingROIMode(false);
}

void vtkInteractorStyleWIT::ToggleImages(DTISceneActorID imageId, int ShiftKeyDown)
{
	if (ShiftKeyDown) 
	{
		bool show = !_volViz->Visibility(imageId);
		_volViz->SetVisibility(imageId, show);
		
		//if(!show)
			//NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
		////	frame->ToggleOverlayVisibility(imageId);
		////	this->frame->SetVisibilityCheck(imageId, _Scene->GetActorFromID(imageId)->GetVisibility()?true:false);
	} 
	else
	{
		_volViz->SetActiveImage(imageId);
		//NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
	}
	//this->EventCallbackCommand->SetAbortFlag(1);
	////	_Scene->GetPathwayViz()->SetIntersectionGeometry();
	//_renderer->GetRenderWindow()->Render();
	emit doRender();
	//this->frame->update();
}
/***********************************************************************
*  Method: vtkInteractorStyleWIT::OnKeyPress
*  Params: 
* Returns: void
* Effects: 
***********************************************************************/
void vtkInteractorStyleWIT::OnKeyPress()
{
	if (this->Interactor->GetControlKey())
		return;

	//if(_voiViz->OnKeyDown(Interactor->GetKeyCode()))
	//	return;

	vtkRenderWindowInteractor *rwi = this->Interactor;
	this->FindPokedRenderer(0,0);
	vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	switch (Interactor->GetShiftKey())
	{
	case 1: //Shift is pressed
		switch(Interactor->GetKeyCode())
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			//NotifyAllListeners(PEvent(new EventPathwayGroupVisibilityChanged(Interactor->GetKeyCode() - '0')));
			break;

		case 'S':
			//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_SAGITTAL_TOMO,1) ));
			//ToggleImages(DTI_ACTOR_SAGITTAL_TOMO,1);
			break;

		case 'C':
			//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_CORONAL_TOMO,1) ));
			//ToggleImages(DTI_ACTOR_CORONAL_TOMO,1);
			break;

		case 'A':
			//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_AXIAL_TOMO,1) ));
			//ToggleImages(DTI_ACTOR_AXIAL_TOMO,1);
			break;
				
		case 'W':
			_pathwayViz->IncreaseLineWidth(-1);
			break;
		}
		break;
	case 0: //Shift is not pressed
		switch(Interactor->GetKeyCode())
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			//NotifyAllListeners(PEvent(new EventPathwayGroupSelected(Interactor->GetKeyCode() - '0')));
			break;

		case '0':
			//NotifyAllListeners(PEvent(new EventPathwayGroupVisibilityChanged(Interactor->GetKeyCode() - '0')));
			break;

		case ',':
			_volViz->MoveActiveImage(-1);
			//NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
			break;
		case '.':
			_volViz->MoveActiveImage(1);
			//NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
			break;

			case 'S':
				//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_SAGITTAL_TOMO,0) ));
				//ToggleImages(DTI_ACTOR_SAGITTAL_TOMO,0);
				break;
				
			case 'C':
				//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_CORONAL_TOMO,0) ));
				//ToggleImages(DTI_ACTOR_CORONAL_TOMO,0);
				break;
				
			case 'A':
				//NotifyAllListeners(PEvent ( new EventToggleImagePlane(DTI_ACTOR_AXIAL_TOMO,0) ));
				//ToggleImages(DTI_ACTOR_AXIAL_TOMO,0);
				break;
				
//		case 'S':
//			ToggleImages(DTI_ACTOR_SAGITTAL_TOMO,0);
//			break;
//
//		case 'C':
//			ToggleImages(DTI_ACTOR_CORONAL_TOMO,0);
//			break;
//
//		case 'A':
//			ToggleImages(DTI_ACTOR_AXIAL_TOMO,0);
//			break;

		case 'W':
			_pathwayViz->IncreaseLineWidth(1);
			break;

		case 'h':
		case 'H':
			//NotifyAllListeners(PEvent ( new Event(TOGGLE_MESH_VISIBILITY) ));
			break;
			/*
		case 'v':
		case 'V': 
		  if (_voiEditor->GetEditingROIMode() == true) {
		    NotifyAllListeners(PEvent(new Event(ROI_EDIT_MODE_OFF)));
		  }
		  else {
		  NotifyAllListeners(PEvent (new Event(ROI_EDIT_MODE_ON)));
		  }
		  //ToggleROIEditingMode();
		break;
		case 'd':
		case 'D':
		  NotifyAllListeners(PEvent( new Event(ROI_DRAW_NEW_ROI)));
		//		  HandDrawROI();
		break;
		case '-':
		  //		  _voiEditor->DecreaseBrushSize();
		  //		  NotifyAllListeners(PEvent ( new Event(REFRESH_VIZ) ));
		  break;
		case '=':
		  //		  _voiEditor->IncreaseBrushSize();
		  //		  NotifyAllListeners(PEvent ( new Event(REFRESH_VIZ) ));
		  break;
			*/
		}
		break;
	}

}

void point3d(int x, int y, vtkRenderer * _renderer )
{
	double selpt[3]={x, y, 0};
	pp->Pick(selpt,_renderer);
	pp->GetPickPosition(selpt);


	vtkWindowToImageFilter* filter = vtkWindowToImageFilter::New();
	filter->SetInput( _renderer->GetRenderWindow() );
	filter->SetInputBufferType (VTK_ZBUFFER);
	filter->SetReadFrontBuffer (false);
	filter->Modified();
	filter->Update();
	vtkImageData *data = filter->GetOutput();
	data->Update();
	data->Modified();


	double display[3],pickPoint[3];
	float z = data->GetScalarComponentAsFloat (x, y, 0,0);
	if (z > 1.0) z = 1.0;
	// now convert the display point to world coordinates
	display[0] = x;
	display[1] = y;
	display[2] = z;
	_renderer->SetDisplayPoint (display);
	_renderer->DisplayToWorld ();
	double *world = _renderer->GetWorldPoint ();
	for (int i=0; i < 3; i++) {
		pickPoint[i] = world[i] / world[3];
	}

	//char out[1000];
	//sprintf(out,"%.1lf, %.1lf, %.1lf\n%.1lf, %.1lf, %.1lf", selpt[0], selpt[1], selpt[2], pickPoint[0], pickPoint[1], pickPoint[2]);
	//MessageBox(0,out,out,MB_OK);
}
void vtkInteractorStyleWIT::OnMouseMove()
{
  _mouse_moved = true;
	vtkRenderWindowInteractor *rwi = this->Interactor;
	int x = rwi->GetEventPosition()[0];
	int y = rwi->GetEventPosition()[1];
	int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
	int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
	this->FindPokedRenderer(x,y);

	//If we are still drawing a gesture, continue doing so
	if(_interaction_mode == INTERACTION_DRAW_GESTURE)
	{	  
		_GestureInteractor->AddPoint(x,y);
		//NotifyAllListeners(PEvent ( new EventRefreshViz(true) ) );
		//this->frame->update();
		emit doRender();
		//CurrentRenderer->GetRenderWindow()->Render();
		return;
	}

	if (_interaction_mode == INTERACTION_PATHWAYS)
	{
		//_pathwayViz->OnMouseMove(x, abs(y));
	}

	if (_interaction_mode == INTERACTION_ROI_EDIT)
	  {
	    //_voiEditor->OnMouseMove(x,abs(y));
	  }//

	/*	if(_editing_voi && (_left_button_down || _right_button_down) && _voiEditor->OnMouseMove(x,abs(y)))
		return; //ROI editing in process.
	*/
	else {
	  switch (this->State)  
	    {
	    case VTKIS_ROTATE:
	      Rotate();
	      InvokeEvent(vtkCommand::InteractionEvent, NULL);
	      break;
	    case VTKIS_PAN:
	      Pan();
	      InvokeEvent(vtkCommand::InteractionEvent, NULL);
	      break;
	    case VTKIS_DOLLY:
	      {
		double *center = CurrentRenderer->GetCenter();
		double dyf = MotionFactor * (double)(dy) / (double)(center[1]);
		double scale = pow((double)1.1, dyf);
		Dolly(scale);
		//_voiViz->Dolly(scale);
		emit doRender();
		//CurrentRenderer->GetRenderWindow()->Render();
		//this->frame->update();
	      }
	      InvokeEvent(vtkCommand::InteractionEvent, NULL);
	      break;
	    case VTKIS_SPIN:
	      Spin();
	      InvokeEvent(vtkCommand::InteractionEvent, NULL);
	      //this->frame->RefreshViz ();
	      break;
	      //default:
	      // if(_Scene->GetPathwayViz()->PickManipulationTools(x,y) != -1)
	      //  frame->RefreshViz();
	      // break;
	    }
	}

}
void vtkInteractorStyleWIT::OnLeftButtonDown()
{
	_mouse_moved = false;
	_left_button_down = 1;
	FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);

	_mousedown_x = this->Interactor->GetEventPosition()[0];
	_mousedown_y = abs(this->Interactor->GetEventPosition()[1]);

	if (this->Interactor->GetControlKey()) 
	{
	  _GestureInteractor->BeginSelect(_mousedown_x, _mousedown_y);
	  //NotifyAllListeners(PEvent ( new EventRefreshViz(true) ) );
	  //_renderer->GetRenderWindow()->Render();
	  emit doRender();
	  //this->frame->update();
	  _interaction_mode = INTERACTION_DRAW_GESTURE;
	  return;
	} 

	// xxxdla trying to fix bug with interactions

	//if( _voiViz->OnLeftDown(_mousedown_x, _mousedown_y) )
	//	return ;
	Interactor->GetShiftKey()?StartPan():StartRotate();
}
void vtkInteractorStyleWIT::OnLeftButtonUp()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;
	int x = rwi->GetEventPosition()[0];
	int y = rwi->GetEventPosition()[1];

	if(_interaction_mode == INTERACTION_DRAW_GESTURE)
	{
	    _interaction_mode = INTERACTION_IDLE;
		_GestureInteractor->EndSelect(x,y,false);
		bool pathwayVisibility = this->frame->getPathwayActor()->GetVisibility();
		//bool pointsVisiblity  = _pathwayViz->PointsVisibility();

		// Hide Pathways
		bool bfalse = false; 
		if(this->frame->getPathwayActor())
			this->frame->getPathwayActor()->SetVisibility(bfalse);
		
		//_pathwayViz->SetPointsVisibility(bfalse);

		PCollModel model;

		// TODO: Handle either surface or touch mode
		if (true) {
			model = _GestureInteractor->PruneGesture2CollModel();
		} else if (false) {
			model = _GestureInteractor->SurfaceIntersectionGesture2CollModel();
		} else {
			cerr << "WARNING: Don't recognize gesture mode." << endl;
		}

		if(this->frame->getPathwayActor())
			this->frame->getPathwayActor()->SetVisibility(pathwayVisibility);
		//_pathwayViz->SetPointsVisibility(pointsVisiblity);

		emit this->filterByGesture(model);
		//NotifyAllListeners(PEvent ( new EventFilterPathwaysByGesture(model) ) );
		emit doRender();
		//_renderer->GetRenderWindow()->Render();
		//this->frame->update();
		return;
	}
	_left_button_down=0;

	_interaction_mode = INTERACTION_IDLE;

	if(_interaction_mode == INTERACTION_ROI_EDIT)
	{
		_interaction_mode = INTERACTION_IDLE;
		return;
	}

	switch (this->State)  
	{
	case VTKIS_DOLLY:
		EndDolly();
		break;
	case VTKIS_PAN:
		EndPan();
		break;
	case VTKIS_SPIN:
		EndSpin();
		break;
	case VTKIS_ROTATE:
		// only happens in mouse mode.
		EndRotate();
		break;
	}
	State = VTKIS_NONE;
	_interaction_mode = INTERACTION_IDLE;
	//this->frame->update();
}
void vtkInteractorStyleWIT::OnMiddleButtonDown()
{
	
	_mouse_moved = false;
	FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
	if (CurrentRenderer == NULL)
	  return;
	StartPan();
}
void vtkInteractorStyleWIT::OnRightButtonDown()
{
	qDebug("RIGHT MOUSE DOWN");
  _mouse_moved = false;
  _right_button_down = 1;

  FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
  if (CurrentRenderer == NULL)
    return;
  _mousedown_x = Interactor->GetEventPosition()[0];
  _mousedown_y = abs(Interactor->GetEventPosition()[1]);

  this->StartDolly();
  
  //cerr << "here" << endl;
  /*

  TODO: Port from Quench
  if (_voiEditor->GetEditingROIMode() && _voiEditor->ClickInsideTomogram(_mousedown_x, _mousedown_y))
    {
      _voiEditor->OnRightDown(_mousedown_x, _mousedown_y);
      _interaction_mode = INTERACTION_ROI_EDIT;
      return;
    }
	*/
  
}

void vtkInteractorStyleWIT::OnRightButtonUp()
{
  
  _interaction_mode = INTERACTION_IDLE;
        _right_button_down = 0;
	if(State == VTKIS_DOLLY)
		EndDolly();
	
	FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);

	_mousedown_x =     Interactor->GetEventPosition()[0];
	_mousedown_y = abs(Interactor->GetEventPosition()[1]);

}
void vtkInteractorStyleWIT::Notify()
{
	StartDolly();
}
void vtkInteractorStyleWIT::OnMouseWheelForward()
{
	FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
	emit doMoveActiveVolumeSlice(1);
}
void vtkInteractorStyleWIT::OnMouseWheelBackward()
{
	this->FindPokedRenderer(Interactor->GetEventPosition()[0], Interactor->GetEventPosition()[1]);
	emit doMoveActiveVolumeSlice(-1);
}
void vtkInteractorStyleWIT::Rotate()
{
	if (this->CurrentRenderer == NULL)
		return;

	vtkRenderWindowInteractor *rwi = this->Interactor;
	int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
	int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

	int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();
	double delta_elevation = -20.0/600.0;
	double delta_azimuth = -20.0/800.0;
	double rxf = (double)dx * delta_azimuth * MotionFactor;
	double ryf = (double)dy * delta_elevation * MotionFactor;
	vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	camera->Azimuth(rxf);
	camera->Elevation(ryf);
	double camvec[3];
	camera->GetDirectionOfProjection(camvec[0], camvec[1], camvec[2]);
	camera->OrthogonalizeViewUp();

	if (this->AutoAdjustCameraClippingRange)
		this->CurrentRenderer->ResetCameraClippingRange();

	if (rwi->GetLightFollowCamera()) 
		this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();

//	_volViz->AdjustOverlayOffset(camvec);
	//_renderer->GetRenderWindow()->Render();
	//this->frame->Frame();
	emit doRender();
	//_renderer->GetRenderWindow()->GetInteractor()->Render();
	
}
void vtkInteractorStyleWIT::Pan()
{
	if (CurrentRenderer == NULL)
	  return;

	vtkRenderWindowInteractor *rwi = this->Interactor;
	double viewFocus[4], focalDepth, viewPoint[3];
	double newPickPoint[4], oldPickPoint[4], motionVector[3];
	
	// Calculate the focal depth since we'll be using it a lot
	vtkCamera *camera = CurrentRenderer->GetActiveCamera();
	camera->GetFocalPoint(viewFocus);
	ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], 
	                            viewFocus);
	focalDepth = viewFocus[2];
	ComputeDisplayToWorld((double)rwi->GetEventPosition()[0], 
	                            (double)rwi->GetEventPosition()[1],
	                            focalDepth, 
	                            newPickPoint);
	// Has to recalc old mouse point since the viewport has moved,
	// so can't move it outside the loop
	ComputeDisplayToWorld((double)rwi->GetLastEventPosition()[0],
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
	    
	if (AutoAdjustCameraClippingRange)
	  CurrentRenderer->ResetCameraClippingRange();
	if (rwi->GetLightFollowCamera()) 
	  CurrentRenderer->UpdateLightsGeometryToFollowCamera();
	  
	emit doRender();
	//CurrentRenderer->GetRenderWindow()->Render();
	//this->frame->update();
}
void vtkInteractorStyleWIT::Dolly()
{
	//if (this->CurrentRenderer == NULL)
	//  return;
	//
	//vtkRenderWindowInteractor *rwi = this->Interactor;
	//double *center = this->CurrentRenderer->GetCenter();
	//int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
	//double dyf = MotionFactor * (double)(dy) / (double)(center[1]);
	//this->Dolly(pow((double)1.1, dyf));
}
int AlignToAxis(double v[3])
{
	int idx=0,sgn=1;
	double val = fabs(v[0]);
	if(fabs(v[1]) > val){idx = 1; val = fabs(v[1]);}
	if(fabs(v[2]) > val){idx = 2; val = fabs(v[2]);}

	sgn = v[idx] > 0?1:-1;
	v[0]=v[1]=v[2]=0;
	v[idx]=sgn;
	return idx;
}
void vtkInteractorStyleWIT::ResetView()
{
	double viewUp[3], focalpt[3],position[3];
	FindPokedRenderer(0,0);
	vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	camera->GetViewUp(viewUp);
	camera->GetFocalPoint(focalpt);
	camera->GetPosition(position);

	int idx=0;
	double rad=0;
	for(int i = 0; i < 3; i++)
	{
		position[i]-=focalpt[i];
		rad+=position[i]*position[i];
	}
	rad = sqrt(rad);
	position[0]/=rad; position[1]/=rad; position[2]/=rad; 
	AlignToAxis(viewUp); idx=AlignToAxis(position);
	position[idx]*=rad;

	camera->SetViewUp(viewUp);
	camera->SetPosition(position);

	emit doRender();
	//CurrentRenderer->GetRenderWindow()->Render();
	//this->frame->update();
}
void vtkInteractorStyleWIT::SaveScreenImage(const char *filename, bool pristine)
{
	//  int quality;
	//  this->FindPokedRenderer(0,0);
	//  vtkRenderWindow *myWindow = this->CurrentRenderer->GetRenderWindow();
	//  vtkWindowToImageFilter *pWindowImageFilter = vtkWindowToImageFilter::New();
	//  pWindowImageFilter->SetInput( myWindow );
	//  pWindowImageFilter->SetInputBufferType (VTK_RGB);
	//  pWindowImageFilter->ReadFrontBufferOff();
	//  pWindowImageFilter->ShouldRerenderOn();
	//  double bg[3];
	//  this->CurrentRenderer->GetBackground(bg[0],bg[1],bg[2]);
	//  int nCurActiveImage;
	//  if (pristine) {
	////     if (_gesture_type_selector) {
	////       _gesture_type_selector->setVisibility(false);
	////       _selection_mode_selector->setVisibility(false);
	////       scene->SetVisibility(false);
	////     }
	//    pWindowImageFilter->ReadFrontBufferOff();
	//    pWindowImageFilter->ShouldRerenderOn();
	//    myWindow->SetAAFrames(10);
	//    quality = 100;
	//    nCurActiveImage = _Scene->GetVolumeViz()->GetActiveImage();
	//    _Scene->GetVolumeViz()->SetActiveImage(-1);
	//    this->CurrentRenderer->SetBackground(0,0,0);
	//  }
	//  else {
	//    pWindowImageFilter->ReadFrontBufferOn();
	//    pWindowImageFilter->ShouldRerenderOff();
	//    quality = 75;
	//  }

	//  pWindowImageFilter->Modified();
	//  pWindowImageFilter->Update();

	//  //Find out the file extension
	//  int i; 
	//  for(i = strlen(filename)-1; i >=0 && filename[i] != '.'; i--);
	//  vtkImageWriter *pImageWriter = 0;
	//  if(_stricmp(".png", filename+i)==0)
	//   pImageWriter = vtkPNGWriter::New();
	//  else
	//  {
	//   pImageWriter = vtkJPEGWriter::New();
	//   ((vtkJPEGWriter*)pImageWriter)->SetQuality( quality );
	//   ((vtkJPEGWriter*)pImageWriter)->ProgressiveOff();
	//  }
	//  pImageWriter->SetInput( pWindowImageFilter->GetOutput() );
	//  pImageWriter->SetFileName( filename );
	//  pImageWriter->Write();
	//  pImageWriter->Delete();

	//  pWindowImageFilter->Delete();
	//  if (pristine) {
	//    this->CurrentRenderer->GetRenderWindow()->SetAAFrames(0);
	////     if (_gesture_type_selector) {
	////       _gesture_type_selector->setVisibility(true);
	////       _selection_mode_selector->setVisibility(true);
	////       scene->SetVisibility(true);
	////     }
	//    _Scene->GetVolumeViz()->SetActiveImage(nCurActiveImage);
	//    this->CurrentRenderer->SetBackground(bg[0],bg[1],bg[2]);
	//    this->frame->RefreshViz();    
	//  }
}
void vtkInteractorStyleWIT::Dolly(double factor)
{
	 if (CurrentRenderer == NULL)
	   return;
	 
	 vtkCamera *camera = CurrentRenderer->GetActiveCamera();
	 if (camera->GetParallelProjection())
	   camera->SetParallelScale(camera->GetParallelScale() / factor);
	 else
	   camera->Dolly(factor);
	 
	 if (AutoAdjustCameraClippingRange)
		CurrentRenderer->ResetCameraClippingRange();
	 if (Interactor->GetLightFollowCamera()) 
		CurrentRenderer->UpdateLightsGeometryToFollowCamera();
}
void vtkInteractorStyleWIT::StartSelect()
{
	//_GestureInteractor->SetRenderer(this->CurrentRenderer);
	//_GestureInteractor->SetPathwayViz(_Scene->GetPathwayViz());
	//_GestureInteractor->SetVolumeViz(_Scene->GetVolumeViz());
	//_GestureInteractor->StartSelect(this->Interactor->GetEventPosition()[0],
	//		  this->Interactor->GetEventPosition()[1]);
}
void vtkInteractorStyleWIT::EndSelect()
{
	//DTIPathwayAssignment *oldAssn = _Scene->GetPathwayViz()->GetAssignedPathway()->copy();
	//_GestureInteractor->EndSelect();
	//_Scene->GetPathwayViz()->PathwaySelectionGesture(_GestureInteractor, CurrentRenderer);
	//this->frame->AssignmentMade(oldAssn);
	//this->frame->RefreshViz();
}

