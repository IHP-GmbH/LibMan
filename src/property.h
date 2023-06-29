#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>
#include <QVariant>

#include <QMap>
#include <QList>
#include <QString>
#include <QVariant>
#include <QMessageBox>

/*!*********************************************************************************************************************
 * \brief The PropertyItem class creates a single object of any type used by the Properties class.
 **********************************************************************************************************************/
class PropertyItem {
public:
    PropertyItem(const QVariant, bool modyfied = false);
    ~PropertyItem();

    QList<QVariant>             getValue() const;
    bool                        isModified() const;

    void                        addValue(const QVariant);
private:
    QList<QVariant>             m_value;   /*!< List of property values to store.*/
    bool                        m_status;  /*!< State if item has been modified or not.*/
};

/*!*********************************************************************************************************************
 * \brief Returns item's property value.
 **********************************************************************************************************************/
inline QList<QVariant> PropertyItem::getValue() const
{
    return m_value;
}

/*!*********************************************************************************************************************
 * \brief Returns the state if property has been modified or not.
  **********************************************************************************************************************/
inline bool PropertyItem::isModified() const
{
    return m_status;
}

/*!*********************************************************************************************************************
 * \brief PropertyItem::addValue
 * \param value
 **********************************************************************************************************************/
inline void PropertyItem::addValue(const QVariant value)
{
    m_value.push_back(value);
}

/*!*********************************************************************************************************************
 * \brief The Properties class stores and manipulates all kind of data types. It is used for managing LibMan settings.
 **********************************************************************************************************************/
class Properties {
public:
    Properties();
    ~Properties();

    unsigned                    count() const;
    unsigned                    getValueCount(const QString &name) const;

    void                        remove(const QString &name);

    bool                        exists(const QString &name) const;
    bool                        isModified(const QString &name) const;

    QVariant                    getVariant(const QString &name) const;

    template <class T> T        get(const QString &name) const;
    template <class T> QList<T> getList(const QString &name) const;

    template <class T> void     set(const QString &name, const T &value);
    template <class T> void     add(const QString &name, const T &value);

    template <class T> void     setDefault(const QString &name, const T &value);

    const QMap<QString,
               PropertyItem*>   getMap() const;

private:
    QMap<QString, PropertyItem*>           m_data;
};

/*!*********************************************************************************************************************
 * \brief Returns collection's map of property items.
 **********************************************************************************************************************/
inline const QMap<QString, PropertyItem*> Properties::getMap() const
{
    return(m_data);
}

/*!*********************************************************************************************************************
 * \brief Returns number of stored properties.
 **********************************************************************************************************************/
inline unsigned Properties::count() const
{
    return m_data.count();
}

/*!*********************************************************************************************************************
 * \brief Removes property item by name.
 * \param name       Name of the property to be removed.
 **********************************************************************************************************************/
inline void Properties::remove(const QString &name)
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it != m_data.end()) {
        PropertyItem *item = it.value();
        if(item) {
            delete item;
        }

        m_data.remove(name);
    }
}

/*!*********************************************************************************************************************
 * \brief Assigns a new value to property item
 * \param name       Name of the property item.
 * \param name       Value of the property item. Property can be any of allowed type.
 **********************************************************************************************************************/
template <class T> inline void Properties::set(const QString &name, const T &value)
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it != m_data.end()) {
        PropertyItem *item = it.value();
        if(item) {
            delete item;
        }
    }

    m_data[name] = new PropertyItem(QVariant(value), true);
}

/*!*********************************************************************************************************************
 * \brief Sets the default value of property item.
 * \param name       Name of the property item.
 * \param value      Value of the property item. Property can be any of allowed type.
 **********************************************************************************************************************/
template <class T> inline void Properties::setDefault(const QString &name, const T &value)
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it != m_data.end()) {
        PropertyItem *item = it.value();
        if(item) {
            delete item;
        }
    }

    m_data[name] = new PropertyItem(QVariant(value));
}

/*!*********************************************************************************************************************
 * \brief Removes property item by name.
 * \param name       Name of the property item to be removed.
 **********************************************************************************************************************/
inline unsigned Properties::getValueCount(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "RdeProperty Critical" ), msg );
        return 0;
    }

    return( it.value()->getValue().count());
}

/*!*********************************************************************************************************************
 * \brief Returns property item by name.
 * \param name       Name of the property item.
 **********************************************************************************************************************/
template <class T> inline T Properties::get(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "RdeProperty Critical" ), msg );
        return 0;
    }

#if QT_VERSION >= 0x050000
    return( it.value()->getValue()[0].value<T>() );
#else
    return( qVariantValue<T>(it.value()->getValue()[0]) );
#endif

    // to avaoid warnings
    return 0;
}

/*!*********************************************************************************************************************
 * \brief Return list of the property items by property name.
 * \param name       Name of the property item.
 **********************************************************************************************************************/
template <class T> inline QList<T> Properties::getList(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    QList<T> list;
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "RdeProperty Critical" ), msg );
        return list;
    }

    QList<QVariant> values = it.value()->getValue();
    for(unsigned i = 0; i < values.count(); ++i) {
#if QT_VERSION >= 0x050000
        list.push_back(it.value()->getValue()[i].value<T>());
#else
        list.push_back(qVariantValue<T>(it.value()->getValue()[i]));
#endif
    }

    return( list );
}

/*!*********************************************************************************************************************
 * \brief Adds property value to collection.
 * \param name       Name of the property item.
 * \param value      Value of the property item. Property can be any of allowed type.
 **********************************************************************************************************************/
template <class T> inline void Properties::add(const QString &name, const T &value)
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "RdeProperty Critical" ), msg );
        return;
    }

    it.value()->addValue(QVariant(value));
}

#endif // PROPERTY_H
