/****************************************************************************
**
** Copyright (C) VCreate Logic Private Limited, Bangalore
**
** Use of this file is limited according to the terms specified by
** VCreate Logic Private Limited, Bangalore.  Details of those terms
** are listed in licence.txt included as part of the distribution package
** of this file. This file may not be distributed without including the
** licence.txt file.
**
** Contact info@vcreatelogic.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "vtkQtGraphicsViewRenderWindow.h"

#ifdef VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_SUPPORTED

#include <QGraphicsView>
#include <QGLWidget>
#include <QGLColormap>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSysInfo>
#include <QDebug>

#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <QVTKWidget.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>

class vtkQtGraphicsViewRenderWindowMonitor;

struct vtkQtGraphicsViewRenderWindowData
{
    vtkQtGraphicsViewRenderWindowData (): itemUnderMouse(false), 
        iren(NULL), inPaintEvent(false), autoTiling(true), scene(0) {}

    bool itemUnderMouse;
    QVTKInteractor* iren;
    bool inPaintEvent;
    vtkRenderWindow* renderWindow;
    vtkQtGraphicsViewRenderWindowMonitor* monitor;
	QGraphicsScene* scene;
    bool autoTiling;
	QList<vtkQtGraphicsProxyWidget*> proxyWidgetList;

	void positionWidget(vtkQtGraphicsProxyWidget* widget, ProxyWidget::Position pos){
		if(!scene)
			return;

		switch(pos)
		{
		case ProxyWidget::TopLeft:
			widget->setPos(scene->sceneRect().left(), 0);
			break;
		case ProxyWidget::TopMiddle:
			{
			int x = scene->sceneRect().center().x();
			widget->setPos(x, 0);
			}
			break;
		case ProxyWidget::TopRight:
			{
			int x = scene->sceneRect().right();
			x = x - widget->size().width();
			widget->setPos(x, 0);
			}
			break;

		case ProxyWidget::RightMiddle:
			{
			int x = scene->sceneRect().right();
			x = x - widget->size().width();
			int y = scene->sceneRect().center().y();
			widget->setPos(x, y);
			}
			break;
		case ProxyWidget::BottomRight:
			{
				int x = scene->sceneRect().right();
				x = x - widget->size().width();
				int y = scene->sceneRect().bottom();
				y = y - widget->size().height();
				widget->setPos(x, y);
			}
			break;
		case ProxyWidget::BottomMiddle:
			{
				int x = scene->sceneRect().center().x();
				int y = scene->sceneRect().bottom();
				y = y - widget->size().height();
				widget->setPos(x, y);
			}
			break;
		case ProxyWidget::BottomLeft:
			{
				int x = scene->sceneRect().left();
				int y = scene->sceneRect().bottom();
				y = y - widget->size().height();
				widget->setPos(x, y);
			}
			break;
		case ProxyWidget::LeftMiddle:
			{
				int x = scene->sceneRect().left();
				int y = scene->sceneRect().center().y();
				widget->setPos(x, y);
			}
			break;
		default:
			break;
		}
	}

};

class vtkQtGraphicsViewRenderWindowMonitor : public vtkCommand
{
public:
    vtkQtGraphicsViewRenderWindow* RenderWindow;

    void Execute(vtkObject * /*caller*/, unsigned long /*eventId*/, void * /*callData*/) {
        if( RenderWindow->isAutoRendererTiligEnabled() )
            QMetaObject::invokeMethod(RenderWindow, "tileRenderers", Qt::QueuedConnection);
    }
};

vtkQtGraphicsViewRenderWindow::vtkQtGraphicsViewRenderWindow(QWidget* parent )
    : QGraphicsView(parent), vtkRenderWindowClass()
{
    d = new vtkQtGraphicsViewRenderWindowData;
    d->renderWindow = (vtkRenderWindow*)(this);

    d->monitor = new vtkQtGraphicsViewRenderWindowMonitor;
    d->monitor->RenderWindow = this;
    d->renderWindow->GetRenderers()->AddObserver(vtkCommand::ModifiedEvent, d->monitor);

	d->scene = new QGraphicsScene(this);
	this->setScene(d->scene);
    this->setViewport(new QGLWidget(QGLFormat()));
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setMouseTracking(false);

    d->iren = QVTKInteractor::New();
    this->SetInteractor(d->iren);
    d->iren->Initialize();

    vtkInteractorStyle* s = vtkInteractorStyleTrackballCamera::New();
    d->iren->SetInteractorStyle(s);

    d->iren->Delete();
    s->Delete();
}

