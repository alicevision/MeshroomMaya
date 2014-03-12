#pragma once

#include "ui_menuItem.h"
#include <QWidget>

namespace mayaMVG {

class MVGCamera;

class MVGMenuItem: public QWidget {
	Q_OBJECT

	public:
		MVGMenuItem(const MVGCamera& camera, QWidget * parent = 0);
		~MVGMenuItem();

	public:
		const MVGCamera& camera() const;
		
	public slots:
		void clearSelectedView(const QString&);

	protected slots:
		void on_leftButton_clicked();
		void on_rightButton_clicked();

	signals:
		void selectedViewChanged(const QString&);
		
	private:
		Ui::MVGMenuItem ui;
		const MVGCamera& _camera;
};

} // mayaMVG
