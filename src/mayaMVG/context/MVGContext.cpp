#include <mayaMVG/geometry/oneView2DpolygonTo3D.hpp>
#include <openMVG/cameras/PinholeCamera.hpp>

#include <maya/MFnParticleSystem.h>
#include <maya/MStatus.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>
#include <maya/MVectorArray.h>

#include <QWidget>
#include "mayaMVG/qt/MVGEventFilter.h"
#include "mayaMVG/util/MVGUtil.h"
#include "mayaMVG/util/MVGLog.h"
#include "mayaMVG/context/MVGManipContainer.h"
#include "mayaMVG/context/MVGContext.h"
#include <maya/M3dView.h>
#include <maya/MToolsInfo.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MObject.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>

#include <maya/MFnNumericAttribute.h>  
#include <maya/MPlugArray.h>  

using namespace mayaMVG;

namespace {
  M3dView currentView() {
    return M3dView::active3dView();
  }
}

openMVG::PinholeCamera getCameraMVG( const MDagPath& dagPathCamera )
{
  MStatus status;
  
  MObject cameraShape = dagPathCamera.node();
  MObject cameraTransform = dagPathCamera.transform();
  MFnCamera fnCamera( cameraShape );     
  MFnTransform transformCamera( cameraTransform );  
    
  openMVG::Mat3 mat_K;
  openMVG::Mat3 mat_R;
  openMVG::Vec3 vec_t;
  
  // Get rotation MVG matrix
  MQuaternion quaternion;
  transformCamera.getRotation( quaternion,
                                MSpace::kTransform );
  MMatrix matrixRotationMaya = quaternion.asMatrix();
  
  // TODO To simplify        
  for( int i = 0; i < 3; i++ )
  {
    for( int j = 0; j < 3; j++ )
    {
      if ( i == 0 )
        mat_R(i, j) = matrixRotationMaya[i][j];
      else
        mat_R(i, j) = -matrixRotationMaya[i][j];
    }
  }
  
  // Get translation MVG vector
  MVector opticalCenterMaya = transformCamera.getTranslation( MSpace::kTransform );
  openMVG::Vec3 opticalCenterOpenMVG( opticalCenterMaya[0], opticalCenterMaya[1], opticalCenterMaya[2] );
  vec_t = (-1) * mat_R * opticalCenterOpenMVG;
  
  // Get intrinsic MVG matrix
  MPlug plugCamera = fnCamera.findPlug( "imagePlane" ); 
  MPlugArray array_plug;
  plugCamera.connectedTo( array_plug, true, true );
  MObject objectPlane = array_plug[0].node();        
  MFnDagNode fnDagNodePlane( objectPlane, &status );
  MDagPath dagPathImagePlane;
  fnDagNodePlane.getPath( dagPathImagePlane );
  dagPathImagePlane.extendToShape( );
  MFnDependencyNode depNodePlane( dagPathImagePlane.node(), &status );
  
  int width = 0; 
  int height = 0; 
  
  depNodePlane.findPlug( "width" ).getValue( width );
  depNodePlane.findPlug( "height" ).getValue( height );
  
  double fov = fnCamera.horizontalFieldOfView( );
  double focal = (double)width / tan( fov / 2. ) / 2.;
  
  mat_K << focal, 0, width / 2,
            0, focal, height / 2,
            0, 0, 1;

  return openMVG::PinholeCamera( mat_K, mat_R, vec_t );
}
MVGContext::MVGContext()
  : m_eventFilter(NULL)
{
  setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent & event)
{
  updateManipulators(this);
  // install context event filter
  m_eventFilter = new MVGContextEventFilter(this);
  QWidget* leftViewport = MVGUtil::getMVGLeftViewportLayout();
  QWidget* rightViewport = MVGUtil::getMVGRightViewportLayout();
  if(!leftViewport || !rightViewport)
    return;
  leftViewport->installEventFilter(m_eventFilter);
  rightViewport->installEventFilter(m_eventFilter);
}

void MVGContext::toolOffCleanup()
{
  // remove context event filter
  QWidget* leftViewport = MVGUtil::getMVGLeftViewportLayout();
  QWidget* rightViewport = MVGUtil::getMVGRightViewportLayout();
  if(leftViewport)
    leftViewport->installEventFilter(m_eventFilter);
  if(rightViewport)
    rightViewport->installEventFilter(m_eventFilter);
  MPxContext::toolOffCleanup();
}

