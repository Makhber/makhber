#ifndef MAKHBEROBJECT_H
#define MAKHBEROBJECT_H

#include <QObject>
#include <QWidget>

/**
   Generic base class for Qt based classes in Makhber
**/
template<class Base>
class MakhberObject : public Base
{
    void m_setParent(QObject *child, QObject *parent) { child->setParent(parent); }
    // overload that preserves the default window flags
    void m_setParent(QWidget *child, QWidget *parent)
    {
        child->setParent(parent, child->windowFlags());
    }

public:
    template<class... A>
    explicit MakhberObject(A... args) : Base(std::forward<A>(args)...)
    {
    }

    /// generic factory function to add a child object, to avoid bare pointers
    template<class T, class... A>
    T &addChild(A... args)
    {
        T *child = new T(std::forward<A>(args)...);
        m_setParent(child, this);
        return *child;
    }
};

#endif
