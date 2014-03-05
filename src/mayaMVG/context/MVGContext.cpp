#include <mayaMVG/geometry/nView2DpointTo3D.hpp>
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
#include <maya/MFloatPointArray.h>  
#include <maya/MItDependencyNodes.h>
#include <maya/MNodeMessage.h>


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

/** 
 * Remove node attributes when it is remove
 */
void removeNode(MNodeMessage::AttributeMessage msg, MPlug &plug, void *clientData)
{
  if ( msg & MNodeMessage::kAttributeEval )
  {
    std::cout << "kAttributeEval    an attribute of this node has been evaluated" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeSet )
  {
    std::cout << "kAttributeSet   an attribute value of this node has been set" << std::endl;
  }
  
  else if ( msg & MNodeMessage::kConnectionBroken )
  {
    std::cout << "kConnectionBroken   a connection has been broken for an attribute of this node" << std::endl;
  }
  else if ( msg & MNodeMessage::kConnectionMade )
  {
    std::cout << "kConnectionMade   a connection has been made to an attribute of this node" << std::endl;
  }
  
  else if ( msg & MNodeMessage::kAttributeLocked )
  {
    std::cout << "kAttributeLocked    an attribute of this node has been locked" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeUnlocked )
  {
    std::cout << "kAttributeUnlocked    an attribute of this node has been unlocked" << std::endl;
  }
  
  else if ( msg & MNodeMessage::kAttributeAdded )
  {
    std::cout << "kAttributeAdded   an attribute has been added to this node" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeRemoved )
  {
    std::cout << "kAttributeRemoved   an attribute has been removed from this node" << std::endl;
  }
  
  else if ( msg & MNodeMessage::kAttributeRenamed )
  {
    std::cout << "kAttributeRenamed   an attribute of this node has been renamed" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeKeyable )
  {
    std::cout << "kAttributeKeyable   an attribute of this node has been marked keyable" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeUnkeyable )
  {
    std::cout << "kAttributeUnkeyable   an attribute of this node has been marked unkeyable" << std::endl;
  }
  else if ( msg & MNodeMessage::kIncomingDirection )
  {
    std::cout << "kIncomingDirection    the connection was coming into the node" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeArrayAdded )
  {
    std::cout << "kAttributeArrayAdded    an array attribute has been added to this node" << std::endl;
  }
  else if ( msg & MNodeMessage::kAttributeArrayRemoved )
  {
    std::cout << "kAttributeArrayRemoved    an array attribute has been removed from this node" << std::endl;
  }
  else if ( msg & MNodeMessage::kOtherPlugSet )
  {
    std::cout << "kOtherPlugSet   the otherPlug data has been set" << std::endl;
  }
  else if ( msg & MNodeMessage::kLast )
  {
    std::cout << "kLast   last value of the enum" << std::endl;
  }
  else
    std::cout << "Banane" << std::endl;
  
  
  std::cout << "removeNode" << std::endl;
  std::cout << "removeNode : " << msg <<  std::endl;
  if ( !(msg & MNodeMessage::kAttributeRemoved) )
  {
    return;
  }
  
  std::cout << "removeNode2" << std::endl;
  MStatus status;
  MVGContext* contextMVG = (MVGContext*) clientData;
  int index = plug.logicalIndex();
  for(MItDependencyNodes it(MFn::kDependencyNode); !it.isDone(); it.next()) 
  {
    MDagPath p;
    MDagPath::getAPathTo(it.item(), p);
    MFnDependencyNode fn(p.node());
    MFnDependencyNode depNodeMesh( contextMVG->m_meshPath.node() );
    if(fn.typeName() == "camera") 
    {
      MFnDependencyNode fn(p.transform());
      MString attributeName = fn.name();
      
      // Get 2D point
      MPlug plugAttribute = depNodeMesh.findPlug( attributeName, &status );
      if ( !status )
        continue;
      MPoint point3D;
      if ( plugAttribute.isArray() )
      {
        MPlug plugtmp = plugAttribute.elementByPhysicalIndex( index );
        
        //Remove the plug
        //TODO
        MSelectionList list;
        list.add( plugtmp );
        list.remove( 0 );
        std::cout << "Remove index : " << index << std::endl;
      }
    }
  }
}

