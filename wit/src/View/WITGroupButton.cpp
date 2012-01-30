#include "WITGroupButton.h"
#include <sstream>
#include <iostream>

WITGroupButton::WITGroupButton(int ID, int r, int g, int b, QWidget *parent) :
    QPushButton(parent)
{
	std::stringstream out;
    out << ID;
    this->setText(out.str().c_str());

    std::stringstream style;
    style << "QPushButton { padding:2px;min-width:2em;background:rgb(";
    style << r << ", " << g << ", " << b << ");}";
    setStyleSheet(style.str().c_str());

}