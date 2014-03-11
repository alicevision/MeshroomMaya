#pragma once

#include "ui_menuItem.h"
#include <QWidget>

namespace mayaMVG {

class MVGMenuItem: public QWidget {
	Q_OBJECT

public:
	MVGMenuItem(const QString & cameraName, QWidget * parent = 0);
	~MVGMenuItem();

public slots:
	void clearSelectedView(const QString&);

protected slots:
	void on_leftButton_clicked();
	void on_rightButton_clicked();

signals:
	void selectedViewChanged(const QString&);
	
private:
	Ui::MVGMenuItem ui;
	QString m_cameraName;
};

} // mayaMVG
