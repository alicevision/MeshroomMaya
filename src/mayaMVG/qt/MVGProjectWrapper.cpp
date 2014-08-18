#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>
#include "mayaMVG/core/MVGGeometryUtil.h"

using namespace mayaMVG;

MVGProjectWrapper::MVGProjectWrapper()
{
	_allPanelNames.append("mvgLPanel");
	_allPanelNames.append("mvgRPanel");
	_visiblePanelNames = _allPanelNames;
	
	_project = MVGProject(MVGProject::_PROJECT);
	if(!_project.isValid()) {
		_project = MVGProject::create(MVGProject::_PROJECT);
		LOG_INFO("New OpenMVG Project.")
	}
    
    // // Initialize currentContext
    // MString context;
    // MVGMayaUtil::getCurrentContext(context);
    // _currentContext = context.asChar();
}

MVGProjectWrapper::~MVGProjectWrapper()
{
}

const QString MVGProjectWrapper::moduleDirectory() const
{
	return QString(_project.moduleDirectory().c_str());
}

const QString MVGProjectWrapper::projectDirectory() const
{
	return QString(_project.projectDirectory().c_str());
}

const QString MVGProjectWrapper::logText() const
{
	return _logText;
}

void MVGProjectWrapper::setLogText(const QString& text)
{
	_logText = text;
	Q_EMIT logTextChanged();
}

// const QString MVGProjectWrapper::currentContext() const
// {
//     return _currentContext;
// }

// void MVGProjectWrapper::setCurrentContext(const QString& context)
// {
//     _currentContext = context;
//     Q_EMIT currentContextChanged();
// }

void MVGProjectWrapper::appendLogText(const QString& text)
{
	_logText.append(text + "\n");
	Q_EMIT logTextChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	_project.setProjectDirectory(directory.toStdString());
	Q_EMIT projectDirectoryChanged();
}

