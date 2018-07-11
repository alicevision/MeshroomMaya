#pragma once

#include "meshroomMaya/core/MVGMesh.hpp"
#include <QObject>
#include <maya/MString.h>
//#include <QSize>
//#include <QStringList>

namespace meshroomMaya
{

class MVGMeshWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString dagPath READ getDagPathAsString CONSTANT)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isSelected READ isSelected WRITE setIsSelected NOTIFY isSelectedChanged)

public:
    MVGMeshWrapper(const MVGMesh& mesh, QObject* parent=nullptr);
    MVGMeshWrapper(const MVGMeshWrapper& other);
    ~MVGMeshWrapper();

public slots:
    const QString getName() const { return QString::fromStdString(_mesh.getName()); }
    const QString getDagPathAsString() const;

signals:
    void isActiveChanged();
    void isSelectedChanged();

public:
    const MVGMesh& getMesh() const;
    const bool isActive() const;
    void setIsActive(const bool isActive);
    bool isSelected() const { return _isSelected; }
    void setIsSelected(const bool isSelected);

private:
    const MVGMesh _mesh;
    bool _isSelected;
};

} // namespace
