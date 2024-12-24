#include "stdafx.h"
#include "WMVx.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    if (a.styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        // the default theme isnt great in dark mode, fusion looks slightly better.
        a.setStyle("fusion");
    }

    WMVx w;
    w.show();
    return a.exec();
}