vtkQtGraphicsViewRenderWindow::~vtkQtGraphicsViewRenderWindow()
{
    d->renderWindow->GetRenderers()->RemoveObserver(d->monitor);
    delete d->monitor;
    delete d;
}

void vtkQtGraphicsViewRenderWindow::setAutoRendererTilingEnabled(bool val)
{
    d->autoTiling = val;
}

bool vtkQtGraphicsViewRenderWindow::isAutoRendererTiligEnabled() const
{
    return d->autoTiling;
}

vtkQtGraphicsProxyWidget* vtkQtGraphicsViewRenderWindow::addWidget(QWidget* widget,
																   ProxyWidget::Position pos,
																   Qt::WindowFlags flags)
{
	if(!widget)
		return 0;

	if(flags == 0)
		flags = Qt::Tool|Qt::FramelessWindowHint|Qt::CustomizeWindowHint;

	vtkQtGraphicsProxyWidget* widgetItem = new vtkQtGraphicsProxyWidget(this, flags);
    widgetItem->setWidget(widget);
    widgetItem->setPos(-widgetItem->boundingRect().topLeft());
    widgetItem->setFlag(QGraphicsItem::ItemIsMovable);
    widgetItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    widgetItem->setWindowTitle(widget->windowTitle());
	widgetItem->setWidgetPosition(pos);

	if(!d->proxyWidgetList.contains(widgetItem))
		d->proxyWidgetList.append(widgetItem);

	d->positionWidget(widgetItem, pos);
    this->scene()->addItem(widgetItem);

	return widgetItem;
}

void vtkQtGraphicsViewRenderWindow::MakeCurrent()
{
    QGLWidget* glw = qobject_cast<QGLWidget*>(this->viewport());
    if(glw)
        glw->makeCurrent();
}

void vtkQtGraphicsViewRenderWindow::Initialize(void)
{
#ifdef Q_WS_WIN
    this->DeviceContext = this->getDC();
    this->WindowId = this->viewport()->winId();
    this->OwnWindow = 1;
#endif

#ifdef Q_WS_X11
    this->WindowId = this->viewport()->winId();
    this->OwnWindow = 1;
    this->DisplayId = QX11Info::display();
#endif

    this->SetSize(this->viewport()->width(), this->viewport()->height());
}

void vtkQtGraphicsViewRenderWindow::Finalize(void)
{
    vtkRendererCollection* rCol = this->GetRenderers();
    rCol->InitTraversal();
    QList<vtkRenderer*> rendererList;

    while(1)
    {
        vtkRenderer* renderer = rCol->GetNextItem();
        if(!renderer)
            break;
        rendererList.append(renderer);
    }

    Q_FOREACH(vtkRenderer* renderer, rendererList)
        this->RemoveRenderer(renderer);
}

void vtkQtGraphicsViewRenderWindow::CreateAWindow()
{
}

void vtkQtGraphicsViewRenderWindow::DestroyWindow()
{
}

void vtkQtGraphicsViewRenderWindow::Render()
{
    if(d->inPaintEvent)
        vtkRenderWindowClass::Render();
    else
        this->viewport()->repaint();
}

void vtkQtGraphicsViewRenderWindow::drawBackground(QPainter *painter, const QRectF& rect )
{
    painter->beginNativePainting();

    // We will need to check, what other GL flags have to be enabled.
    glEnable(GL_DEPTH_TEST);
    this->GetInteractor()->Render();
    glDisable(GL_DEPTH_TEST);

    painter->endNativePainting();

#ifdef Q_WS_WIN
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(rect.left(), rect.right(), rect.bottom(), rect.top(), -1000, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_LIGHTING);
#endif
}

