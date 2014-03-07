#pragma once

#include <maya/MPxManipContainer.h>
#include <maya/MDagPath.h>

namespace mayaMVG {

// forward declaration
class MVGContext;

class MVGManipContainer: public MPxManipContainer
{
public:
	MVGManipContainer();
	virtual ~MVGManipContainer();
	
public:
	static void * creator();
	static MStatus initialize();
	virtual MStatus createChildren();
	virtual void draw(M3dView & view, const MDagPath & path,
	                  M3dView::DisplayStyle style, M3dView::DisplayStatus status);
public:
	void setContext(MVGContext * ctx);

public:
	static MTypeId id;
	static MString drawDbClassification;
	static MString drawRegistrantId;
	MVGContext* m_ctx;
};

} // mayaMVG
