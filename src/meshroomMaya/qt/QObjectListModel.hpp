#pragma once

#include <QAbstractListModel>

/*
    Open issues:
        object ownership: is it helpful for the model to own the objects?
                          can we guard the objects so they are automatically
   removed
                              from the model when deleted?
        add additional QList convenience functions (operator<<, etc.)
*/

class QObjectListModel : public QAbstractListModel
{

    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit QObjectListModel(QObject* parent = 0);
    QObjectListModel(QObjectList objects, QObject* parent = 0);

    // model API
    enum Roles
    {
        ObjectRole = Qt::UserRole + 1
    };

    QHash<int, QByteArray> roleNames() const override {
        return _roles;
    }

    QObjectList::iterator begin() { return _objects.begin(); }
    QObjectList::const_iterator begin() const { return _objects.begin(); }
    QObjectList::iterator end() { return _objects.end(); }
    QObjectList::const_iterator end() const { return _objects.end(); }

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QObjectList objectList() const;
    void setObjectList(QObjectList objects);
    void append(QObject* object);
    void append(const QObjectList& objects);
    void insert(int i, QObject* object);
    void insert(int i, const QObjectList& objects);
    inline QObject* at(int i) const { return _objects.at(i); }
    inline QObject* operator[](int i) const { return _objects[i]; }
    void replace(int i, QObject* object);
    void move(int from, int to);
    void removeAt(int i, int count = 1);
    QObject* takeAt(int i);
    void clear();
    inline bool contains(QObject* object) const { return _objects.contains(object); }

    inline int indexOf(QObject* object, int from = 0) const
    {
        return _objects.indexOf(object, from);
    }

    inline int lastIndexOf(QObject* object, int from = -1) const
    {
        return _objects.lastIndexOf(object, from);
    }

    void remove(QObject* object) { removeAt(_objects.indexOf(object)); }

    inline int count() const { return _objects.count(); }
    inline int size() const { return _objects.size(); }
    inline bool isEmpty() const { return _objects.isEmpty(); }

    // additional QML API
    Q_INVOKABLE QObject* get(int i) const;
    inline void setAutoEmit(bool e) { _autoEmit = e; }
    void emitModified();

    template <class T>
    const QList<T*>& asQList() const
    {
        return reinterpret_cast<const QList<T*>&>(_objects);
    }

    template <class T>
    QList<T*>& asQList()
    {
        return reinterpret_cast<QList<T*>&>(_objects);
    }

Q_SIGNALS:
    void countChanged();

private:
    void referenceItem(QObject* obj);
    void dereferenceItem(QObject* obj);
    void internEmitCountChanged();

private:
    bool _autoEmit;
    Q_DISABLE_COPY(QObjectListModel)
    QList<QObject*> _objects;
    QHash<int, QByteArray> _roles;
};
