#include "stdafx.h"
#include "WMVx.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WMVx w;
    w.show();
    return a.exec();
}
