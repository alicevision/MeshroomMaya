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
       void on_cameraList_itemSelectionChanged();
       void on_cameraImportButton_clicked();
private:
	Ui::MVGMenu ui;
};

} // mayaMVG