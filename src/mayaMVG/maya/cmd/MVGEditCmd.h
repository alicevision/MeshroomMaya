#pragma once

#include <maya/MPxToolCommand.h>
#include <maya/MDagPath.h>
#include <maya/MPointArray.h>

namespace mayaMVG {

class MVGEditCmd : public MPxToolCommand {

	enum CMD_FLAG {
		CMD_CREATE = 1 << 0,
		CMD_MOVE = 1 << 1,
		CMD_DELETE = 1 << 2
	};
	public:
		MVGEditCmd();
		virtual ~MVGEditCmd();
	public:
		static void* creator();
		virtual MStatus doIt(const MArgList& args);
		static MSyntax newSyntax();
	public:
		MStatus redoIt();
		MStatus undoIt();
		bool isUndoable() const;
		MStatus finalize();
	public:
		void doAddPolygon(const MDagPath& meshPath, const MPointArray& points);
		void doMove(const MDagPath& meshPath, const MPointArray& points, const MIntArray& verticesIndexes);
		void doDelete();
	public:
		static MString name;
	private:
		size_t _flags;
		MDagPath _meshPath;
		MPointArray _points;
		MPointArray _oldPoints;
		MIntArray _indexes;
};

} // namespace
