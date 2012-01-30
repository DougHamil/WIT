/*
 * vtkInteractorStyleViewFrame.h
 *
 *  Created on: Jun 20, 2011
 *      Author: doug
 */

#ifndef VTKINTERACTORSTYLEVIEWFRAME_H_
#define VTKINTERACTORSTYLEVIEWFRAME_H_

#include "DTI_typedefs.h"
#include "vtkInteractorStyleDTI.h"
#include "SurfaceInstance.h"

class VOI;
class ViewCone;

#include <vector>
#include <util/VOIManager.h>

class vtkPolyData;
class vtkImageData;
class DTIFilterPathwayScalar;
class DTIPathway;
class vtkLookupTable;
class DTIFrame;
class VOIMesh;

#include <list>
#include <map>
#include <util/typedefs.h>

class CameraControls;
class ScalarVolume;
class DTIFilterAlgorithm;
class vtkCellPicker;
class vtkCellLocator;
class wxArrayString;
class vtkColorTransferFunction;
class DTITomograms;
class vtkCamera;
class vtkPropPicker;
class SurfaceVisualization;
class vtkTextActor;

#include <vtkSmartPointer.h>
// vtkInteractorStyle reserves 0-9
#define VTKIS_MOVEVOI 10
#define VTKIS_PAINTSURFACE 11
#define VTKIS_PICKCELL 12

typedef std::vector<VOI*>::iterator voiIter;

class vtkInteractorStyleViewFrame : public vtkInteractorStyleDTI
{
  public:
    friend class DTIFrame;
    friend class DTIViewFrame;
    static vtkInteractorStyleViewFrame *New();
    vtkTypeRevisionMacro(vtkInteractorStyleViewFrame,vtkInteractorStyle);
    void PrintSelf(ostream& os, vtkIndent indent);

    void SetScreenShotMode(bool pred);

    void LoadFromXML(TiXmlElement *elem);
    void SaveToXML(TiXmlElement *elem);
     // Description:
    // The sub styles need the interactor too.

    // Description:
    // Event bindings controlling the effects of pressing mouse buttons
    // or moving the mouse.
    virtual void OnChar();
    virtual void OnKeyPress();
    virtual void OnMouseMove();
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
    virtual void OnMiddleButtonDown();
    virtual void OnMiddleButtonUp();
    virtual void OnRightButtonDown();
    virtual void OnRightButtonUp();
    virtual void OnMouseWheelForward();
    virtual void OnMouseWheelBackward();
    virtual void OnRefresh(DTIFrame *frame);

    void OnDataStoreSet();
    void OnSurfaceLoaded(SurfaceVisualization *surf);
    void OnVOIInserted(VOI* voi);
    void OnVOIDeleted(VOI* voi);
    void OnActiveVOIChanged(VOI* oldVOI);
    void SetTomogramSlices(unsigned int, unsigned int, unsigned int);
    void OnSetSurfaceColor(unsigned char r, unsigned char g, unsigned char b);

    void CycleSurface();

    void NextLineWidth ();
    void SelectTomogram(AnatomicalAxis axis);
    void SelectNextVisibleTomogram();
    void ToggleVOIVisibility();
    void TogglePathwaysVisibility();
    void ToggleTomogramVisibility(AnatomicalAxis axis);
    void ToggleSurfaceVisibility(Hemisphere hemi);
    void ToggleArcsVisibility();
    void SetSurfaceRepresentation(Hemisphere hemi, int rep);

    void SetActiveVOI(VOI* voi);
    void SetActiveSurface(SurfaceVisualization* surf);

    void SelectSlicePoint(AnatomicalAxis slice, int selectionX, int selectionY, double pick[3]);

    std::vector<int> *constructMeshPatch (vtkPolyData *inputData,
             int cellID,
             const double patchDim[3]);

    void SetVOIMotion (VOIMotionType type, int id);

    vtkActor *RebuildPatch(VOI *voi, vtkPolyData *patchData, bool ghostPatch);
    void MoveVOI();
    void DrawOnSlice();
    void InsertVOIFromFile();

