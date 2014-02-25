#pragma once

#include "ui_menuItem.h"
#include <QWidget>

namespace mayaMVG {

class MVGMenuItem: public QWidget {
	Q_OBJECT

public:
	MVGMenuItem(const QString & cameraName, QWidget * parent = 0);
	~MVGMenuItem();

	void clearView(const QString& view);
	
protected slots:
	void on_leftButton_clicked();
	void on_rightButton_clicked();

signals:
	void signalWillChangeSelectedView(const QString& view);
	
private:
	Ui::MVGMenuItem ui;
	QString m_cameraName;
};

} // mayaMVG
