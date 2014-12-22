#pragma once

#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <stlplus3/filesystemSimplified/file_system.hpp>
#include <fstream>

namespace
{ // empty namespace

std::string getCameraName(const std::string& binaryName)
{
    std::string name = binaryName;
    std::string::size_type i = name.find_last_of('.');
    if(i != 0 && i != std::string::npos)
        name.erase(i, name.size() - i);
    return "camera_" + name;
}

bool setPinholeFromBinary(mayaMVG::MVGCamera& camera, const std::string& binaryPath)
{
    std::ifstream infile(binaryPath.c_str(), std::ios::binary);
    if(!infile.is_open())
    {
        LOG_ERROR("Camera binary not found (" << binaryPath << ")")
        return false;
    }
    openMVG::Mat34 P;
    infile.read((char*)P.data(), (std::streamsize)(3 * 4) * sizeof(double));
    openMVG::PinholeCamera pinholeCamera(P);
    camera.setPinholeCamera(pinholeCamera);
    infile.close();
    return true;
}

} // empty namespace

namespace mayaMVG
{

bool readCameras(const std::string& filePath, const std::string& imageDir,
                 const std::string& cameraDir)
{
    std::ifstream infile(filePath.c_str());
    if(!infile.is_open())
    {
        LOG_ERROR("Camera file not found (" << filePath << ")")
        return false;
    }

    // header
    std::string tmp;
    getline(infile, tmp); // images directory - Not used
    getline(infile, tmp); // cameras directory - Not used
    getline(infile, tmp); // count - Not used

    // cameras description
    int cameraId = 0;
    std::string line;
    for(unsigned int i = 0; std::getline(infile, line); ++i)
    {
        std::istringstream iss(line);
        std::string imageName, binaryName;
        size_t width, height;
        // double near, far;
        iss >> imageName >> width >> height >> binaryName; // >> near >> far;

        if(iss.fail())
        {
            LOG_ERROR("Invalid file format. Can't parse image at line " << i << ".")
            LOG_ERROR("Line is: \"" << line << "\"")
            continue;
        }

        // get or create camera
        std::string cameraName = getCameraName(binaryName);
        MVGCamera camera(cameraName);
        if(!camera.isValid())
            camera = MVGCamera::create(cameraName);

        setPinholeFromBinary(camera, stlplus::create_filespec(cameraDir, binaryName));
        camera.setImagePlane(stlplus::create_filespec(imageDir, imageName), width, height);
        // TODO: camera.setImageName(imageName)
        camera.setId(cameraId);
        cameraId++;

        line.clear();
    }

    infile.close();
    return true;
}

} // namespace
