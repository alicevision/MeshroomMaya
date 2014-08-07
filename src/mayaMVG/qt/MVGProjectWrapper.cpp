#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
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
    
    // Initialize currentContext
    MString context;
    MVGMayaUtil::getCurrentContext(context);
    _currentContext = context.asChar();
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

const QString MVGProjectWrapper::cameraDirectory() const
{
	return QString(_project.cameraDirectory().c_str());
}

const QString MVGProjectWrapper::imageDirectory() const
{
	return QString(_project.imageDirectory().c_str());
}

const QString MVGProjectWrapper::pointCloudFile() const
{
	return QString(_project.pointCloudFile().c_str());
}

const QString MVGProjectWrapper::logText() const
{
	return _logText;
}

void MVGProjectWrapper::setLogText(const QString& text)
{
	_logText = text;
	emit logTextChanged();
}

const QString MVGProjectWrapper::currentContext() const
{
    return _currentContext;
}

void MVGProjectWrapper::setCurrentContext(const QString& context)
{
    _currentContext = context;
    Q_EMIT currentContextChanged();
}

void MVGProjectWrapper::appendLogText(const QString& text)
{
	_logText.append(text + "\n");
	emit logTextChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	_project.setProjectDirectory(directory.toStdString());
	emit projectDirectoryChanged();
}

