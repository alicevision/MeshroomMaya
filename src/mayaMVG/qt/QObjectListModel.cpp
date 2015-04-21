#include "QObjectListModel.hpp"
#include <QtDeclarative/QDeclarativeEngine>

/*!
    \class QObjectListModel
    \brief The QObjectListModel class provides a model that supplies objects to
   QML views.

    QObjectListModel provides a more powerful, but still easy to use,
   alternative to using
    QObjectList lists as models for QML views. As a QAbstractListModel, it has
   the ability to
    automatically notify the view of specific changes to the list, such as
   adding or removing
    items. At the same time it provides QList-like convenience functions such as
   append, at,
    and removeAt for easily working with the model from C++.

    \code
    QObjectListModel model;
    model.setObjectList(myQList);
    model.append(myNewObject);
    ...
    int pos = model.indexOf(myObject);
    model.insert(pos, myOtherNewObject);
    ...
    model.removeAt(0);
    \endcode

    QObjectListModel exposes a single \c object role to QML,
    as well as a \c count property, and a \c get(int i) function.

    \qml
    ListView {
        ...
        delegate: Text { text: object.someProperty }
    }
    \endqml
*/

/*!
    Constructs an object list model with the given \a parent.
*/
QObjectListModel::QObjectListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _autoEmit(true)
{
    QHash<int, QByteArray> roles;
    roles[ObjectRole] = "object";
    setRoleNames(roles);
}

/*!
    Constructs an object list model containing the specified \a objects
    with the given \a parent.
*/
QObjectListModel::QObjectListModel(QObjectList objects, QObject* parent)
    : QAbstractListModel(parent)
    , _autoEmit(true)
    , _objects(objects)
{
    QHash<int, QByteArray> roles;
    roles[ObjectRole] = "object";
    setRoleNames(roles);
}

/*!
    Returns data for the specified \a role, from the item with the
    given \a index. The only valid role is \c ObjectRole.

    If the view requests an invalid index or role, an invalid variant
    is returned.
*/
QVariant QObjectListModel::data(const QModelIndex& index, int role) const
{
    if(index.row() < 0 || index.row() >= _objects.size())
        return QVariant();

    if(role == ObjectRole)
        return QVariant::fromValue(_objects.at(index.row()));

    return QVariant();
}

/*!
    Returns the number of rows in the model. This value corresponds to the
    number of items in the model's internal object list.
*/
int QObjectListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return count();
}

/*!
    Returns the object list used by the model to store data.
*/
QObjectList QObjectListModel::objectList() const
{
    return _objects;
}

/*!
    Sets the model's internal objects list to \a objects. The model will
    notify any attached views that its underlying data has changed.
*/
void QObjectListModel::setObjectList(QObjectList objects)
{
    int oldCount = _objects.count();
    beginResetModel();
    _objects = objects;
    endResetModel();
    Q_EMIT dataChanged(index(0), index(_objects.count()));
    if(_objects.count() != oldCount)
        internEmitCountChanged();
}

/*!
    Inserts \a object at the end of the model and notifies any views.

    This is the same as model.insert(size(), \a object).

    \sa insert()
*/
void QObjectListModel::append(QObject* object)
{
    beginInsertRows(QModelIndex(), _objects.count(), _objects.count());
    _objects.append(object);
    endInsertRows();
    internEmitCountChanged();
}

/*!
    \overload
    Appends the items of the \a objects list to this model and notifies any
   views.
*/
void QObjectListModel::append(const QObjectList& objects)
{
    beginInsertRows(QModelIndex(), _objects.count(), _objects.count() + objects.count() - 1);
    _objects.append(objects);
    endInsertRows();
    internEmitCountChanged();
}

/*!
    Inserts \a object at index position \a i in the model and notifies
    any views. If \a i is 0, the object is prepended to the model. If \a i
    is size(), the object is appended to the list.

    \sa append(), replace(), removeAt()
*/
void QObjectListModel::insert(int i, QObject* object)
{
    beginInsertRows(QModelIndex(), i, i);
    _objects.insert(i, object);
    endInsertRows();
    internEmitCountChanged();
}

/*!
    \overload
    Inserts the items of the \a objects list at index position \a i in the model
    and notifies any views. If \a i is 0, the items are prepended to the model.
   If \a i
    is size(), the items are appended to the list.
*/
void QObjectListModel::insert(int i, const QObjectList& objects)
{
    if(objects.isEmpty())
        return;

    beginInsertRows(QModelIndex(), i, i + objects.count() - 1);
    for(int j = objects.count() - 1; j > -1; --j)
        _objects.insert(i, objects.at(j));
    endInsertRows();
    internEmitCountChanged();
}

/*!
    Replaces the item at index position \a i with \a object and
    notifies any views. \a i must be a valid index position in the list
    (i.e., 0 <= \a i < size()).

    \sa removeAt()
*/
void QObjectListModel::replace(int i, QObject* object)
{
    _objects.replace(i, object);
    Q_EMIT dataChanged(index(i), index(i));
}

/*!
    Moves the item at index position \a from to index position \a to
    and notifies any views.

    This function
    assumes that both \a from and \a to are at least 0 but less than
    size(). To avoid failure, test that both \a from and \a to are at
    least 0 and less than size().
*/

void QObjectListModel::move(int from, int to)
{
    if(!beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to))
        return; // should only be triggered for our simple case if from == to.
    _objects.move(from, to);
    endMoveRows();
}

/*!
    Removes \a count number of items from index position \a i and notifies any
   views.
    \a i must be a valid index position in the model (i.e., 0 <= \a i < size()),
   as
    must \c{i + count - 1}.

    \sa takeAt()
*/
void QObjectListModel::removeAt(int i, int count)
{
    beginRemoveRows(QModelIndex(), i, i + count - 1);
    for(int j = 0; j < count; ++j)
        _objects.removeAt(i);
    endRemoveRows();
    internEmitCountChanged();
}

/*!
    Removes the item at index position \a i (notifying any views) and returns
   it.
    \a i must be a valid index position in the model (i.e., 0 <= \a i < size()).

    \sa removeAt()
*/
QObject* QObjectListModel::takeAt(int i)
{
    beginRemoveRows(QModelIndex(), i, i);
    QObject* obj = _objects.takeAt(i);
    endRemoveRows();
    internEmitCountChanged();
    return obj; // INVOKABLE => by default tranfer ownership to qml (QQmlEngine::JavaScriptOwnership)
}

/*!
    Removes all items from the model and notifies any views.
*/
void QObjectListModel::clear()
{
    if(_objects.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, _objects.count() - 1);
    _objects.clear();
    endRemoveRows();
    internEmitCountChanged();
}

/*!
    \internal
    For usage from QML.
*/
QObject* QObjectListModel::get(int i) const
{
    QObject* obj = _objects.at(i);
    // Explicitly keep the ownership which is not the default behavior in INVOKABLE methods.
    QDeclarativeEngine::setObjectOwnership(obj, QDeclarativeEngine::CppOwnership);
    return obj;
}

void QObjectListModel::emitModified()
{
    Q_EMIT countChanged();
}

void QObjectListModel::internEmitCountChanged()
{
    if(_autoEmit)
        Q_EMIT countChanged();
}
