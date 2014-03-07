#pragma once

#include <openMVG/cameras/PinholeCamera.hpp>
#include <openMVG/numeric/numeric.h>
#include <third_party/stlplus3/filesystemSimplified/file_system.hpp>

namespace mayaMVG {

struct MVGCameraReader {
	
	/**
	 * Read in the binary file to create a openMVG pinhole camera 
	 * \param[in] sPathCameraBin path to binary file
	 * \param[out] camera output openMVG pinhole camera 
	 * \return false if the file doesn' exist
	 */
	static bool binaryFiletoCameraOpenMVG(const std::string& sPathCameraBin,
								openMVG::PinholeCamera& camera)
	{
		std::ifstream file(sPathCameraBin.c_str(), std::ios::binary);
		if (file.fail())
			return false;
		file.read((char*)camera._P.data(), (std::streamsize)(3 * 4) * sizeof(double));
		openMVG::KRt_From_P(camera._P, &camera._K, &camera._R, &camera._t);
		file.close();
		return true;
	}

	/**
	* Write in the binary file to create a openMVG pinhole camera 
	* \param[in] sPathCameraBin path to binary file
	* \param[in] camera output openMVG pinhole camera 
	* \return false if the file can't be written
	*/
	static bool cameraOpenMVGtoBinaryFile(const std::string& sPathCameraBin,
	                        const openMVG::PinholeCamera& camera)
	{
		const openMVG::Mat34 & matrixP = camera._P;
		std::ofstream file(sPathCameraBin.c_str(), std::ios::out|std::ios::binary);
		if (file.fail())
			return false;
		file.write((const char*)matrixP.data(), (std::streamsize)(3 * 4) * sizeof(double));
		file.close();
		return true;
	}

	/**
	* Parse view.txt to read binary camera fileView
	* \param[in] pathViewTxt path to file containing camera binary filename
	* \param[out] vec_cameraOpenMVG vector containing all pinhole camera
	* \return false if the file doesn't exist
	*/
	static bool readCameras( const std::string& pathViewTxt,
	          std::vector<CameraOpenMVG>& vec_cameraOpenMVG )
	{
		std::ifstream fileView(pathViewTxt.c_str());
		if(!fileView.is_open())
		{
			std::cout << "File doesn't exist" << std::endl;
			return false;
		}
		std::string sDirectoryCamera = stlplus::create_filespec(stlplus::folder_part( pathViewTxt ), "cameras");
		std::string sDirectoryImagePlane = stlplus::create_filespec(stlplus::folder_part( pathViewTxt ), "imagesOffset");
		std::string temp;
		getline(fileView,temp); // directory name
		getline(fileView,temp); // directory name
		size_t nbImages;
		fileView>> nbImages;
		while(fileView.good())
		{
			getline(fileView,temp);
			if (!temp.empty())
			{
				std::stringstream sStream(temp);
				std::string sImageName, sCamName;
				size_t w,h;
				float znear, zfar;
				sStream >> sImageName >> w >> h >> sCamName >> znear >> zfar;
				// read the corresponding camera
				openMVG::PinholeCamera camera;
				if (!binaryFiletoCameraOpenMVG( stlplus::create_filespec(sDirectoryCamera, sCamName), camera))
				{
					std::cerr << "Cannot read camera : " << sCamName << std::endl;
					return false;
				}
				vec_cameraOpenMVG.push_back(CameraOpenMVG(camera, 
				                                          stlplus::basename_part(sImageName), 
				                                          stlplus::create_filespec(sDirectoryImagePlane, 
			                                          			stlplus::basename_part(sImageName) + "_of", stlplus::extension_part(sImageName))));
			}
			temp.clear();
		} 
		return true;
	}

};

} // mayaMVG
