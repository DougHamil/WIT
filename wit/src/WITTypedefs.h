#ifndef WIT_TYPEDEFS_H
#define WIT_TYPEDEFS_H

#include <util/typedefs.h>

#define VTK_SAFE_DELETE(a) if(a){ (a)->Delete(); a = NULL;} 
#define MAX_LINE_WIDTH 7
static Colord USER_GESTURE_COLOR(1, 23/255., 23/255.);
//! Stores the colors of a particular colormap
struct ColorMap
{
	ColorMap(std::string name):Name(name){}
	std::string Name;
	std::vector<Coloruc> Colors;
};

enum DTISceneActorID 
{
  DTI_ACTOR_SAGITTAL_TOMO = 0,
  DTI_ACTOR_CORONAL_TOMO  = 1,
  DTI_ACTOR_AXIAL_TOMO= 2,
  DTI_ACTOR_ROI = 3,
  DTI_ACTOR_TRACTS = 4,
  DTI_ACTOR_SURFACE = 5,
  DTI_ACTOR_LOCAL = 6,
  DTI_ACTOR_NONE = 7
};


//! Forward declaration of classes to declare shared pointers

class Overlay;
class OverlayItem;
/*	
	shared_ptr is not a part of main STL library in linux
	hence we need two different implementation.
	
	shared_ptr are extensively used to automate memory 
	management. shared_ptr of any class has a suffix P
	for pointer.
*/

#ifdef _WIN32
	#include <memory>

	typedef std::shared_ptr<Overlay>			POverlay;
	typedef std::shared_ptr<OverlayItem>		POverlayItem;
#else
	#include <tr1/memory>
	typedef std::tr1::shared_ptr<Overlay>		POverlay;
	typedef std::tr1::shared_ptr<OverlayItem>	POverlayItem;
#endif

#endif