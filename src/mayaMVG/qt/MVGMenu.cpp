#include <QFileDialog>
#include "mayaMVG/qt/MVGMenu.h"
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGMenu::MVGMenu(QWidget * parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.progressBar->setVisible(false);
	ui.cancelButton->setVisible(false);
	ui.settingsWidget->setVisible(false);
}

MVGMenu::~MVGMenu()
{
}

void MVGMenu::clear()
{
	ui.cameraList->clear();
}

void MVGMenu::selectItems(const QList<QString>& itemNames)
{
	for(size_t i = 0; i <  ui.cameraList->count(); ++i)
		ui.cameraList->item(i)->setSelected(itemNames.contains(ui.cameraList->item(i)->text()));
}

void MVGMenu::on_cameraList_itemSelectionChanged()
{
	QList<QListWidgetItem*> selectedItems = ui.cameraList->selectedItems();
	MVGMayaUtil::clearMayaSelection();
	for(size_t i = 0; i < selectedItems.size(); ++i)
		qobject_cast<MVGMenuItem*>(ui.cameraList->itemWidget(selectedItems[i]))->camera().select();
}

void MVGMenu::on_browseButton_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, "Choose directory");
	if(directory.isEmpty())
		return;
	// load project
	ui.directoryLineEdit->setText(directory);
	MVGScene::setProjectDirectory(directory.toStdString());
	if(!MVGScene::load())
		LOG_ERROR("An error occured when loading project.")
	// populate menu
	const std::vector<MVGCamera>& cameraList = MVGScene::cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	for(; it != cameraList.end(); ++it)
		addItem(*it);
}

void MVGMenu::on_selectContextButton_clicked()
{
}

void MVGMenu::on_placeContextButton_clicked()
{
	MVGMayaUtil::activeContext();
}

void MVGMenu::on_moveContextButton_clicked()
{
}

void MVGMenu::addItem(const MVGCamera& camera)
{
	MVGMenuItem * itemWidget = new MVGMenuItem(camera);
	ui.cameraList->addItem(camera.name().c_str());
	connect(itemWidget, SIGNAL(selectedViewChanged(const QString&)), this, SLOT(selectedViewChanged(const QString&)));
	QListWidgetItem * item = ui.cameraList->item(ui.cameraList->count()-1);
	ui.cameraList->setItemWidget(item, itemWidget);
	item->setSizeHint(QSize(item->sizeHint().width(), 66));
}

void MVGMenu::selectedViewChanged(const QString& view) 
{
	for(size_t i = 0; i < ui.cameraList->count(); ++i)
		((MVGMenuItem *)ui.cameraList->itemWidget(ui.cameraList->item(i)))->clearSelectedView(view);
}