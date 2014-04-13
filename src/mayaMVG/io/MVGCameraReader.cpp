#include "mayaMVG/io/MVGCameraReader.h"
#include "mayaMVG/core/MVGScene.h"
#include "mayaMVG/core/MVGLog.h"
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>
#include <fstream>

using namespace mayaMVG;

namespace { // empty namespace

	bool setPinholeFromBinary(MVGCamera& camera, const std::string& binaryName)
	{
		std::string file = MVGScene::fullPath(MVGScene::cameraDirectory(), binaryName);
		openMVG::PinholeCamera pinholeCamera;
		std::ifstream infile(file.c_str(), std::ios::binary);
		if(infile.fail())
			return false;
		infile.read((char*)pinholeCamera._P.data(), (std::streamsize)(3 * 4) * sizeof(double));
		openMVG::KRt_From_P(pinholeCamera._P, &pinholeCamera._K, &pinholeCamera._R, &pinholeCamera._t);
		camera.setPinholeCamera(pinholeCamera);
		infile.close();
		return true;
	}

} // empty namespace


bool MVGCameraReader::read(std::vector<MVGCamera>& cameraList)
{
	std::string file = MVGScene::fullPath(MVGScene::projectDirectory(), "views.txt");

	std::ifstream infile(file.c_str());
	if (!infile.is_open())
	{
		LOG_ERROR("Camera file not found.")
		return false;
	}

	// header
	std::string imageDir, cameraDir, tmp;
	getline(infile, imageDir); // images directory
	getline(infile, cameraDir); // cameras directory
	getline(infile, tmp); // count
	MVGScene::setImageDirectoryName(imageDir);
	MVGScene::setCameraDirectoryName(cameraDir);

	// cameras description
	std::string line;
	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
		std::string imageName, cameraBin;
		size_t width, height;
		float near, far;
	    if (!(iss >> imageName >> width >> height >> cameraBin >> near >> far))
  			break;
	    // create camera
	    MVGCamera camera = MVGCamera::create(std::string("camera_") + stlplus::basename_part(cameraBin));
		// MVGCamera camera(std::string("camera_") + stlplus::basename_part(cameraBin));
		setPinholeFromBinary(camera, cameraBin);
		camera.setImagePlane(imageName);
		// register camera
		cameraList.push_back(camera);
		line.clear();
	}
	
	infile.close();
	return true;
}