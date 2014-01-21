#pragma once

#include "ui_menuItem.h"
#include <QWidget>

namespace mayaMVG {

class MVGMenuItem: public QWidget {
	Q_OBJECT

public:
	MVGMenuItem(const QString & cameraName, QWidget * parent = 0);
	~MVGMenuItem();

protected slots:
	void on_leftButton_clicked();
	void on_rightButton_clicked();

private:
	Ui::MVGMenuItem ui;
	QString m_cameraName;
};

} // mayaMVG
