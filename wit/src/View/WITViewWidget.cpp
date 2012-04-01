#include "WITViewWidget.h"

// - VTK
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>

WITViewWidget::WITViewWidget(QWidget *parent, const QGLWidget *shared)
	:QVTKWidget2(parent, shared)
{
	this->setSizePolicy(QSizePolicy::Policy::Maximum,QSizePolicy::Policy::Maximum);
	//this->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	this->GetRenderWindow()->SetAAFrames(false);
	this->GetRenderWindow()->SetLineSmoothing(false);
	this->GetRenderWindow()->SetPolygonSmoothing(false);
	this->SetUseTDx(false);

	// Create each of our frames
	for(int i = 0; i < WITFRAME_COUNT; i++) 
	{
		this->frames[i] = new WITFrame();
	}

	this->activeFrameCount = -1;
	this->onSetViewCount(4);
}

void WITViewWidget::contextMenuEvent(QContextMenuEvent *e)
{
	WITFrame *frame = this->getFrameAtPosition(e->x(), e->y());

	if(!frame)
		return;

	frame->showContextMenu(e);
}

void WITViewWidget::onSetViewCount(int count)
{
	// Don't do anything if this count is already shown
	if(this->activeFrameCount == count)
		return;

	// Remove all renderers
	this->GetRenderWindow()->GetRenderers()->RemoveAllItems();
	
	// Add the frames' renderers to the render window
	for(int i = 0; i < count; i++)
	{
		this->GetRenderWindow()->AddRenderer(this->frames[i]->getRenderer());
	}

	// Setup viewport sizes based on count
	switch(count)
	{
	case 1:
		this->frames[0]->getRenderer()->SetViewport(0.0, 0.0, 1.0, 1.0);
		break;
	case 2:
		this->frames[0]->getRenderer()->SetViewport(0.0, 0.0, 0.5, 1.0);
		this->frames[1]->getRenderer()->SetViewport(0.5, 0.0, 1.0, 1.0);
		break;
	case 4:
		this->frames[0]->getRenderer()->SetViewport(0.0, 0.0, 0.5, 0.5);
		this->frames[1]->getRenderer()->SetViewport(0.5, 0.0, 1.0, 0.5);
		this->frames[2]->getRenderer()->SetViewport(0.0, 0.5, 0.5, 1.0);
		this->frames[3]->getRenderer()->SetViewport(0.5, 0.5, 1.0, 1.0);
		break;
	default:
		break;
	}
}

WITFrame *WITViewWidget::getFrameAtPosition(int x, int y)
{
	qDebug("Get Frame Pos: %d, %d",x,y);
	float w = (float)this->width();
	float h = (float)this->height();

	y = this->height() - y;

	// Go through all frames and see if they contain these coordinates
	for(int i = 0; i < WITFRAME_COUNT; i++)
	{
		WITFrame *frame = this->frames[i];
		double *viewport = frame->getRenderer()->GetViewport();
		qDebug("Testing viewport %p: %f, %f, %f, %f",frame,viewport[0], viewport[1], viewport[2], viewport[3]);
		if(this->GetRenderWindow()->GetRenderers()->IsItemPresent(frame->getRenderer()) != -1
			&& x >= viewport[0] * w && x <= viewport[2] * w
			&& y >= viewport[1] * h && y <= viewport[3] * h)
			return frame;
	}

	// Not found
	return 0;
}


int *WITViewWidget::screenToFrame(WITFrame *frame, int x, int y)
{
	int *ret = new int[2];
	double *viewport = frame->getRenderer()->GetViewport();
	ret[0] = x - viewport[0] * this->width();
	ret[1] = y - viewport[1] * this->height();

	return ret;
}