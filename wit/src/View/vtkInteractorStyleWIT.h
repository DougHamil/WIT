/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef VTKINTERACTORSTYLEWIT_H
#define VTKINTERACTORSTYLEWIT_H

#include "vtkInteractorStyle.h"

#include <vector>
#include <list>
#include <map>

class CameraControls;
class vtkCellPicker;
class vtkCellLocator;
class vtkColorTransferFunction;

class vtkInteractorStyleWIT : public vtkInteractorStyle
{
public:
  ~vtkInteractorStyleWIT();
  static vtkInteractorStyleWIT *New();
  vtkTypeRevisionMacro(vtkInteractorStyleWIT,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void OnChar();
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion. Since
  // they are called by OnTimer, they do not have mouse coord parameters
  // (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void Rotate();
  virtual void Spin();
  virtual void Pan();
  virtual void Dolly();
  
  virtual void ResetView();  

protected:
  virtual void Dolly(double factor);
  vtkInteractorStyleWIT();

  double MotionFactor;

  CameraControls* camera_controls;

private:

};

#endif
