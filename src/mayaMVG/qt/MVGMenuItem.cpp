#include <QHBoxLayout>
#include <QPushButton>
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGMenuItem::MVGMenuItem(const QString & cameraName, QWidget * parent)
	: QWidget(parent)
	, m_cameraName(cameraName)
{
	ui.setupUi(this);
	ui.cameraLabel->setText(cameraName);
}

MVGMenuItem::~MVGMenuItem()
{
}

void MVGMenuItem::on_leftButton_clicked()
{
	selectedViewChanged("L");
	ui.leftButton->setStyleSheet("QToolButton {background-color: rgb(230,230,230); color: rgb(67,67,67);}");
	MVGMayaUtil::setMVGLeftCamera(MQtUtil::toMString(m_cameraName));
}

void MVGMenuItem::on_rightButton_clicked()
{
	selectedViewChanged("R");
	ui.rightButton->setStyleSheet("QToolButton {background-color: rgb(230,230,230); color: rgb(67,67,67);}");
	MVGMayaUtil::setMVGRightCamera(MQtUtil::toMString(m_cameraName));
}

void MVGMenuItem::clearSelectedView(const QString& view)
{
	if(view == "L")
		ui.leftButton->setStyleSheet("");
	if(view == "R")
		ui.rightButton->setStyleSheet("");
}

