#include "property.h"

/*!*********************************************************************************************************************
 * \brief Constructs PropertyItem object.
 * \param value       Name of the property item.
 * \param modyfied    Flag value if item has been modified. Allows setting it to false by adding new value.
 **********************************************************************************************************************/
PropertyItem::PropertyItem(QVariant value, bool modyfied)
    : m_status(modyfied)
{
    m_value.push_back(value);
}

/*!*********************************************************************************************************************
 * \brief Destructs PropertyItem obejct and clears up the property items values.
 **********************************************************************************************************************/
PropertyItem::~PropertyItem()
{
    m_value.clear();
}

/*!*********************************************************************************************************************
 * \brief Constructs Properties object.
 **********************************************************************************************************************/
Properties::Properties()
{
}

/*!*********************************************************************************************************************
 * \brief Destructs Properties obejct and clears up the property items.
 **********************************************************************************************************************/
Properties::~Properties()
{
    QMap<QString, PropertyItem*>::const_iterator it;
    for( it = m_data.begin(); it != m_data.end(); it++ ) {
        if(it.value()) {
            delete it.value();
        }
    }
}

/*!*********************************************************************************************************************
 * \brief Returns propery variant's value.
 * \param name       Name of the property item.
 **********************************************************************************************************************/
QVariant Properties::getVariant(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "Property Critical" ), msg );
        return false;
    }

    return(it.value()->getValue()[0]);
}

/*!*********************************************************************************************************************
 * \brief Cehcks if item has been modified.
 * \param name       Name of the property item.
 **********************************************************************************************************************/
bool Properties::isModified(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        QString msg = "Failed to find value '" + name + "'";
        QMessageBox::critical(0, QString( "Property Critical" ), msg );
        return false;
    }

    return( it.value()->isModified() );
}

/*!*********************************************************************************************************************
 * \brief Checks if property item is part of the collection.
 * \param name       Name of the property item.
 **********************************************************************************************************************/
bool Properties::exists(const QString &name) const
{
    QMap<QString, PropertyItem*>::const_iterator it = m_data.find(name);
    if(it == m_data.end()) {
        return false;
    }
    return( true );
}