void vtkQtGraphicsViewRenderWindow::paintEvent(QPaintEvent* event)
{
    d->inPaintEvent = true;
    MakeCurrent();
    QGraphicsView::paintEvent(event);
    d->inPaintEvent = false;
}

void vtkQtGraphicsViewRenderWindow::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    QRectF sceneRect(this->viewport()->rect());
    setSceneRect(sceneRect);
    this->scene()->setSceneRect(sceneRect);

    this->SetSize(this->viewport()->width(), this->viewport()->height());
    if(d->iren)
        d->iren->SetSize(this->viewport()->width(), this->viewport()->height());

	Q_FOREACH(vtkQtGraphicsProxyWidget* proxyWidget, d->proxyWidgetList)
	{
		d->positionWidget(proxyWidget, proxyWidget->widgetPosition());
	}
}

void vtkQtGraphicsViewRenderWindow::mousePressEvent(QMouseEvent* e)
{
    if(this->itemAt(e->pos()))
    {
        d->itemUnderMouse = true;
        QGraphicsView::mousePressEvent(e);
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;

    if(!iren || !iren->GetEnabled())
        return;

    QPoint p = this->mapToScene(e->x(), e->y()).toPoint();
    iren->SetEventInformationFlipY(p.x(), p.y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                   0,
                                   e->type() == QEvent::MouseButtonDblClick ? 1 : 0);

    switch(e->button())
    {
    case Qt::LeftButton:
        iren->InvokeEvent(vtkCommand::LeftButtonPressEvent, e);
        break;

    case Qt::MidButton:
        iren->InvokeEvent(vtkCommand::MiddleButtonPressEvent, e);
        break;

    case Qt::RightButton:
        iren->InvokeEvent(vtkCommand::RightButtonPressEvent, e);
        break;

    default:
        break;
    }
}

void vtkQtGraphicsViewRenderWindow::mouseMoveEvent(QMouseEvent* e)
{
    if(d->itemUnderMouse)
    {
        QGraphicsView::mouseMoveEvent(e);
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;

    QPoint p = this->mapToScene(e->x(), e->y()).toPoint();
    iren->SetEventInformationFlipY(p.x(), p.y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

    iren->InvokeEvent(vtkCommand::MouseMoveEvent, e);
}

void vtkQtGraphicsViewRenderWindow::mouseReleaseEvent(QMouseEvent* e)
{
    if(d->itemUnderMouse)
    {
        QGraphicsView::mouseReleaseEvent(e);
        d->itemUnderMouse = false;
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;

    QPoint p = this->mapToScene(e->x(), e->y()).toPoint();
    iren->SetEventInformationFlipY(p.x(), p.y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

    switch(e->button())
    {
    case Qt::LeftButton:
        iren->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, e);
        break;

    case Qt::MidButton:
        iren->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, e);
        break;

    case Qt::RightButton:
        iren->InvokeEvent(vtkCommand::RightButtonReleaseEvent, e);
        break;

    default:
        break;
    }
}

void vtkQtGraphicsViewRenderWindow::wheelEvent(QWheelEvent* e)
{
    if(this->itemAt(e->pos()))
    {
        QGraphicsView::wheelEvent(e);
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;

    QPoint p = this->mapToScene(e->x(), e->y()).toPoint();
    iren->SetEventInformationFlipY(p.x(), p.y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

    this->SetSize(this->viewport()->width(), this->viewport()->height());
    this->vtkRenderWindow::SetSize(this->viewport()->width(), this->viewport()->height());
    if(d->iren)
        d->iren->SetSize(this->viewport()->width(), this->viewport()->height());

    if(e->delta() > 0)
        iren->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e);
    else
        iren->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e);
}

// Helper functions for keyPressEvent() and keyReleaseEvent()
static const char* ascii_to_key_sym(int);
static const char* qt_key_to_key_sym(Qt::Key);

void vtkQtGraphicsViewRenderWindow::keyPressEvent(QKeyEvent* e)
{
    if(this->scene()->focusItem())
    {
        QGraphicsView::keyPressEvent(e);
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;
    
    // get key and keysym information
#if QT_VERSION < 0x040000
    int ascii_key = e->text().length() ? e->text().unicode()->latin1() : 0;
#else
    int ascii_key = e->text().length() ? e->text().unicode()->toLatin1() : 0;
#endif
    const char* keysym = ascii_to_key_sym(ascii_key);
    if(!keysym)
        // get virtual keys
        keysym = qt_key_to_key_sym(static_cast<Qt::Key>(e->key()));
    
    if(!keysym)
        keysym = "None";
    
    // give interactor event information
#if QT_VERSION < 0x040000
    iren->SetKeyEventInformation(
            (e->state() & Qt::ControlButton),
            (e->state() & Qt::ShiftButton),
            ascii_key, e->count(), keysym);
#else
    iren->SetKeyEventInformation(
            (e->modifiers() & Qt::ControlModifier),
            (e->modifiers() & Qt::ShiftModifier),
            ascii_key, e->count(), keysym);
#endif
    
    // invoke vtk event
    iren->InvokeEvent(vtkCommand::KeyPressEvent, e);
    
    // invoke char event only for ascii characters
    if(ascii_key)
        iren->InvokeEvent(vtkCommand::CharEvent, e);
}

void vtkQtGraphicsViewRenderWindow::keyReleaseEvent(QKeyEvent* e)
{
    if(this->scene()->focusItem())
    {
        QGraphicsView::keyPressEvent(e);
        return;
    }

    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;
    
    // get key and keysym info
#if QT_VERSION < 0x040000
    int ascii_key = e->text().length() ? e->text().unicode()->latin1() : 0;
#else
    int ascii_key = e->text().length() ? e->text().unicode()->toLatin1() : 0;
#endif
    const char* keysym = ascii_to_key_sym(ascii_key);
    if(!keysym)
        // get virtual keys
        keysym = qt_key_to_key_sym((Qt::Key)e->key());
    
    if(!keysym)
        keysym = "None";
    
    // give event information to interactor
#if QT_VERSION < 0x040000
    iren->SetKeyEventInformation(
            (e->state() & Qt::ControlButton),
            (e->state() & Qt::ShiftButton),
            ascii_key, e->count(), keysym);
#else
    iren->SetKeyEventInformation(
            (e->modifiers() & Qt::ControlModifier),
            (e->modifiers() & Qt::ShiftModifier),
            ascii_key, e->count(), keysym);
#endif
    
    // invoke vtk event
    iren->InvokeEvent(vtkCommand::KeyReleaseEvent, e);
}

void vtkQtGraphicsViewRenderWindow::enterEvent(QEvent* e)
{
    vtkRenderWindowInteractor* iren = d->iren;
    if(!iren || !iren->GetEnabled())
        return;
    
    iren->InvokeEvent(vtkCommand::EnterEvent, e);
}

void vtkQtGraphicsViewRenderWindow::leaveEvent(QEvent* e)
{
    vtkRenderWindowInteractor* iren = NULL;
    if(this)
        iren = this->GetInteractor();
    
    if(!iren || !iren->GetEnabled())
        return;
    
    iren->InvokeEvent(vtkCommand::LeaveEvent, e);
}

void vtkQtGraphicsViewRenderWindow::tileRenderers()
{
    vtkRendererCollection* col = d->renderWindow->GetRenderers();
    int nrItems = col->GetNumberOfItems();
    int index = 0;
    bool odd = (nrItems % 2);
    double x=0.0f, y=0.0f;
    int nrRows = 0;
    double dy = 0.0f;
    int nrCols = 0;
    double dx = 0.0f;
    vtkRenderer* ren = 0;
    bool lastRow = false;

    col->InitTraversal();
    ren = col->GetNextItem();

    if( !nrItems )
        return;

    if( nrItems == 1 )
    {
        ren->SetViewport(0.0f, 0.0f, 1.0f, 1.0f);
        return;
    }

    if(odd)
        nrRows = (nrItems-1)/2;
    else
        nrRows = (nrItems)/2;
    nrCols = nrItems/nrRows;

    dx = 1.0f / (double)nrCols;
    dy = 1.0f / (double)nrRows;

    while(ren)
    {
        ren->SetViewport(x, y, x+dx, y+dy);
        ++index;
        x += dx;
        if( !lastRow && !(index%nrCols) )
        {
            y += dy;
            x = 0.0f;
            if( index + nrCols + 1 == nrItems )
            {
                ++nrCols;
                dx = 1.0f / (double)nrCols;
                lastRow = true;
            }
        }
        ren = col->GetNextItem();
    }

    this->viewport()->update();
}

///////////////////////////////////////////////////////////////////////////////

#define QVTK_HANDLE(x,y) \
  case x : \
    ret = y; \
    break;

static const char *AsciiToKeySymTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, "Tab", 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    "space", "exclam", "quotedbl", "numbersign",
    "dollar", "percent", "ampersand", "quoteright",
    "parenleft", "parenright", "asterisk", "plus",
    "comma", "minus", "period", "slash",
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "colon", "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "bracketleft",
    "backslash", "bracketright", "asciicircum", "underscore",
    "quoteleft", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "Delete",
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const char* ascii_to_key_sym(int i)
{
    if(i >= 0)
        return AsciiToKeySymTable[i];

    return 0;
}

const char* qt_key_to_key_sym(Qt::Key i)
{
    const char* ret = 0;
    switch(i)
    {
        // Cancel
        QVTK_HANDLE(Qt::Key_Backspace, "BackSpace")
        QVTK_HANDLE(Qt::Key_Tab, "Tab")
#if QT_VERSION < 0x040000
        QVTK_HANDLE(Qt::Key_BackTab, "Tab")
#else
        QVTK_HANDLE(Qt::Key_Backtab, "Tab")
#endif
        //QVTK_HANDLE(Qt::Key_Clear, "Clear")
        QVTK_HANDLE(Qt::Key_Return, "Return")
        QVTK_HANDLE(Qt::Key_Enter, "Return")
        QVTK_HANDLE(Qt::Key_Shift, "Shift_L")
        QVTK_HANDLE(Qt::Key_Control, "Control_L")
        QVTK_HANDLE(Qt::Key_Alt, "Alt_L")
        QVTK_HANDLE(Qt::Key_Pause, "Pause")
        QVTK_HANDLE(Qt::Key_CapsLock, "Caps_Lock")
        QVTK_HANDLE(Qt::Key_Escape, "Escape")
        QVTK_HANDLE(Qt::Key_Space, "space")
        //QVTK_HANDLE(Qt::Key_Prior, "Prior")
        //QVTK_HANDLE(Qt::Key_Next, "Next")
        QVTK_HANDLE(Qt::Key_End, "End")
        QVTK_HANDLE(Qt::Key_Home, "Home")
        QVTK_HANDLE(Qt::Key_Left, "Left")
        QVTK_HANDLE(Qt::Key_Up, "Up")
        QVTK_HANDLE(Qt::Key_Right, "Right")
        QVTK_HANDLE(Qt::Key_Down, "Down")
        
        // Select
        // Execute
        QVTK_HANDLE(Qt::Key_SysReq, "Snapshot")
        QVTK_HANDLE(Qt::Key_Insert, "Insert")
        QVTK_HANDLE(Qt::Key_Delete, "Delete")
        QVTK_HANDLE(Qt::Key_Help, "Help")
        QVTK_HANDLE(Qt::Key_0, "0")
        QVTK_HANDLE(Qt::Key_1, "1")
        QVTK_HANDLE(Qt::Key_2, "2")
        QVTK_HANDLE(Qt::Key_3, "3")
        QVTK_HANDLE(Qt::Key_4, "4")
        QVTK_HANDLE(Qt::Key_5, "5")
        QVTK_HANDLE(Qt::Key_6, "6")
        QVTK_HANDLE(Qt::Key_7, "7")
        QVTK_HANDLE(Qt::Key_8, "8")
        QVTK_HANDLE(Qt::Key_9, "9")
        QVTK_HANDLE(Qt::Key_A, "a")
        QVTK_HANDLE(Qt::Key_B, "b")
        QVTK_HANDLE(Qt::Key_C, "c")
        QVTK_HANDLE(Qt::Key_D, "d")
        QVTK_HANDLE(Qt::Key_E, "e")
        QVTK_HANDLE(Qt::Key_F, "f")
        QVTK_HANDLE(Qt::Key_G, "g")
        QVTK_HANDLE(Qt::Key_H, "h")
        QVTK_HANDLE(Qt::Key_I, "i")
        QVTK_HANDLE(Qt::Key_J, "h")
        QVTK_HANDLE(Qt::Key_K, "k")
        QVTK_HANDLE(Qt::Key_L, "l")
        QVTK_HANDLE(Qt::Key_M, "m")
        QVTK_HANDLE(Qt::Key_N, "n")
        QVTK_HANDLE(Qt::Key_O, "o")
        QVTK_HANDLE(Qt::Key_P, "p")
        QVTK_HANDLE(Qt::Key_Q, "q")
        QVTK_HANDLE(Qt::Key_R, "r")
        QVTK_HANDLE(Qt::Key_S, "s")
        QVTK_HANDLE(Qt::Key_T, "t")
        QVTK_HANDLE(Qt::Key_U, "u")
        QVTK_HANDLE(Qt::Key_V, "v")
        QVTK_HANDLE(Qt::Key_W, "w")
        QVTK_HANDLE(Qt::Key_X, "x")
        QVTK_HANDLE(Qt::Key_Y, "y")
        QVTK_HANDLE(Qt::Key_Z, "z")
        // KP_0 - KP_9
        QVTK_HANDLE(Qt::Key_Asterisk, "asterisk")
        QVTK_HANDLE(Qt::Key_Plus, "plus")
        // bar
        QVTK_HANDLE(Qt::Key_Minus, "minus")
        QVTK_HANDLE(Qt::Key_Period, "period")
        QVTK_HANDLE(Qt::Key_Slash, "slash")
        QVTK_HANDLE(Qt::Key_F1, "F1")
        QVTK_HANDLE(Qt::Key_F2, "F2")
        QVTK_HANDLE(Qt::Key_F3, "F3")
        QVTK_HANDLE(Qt::Key_F4, "F4")
        QVTK_HANDLE(Qt::Key_F5, "F5")
        QVTK_HANDLE(Qt::Key_F6, "F6")
        QVTK_HANDLE(Qt::Key_F7, "F7")
        QVTK_HANDLE(Qt::Key_F8, "F8")
        QVTK_HANDLE(Qt::Key_F9, "F9")
        QVTK_HANDLE(Qt::Key_F10, "F10")
        QVTK_HANDLE(Qt::Key_F11, "F11")
        QVTK_HANDLE(Qt::Key_F12, "F12")
        QVTK_HANDLE(Qt::Key_F13, "F13")
        QVTK_HANDLE(Qt::Key_F14, "F14")
        QVTK_HANDLE(Qt::Key_F15, "F15")
        QVTK_HANDLE(Qt::Key_F16, "F16")
        QVTK_HANDLE(Qt::Key_F17, "F17")
        QVTK_HANDLE(Qt::Key_F18, "F18")
        QVTK_HANDLE(Qt::Key_F19, "F19")
        QVTK_HANDLE(Qt::Key_F20, "F20")
        QVTK_HANDLE(Qt::Key_F21, "F21")
        QVTK_HANDLE(Qt::Key_F22, "F22")
        QVTK_HANDLE(Qt::Key_F23, "F23")
        QVTK_HANDLE(Qt::Key_F24, "F24")
        QVTK_HANDLE(Qt::Key_NumLock, "Num_Lock")
        QVTK_HANDLE(Qt::Key_ScrollLock, "Scroll_Lock")
    default:
        break;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

#include <QEvent>
#include <QPointF>
#include <QDebug>
#include <QRectF>
#include <QSize>
#include <QResizeEvent>

struct vtkQtGraphicsProxyWidgetData
{
	double xAspectRatio;
	double yAspectRatio;
	int quadrant;
	QPointF previousPos;
	bool resizing;
    vtkQtGraphicsViewRenderWindow* renderWindow;
	ProxyWidget::Position position;
};

vtkQtGraphicsProxyWidget::vtkQtGraphicsProxyWidget(vtkQtGraphicsViewRenderWindow* rWin, Qt::WindowFlags wFlags)
: QGraphicsProxyWidget(0, wFlags)
{
    d = new vtkQtGraphicsProxyWidgetData;
    d->renderWindow = rWin;
	d->xAspectRatio = 1;
	d->yAspectRatio = 1;
	d->previousPos = QPointF(0, 0);
	d->resizing = false;
	d->quadrant = 0;
	d->position = ProxyWidget::TopLeft;

	setAcceptHoverEvents(true);
    d->renderWindow->viewport()->installEventFilter(this);
    QGraphicsProxyWidget::setFlags(ItemIsMovable|ItemSendsGeometryChanges);
}

vtkQtGraphicsProxyWidget::~vtkQtGraphicsProxyWidget()
{
    delete d;
}

void vtkQtGraphicsProxyWidget::setWidgetPosition(ProxyWidget::Position position)
{
	if(d->position == position)
		return;

	d->position = position;
}

ProxyWidget::Position vtkQtGraphicsProxyWidget::widgetPosition() const
{
	return d->position;
}

bool vtkQtGraphicsProxyWidget::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);

    if(event->type() == QEvent::Resize)
	{
		d->resizing = true;
		QResizeEvent* resizeEvent = (QResizeEvent*)event;
		QSize oldSize = resizeEvent->oldSize();
		QSize size = resizeEvent->size();
		d->xAspectRatio = oldSize.rwidth() - size.rwidth();
		d->yAspectRatio = oldSize.rheight() - size.rheight();
        this->ensureVisible();
	}
	else
		d->resizing = false;

    return false;
}

QVariant vtkQtGraphicsProxyWidget::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if(change == QGraphicsItem::ItemPositionChange)
    {
        // before position changes..
    }
    else if(change == QGraphicsItem::ItemPositionHasChanged)
    {
        // after position changes...
		if(!d->resizing)
		{
            if(this->scene())
            {
                QRectF sceneRect = this->scene()->sceneRect();
                QRectF rect(0, 0,sceneRect.width()/2, sceneRect.height()/2 ) ;
               
				if(rect.contains(this->geometry().center()))
					d->quadrant = 2;

                rect = QRectF(sceneRect.width()/2, 0,sceneRect.width()/2, sceneRect.height()/2 ) ;
				if(rect.contains(this->geometry().center()))
					d->quadrant = 1;
                
				rect = QRectF(0, sceneRect.height()/2,sceneRect.width()/2, sceneRect.height()/2 ) ;
				if(rect.contains(this->geometry().center()))
				 d->quadrant = 3;
				
				rect = QRectF(sceneRect.width()/2, sceneRect.height()/2,
                              sceneRect.width()/2, sceneRect.height()/2 ) ;
                if(rect.contains(this->geometry().center()))
				  d->quadrant = 4;
            }
		}
    }

    return QGraphicsItem::itemChange(change, value);
}

void vtkQtGraphicsProxyWidget::ensureVisible()
{
    QRectF rect = geometry();

    if(d->quadrant == 1)
        this->setGeometry(QRectF(rect.left()- d->xAspectRatio,
                         rect.top(), rect.width(), rect.height()));
    else if(d->quadrant == 2)
        this->setGeometry(QRectF(rect.left(),
                         rect.top() , rect.width(), rect.height()));
    else if(d->quadrant == 3)
        this->setGeometry(QRectF(rect.left(),
                         rect.top()- d->yAspectRatio, rect.width(), rect.height()));
    else if(d->quadrant == 4)
        this->setGeometry(QRectF(rect.left()- d->xAspectRatio,
                         rect.top()- d->yAspectRatio, rect.width(), rect.height()));
}

#endif // VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_SUPPORTED
