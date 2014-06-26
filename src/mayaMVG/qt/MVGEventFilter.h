#pragma once

#include "mayaMVG/qt/MVGQt.h"
#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

template<class T>
class MVGEventFilter: public QObject {
	
	public:
		MVGEventFilter(QObject* window, T* userObj, void* userData);
		~MVGEventFilter();
		
	protected:
		bool eventFilter(QObject* obj, QEvent* e);

	protected:
		QObject* _window;
		T* _userObj;
		void* _userData;
};

} // namespace

template<class T>
mayaMVG::MVGEventFilter<T>::MVGEventFilter(QObject* window, T* userObj, void* userData)
	: QObject(NULL) // no parent
	, _window(window)
	, _userObj(userObj)
	, _userData(userData)
{
	if(_window)
		_window->installEventFilter(this);
}

template<class T>
mayaMVG::MVGEventFilter<T>::~MVGEventFilter()
{
}

template<class T>
bool mayaMVG::MVGEventFilter<T>::eventFilter(QObject* obj, QEvent* e)
{
	if((e->type() == QEvent::Close)) {
		if(_window) {
			_window->removeEventFilter(this);
		}
		return false;
	}
	if(_userObj)
		return _userObj->eventFilter(obj, e, _userData);
	return QObject::eventFilter(obj, e);
}
