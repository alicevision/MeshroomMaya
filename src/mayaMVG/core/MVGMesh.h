#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"

namespace mayaMVG {

class MVGFace3D;

class MVGMesh : public MVGNodeWrapper  {

	public:
		MVGMesh(const std::string& name);
		MVGMesh(const MDagPath& dagPath);
		virtual ~MVGMesh();

	public:
		virtual bool isValid() const;
		
	public:
		static MVGMesh create(const std::string& name);

	public:
		void addPolygon(const MVGFace3D& face3d);

};

} // mayaMVG
