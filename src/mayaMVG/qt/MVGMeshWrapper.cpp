#include "mayaMVG/qt/MVGMeshWrapper.hpp"
#include <QImage>
#include <QFileInfo>

namespace mayaMVG
{

MVGMeshWrapper::MVGMeshWrapper(const MVGMesh& mesh)
    : _mesh(mesh)
{
}

MVGMeshWrapper::MVGMeshWrapper(const MVGMeshWrapper& other)
    : _mesh(other._mesh)
{
}

MVGMeshWrapper::~MVGMeshWrapper()
{
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

} // namespace