MStatus MVGContext::createMesh( MEvent & event )
{
  // get distance to origin
  double distance = m_points[0].wpos.distanceTo(MPoint(0,0,0));

  // as MPointArray
  MPointArray vertices;
  for(size_t i = 0; i < m_points.size(); ++i)
    vertices.append(m_points[i].wpos+m_points[i].wdir*10); // FIXME

        
  //-----------------------------------------
  // Compute projected position
  //-----------------------------------------
  
  // Get visible points for the current camera //TODO
  MSelectionList selectionList;
  selectionList.clear();
  selectionList.add( "PointCloudParticle" );
  MObject objectParticles;
  MStatus status = selectionList.getDependNode( 0, objectParticles );
  if ( status == MStatus::kInvalidParameter )
  {
    LOG_ERROR("MVGContext::doPress", "Point cloud doesn't exist")
    return status;
  }
  
  MObject particleNode;
  MFnDagNode fnDag(objectParticles);
  particleNode = fnDag.child(0, &status);
  
  MFnParticleSystem particles( particleNode, &status );
  if ( status == MStatus::kInvalidParameter )
  {
    LOG_ERROR("MVGContext::doPress", "Invalid particle object")
    return status;
  }

  
  MVectorArray array_position;
  particles.getPerParticleAttribute( MString("position"), array_position, &status );
  
  std::vector<openMVG::Vec3> vec_particles;
  for( int i = 0; i < array_position.length(); i++ )
  {
    vec_particles.push_back( openMVG::Vec3( array_position[i][0], array_position[i][1], array_position[i][2] ) );
  }
  
  // Get current pinhole camera
  MDagPath dagPathCamera;
  currentView().getCamera( dagPathCamera );        
  MObject cameraShape = dagPathCamera.node();
  MObject cameraTransform = dagPathCamera.transform();
  MFnCamera fnCamera( cameraShape );     
  MFnTransform transformCamera( cameraTransform );  
  
  openMVG::PinholeCamera camera;
  camera = getCameraMVG( dagPathCamera );
  
  // Get 2D position
  std::vector<openMVG::Vec2> vec_point2D;
  for(size_t i = 0; i < m_points.size(); ++i)
  {
    vec_point2D.push_back( camera.Project( openMVG::Vec3( vertices[i][0], 
                                                          vertices[i][1], 
                                                          vertices[i][2] ) ) );
  }
  
  // Compute new position
  // Get 3D points inside the face
  std::vector<openMVG::Vec3> vec_insidePoints;
  geometry::getInsidePoints( vec_point2D,
                             vec_particles,
                             camera, 
                             vec_insidePoints );
  
          
  // Compute the plane equation
  std::vector<openMVG::Vec3> vec_facePoints3D;
  geometry::computePlanEquation( vec_point2D,
                                 vec_insidePoints,
                                 camera,
                                 vec_facePoints3D );
  
    
  MPointArray verticesReprojected;
  for( std::vector<openMVG::Vec3>::const_iterator iter_point3D = vec_facePoints3D.begin();
            iter_point3D != vec_facePoints3D.end();
            iter_point3D++
      )
  {
    verticesReprojected.append( (*iter_point3D)[0], (*iter_point3D)[1], (*iter_point3D)[2], 0 );
    openMVG::Vec2 test = camera.Project(*iter_point3D);
  }  
  
  //-----------------------------------------
         
  MObject meshObj;       
  size_t nbVerticesBefore = 0, nbVerticesAfter;
  if(!m_meshPath.isValid() || (m_meshPath.length() <= 0)) {
    MFnMesh fn;
    meshObj = fn.addPolygon(verticesReprojected, true, kMFnMeshPointTolerance, MObject::kNullObj, &status);        
    MDagPath::getAPathTo(meshObj, m_meshPath);
    m_meshPath.extendToShape();
    nbVerticesAfter = fn.numVertices();    
  } else {
    MFnMesh fn(m_meshPath, &status);
    nbVerticesBefore = fn.numVertices();
    MPlug outMeshPlug = fn.findPlug("inMesh", &status);
    fn.addPolygon(verticesReprojected, true, kMFnMeshPointTolerance, outMeshPlug.node(), &status);
    nbVerticesAfter = fn.numVertices();
  }
  
  // Create the attribute if it doesn't exists
  createCameraAttribute( transformCamera.partialPathName() );
  
  int index4 = 0;
  for( size_t index = nbVerticesBefore; index < nbVerticesAfter; index++, index4++ )
  {
    addPointToAttribute( transformCamera.partialPathName(), index, verticesReprojected[ index4 ] );
  }
  
  m_points.clear();
  return MPxContext::doPress(event);
}