QString MVGProjectWrapper::openFileDialog() const
{
	MString directoryPath;
	MVGMayaUtil::openFileDialog(directoryPath);	
    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::activeSelectionContext() {
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::activeMVGContext() 
{
	MVGMayaUtil::activeContext();
    rebuildAllMeshesCacheFromMaya();
    rebuildCacheFromMaya();
}

void MVGProjectWrapper::loadProject(const QString& projectDirectoryPath)
{	
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	if(!_project.load()) {
		LOG_ERROR("An error occured when loading project.");
		appendLogText(QString("An error occured when loading project."));
	}
	Q_EMIT projectDirectoryChanged();
	
	reloadProjectFromMaya();
	
	// Select the two first cameras for the views
	if(_cameraList.size() > 1) {
		QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
		setCameraToView(cameras[0], _visiblePanelNames[0]);
		setCameraToView(cameras[1], _visiblePanelNames[1]);
	}
	rebuildCacheFromMaya();
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames)
{
    foreach(MVGCameraWrapper* camera, _cameraList.asQList<MVGCameraWrapper>())
        camera->setIsSelected(cameraNames.contains(camera->name()));
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName)
{
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper*cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);
}


DisplayData* MVGProjectWrapper::getCachedDisplayData(M3dView& view)
{		
	if(!MVGMayaUtil::isMVGView(view))
		return NULL;
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	std::map<std::string, DisplayData>::iterator it = _cacheCameraToDisplayData.find(cameraPath.fullPathName().asChar());
	
	if(it == _cacheCameraToDisplayData.end())
	{	
		return NULL;
	}
	else {
		return &(it->second);
	}
	
	return NULL;
}

void MVGProjectWrapper::reloadProjectFromMaya()
{
	// Cameras
	const std::vector<MVGCamera>& cameraList = _project.cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	_cameraList.clear();
	for(; it != cameraList.end(); ++it) {
		_cameraList.append(new MVGCameraWrapper(*it));	
	}
	emit cameraModelChanged();
	
	// TODO : Camera selection
}

void MVGProjectWrapper::rebuildCacheFromMaya() 
{	
	_cacheCameraToDisplayData.clear();
	// Rebuild for temporary cache
	// TODO: remove to use directly data from Maya
	for(QStringList::iterator panelIt = _visiblePanelNames.begin(); panelIt!= _visiblePanelNames.end(); ++panelIt)
	{
		M3dView view;
		MStatus status;
		status = M3dView::getM3dViewFromModelPanel(panelIt->toStdString().c_str(), view);
		CHECK(status);
			
		MDagPath cameraPath;
		view.getCamera(cameraPath);
		view.updateViewingParameters();
		MVGCamera c(cameraPath);
		if(c.isValid()) {
			DisplayData data;
			data.camera = c;
			
			// Browse meshes
			std::map<std::string, std::vector<MVGPoint2D> > newMap;
			for(std::map<std::string, MPointArray>::iterator it = _cacheMeshToPointArray.begin(); it != _cacheMeshToPointArray.end(); ++it)
			{
				std::vector<MVGPoint2D> points2D;
				// Browse points
				MPointArray& points3D = it->second;
				for(int i = 0; i < points3D.length(); ++i)
				{
					MVGPoint2D mvgPoint;;
					mvgPoint.point3D = points3D[i];
					MPoint point2D;
					MVGGeometryUtil::worldToCamera(view, points3D[i], point2D);
					mvgPoint.projectedPoint3D = point2D;
					mvgPoint.movableState = _cacheMeshToMovablePoint[it->first].at(i);
					
					points2D.push_back(mvgPoint);
				}
				
				newMap[it->first] = points2D;
			}
			
			data.allPoints2D = newMap;		
			_cacheCameraToDisplayData[cameraPath.fullPathName().asChar()] = data;
		}
	}
}

MStatus MVGProjectWrapper::rebuildAllMeshesCacheFromMaya()
{
	MStatus status;
	_cacheMeshToPointArray.clear();
	_cacheMeshToMovablePoint.clear();
	_cacheMeshToEdgeArray.clear();
	// Retrieves all meshes
	MDagPath path;
	MItDependencyNodes it(MFn::kMesh);
	for(; !it.isDone(); it.next()) {
		MFnDependencyNode fn(it.thisNode());
		MDagPath::getAPathTo(fn.object(), path);
		status = rebuildMeshCacheFromMaya(path);
	}
	
	return status;
}

MStatus MVGProjectWrapper::rebuildMeshCacheFromMaya(MDagPath& meshPath)
{	
	MStatus status;
	MFnMesh fnMesh(meshPath);
	MPointArray meshPoints;
	std::vector<MIntArray> meshEdges;
	
	// Mesh points
	if(!fnMesh.getPoints(meshPoints, MSpace::kWorld))
		return MS::kFailure;
	_cacheMeshToPointArray[meshPath.fullPathName().asChar()] = meshPoints;
	
	// Connected face
	std::vector<EPointState> movableStates;
	MItMeshVertex vertexIt(meshPath, MObject::kNullObj, &status);
	if(!status)
		return MS::kFailure;
	
	MIntArray faceList;
	while(!vertexIt.isDone())
	{
		vertexIt.getConnectedFaces(faceList);
		// Point connected to several faces
		if(faceList.length() > 1)
			movableStates.push_back(eUnMovable);
		
		// Face points connected to several face
		else if(faceList.length() > 0)
		{			
			// Get the points of the first face
			MItMeshPolygon faceIt(meshPath, MObject::kNullObj);
			int prev;
			faceIt.setIndex(faceList[0], prev);
			MIntArray faceVerticesIndexes;
			faceIt.getVertices(faceVerticesIndexes);
			
			// For each point, check number of connected faces
			int numConnectedFace;
			bool check = false;
			for(int i = 0; i < faceVerticesIndexes.length(); ++i)
			{
				MItMeshVertex vertexIter(meshPath, MObject::kNullObj);
				vertexIter.setIndex(faceVerticesIndexes[i], prev);
				vertexIter.numConnectedFaces(numConnectedFace);
				if(numConnectedFace > 1)
				{
					movableStates.push_back(eMovableInSamePlane);
					check = true;
					break;
				}		
			}
			if(!check)
				movableStates.push_back(eMovableRecompute);
		}
		vertexIt.next();
	}
	_cacheMeshToMovablePoint[meshPath.fullPathName().asChar()] = movableStates;	
	
	// Mesh edges
	MItMeshEdge edgeIt(meshPath, MObject::kNullObj, &status);
	if(!status)
		return MS::kFailure;
	
	while(!edgeIt.isDone())
	{
		MIntArray pointIndexArray;
		pointIndexArray.append(edgeIt.index(0));
		pointIndexArray.append(edgeIt.index(1));
		meshEdges.push_back(pointIndexArray);
		edgeIt.next();
	}
	_cacheMeshToEdgeArray[meshPath.fullPathName().asChar()] = meshEdges;	
	return status;
}