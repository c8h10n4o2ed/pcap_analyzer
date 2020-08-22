/**@file IntegerFields.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "IntegerFields.h"
#include "DataSource.h"

#ifndef _DARWIN
#include <endian.h>
#else
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BYTE_ORDER    BYTE_ORDER
#endif

//=============================================================================
// IMPLEMENTATION
//=============================================================================
//=============================================================================
// IntField
//=============================================================================
IntField::IntField (DataSource* pDataSource, std::string sFieldName, size_t offset, size_t size, IntegerEndian_T endian) :
    BinaryField(pDataSource, sFieldName, offset, size),
    m_endian(endian)
{
}

IntField::~IntField (void) {
}

IntegerEndian_T IntField::getEndian (void) {
    return m_endian;
}

void IntField::setEndian (IntegerEndian_T endian) {
    m_endian = endian;
}

//=============================================================================
// Int8Field
//=============================================================================
Int8Field::Int8Field (void) :
    IntField(NULL, "Int8Field", 0, sizeof(uint8_t), ENDIAN_LITTLE)
{
}

Int8Field::Int8Field (DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, sFieldName, offset, sizeof(uint8_t), endian)
{
}

Int8Field::Int8Field (DataSource* pDataSource, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, "Int8Field", offset, sizeof(uint8_t), endian)
{
}

Int8Field::~Int8Field (void) {

}

uint8_t Int8Field::getUInt8 (void) {
    return (uint8_t)parent()->data()[m_offset];
}

int8_t Int8Field::getInt8 (void) {
    return (int8_t)parent()->data()[m_offset];
}

//=============================================================================
// Int16Field
//=============================================================================
Int16Field::Int16Field (void) :
    IntField(NULL, "Int16Field", 0, sizeof(uint16_t), ENDIAN_LITTLE)
{
}

Int16Field::Int16Field (DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, sFieldName, offset, sizeof(uint16_t), endian)
{
}

Int16Field::Int16Field (DataSource* pDataSource, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, "Int16Field", offset, sizeof(uint16_t), endian)
{
}

Int16Field::~Int16Field (void) {

}

uint16_t Int16Field::getUInt16 (void) {
    uint16_t *p = (uint16_t*)&parent()->data()[m_offset];
    if (m_endian == ENDIAN_BIG) {
        return be16toh(*p);
    } else {
        return *p;
    }
}

int16_t Int16Field::getInt16 (void) {
    int16_t *p = (int16_t*)&parent()->data()[m_offset];
    if (m_endian == ENDIAN_BIG) {
        return (int16_t)be16toh(*p);
    } else {
        return *p;
    }
}

//=============================================================================
// Int32Field
//=============================================================================
Int32Field::Int32Field (void) :
    IntField(NULL, "Int32Field", 0, sizeof(uint32_t), ENDIAN_LITTLE)
{
}

Int32Field::Int32Field (DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, sFieldName, offset, sizeof(uint32_t), endian)
{
}

Int32Field::Int32Field (DataSource* pDataSource, size_t offset, IntegerEndian_T endian) :
    IntField(pDataSource, "Int32Field", offset, sizeof(uint32_t), endian)
{
}

Int32Field::~Int32Field (void) {

}

uint32_t Int32Field::getUInt32 (void) {
    uint32_t *p = (uint32_t*)&parent()->data()[m_offset];
    if (m_endian == ENDIAN_BIG) {
        return be32toh(*p);
    } else {
        return *p;
    }
}

int32_t Int32Field::getInt32 (void) {
    int32_t *p = (int32_t*)&parent()->data()[m_offset];
    if (m_endian == ENDIAN_BIG) {
        return (int16_t)be32toh(*p);
    } else {
        return *p;
    }
}

//=============================================================================
