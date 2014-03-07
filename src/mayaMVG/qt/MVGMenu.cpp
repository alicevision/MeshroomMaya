#include <QFileDialog>
#include "mayaMVG/qt/MVGMenu.h"
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/core/MVGLog.h"
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

void MVGMenu::addItem(const QString& itemName)
{
	if(itemName.isEmpty())
		return;
	MVGMenuItem * itemWidget = new MVGMenuItem(itemName);
	ui.cameraList->addItem(itemName);
	QListWidgetItem * item = ui.cameraList->item(ui.cameraList->count()-1);
	ui.cameraList->setItemWidget(item, itemWidget);
	item->setSizeHint(QSize(item->sizeHint().width(), 66));
}

void MVGMenu::selectItems(const QList<QString>& itemNames)
{
	for(size_t i = 0; i <  ui.cameraList->count(); ++i)
		ui.cameraList->item(i)->setSelected(itemNames.contains(ui.cameraList->item(i)->text()));
}

void MVGMenu::on_cameraList_itemSelectionChanged()
{
	QList<QListWidgetItem *> selectedItems = ui.cameraList->selectedItems();
	MVGMayaUtil::clearMayaSelection();
	for(size_t i = 0; i < selectedItems.size(); ++i)
	{
		MVGMayaUtil::addToMayaSelection(MQtUtil::toMString(selectedItems[i]->text()));
	}
}

void MVGMenu::on_browseButton_clicked()
{
  QString directory = QFileDialog::getExistingDirectory(this, "Choose directory");
  if(directory.isEmpty())
    return;
  // load project
  ui.directoryLineEdit->setText(directory);
}

void MVGMenu::on_selectContextButton_clicked()
{
}

void MVGMenu::on_placeContextButton_clicked()
{
}

void MVGMenu::on_moveContextButton_clicked()
{
}
