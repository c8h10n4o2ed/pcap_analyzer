/**@file DataSource.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdlib.h>
#include <string.h>
#include "DataSource.h"
#include "BinaryField.h"

//=============================================================================
// IMPLEMENTATION
//=============================================================================

DataSource::DataSource (std::string sData) :
    m_fields()
{
    initBuffer(sData.c_str(), sData.length());
    parseBuffer();
}

DataSource::DataSource (const void* pData, size_t dataSize) :
    m_fields()
{
    initBuffer(pData, dataSize);
    parseBuffer();
}

DataSource::~DataSource (void) {
    //DEBUG: This is currently disabled because the destructor automatically
    //       frees member variables that otherwise register themselves as fields.
    //       Leaving this code enabled would result in a double free of each field.
#if 0
    //Delete all nested fields
    std::vector<BinaryField*>::iterator iter;
    iter = m_fields.begin();
    while (iter != m_fields.end()) {
        delete *iter;
        iter++;
    }
#endif
    m_fields.clear();
    
    //Delete the data buffer
    if (m_pData) {
        delete m_pData;
    }
}

void DataSource::initBuffer (const void* pBuffer, const size_t bufferSize) {
    m_dataSize = bufferSize;
    //m_data = (uint8_t*)malloc(m_dataSize);
    m_pData = new uint8_t [bufferSize];
    memcpy(m_pData, pBuffer, m_dataSize);
}

void DataSource::parseBuffer (void) {
    //@note This method is supposed to be overridden by a child class
}

const uint8_t* DataSource::data (void) const {
    return (const uint8_t*)m_pData;
}

const size_t DataSource::size (void) const {
    return (const size_t)m_dataSize;
}

const size_t DataSource::getFieldCount (void) const {
    return m_fields.size();
}

BinaryField* DataSource::getField (unsigned int index) {
    BinaryField* pRetValue = NULL;
    if (index < getFieldCount()) {
        pRetValue = m_fields[index];
    }
    return pRetValue;
}

BinaryField* DataSource::getFieldByName (std::string sFieldName) {
    BinaryField* pRetValue = NULL;
    std::vector<BinaryField*>::iterator iter;

    iter = m_fields.begin();
    while (iter != m_fields.end()) {
        BinaryField* pField = *iter;
        if (pField->getName() == sFieldName) {
            pRetValue = pField;
            break;
        }
        iter++;
    }

Exit:
    return pRetValue;
}

void DataSource::addField (BinaryField* pField) {
    //Check to make sure the field isnt already part of the m_field vector
    //If its not, then add to the vector.
    std::vector<BinaryField*>::iterator iter;
    bool isFound = false;

    iter = m_fields.begin();
    while (iter != m_fields.end()) {
        if (pField == *iter) {
            isFound = true;
            break;
        }
        iter++;
    }

    if (!isFound) {
        m_fields.push_back(pField);
    }
}

BinaryField* DataSource::operator[] (std::string sFieldName) {
    return getFieldByName(sFieldName);
}

BinaryField* DataSource::operator[] (unsigned int index) {
    return getField(index);
}
//=============================================================================
