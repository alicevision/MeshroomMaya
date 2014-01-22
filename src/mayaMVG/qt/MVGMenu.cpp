#include <QHBoxLayout>
#include <QPushButton>
#include "MVGMenu.h"
#include "MVGMenuItem.h"
#include "util/MVGLog.h"
#include "util/MVGUtil.h"
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGMenu::MVGMenu(QWidget * parent) : QWidget(parent) {
	ui.setupUi(this);
}

MVGMenu::~MVGMenu() {
}

void MVGMenu::addCamera(const QString& cameraName) {
	if(cameraName.isEmpty())
		return;
	if(cameraName == "persp" || cameraName == "top"
		|| cameraName == "front" || cameraName == "side")
		return;
	MVGMenuItem * itemWidget = new MVGMenuItem(cameraName);
	ui.camList->addItem(cameraName);
	QListWidgetItem * item = ui.camList->item(ui.camList->count()-1);
	ui.camList->setItemWidget(item, itemWidget);
	item->setSizeHint(QSize(item->sizeHint().width(), 66));
}

void MVGMenu::clear() {
	ui.camList->clear();
}

void MVGMenu::selectCameras(const QList<QString>& cameraNames) {
	for(size_t i = 0; i <  ui.camList->count(); ++i)
		ui.camList->item(i)->setSelected(cameraNames.contains(ui.camList->item(i)->text()));
}

// slot
void MVGMenu::on_camList_itemSelectionChanged() {
	QList<QListWidgetItem *> selectedItems = ui.camList->selectedItems();
	MVGUtil::clearMayaSelection();
	for(size_t i = 0; i < selectedItems.size(); ++i) {
		MVGUtil::addToMayaSelection(MQtUtil::toMString(selectedItems[i]->text()));
	}
}
