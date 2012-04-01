/** 
	This code was ported from Quench
**/

#ifndef __vtkInteractorStyleWIT_h
#define __vtkInteractorStyleWIT_h

#include <QObject>

#include "vtkInteractorStyle.h"
#include "../WITTypedefs.h"

#include <util/typedefs.h>
#include <string>

class WITViewWidget;
class vtkRenderWindowInteractor;
class vtkCellLocator;
class ModeSelector;
class VisibilityPanel;
class WITGestureController;
class vtkPropPicker;
class SceneWindow;
class WITVolumeViz;
class WITPathwayViz;
class WITROIViz;

enum WITInteractionMode
{
	INTERACTION_IDLE,
	INTERACTION_DRAW_GESTURE,
	INTERACTION_ROI_MANIPULATE,
	INTERACTION_PATHWAYS,
	INTERACTION_ROI_EDIT,
};

//! Main class to handle interaction with the vtk renderer
class vtkInteractorStyleWIT : public QObject, public vtkInteractorStyle
{
	Q_OBJECT
signals:
	
	void beginGesture(int,int);
	void addGesturePoint(int,int);
	void endGesture(int,int,bool);
	void doRender();
	void doMoveActiveVolumeSlice(int dist);
public:
	static vtkInteractorStyleWIT *New();
	vtkTypeRevisionMacro(vtkInteractorStyleWIT, vtkInteractorStyle);
	void PrintSelf(ostream& os, vtkIndent indent);

	void Init(WITViewWidget *widget);
	// Description:
	// Event bindings controlling the effects of pressing mouse buttons
	// or moving the mouse.
	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp() { this->OnLeftButtonUp(); }
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp()  ;
	virtual void OnMouseWheelForward();
	virtual void OnMouseWheelBackward();
	// These methods for the different interactions in different modes
	// are overridden in subclasses to perform the correct motion. Since
	// they are called by OnTimer, they do not have mouse coord parameters
	// (use interactor's GetEventPosition and GetLastEventPosition)
	virtual void Rotate();
	virtual void Pan();
	virtual void Dolly();

	//! This function is called to handle the timer fired after a right down.
	void Notify();
	
	void ResetView();
	void SaveScreenImage (const char *filename, bool pristine = false);

	// Gestures
	void StartSelect();
	void EndSelect();
 
	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);


	PROPERTY_READONLY(vtkRenderer*, _renderer, Renderer);
	PROPERTY_READONLY(WITVolumeViz*, _volViz, VolumeViz);
	PROPERTY_READONLY(WITPathwayViz*, _pathwayViz, PathwayViz);
	PROPERTY_READONLY(WITROIViz*, _voiViz, ROIViz);
	PROPERTY_READONLY(WITGestureController*, _GestureInteractor, GestureInteractor);
	//PROPERTY_READONLY(qROIEditor*, _voiEditor, voiEditor);
	
protected:
	vtkInteractorStyleWIT();
	~vtkInteractorStyleWIT();
	//int PickCell (vtkActor *actor, vtkCellLocator *locator, int mouseX, int mouseY, double pickedPoint[3]){return 0;}
	WITViewWidget *view;
	virtual void Dolly(double factor);
	vtkSetMacro(State,int);

	void ToggleROIEditingMode();
	void HandDrawROI();

private:
	vtkPropPicker* _propPicker ;
	vtkInteractorStyleWIT(const vtkInteractorStyleWIT&);  // Not implemented.
	long _old_cursor_x;
	long _old_cursor_y;
	int _mousedown_x;
	int _mousedown_y;
	int _saved_mouse_x;
	int _saved_mouse_y;
	bool _mouse_moved;
	int _left_button_down;
	int _right_button_down;
	double MotionFactor;

	WITInteractionMode _interaction_mode;

	// UI Viz
	std::string _strIconsPath;
};

#endif
