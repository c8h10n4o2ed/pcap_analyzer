/**@file IntegerFields.h 
 */
#ifndef INTEGER_FIELDS_H_
#define INTEGER_FIELDS_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <string>
#include "BinaryField.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
/** 
 * This enum describes the different types of endianness. 
 */
typedef enum {
    ENDIAN_LITTLE=0,
    ENDIAN_BIG
} IntegerEndian_T;

/**
 * This object is a base class for all integer fields.
 */
class IntField : public BinaryField {
public:
    IntField(DataSource* pDataSource, std::string sFieldName, size_t offset, size_t size, IntegerEndian_T endian = ENDIAN_LITTLE);
    virtual ~IntField (void);

    virtual IntegerEndian_T getEndian (void);
    virtual void setEndian (IntegerEndian_T endian);

protected:
    IntegerEndian_T m_endian;
};

/**
 * This object is a wrapper for an 8-bit integer.
 */
class Int8Field : public IntField {
public:
    Int8Field(void);
    Int8Field(DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    Int8Field(DataSource* pDataSource, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    virtual ~Int8Field (void);

    uint8_t getUInt8 (void);
    int8_t getInt8 (void);
};

/**
 * This object is a wrapper for a 16-bit integer. 
 */
class Int16Field : public IntField {
public:
    Int16Field(void);
    Int16Field(DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    Int16Field(DataSource* pDataSource, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    virtual ~Int16Field (void);

    uint16_t getUInt16 (void);
    int16_t getInt16 (void);
};

/**
 * This object is a wrapper for a 32-bit integer. 
 */
class Int32Field : public IntField {
public:
    Int32Field(void);
    Int32Field(DataSource* pDataSource, std::string sFieldName, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    Int32Field(DataSource* pDataSource, size_t offset, IntegerEndian_T endian = ENDIAN_LITTLE);
    virtual ~Int32Field (void);

    uint32_t getUInt32 (void);
    int32_t getInt32 (void);
};


//=============================================================================
#endif //INTEGER_FIELDS_H_
