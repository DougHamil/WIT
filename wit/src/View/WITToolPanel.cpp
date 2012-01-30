#include "WITToolPanel.h"
#include <QtGui>

WITToolPanel::WITToolPanel(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *vbox = new QHBoxLayout();

    QButtonGroup *bgroup = new QButtonGroup();
    QPushButton *b1 = new QPushButton("Touch");
    QPushButton *b2 = new QPushButton("Surface");
    QPushButton *b3 = new QPushButton("Stats");
    QPushButton *b4 = new QPushButton("VOI");

    b1->setCheckable(true);
    b2->setCheckable(true);
    b3->setCheckable(true);
    b4->setCheckable(true);

    bgroup->addButton(b1);
    bgroup->addButton(b2);
    bgroup->addButton(b3);
    bgroup->addButton(b4);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(b1);
    hbox->addWidget(b2);
    hbox->addWidget(b3);
    hbox->addWidget(b4);

    hbox->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    vbox->addLayout(hbox);

    setLayout(vbox);
}