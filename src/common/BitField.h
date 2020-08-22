/**@file BitField.h 
 */
#ifndef BIT_FIELD_H_
#define BIT_FIELD_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include "BinaryField.h"
#include <vector>
#include <string>

//=============================================================================
// DEFINITIONS
//=============================================================================
class BitSubField {
public:
    BitSubField (std::string sSubFieldName, unsigned int shift, unsigned int mask)
    {

    }
};


/**
 * This is a base class for more specific bit fields that are configured within child objects. 
 *  
 * Child objects register bit sub-fields that are accessed via method in this class. 
 */
class BitField : public BinaryField {
public:
    BitField (DataSource* pDataSource, std::string sFieldName, size_t offset, size_t size);

protected:
    /**
     * Adds a sub-field to this bit field.
     * 
     * @param sSubFieldName 
     * @param shift 
     * @param mask 
     */
    bool addSubField (std::string sSubFieldName, unsigned int shift, unsigned int mask);

protected:
    std::vector<BitSubField*> m_subFields;
};


//=============================================================================
#endif //BIT_FIELD_H_