/**
 * Add an array point attribute to the mesh
 *\param[in] attributeName 
 */
void MVGContext::createCameraAttribute( const MString& attributeName )
{
  MFnDependencyNode depNodeMesh( m_meshPath.node() );
  if ( depNodeMesh.attribute( attributeName ).isNull( ) )
  {
    MFnNumericAttribute nAttr;
    nAttr.setArray( true );
    nAttr.setStorable( false );
    nAttr.setWritable( false );  
    
    MObject pointObj = nAttr.createPoint( attributeName, attributeName );
    nAttr.setArray( true );
    depNodeMesh.addAttribute( pointObj, MFnDependencyNode::kLocalDynamicAttr );
  }  
}

/**
 * Add a point on the attribute
 * \param[in] attributeName 
 * \param[in] vertexIndex Index of the vertex into the mesh
 * \param[in] point coordinate of the point
 */
void MVGContext::addPointToAttribute( const MString& attributeName,
                                      const size_t vertexIndex,
                                      const MPoint& point )
{
  MStatus status;
  MFnDependencyNode depNodeMesh( m_meshPath.node() );
  MPlug plugMeshIndex = depNodeMesh.findPlug( attributeName, true, &status );  
  if ( plugMeshIndex.isArray() )
  {
    MPlug plugtmp = plugMeshIndex.elementByLogicalIndex( vertexIndex );
    for( unsigned i = 0; i<3; i++ )
    {
      MPlug child = plugtmp.child( i );
      child.setValue( point[ i ]);
    }
  }
}
       

MStatus MVGContext::doPress(MEvent & event)
{
  if (event.mouseButton() == MEvent::kLeftMouse) {
    if (!event.isModifierShift() && !event.isModifierControl()
            && !event.isModifierMiddleMouseButton()) {

      short x, y;
      event.getPosition(x, y);

      if(m_points.size() > 3) 
      {        
        return createMesh( event );
      }
      MVGPoint p;
      currentView().viewToWorld(x, y, p.wpos, p.wdir);
      p.spos[0] = x;
      p.spos[1] = y;
      p.wdir.normalize();
      m_points.push_back(p);
      currentView().refresh();
    }
  }

  return MPxContext::doPress(event);
}

MStatus MVGContext::doDrag(MEvent & event)
{
  return MS::kSuccess;
}

MStatus MVGContext::doRelease(MEvent & event)
{
  return MPxContext::doRelease(event);
}

void MVGContext::getClassName(MString & name) const
{
  name.set("MVGTool");
}

// static
void MVGContext::updateManipulators(void * data)
{
  // delete all manipulators
  MVGContext * ctxPtr = static_cast<MVGContext *>(data);
  ctxPtr->deleteManipulators();

  // then add a new one
  MString manipName("MVGManip");
  MObject manipObject;
  MVGManipContainer * manipulator =
      static_cast<MVGManipContainer *>(MVGManipContainer::newManipulator(
              manipName, manipObject));
  if (manipulator) {
    ctxPtr->addManipulator(manipObject);
    manipulator->setContext(ctxPtr);
  }
}

void MVGContext::setMousePos(size_t x, size_t y) {
  m_mousePosX = x;
  m_mousePosY = currentView().portHeight() - y;
  updateManipulators(this);
}

size_t MVGContext::mousePosX() const 
{
  return m_mousePosX;
}

size_t MVGContext::mousePosY() const 
{
  return m_mousePosY;
}

MVGContext::vpoint_t MVGContext::points() const
{
  return m_points;  
}
