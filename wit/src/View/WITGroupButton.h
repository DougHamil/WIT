#ifndef WIT_GROUP_BUTTONH
#define WIT_GROUP_BUTTONH

#include <QPushButton>

class WITGroupButton : public QPushButton
{
    Q_OBJECT
public:
    WITGroupButton(int ID, int r, int g, int b, QWidget *parent = 0);
    WITGroupButton(QString &ID, int r, int g, int b, QWidget *parent =0);
protected:
    bool groupVisible;
    int r, g, b;
signals:

public slots:

};

#endif