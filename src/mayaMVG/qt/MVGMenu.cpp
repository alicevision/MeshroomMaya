#include <QHBoxLayout>
#include <QPushButton>
#include "mayaMVG/qt/MVGMenu.h"
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/util/MVGLog.h"
#include "mayaMVG/util/MVGUtil.h"
#include <maya/MQtUtil.h>
#include <maya/MDagModifier.h>
#include <maya/MFnCamera.h>
#include <maya/MDagPath.h>


using namespace mayaMVG;

MVGMenu::MVGMenu(QWidget * parent) : QWidget(parent) {
	ui.setupUi(this);
	ui.progressBar->setVisible(false);
	ui.cancelButton->setVisible(false);
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
	ui.cameraList->addItem(cameraName);
	QListWidgetItem * item = ui.cameraList->item(ui.cameraList->count()-1);
	ui.cameraList->setItemWidget(item, itemWidget);
	item->setSizeHint(QSize(item->sizeHint().width(), 66));
}

void MVGMenu::clear() {
	ui.cameraList->clear();
}

void MVGMenu::selectCameras(const QList<QString>& cameraNames) {
	for(size_t i = 0; i <  ui.cameraList->count(); ++i)
		ui.cameraList->item(i)->setSelected(cameraNames.contains(ui.cameraList->item(i)->text()));
}

// slot
void MVGMenu::on_cameraList_itemSelectionChanged() {
	QList<QListWidgetItem *> selectedItems = ui.cameraList->selectedItems();
	MVGUtil::clearMayaSelection();
	for(size_t i = 0; i < selectedItems.size(); ++i) {
		MVGUtil::addToMayaSelection(MQtUtil::toMString(selectedItems[i]->text()));
	}
}

// slot
void MVGMenu::on_cameraImportButton_clicked() {
	ui.progressBar->setVisible(true);
	ui.cancelButton->setVisible(true);
	MDagModifier dagModifier;
	MStatus status;
	MFnCamera fnCamera;
	for(size_t i = 0; i < 10; ++i) {
		// create camera
		MObject cameraTransform = fnCamera.create(&status);
		MFnDependencyNode dep(cameraTransform);
		dep.setName("mayaMVGCam#");
		// create image plane
		MObject planeTransform = dagModifier.createNode("imagePlane", MObject::kNullObj, &status);
		MFnDependencyNode fnPlane(planeTransform);
		fnPlane.setName("imagePlane#");
		// reparent image plane
		MDagPath path;
		fnCamera.getPath(path);
		dagModifier.reparentNode(planeTransform, path.node());
        dagModifier.doIt();
        ui.progressBar->setValue((i+1)*10);
	}
	ui.progressBar->setVisible(false);
	ui.cancelButton->setVisible(false);
}

// QString path = QFileDialog::getExistingDirectory (this, tr("Directory"), directory.path());
// if ( path.isNull() == false )
// {
//     directory.setPath(path);
// }
