#pragma once

#include <QObject>
#include <QFileDialog>
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/patterns/Singleton.h"

namespace mayaMVG {

class MVGProjectWrapper : public QObject, public Singleton<MVGProjectWrapper> {

		Q_OBJECT

		Q_PROPERTY(QString projectDirectory READ projectDirectory NOTIFY projectDirectoryChanged);
		Q_PROPERTY(QString cameraDirectory READ cameraDirectory NOTIFY cameraDirectoryChanged);
		Q_PROPERTY(QString imageDirectory READ imageDirectory NOTIFY imageDirectoryChanged);
		Q_PROPERTY(QList<QObject*> cameraModel READ cameraModel NOTIFY cameraModelChanged);

		MAKE_SINGLETON_WITHCONSTRUCTORS(MVGProjectWrapper)

	public:

		// Getters & Setters
		Q_INVOKABLE const QString projectDirectory() const;
		Q_INVOKABLE const QString cameraDirectory() const;
		Q_INVOKABLE const QString imageDirectory() const;
		Q_INVOKABLE const QList<QObject*>& cameraModel() const;
		Q_INVOKABLE QObject* getCameraAtIndex(int index) const;
		Q_INVOKABLE void setProjectDirectory(const QString& directory);

		// Functions exposed to QML
		Q_INVOKABLE void onBrowseDirectoryButtonClicked();
		Q_INVOKABLE void onSelectContextButtonClicked();
		Q_INVOKABLE void onPlaceContextButtonClicked();
		Q_INVOKABLE void onMoveContextButtonClicked();	
		Q_INVOKABLE void loadProject(QString projectDirectoryPath);
		void selectItems(const QList<QString>& cameraNames);
		
		void setLeftView(MVGCameraWrapper& camera) const;
		void setRightView(MVGCameraWrapper& camera) const;

	private:
		void addCamera(const MVGCamera& camera);

	signals:
		void projectDirectoryChanged();
		void cameraDirectoryChanged();
		void imageDirectoryChanged();
		void cameraModelChanged();

	private:
		QList<QObject*>	_cameraList;
		MVGProject*	_project;

};

} // mayaMVG
