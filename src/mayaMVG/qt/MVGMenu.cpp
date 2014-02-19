#include <mayaMVG/readerMVG/cameraReaderMVG.hpp>
#include <mayaMVG/readerMVG/pointCloudReaderMVG.hpp>

#include <maya/MFnParticleSystem.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MVector.h>

#include <QHBoxLayout>
#include <QPushButton>
#include "mayaMVG/qt/MVGMenu.h"
#include "mayaMVG/qt/MVGMenuItem.h"
#include "mayaMVG/util/MVGLog.h"
#include "mayaMVG/util/MVGUtil.h"
#include <maya/MQtUtil.h>
#include <maya/MDagModifier.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MDagPath.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>




using namespace mayaMVG;

MVGMenu::MVGMenu(QWidget * parent) : QWidget(parent) {
	ui.setupUi(this);
	ui.progressBar->setVisible(false);
	ui.cancelButton->setVisible(false);
}

MVGMenu::~MVGMenu() {
}

void MVGMenu::addCamera(const QString& cameraName) {
	if(cameraName.isEmpty())
		return;
	if(cameraName == "persp" || cameraName == "top"
		|| cameraName == "front" || cameraName == "side")
		return;
	MVGMenuItem * itemWidget = new MVGMenuItem(cameraName);
	ui.cameraList->addItem(cameraName);
	QListWidgetItem * item = ui.cameraList->item(ui.cameraList->count()-1);
	ui.cameraList->setItemWidget(item, itemWidget);
	item->setSizeHint(QSize(item->sizeHint().width(), 66));
}

void MVGMenu::clear() {
	ui.cameraList->clear();
}

void MVGMenu::selectCameras(const QList<QString>& cameraNames) {
	for(size_t i = 0; i <  ui.cameraList->count(); ++i)
		ui.cameraList->item(i)->setSelected(cameraNames.contains(ui.cameraList->item(i)->text()));
}

// slot
void MVGMenu::on_cameraList_itemSelectionChanged() {
	QList<QListWidgetItem *> selectedItems = ui.cameraList->selectedItems();
	MVGUtil::clearMayaSelection();
	for(size_t i = 0; i < selectedItems.size(); ++i) {
		MVGUtil::addToMayaSelection(MQtUtil::toMString(selectedItems[i]->text()));
	}
}

void createMayaCamera( const readerMVG::CameraOpenMVG& cameraOpenMVG )
{
  MDagModifier dagModifier;
  MFnCamera fnCamera;
  MStatus status;
  
  // Get intrinsic parameters
  size_t width  = cameraOpenMVG.camera._K( 0, 2 ) * 2;
  size_t height = cameraOpenMVG.camera._K( 1, 2 ) * 2;
  size_t focal  = cameraOpenMVG.camera._K( 0, 0 );
  
  double defHFA = fnCamera.horizontalFilmAperture( );
  double defVFA = ( (double)height / (double)width ) * defHFA;
  double fov = ( 2. * atan( (double)width / ( 2. * (double)focal ) ) );
  
  // Create camera
  MObject objectCamera = fnCamera.create(&status);
  MFnTransform fnTransformCamera( objectCamera );
  MFnDependencyNode depNodeCamera( objectCamera );
  
  MDagPath dagPathCamera;
  fnCamera.getPath( dagPathCamera );
  MFnCamera camera( dagPathCamera.node() );
  
  // Change camera name
  std::string cameraName =  "camera_" + cameraOpenMVG.name;
  depNodeCamera.setName( cameraName.c_str() );
  
  // Insert intrinsic parameters
  camera.setVerticalFilmAperture( defVFA );
  camera.setFilmFit( MFnCamera:: kVerticalFilmFit );
  camera.setHorizontalFieldOfView( fov );
  
  // Apply rotation
  // TODO Simplify
  double m[4][4];
  for( int i = 0; i < 4; i++ )
  {
    for( int j = 0; j < 4; j++ )
    {
      if ( i == 0 )
        m[i][j] = cameraOpenMVG.camera._R(i, j);
      else
        m[i][j] = -cameraOpenMVG.camera._R(i, j);
    }
  }
  
  MMatrix matrixMaya( m );
  MQuaternion rotationQuaternion;
  rotationQuaternion = matrixMaya;
  fnTransformCamera.setRotation( rotationQuaternion,
                                MSpace::kTransform );
  
  // Apply transation
  openMVG::Vec3 optical_center = (-1) * cameraOpenMVG.camera._R.transpose() * cameraOpenMVG.camera._t;
  fnTransformCamera.setTranslation( MVector( optical_center( 0 ),
                                             optical_center( 1 ),
                                             optical_center( 2 ) ),
                                  MSpace::kTransform );
  
  // Lock transformation
  depNodeCamera.findPlug( "translateX" ).setLocked( true );
  depNodeCamera.findPlug( "translateY" ).setLocked( true );
  depNodeCamera.findPlug( "translateZ" ).setLocked( true );
  depNodeCamera.findPlug( "rotateX" ).setLocked( true );
  depNodeCamera.findPlug( "rotateY" ).setLocked( true );
  depNodeCamera.findPlug( "rotateZ" ).setLocked( true );
  
  
  // Create image plane
  MObject planeTransform = dagModifier.createNode("imagePlane", MObject::kNullObj, &status);
  dagModifier.doIt();
  
  MFnDagNode nodeFn( planeTransform, &status );
  MDagPath dagImagePlane;
  nodeFn.getPath( dagImagePlane );
  dagImagePlane.extendToShape( );
  MFnDependencyNode depNodePlane( dagImagePlane.node(), &status );
  
  // Change image plane name
  depNodePlane.setName( ( "imagePlane_" + cameraOpenMVG.name ).c_str(), &status );
  
  // Set parameters
  depNodePlane.findPlug( "imageName" ).setValue( cameraOpenMVG.imagePlaneName.c_str() );
  depNodePlane.findPlug( "depth" ).setValue( 50 );
  depNodePlane.findPlug( "verticalFilmAperture" ).setValue( defVFA );
  depNodePlane.findPlug( "alphaGain" ).setValue( 0.55 );
  depNodePlane.findPlug( "dic" ).setValue( 1 );
  depNodePlane.findPlug( "width" ).setValue( (int)width );
  depNodePlane.findPlug( "height" ).setValue( (int)height );
  
  // Connect image plane to camera
  MPlug plugMessage = depNodePlane.findPlug( "message" );
  MPlug plugCamera  = camera.findPlug( "imagePlane" ); 
  
  dagModifier.connect( plugMessage, plugCamera );
  dagModifier.reparentNode( planeTransform, dagPathCamera.node( ) );
  dagModifier.doIt( );
}

