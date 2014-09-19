#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"
#undef Success // needed by eigen..
#include "openMVG/cameras/PinholeCamera.hpp"
#include <vector>

class MString;

namespace mayaMVG {

class MVGPointCloudItem;

class MVGCamera : public MVGNodeWrapper {

	public:
		MVGCamera(): MVGNodeWrapper() {}
		MVGCamera(const std::string& name);
		MVGCamera(const MDagPath& dagPath);
		MVGCamera(const int& id);
		virtual ~MVGCamera();

	public:
		bool operator<(const MVGCamera&) const;

	public:
		virtual bool isValid() const;

	public:
		static MVGCamera create(const std::string& name);
		static std::vector<MVGCamera> getCameras();

	public:
		int getId() const;
		void setId(const int&) const;
		MDagPath getImagePath() const;
		std::string getImagePlane() const;
		void setImagePlane(const std::string&) const;
		void loadImagePlane() const;
		openMVG::PinholeCamera getPinholeCamera() const;
		void setPinholeCamera(const openMVG::PinholeCamera&) const;
		std::vector<MVGPointCloudItem> getVisibleItems() const;
		void setVisibleItems(const std::vector<MVGPointCloudItem>& item) const;

		double getZoom() const;
		void setZoom(const double zoom) const;
		double getHorizontalPan() const;
		void setHorizontalPan(const double pan) const;
		double getVerticalPan() const;
		void setVerticalPan(const double pan) const;
        void setPan(const double hpan, const double vpan) const;
        double getHorizontalFilmAperture() const;
  
        void resetZoomAndPan() const;
        void setInView(const std::string& viewName) const;

	private:
		// dynamic attributes
		static MString _ID;
		static MString _PINHOLE;
		static MString _ITEMS;
		static MString _DEFERRED;
};

} // namespace