void MVGContext::createMesh()
{
//  // get distance to origin
//  double distance = m_points[0].wpos.distanceTo(MPoint(0,0,0));

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
  // TODO: use all clouds loaded: main points cloud and/or dense points cloud
  selectionList.add( "PointCloudParticle" );
  MObject objectParticles;
  MStatus status = selectionList.getDependNode( 0, objectParticles );
  if ( status == MStatus::kInvalidParameter )
  {
    LOG_ERROR("MVGContext::createMesh", "Point cloud doesn't exist")
    return;
  }
  
  MObject particleNode;
  MFnDagNode fnDag(objectParticles);
  particleNode = fnDag.child(0, &status);
  
  MFnParticleSystem particles( particleNode, &status );
  if ( status == MStatus::kInvalidParameter )
  {
    LOG_ERROR("MVGContext::doPress", "Invalid particle object")
    return;
  }

  MVectorArray array_position;
  particles.getPerParticleAttribute( MString("position"), array_position, &status );
  
  // TODO: use visibility to only use points visible from that camera.
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
  if(!m_meshPath.isValid() || (m_meshPath.length() <= 0))
  {
    MFnMesh fn;
    meshObj = fn.addPolygon(verticesReprojected, true, kMFnMeshPointTolerance, MObject::kNullObj, &status);        
    MDagPath::getAPathTo(meshObj, m_meshPath);
    m_meshPath.extendToShape();
    nbVerticesAfter = fn.numVertices();    
    MObject obj = m_meshPath.node();
    MNodeMessage::addAttributeAddedOrRemovedCallback ( obj, removeNode, this );
  }
  else
  {
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
    for( unsigned i = 0; i < 3; i++ )
    {
      MPlug child = plugtmp.child( i );
      child.setValue( point[ i ]);
    }
  }
}
       