    // These methods for the different interactions in different modes
    // are overridden in subclasses to perform the correct motion. Since
    // they are called by OnTimer, they do not have mouse coord parameters
    // (use interactor's GetEventPosition and GetLastEventPosition)
    void Rotate();
    void Spin();
    void Pan();
    void Dolly();

    void setFilterOp (DTIQueryOperation op);
    bool setFilterText (const char *str);

    void setVOISymmetry (int otherID);
    void StainPathways();
    void SetVOI (int activeID);
    void CycleVOI ();

    vtkActor *BuildActorFromMesh(vtkPolyData *mesh);
    vtkPolyData *StripTriangles(vtkPolyData *mesh);

    void SaveState();
    void RestoreState();

    void SavePathwaysPDB();

    void CycleBackground(bool dir);

    void SetSubjectDatapathInternal(const char *path);

    void OnBackgroundsLoaded(bool);
    void OnSelectedBackgroundChanged(ScalarVolume* oldBackground);

    void ResetView();

    void SetTemporaryCamera(vtkCamera *cam);

    SurfaceVisualization  *GetActiveSurface() const {return activeSurface;}

    VOI                   *GetActiveVOI() const {return activeVOI;}
    bool displayDirty;
  protected:
    vtkInteractorStyleViewFrame();
    ~vtkInteractorStyleViewFrame();

    ScalarVolume *GetCurrentBackground ();
    void MoveCurrentTomogram (int amount);
    void ToggleVisibility(vtkProp *actor);

    void SavePathwaysImageInternal(const char *path);
    void SavePathwaysPDBInternal(const char *path);

    void PickPoint();
    int PickCell(vtkCellLocator *locator, int mouseX, int mouseY, double pickedPoint[3]);
    void AnimRotate(int dx, int dy);
    bool bAnimRotate;
    bool bAnimAxial;
    bool bAnimSagittal;
    bool bAPosSlice;
    bool bSPosSlice;
    void AnimSSlice();
    void AnimASlice();

    void SetArcsVisibility(bool pred);

    VOI *findVOIByID(int id);

    double MotionFactor;

    bool binitializedTomograms;

    //  int cellID;
    int lineWidthSetting;
    VOI* activeVOI;

    int current_background_id;

    double minPathwayLength;

    virtual void Dolly(double factor);

    void OnPathwaysLoaded();

    int my_meshVisibility;
    // Active tomogram border
    vtkPolyData         *TomoBorderPolyData;
    vtkPolyDataMapper   *TomoBorderMapper;
    vtkActor            *TomoBorderActor;
    vtkActor            *pathwaysActor;
    void DisplayTomoBorder(bool on);

    std::map<SurfaceVisualization*, SurfaceInstance*>  surfaceMapping;
    SurfaceVisualization                        *activeSurface;
    std::pair<Hemisphere, vtkCellLocator*>      activeSurfaceCellLocator;

  private:

    int surfaceRepresentation;
    vtkTextActor *sceneInfo;

    void SetCellLocatorFromVisibility(unsigned int vis);
    void UpdateSceneInfo();
    void UpdateLUT (double window, double level);

    vtkActor *CreatePathwayArcs();

    double _lut_level;
    double _lut_window;

    DTITomograms *tomograms;
    AnatomicalAxis selectedTomogram;


    bool voiVisibilityToggle;
    bool voiLabelsToggle;
    bool pathwayVisibilityToggle;
    bool arcsVisible;

    vtkInteractorStyleViewFrame(const vtkInteractorStyleViewFrame&);  // Not implemented.
    void operator=(const vtkInteractorStyleViewFrame&);  // Not implemented.

    void ResetState();
    void SavePathwayState(std::ostream &os);
    void RestorePathwayState(std::istream &is);

    vtkSmartPointer<vtkCamera> oldCamera;

    DTIPathwayVisualization *currentPV;

};


#endif /* VTKINTERACTORSTYLEVIEWFRAME_H_ */