QString MVGProjectWrapper::openFileDialog() const
{
	MString directoryPath;
	MVGMayaUtil::openFileDialog(directoryPath);	
    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::activeSelectionContext() const
{
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::activeMVGContext() 
{
	MVGMayaUtil::activeContext();
    // rebuildAllMeshesCacheFromMaya();
    // rebuildCacheFromMaya();
}

void MVGProjectWrapper::loadProject(const QString& projectDirectoryPath)
{	
    if(projectDirectoryPath.isEmpty())
        return;
	if(!_project.load(projectDirectoryPath.toStdString())) {
		LOG_ERROR("An error occured when loading project.");
		appendLogText(QString("An error occured when loading project."));
        return;
	}
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	Q_EMIT projectDirectoryChanged();
	
	reloadMVGCamerasFromMaya();
	// Select the two first cameras for the views
	if(_cameraList.size() > 1) {
		QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
		setCameraToView(cameras[0], _visiblePanelNames[0]);
		setCameraToView(cameras[1], _visiblePanelNames[1]);
	}
	// rebuildAllMeshesCacheFromMaya();
	// rebuildCacheFromMaya();
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames) const
{
    foreach(MVGCameraWrapper* camera, _cameraList.asQList<MVGCameraWrapper>())
        camera->setIsSelected(cameraNames.contains(camera->name()));
}

void MVGProjectWrapper::selectCameras(const QStringList& cameraNames) const
{
    std::vector<std::string> cameras;
    for(QStringList::const_iterator it = cameraNames.begin(); it != cameraNames.end(); ++it)
        cameras.push_back(it->toStdString());
    
    _project.selectCameras(cameras);
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName) const
{
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper*cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);
}

// <<<<<<< HEAD
// void MVGProjectWrapper::reloadProjectFromMaya()
// =======

// DisplayData* MVGProjectWrapper::getCachedDisplayData(M3dView& view)
// {		
// 	if(!MVGMayaUtil::isMVGView(view))
// 		return NULL;
// 	MDagPath cameraPath;
// 	view.getCamera(cameraPath);
// 	std::map<std::string, DisplayData>::iterator it = _cacheCameraToDisplayData.find(cameraPath.fullPathName().asChar());
	
// 	if(it == _cacheCameraToDisplayData.end())
// 	{	
// 		return NULL;
// 	}
// 	else {
// 		return &(it->second);
// 	}
	
// 	return NULL;
// }

void MVGProjectWrapper::reloadMVGCamerasFromMaya()
{
    Q_EMIT projectDirectoryChanged();
	// Cameras
	const std::vector<MVGCamera>& cameraList = _project.cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	_cameraList.clear();
	for(; it != cameraList.end(); ++it) {
		_cameraList.append(new MVGCameraWrapper(*it));
	}
	Q_EMIT cameraModelChanged();
	// TODO : Camera selection
}

// <<<<<<< HEAD
// =======
// void MVGProjectWrapper::rebuildCacheFromMaya() 
// {	
// 	_cacheCameraToDisplayData.clear();
// 	// Rebuild for temporary cache
// 	// TODO: remove to use directly data from Maya
// 	for(QStringList::iterator panelIt = _visiblePanelNames.begin(); panelIt!= _visiblePanelNames.end(); ++panelIt)
// 	{
// 		M3dView view;
// 		MStatus status;
// 		status = M3dView::getM3dViewFromModelPanel(panelIt->toStdString().c_str(), view);
// 		CHECK(status);
        
// 		MDagPath cameraPath;
// 		view.getCamera(cameraPath);
// 		view.updateViewingParameters();
// 		MVGCamera c(cameraPath);
// 		if(c.isValid()) {
// 			DisplayData data;
// 			data.camera = c;
			
// 			// Browse meshes
// 			std::map<std::string, std::vector<MVGPoint2D> > newMap;
// 			for(std::map<std::string, MPointArray>::iterator it = _cacheMeshToPointArray.begin(); it != _cacheMeshToPointArray.end(); ++it)
// 			{
// 				std::vector<MVGPoint2D> points2D;
// 				// Browse points
// 				MPointArray& points3D = it->second;
// 				for(int i = 0; i < points3D.length(); ++i)
// 				{
// 					MVGPoint2D mvgPoint;;
// 					mvgPoint.point3D = points3D[i];
// 					MPoint point2D;
// 					MVGGeometryUtil::worldToCamera(view, points3D[i], point2D);
// 					mvgPoint.projectedPoint3D = point2D;
// 					mvgPoint.movableState = _cacheMeshToMovablePoint[it->first].at(i);
					
// 					points2D.push_back(mvgPoint);
// 				}
				
// 				newMap[it->first] = points2D;
// 			}
			
// 			data.allPoints2D = newMap;		
// 			_cacheCameraToDisplayData[cameraPath.fullPathName().asChar()] = data;
// 		}
// 	}
// }

// void MVGProjectWrapper::clear()
// {
//     _cacheCameraToDisplayData.clear();
//     _cacheMeshToEdgeArray.clear();
//     _cacheMeshToMovablePoint.clear();
//     _cacheMeshToPointArray.clear();
//     _cameraList.clear();
    
//     Q_EMIT projectDirectoryChanged();
// }

// MStatus MVGProjectWrapper::rebuildAllMeshesCacheFromMaya()
// {
// 	MStatus status;
// 	_cacheMeshToPointArray.clear();
// 	_cacheMeshToMovablePoint.clear();
// 	_cacheMeshToEdgeArray.clear();
// 	// Retrieves all meshes
// 	MDagPath path;
// 	MItDependencyNodes it(MFn::kMesh);
// 	for(; !it.isDone(); it.next()) {
// 		MFnDependencyNode fn(it.thisNode());
// 		MDagPath::getAPathTo(fn.object(), path);
// 		status = rebuildMeshCacheFromMaya(path);
// 	}
	
// 	return status;
// }

// MStatus MVGProjectWrapper::rebuildMeshCacheFromMaya(MDagPath& meshPath)
// {	
// 	MStatus status;
//     MVGMesh mesh(meshPath);
// 	MPointArray meshPoints;
// 	std::vector<MIntArray> meshEdges;
	
// 	// Mesh points
// 	if(!mesh.getPoints(meshPoints))
// 		return MS::kFailure;
// 	_cacheMeshToPointArray[meshPath.fullPathName().asChar()] = meshPoints;
	
// 	// Connected face
// 	std::vector<EPointState> movableStates;
// 	MItMeshVertex vertexIt(meshPath, MObject::kNullObj, &status);
// 	if(!status)
// 		return MS::kFailure;
	
// 	MIntArray faceList;
// 	while(!vertexIt.isDone())
// 	{
// 		vertexIt.getConnectedFaces(faceList);
// 		// Point connected to several faces
// 		if(faceList.length() > 1)
// 			movableStates.push_back(eUnMovable);
		
// 		// Face points connected to several face
// 		else if(faceList.length() > 0)
// 		{			
// 			// Get the points of the first face
// 			MItMeshPolygon faceIt(meshPath, MObject::kNullObj);
// 			int prev;
// 			faceIt.setIndex(faceList[0], prev);
// 			MIntArray faceVerticesIndexes;
// 			faceIt.getVertices(faceVerticesIndexes);
			
// 			// For each point, check number of connected faces
// 			int numConnectedFace;
// 			bool check = false;
// 			for(int i = 0; i < faceVerticesIndexes.length(); ++i)
// 			{
// 				MItMeshVertex vertexIter(meshPath, MObject::kNullObj);
// 				vertexIter.setIndex(faceVerticesIndexes[i], prev);
// 				vertexIter.numConnectedFaces(numConnectedFace);
// 				if(numConnectedFace > 1)
// 				{
// 					movableStates.push_back(eMovableInSamePlane);
// 					check = true;
// 					break;
// 				}		
// 			}
// 			if(!check)
// 				movableStates.push_back(eMovableRecompute);
// 		}
// 		vertexIt.next();
// 	}
// 	_cacheMeshToMovablePoint[meshPath.fullPathName().asChar()] = movableStates;	
	
// 	// Mesh edges
// 	MItMeshEdge edgeIt(meshPath, MObject::kNullObj, &status);
// 	if(!status)
// 		return MS::kFailure;
	
// 	while(!edgeIt.isDone())
// 	{
// 		MIntArray pointIndexArray;
// 		pointIndexArray.append(edgeIt.index(0));
// 		pointIndexArray.append(edgeIt.index(1));
// 		meshEdges.push_back(pointIndexArray);
// 		edgeIt.next();
// 	}
// 	_cacheMeshToEdgeArray[meshPath.fullPathName().asChar()] = meshEdges;	
// 	return status;
// }
// >>>>>>> minimalVersion
