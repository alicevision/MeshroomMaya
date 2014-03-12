#include <QHBoxLayout>
#include <QPushButton>
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGMenuItem::MVGMenuItem(const MVGCamera& camera, QWidget * parent)
	: QWidget(parent)
	, _camera(camera)
{
	ui.setupUi(this);
	ui.cameraLabel->setText(camera.name().c_str());
}

MVGMenuItem::~MVGMenuItem()
{
}

const MVGCamera& MVGMenuItem::camera() const
{
	return _camera;
}

void MVGMenuItem::on_leftButton_clicked()
{
	selectedViewChanged("L");
	ui.leftButton->setStyleSheet("QToolButton {background-color: rgb(230,230,230); color: rgb(67,67,67);}");
	_camera.select();
	MVGMayaUtil::setMVGLeftCamera(_camera);
}

void MVGMenuItem::on_rightButton_clicked()
{
	selectedViewChanged("R");
	ui.rightButton->setStyleSheet("QToolButton {background-color: rgb(230,230,230); color: rgb(67,67,67);}");
	_camera.select();
	MVGMayaUtil::setMVGRightCamera(_camera);
}

void MVGMenuItem::clearSelectedView(const QString& view)
{
	if(view == "L")
		ui.leftButton->setStyleSheet("");
	if(view == "R")
		ui.rightButton->setStyleSheet("");
}

