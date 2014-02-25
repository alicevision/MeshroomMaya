#include <QHBoxLayout>
#include <QPushButton>
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/util/MVGUtil.h"
#include "mayaMVG/util/MVGLog.h"
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGMenuItem::MVGMenuItem(const QString & cameraName, QWidget * parent)
: QWidget(parent)
, m_cameraName(cameraName)
{
	ui.setupUi(this);
	ui.cameraLabel->setText(cameraName);
	QMetaObject::connectSlotsByName(this);
}

MVGMenuItem::~MVGMenuItem()
{
}

void MVGMenuItem::clearView(const QString& view)
{
	if( view == "L" )
		ui.leftButton->setStyleSheet("");
	else if( view == "R" )
		ui.rightButton->setStyleSheet("");
}

void MVGMenuItem::on_leftButton_clicked()
{
	signalWillChangeSelectedView("L");
	ui.leftButton->setStyleSheet("QToolButton { background-color: DarkOrange }");
	MVGUtil::setMVGLeftCamera(MQtUtil::toMString(m_cameraName));
}

void MVGMenuItem::on_rightButton_clicked()
{
	signalWillChangeSelectedView("R");
	ui.rightButton->setStyleSheet("QToolButton { background-color: DarkOrange }");
	MVGUtil::setMVGRightCamera(MQtUtil::toMString(m_cameraName));
}
