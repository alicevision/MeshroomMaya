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
		Q_PROPERTY(QString pointCloudFile READ pointCloudFile NOTIFY pointCloudFileChanged);
		Q_PROPERTY(QList<QObject*> cameraModel READ cameraModel NOTIFY cameraModelChanged);
		
		Q_PROPERTY(bool connectFace READ connectFace NOTIFY connectFaceChanged);
		Q_PROPERTY(bool computeLastPoint READ computeLastPoint NOTIFY computeLastPointChanged);
		
		Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged);

		MAKE_SINGLETON_WITHCONSTRUCTORS(MVGProjectWrapper)

	public:

		// Getters & Setters
		Q_INVOKABLE const QString moduleDirectory() const;
		Q_INVOKABLE const QString projectDirectory() const;
		Q_INVOKABLE const QString cameraDirectory() const;
		Q_INVOKABLE const QString imageDirectory() const;
		Q_INVOKABLE const QString pointCloudFile() const;
		Q_INVOKABLE const QList<QObject*>& cameraModel() const;
		Q_INVOKABLE const bool connectFace() const;
		Q_INVOKABLE const bool computeLastPoint() const;
		Q_INVOKABLE QObject* getCameraAtIndex(int index) const;
		Q_INVOKABLE void setProjectDirectory(const QString& directory);
		
		Q_INVOKABLE const QString logText() const;
		Q_INVOKABLE void setLogText(const QString);
		void appendLogText(const QString);

		// Functions exposed to QML
		Q_INVOKABLE void onBrowseDirectoryButtonClicked();
		Q_INVOKABLE void onSelectContextButtonClicked();
		Q_INVOKABLE void onPlaceContextButtonClicked();
		Q_INVOKABLE void onMoveContextButtonClicked();	
		Q_INVOKABLE void onComputeLastPointCheckBoxClicked(bool checked);	
		Q_INVOKABLE void onConnectFaceCheckBoxClicked(bool checked);	
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
		void pointCloudFileChanged();
		void cameraModelChanged();
		void connectFaceChanged();
		void computeLastPointChanged();
		void logTextChanged();

	private:
		QList<QObject*>	_cameraList;
		MVGProject _project;
		QString _logText;

};

} // mayaMVG
