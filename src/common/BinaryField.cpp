/**@file BinaryField.h 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdio.h>
#include <string.h>
#include "BinaryField.h"
#include "DataSource.h"

//=============================================================================
// DEFINITIONS
//=============================================================================

BinaryField::BinaryField (DataSource* pDataSource, std::string sName, size_t offset, size_t size) :
    m_pParent(NULL),
    m_offset(offset),
    m_size(size),
    NamedObject(sName)
{
    setDataSource(pDataSource);
    if (m_pParent) {
        m_pParent->addField(this);
    }
}

BinaryField::BinaryField (DataSource* pDataSource, size_t offset, size_t size) :
    m_pParent(NULL),
    m_offset(offset),
    m_size(size),
    NamedObject("UnnamedField")
{
    setDataSource(pDataSource);
    if (m_pParent) {
        m_pParent->addField(this);
    }
}

void BinaryField::setDataSource (DataSource* pDataSource) {
    m_pParent = pDataSource;
}

DataSource* BinaryField::parent (void) {
    return m_pParent;
}

const size_t BinaryField::offset (void) const {
    return (const size_t)m_offset;
}

const size_t BinaryField::size (void) const {
    return (const size_t)m_size;
}

std::string BinaryField::getStringRepr (void) {
    char tempBuffer[128];
    snprintf(tempBuffer, sizeof(tempBuffer), "(offset=%lu,size=%lu)", offset(), size());
    std::string sRetValue = std::string(getName()) + tempBuffer;
    return sRetValue;
}

//=============================================================================

