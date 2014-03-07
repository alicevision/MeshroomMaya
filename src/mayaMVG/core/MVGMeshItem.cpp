#include "mayaMVG/core/MVGMeshItem.h"

using namespace mayaMVG;

MVGMeshItem::MVGMeshItem()
{
}

MVGMeshItem::~MVGMeshItem()
{
}

MPoint MVGMeshItem::point() const 
{
	return _point;
}

MColor MVGMeshItem::color() const
{
	return _color;
}
