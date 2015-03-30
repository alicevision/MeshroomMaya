#pragma once

#include "mayaMVG/core/MVGMesh.hpp"
#include <QObject>
//#include <QSize>
//#include <QStringList>

namespace mayaMVG
{

class MVGMeshWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)

public:
    MVGMeshWrapper(const MVGMesh& mesh);
    MVGMeshWrapper(const MVGMeshWrapper& other);
    ~MVGMeshWrapper();

public slots:
    const QString getName() const { return QString::fromStdString(_mesh.getName()); }

signals:
    void isActiveChanged();

public:
    const MVGMesh& getMesh() const;
    const bool isActive() const;
    void setIsActive(const bool isActive);

private:
    const MVGMesh _mesh;
};

} // namespace
