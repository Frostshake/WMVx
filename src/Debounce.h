#pragma once

#include <QObject>

class Debounce : public QObject
{
	Q_OBJECT

public:
	Debounce(QObject *parent, int delayMs = 250);
	~Debounce();

public slots:
	void absorb();

signals:
	void triggered();

private:
	QTimer* timer;
};
