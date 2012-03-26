#ifndef WIT_GROUP_BUTTONH
#define WIT_GROUP_BUTTONH

#include <QPushButton>
#include <util/typedefs.h>

class WITGroupButton : public QPushButton
{
    Q_OBJECT
public:
	WITGroupButton(int ID, Colord* color);
protected:
    bool groupVisible;
    Colord color;
signals:

public slots:

};

#endif