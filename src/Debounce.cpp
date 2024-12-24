#include "stdafx.h"
#include "Debounce.h"

Debounce::Debounce(QObject *parent, int delayMs)
	: QObject(parent)
{
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(250);
	connect(timer, &QTimer::timeout, [&]() {
		emit triggered();
	});
}

Debounce::~Debounce()
{}

void Debounce::absorb() {
	timer->stop();
	timer->start();
}