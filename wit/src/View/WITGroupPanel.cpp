#include "WITGroupPanel.h"
#include "WITGroupButton.h"

#include <QtGui>
#include <QPalette>

WITGroupPanel::WITGroupPanel(QWidget *panel)
{
	QHBoxLayout *hbox = new QHBoxLayout();

    QButtonGroup *bgroup = new QButtonGroup();

    WITGroupButton *b = new WITGroupButton(0, 50,50,50);

    b->setCheckable(true);
    bgroup->addButton(b);
    hbox->addWidget(b);
	hbox->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    for(int i = 0; i < 8; i++)
    {
        WITGroupButton *b = new WITGroupButton(i+1, rand()%255, rand()%255, rand()%255);
        b->setCheckable(true);
        bgroup->addButton(b);

        hbox->addWidget(b);
    }
    
	hbox->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setLayout(hbox);
}