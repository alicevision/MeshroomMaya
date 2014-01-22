#pragma once

#include "ui_menu.h"
#include <QWidget>

namespace mayaMVG {

class MVGMenu: public QWidget {
	Q_OBJECT

public:
	MVGMenu(QWidget * parent = 0);
	~MVGMenu();

public:
	void addCamera(const QString& text);
	void clear();
	void selectCameras(const QList<QString>& cameraNames);

private slots:
	void on_camList_itemSelectionChanged();

private:
	Ui::MVGMenu ui;
};

} // mayaMVG
