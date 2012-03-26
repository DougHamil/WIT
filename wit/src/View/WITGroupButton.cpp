#include "WITGroupButton.h"
#include <sstream>
#include <iostream>

WITGroupButton::WITGroupButton(int ID, Colord *color)
{
	std::stringstream out;
    out << ID;
    this->setText(out.str().c_str());

    std::stringstream style;
    style << "QPushButton { padding:2px;min-width:2em;background:rgb(";
    style << color->r*255 << ", " << color->g*255 << ", " << color->b*255 << ");}";
    setStyleSheet(style.str().c_str());
}