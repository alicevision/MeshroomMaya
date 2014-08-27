#pragma once

#include <maya/MDagPath.h>
#include <string>

namespace mayaMVG {

class MVGNodeWrapper {

	public:
		MVGNodeWrapper();
		MVGNodeWrapper(const std::string& name);
        MVGNodeWrapper(const MString& name);
		MVGNodeWrapper(const MDagPath& dagPath);
		virtual ~MVGNodeWrapper();

	public:
		virtual bool isValid() const = 0;
		void select() const;

	public:
		const MDagPath& dagPath() const;
		std::string name() const;
		void setName(const std::string&);

	protected:
		MDagPath _dagpath;
};

} // namespace