// slot
void MVGMenu::on_cameraImportButton_clicked() {
  ui.progressBar->setVisible(true);
  ui.cancelButton->setVisible(true);
  std::string pathViewTxt = ui.outIncrementalDir->text().toStdString() + "/SfM_output/views.txt";
  
  std::vector<readerMVG::CameraOpenMVG> vec_cameraOpenMVG;
  if ( !readerMVG::readCameras( pathViewTxt, vec_cameraOpenMVG ) )
  {
    return;
  }
    
  
  
  size_t index = 1;
  size_t nbImages = vec_cameraOpenMVG.size();
  
  for( std::vector<readerMVG::CameraOpenMVG>::const_iterator iter_cameraOpenMVG = vec_cameraOpenMVG.begin(); 
            iter_cameraOpenMVG != vec_cameraOpenMVG.end();
            iter_cameraOpenMVG++,
            index++ ) 
  {
    createMayaCamera( *iter_cameraOpenMVG );
    ui.progressBar->setValue( index / nbImages * 100 );
  }
  ui.progressBar->setVisible(false);
  ui.cancelButton->setVisible(false);
}

// QString path = QFileDialog::getExistingDirectory (this, tr("Directory"), directory.path());
// if ( path.isNull() == false )
// {
//     directory.setPath(path);
// }
/**
 * Get all points containing on the dense ply file
 * Create for each points a maya particle
 */
void MVGMenu::on_densePointCloudImportButton_clicked()
{  
  std::string sPathToPly = ui.outIncrementalDir->text().toStdString() + "/PMVS/models/pmvs_optionMiMode/new.ply";
  importPointCloud( sPathToPly, "DensePointCloudParticleShapeDense" ) ;
}


/**
 * Get all points containing on the ply file
 * Create for each points a maya particle
 */
void MVGMenu::on_pointCloudImportButton_clicked()
{
  std::string sPathToPly = ui.outIncrementalDir->text().toStdString() + "/SfM_output/clouds/calib.ply";
  importPointCloud( sPathToPly, "PointCloudParticleShape" );
}

void MVGMenu::importPointCloud( const std::string& sPathToPly,
                                const std::string& sParticleName )
{
  MString particleName = sParticleName.c_str();
  std::vector<readerMVG::Point3D> vec_point3D;
  
  if( !getPointCloudFromPly( sPathToPly, vec_point3D ) )
  {
    return;
  }
  
  // Create particle context
  MStatus status;
  MObject particleNode;
  
  MString nodeType = MString("particle");

  MFnDependencyNode fnDn;
  MObject parent = fnDn.create( nodeType, particleName, &status );
  
  MFnDagNode fnDag(parent);
  particleNode = fnDag.child(0, &status);

  MFnParticleSystem fnParticle( particleNode, &status );
  MVectorArray array_position;
  MVectorArray array_color;

  // Get values
  for( std::vector<readerMVG::Point3D>::const_iterator iter_point = vec_point3D.begin();
          iter_point != vec_point3D.end();
          iter_point++ )
  {
    array_position.append( MVector ( iter_point->position[0], iter_point->position[1], iter_point->position[2] ) );
    array_color.append( MVector ( iter_point->color[0], iter_point->color[1], iter_point->color[2] ) );
  }
  
  // Create new attribute particle
  MFnTypedAttribute fnAttr;
  fnParticle.addAttribute( fnAttr.create( MString( "rgbPP" ), MString( "rgbPP" ), MFnData::kVectorArray), MFnDependencyNode::kLocalDynamicAttr );
  // Create an attribute to save the visibility of every points
  //TODO To remove : fnParticle.addAttribute( fnAttr.create( MString( "visibility" ), MString( "visibility" ), MFnData::kIntArray ), MFnDependencyNode::kLocalDynamicAttr );
  
  // Save data
  fnParticle.setPerParticleAttribute( MString("position"), array_position, &status );
  fnParticle.setPerParticleAttribute( MString("rgbPP"), array_color, &status );
  
  status = fnParticle.saveInitialState();
}
