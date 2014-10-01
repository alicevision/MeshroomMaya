#pragma once

#include "mayaMVG/qt/MVGQt.hpp"

namespace mayaMVG
{

template <class T>
class MVGEventFilter : public QObject
{
public:
    MVGEventFilter(QObject* window, T* userObj);
    ~MVGEventFilter();

protected:
    bool eventFilter(QObject* obj, QEvent* e);

protected:
    QObject* _window;
    T* _userObj;
};

} // namespace

template <class T>
mayaMVG::MVGEventFilter<T>::MVGEventFilter(QObject* window, T* userObj)
    : QObject(NULL) // no parent
    , _window(window)
    , _userObj(userObj)
{
    if(_window)
        _window->installEventFilter(this);
}

template <class T>
mayaMVG::MVGEventFilter<T>::~MVGEventFilter()
{
}

template <class T>
bool mayaMVG::MVGEventFilter<T>::eventFilter(QObject* obj, QEvent* e)
{
    if((e->type() == QEvent::Close))
    {
        if(_window)
        {
            _window->removeEventFilter(this);
        }
        return false;
    }
    if(_userObj)
        return _userObj->eventFilter(obj, e);
    return QObject::eventFilter(obj, e);
}