MStatus MVGContext::doPress(MEvent & event)
{
  // Get current camera
  MDagPath dagPathCamera;
  currentView().getCamera( dagPathCamera );    
  MFnDagNode dagNodeTr ( dagPathCamera.transform() );  
  MDagPath dagPathCameraTr;
  dagNodeTr.getPath( dagPathCameraTr );
  MString cameraName = dagPathCameraTr.partialPathName();
  if(cameraName == "persp" || cameraName == "top"
    || cameraName == "front" || cameraName == "side")
  {
    return MPxContext::doPress(event);
  }

  openMVG::PinholeCamera camera = getCameraMVG( dagPathCamera );
  
  // Get 2D point in the openMVG space
  short x, y;
  event.getPosition(x, y);
  MPoint pos3D;
  MVector vec3D;
  currentView().viewToWorld( x, y, pos3D, vec3D );
    
  openMVG::Vec2 pos2D = camera.Project( openMVG::Vec3( pos3D[0], 
                                                       pos3D[1], 
                                                       pos3D[2] ) );
  // Check if there is a point under the mouse
  MStatus status;
  MFnMesh fnMesh( m_meshPath, &status );
  
  MFloatPointArray vertexArray;
  fnMesh.getPoints( vertexArray );
  if ( status && vertexArray.length( ) >= 4 )
  {
    for( int index = 0; index < vertexArray.length( ); index++ )
    {
      MFloatPoint point3D = vertexArray[index];
      openMVG::Vec2 point2D = camera.Project( openMVG::Vec3( point3D[0], 
                                                             point3D[1], 
                                                             point3D[2] ) );
      
      double distance = (point2D - pos2D).cwiseAbs().sum();
      
      if ( distance < 7.0 )
      {
        selectedPoint.x = pos2D(0);
        selectedPoint.y = pos2D(1);
        selectedPoint.index = index;
        continue;
      }
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
  MStatus status;
  // Check if the 2D point moved 
  
  // Get current camera
  MDagPath dagPathCamera;
  currentView().getCamera( dagPathCamera );    
  
  MFnDagNode dagNodeTr ( dagPathCamera.transform() );  
  MDagPath dagPathCameraTr;
  dagNodeTr.getPath( dagPathCameraTr );
  MString cameraName = dagPathCameraTr.partialPathName();
  if(cameraName == "persp" || cameraName == "top"
    || cameraName == "front" || cameraName == "side")
  {
    return MPxContext::doRelease(event);
  }
  
  openMVG::PinholeCamera camera = getCameraMVG( dagPathCamera );
  
  // Get 2D point in the openMVG space
  short x, y;
  event.getPosition(x, y);
  MPoint pos3D;
  MVector vec3D;
  currentView().viewToWorld( x, y, pos3D, vec3D );
  pos3D += vec3D * 10;
  openMVG::Vec2 pos2D = camera.Project( openMVG::Vec3( pos3D[0], 
                                                       pos3D[1], 
                                                       pos3D[2] ) );
  
  double distance = (openMVG::Vec2( selectedPoint.x, selectedPoint.y ) - pos2D).cwiseAbs().sum();
  
  MFnMesh fnMesh( m_meshPath, &status );
  
  if ( status && selectedPoint.index != -1 && distance != 0.0 )
  {
    // Update the point
    
    // Add the new coordinate into mesh attribute 
    MFnMesh fnMesh( m_meshPath );
    MFloatPointArray vertexArray;
    fnMesh.getPoints( vertexArray );
    MObject cameraTransform = dagPathCamera.transform();  
    MFnTransform transformCamera( cameraTransform );  
    
    // Write the new coordinate into mesh attribute
    createCameraAttribute( transformCamera.partialPathName() );
    addPointToAttribute( transformCamera.partialPathName(), selectedPoint.index, pos3D );
    
    // Compute triangulation if it is possible
    
    // Find all 2D positions and associated openMVG cameras
    std::vector<geometry::PositionByCam> vec_position2dByCamera;
    MFnDependencyNode depNodeMesh( m_meshPath.node( ) );
    for(MItDependencyNodes it(MFn::kDependencyNode); !it.isDone(); it.next()) 
    {
      MDagPath p;
      MDagPath::getAPathTo(it.item(), p);
      MFnDependencyNode fn(p.node());
      if(fn.typeName() == "camera") 
      {
        MFnDependencyNode fn(p.transform());
        MString attributeName = fn.name();
        
        // Get 2D point
        MPlug plugAttribute = depNodeMesh.findPlug( attributeName, &status );
        if ( !status )
          continue;
        MPoint point3D;
        if ( plugAttribute.isArray() )
        {
          MPlug plugtmp = plugAttribute.elementByLogicalIndex( selectedPoint.index );
          for( unsigned i = 0; i<3; i++ )
          {
            MPlug child = plugtmp.child( i );
            child.getValue( point3D[ i ]);
          }
        }
        
        // Get camera
        MDagPath dagCamera;
        MDagPath::getAPathTo( p.node(), dagCamera );
        openMVG::PinholeCamera camera;
        camera = getCameraMVG( dagCamera );
        
        // Project 3D point to 2D point in the camera space
        openMVG::Vec2 point2D = camera.Project( openMVG::Vec3( point3D[0],
                                                              point3D[1],
                                                              point3D[2] ) );
        
        vec_position2dByCamera.push_back( geometry::PositionByCam( point2D, camera ) );
      }
    }
    
    // Triangulate if the point is already setted in 2 cameras
    if ( vec_position2dByCamera.size() >= 2 )
    {
      // Triangulate point positon  
      openMVG::Vec3 outPoint3D;
      geometry::triangulatePosition( vec_position2dByCamera, outPoint3D );
      
      // Apply point triangluation  
      MPlug plugPosition = depNodeMesh.findPlug( "position", true, &status );  
      
      vertexArray[selectedPoint.index][0] = outPoint3D(0);
      vertexArray[selectedPoint.index][1] = outPoint3D(1);
      vertexArray[selectedPoint.index][2] = outPoint3D(2);
      fnMesh.setPoints( vertexArray );
    }
    else
    {
      vertexArray[selectedPoint.index][0] = pos3D(0);
      vertexArray[selectedPoint.index][1] = pos3D(1);
      vertexArray[selectedPoint.index][2] = pos3D(2);
      fnMesh.setPoints( vertexArray );
    }
  }
  else // Add point to create a face
  { 
    if (event.mouseButton() == MEvent::kLeftMouse) {
      if (!event.isModifierShift() && !event.isModifierControl()
              && !event.isModifierMiddleMouseButton()) {

        short x, y;
        event.getPosition(x, y);

        if(m_points.size() > 3) 
        {
          createMesh();
          return MPxContext::doRelease(event);
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
  }
  selectedPoint.index = -1;
  
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
  if (manipulator)
  {
    ctxPtr->addManipulator(manipObject);
    manipulator->setContext(ctxPtr);
  }
}

void MVGContext::setMousePos(size_t x, size_t y)
{
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
