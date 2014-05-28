#pragma once

#include <QObject>
#include <QSize>
#include "mayaMVG/core/MVGCamera.h"

namespace mayaMVG {

class MVGCameraWrapper : public QObject
{
		Q_OBJECT
		
		Q_PROPERTY(QString name READ name CONSTANT)
		Q_PROPERTY(QString imagePath READ imagePath CONSTANT)
		Q_PROPERTY(QString state READ state NOTIFY stateChanged)	
		Q_PROPERTY(bool isLeftChecked READ isLeftChecked WRITE setLeftChecked NOTIFY isLeftCheckedChanged)
		Q_PROPERTY(bool isRightChecked READ isRightChecked WRITE setRightChecked NOTIFY isRightCheckedChanged)
		
		Q_PROPERTY(QSize sourceSize READ sourceSize CONSTANT)
		Q_PROPERTY(qint64 sourceWeight READ sourceWeight CONSTANT)
	public:
		MVGCameraWrapper(const MVGCamera& camera);
		~MVGCameraWrapper();

	public:
		const MVGCamera& camera() const;
		Q_INVOKABLE const QString name() const;
		Q_INVOKABLE const QString imagePath() const;
		Q_INVOKABLE const QString& state() const;	
		Q_INVOKABLE const bool isLeftChecked() const;
		Q_INVOKABLE const bool isRightChecked() const;
		Q_INVOKABLE void setState(const QString& state);	
		Q_INVOKABLE void setLeftChecked(const bool state);
		Q_INVOKABLE void setRightChecked(const bool state);
		
		Q_INVOKABLE const QSize sourceSize() const;
		Q_INVOKABLE const qint64 sourceWeight() const;
		Q_INVOKABLE void onLeftButtonClicked();
		Q_INVOKABLE void onRightButtonClicked();

	signals:
		void stateChanged();
		void isLeftCheckedChanged();
		void isRightCheckedChanged();	

	private:
		const MVGCamera _camera;
		QString _state;
		bool _isLeftChecked;
		bool _isRightChecked;
};

} // mayaMVG
