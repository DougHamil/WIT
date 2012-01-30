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

#ifndef VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_H
#define VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_H

#include <QtGlobal>
#include <QGraphicsView>

#ifdef Q_WS_WIN
    #include <vtkWin32OpenGLRenderWindow.h>
    typedef vtkWin32OpenGLRenderWindow vtkRenderWindowClass;
    #define VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_SUPPORTED
#endif

#ifdef Q_WS_X11
    #include <vtkXOpenGLRenderWindow.h>
    typedef vtkXOpenGLRenderWindow vtkRenderWindowClass;
    #define VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_SUPPORTED
#endif

#ifdef VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_SUPPORTED
class vtkQtGraphicsProxyWidget;

namespace ProxyWidget
{
	enum Position
	{
		TopLeft,
		TopMiddle,
		TopRight,
		RightMiddle,
		BottomLeft,
		BottomMiddle,
		BottomRight,
		LeftMiddle,
	};
}

struct vtkQtGraphicsViewRenderWindowData;
class vtkQtGraphicsViewRenderWindow : public QGraphicsView, public vtkRenderWindowClass
{
    

public:
    vtkQtGraphicsViewRenderWindow(QWidget* parent = 0);
    ~vtkQtGraphicsViewRenderWindow();


    void setAutoRendererTilingEnabled(bool val);
    bool isAutoRendererTiligEnabled() const;

	vtkQtGraphicsProxyWidget* addWidget(QWidget* widget,ProxyWidget::Position position, Qt::WindowFlags flags=0);

protected:
    // vtkOpenGLRenderWindow implementation
    void MakeCurrent();
    void Initialize(void);
    void Finalize(void);
    void CreateAWindow();
    void DestroyWindow();
    void Render();

protected:
    // QGraphicsView implementation
    void drawBackground(QPainter *painter, const QRectF &rect);
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent *event);

    // Event Handlers
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent*);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private slots:
    void tileRenderers();

private:
    vtkQtGraphicsViewRenderWindowData* d;
};

#include <QGraphicsProxyWidget>

struct vtkQtGraphicsProxyWidgetData;
class vtkQtGraphicsProxyWidget : public QGraphicsProxyWidget
{
public:
    vtkQtGraphicsProxyWidget(vtkQtGraphicsViewRenderWindow* rWin, Qt::WindowFlags wFlags=0);
    ~vtkQtGraphicsProxyWidget();

    void setFlags(GraphicsItemFlags flags) {
        QGraphicsProxyWidget::setFlags( flags|ItemSendsGeometryChanges );
    }

	void setWidgetPosition(ProxyWidget::Position position);
	ProxyWidget::Position widgetPosition() const;

protected:
    bool eventFilter(QObject* obj, QEvent* event);
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
    void ensureVisible();

private:
    vtkQtGraphicsProxyWidgetData* d;
};

#endif

#endif // VTK_QT_GRAPHICS_VIEW_RENDER_WINDOW_H