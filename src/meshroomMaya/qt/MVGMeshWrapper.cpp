#include "meshroomMaya/qt/MVGMeshWrapper.hpp"
#include <QImage>
#include <QFileInfo>

namespace meshroomMaya
{

MVGMeshWrapper::MVGMeshWrapper(const MVGMesh& mesh, QObject* parent)
    : QObject(parent)
    , _mesh(mesh)
    , _isSelected(false)
{
}

MVGMeshWrapper::MVGMeshWrapper(const MVGMeshWrapper& other)
    : QObject(other.parent())
    , _mesh(other._mesh)
    , _isSelected(other._isSelected)
{
}

MVGMeshWrapper::~MVGMeshWrapper()
{
}

const QString MVGMeshWrapper::getDagPathAsString() const
{
    return QString::fromStdString(_mesh.getDagPathAsString());
}

const MVGMesh& MVGMeshWrapper::getMesh() const
{
    return _mesh;
}

const bool MVGMeshWrapper::isActive() const
{
    return _mesh.isActive();
}

void MVGMeshWrapper::setIsActive(const bool isActive)
{
    if(_mesh.isActive() == isActive)
        return;
    _mesh.setIsActive(isActive);
    Q_EMIT isActiveChanged();
}

void MVGMeshWrapper::setIsSelected(const bool isSelected)
{
    if(_isSelected == isSelected)
        return;
    _isSelected = isSelected;
    Q_EMIT isSelectedChanged();
}

} // namespace
