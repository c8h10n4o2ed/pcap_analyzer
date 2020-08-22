/**@file BinaryField.h 
 */
#ifndef BINARY_FIELD_H_
#define BINARY_FIELD_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include "NamedObject.h"
#include "PrintableObject.h"

//=============================================================================
// FORWARD DECLARATIONS
//=============================================================================
class DataSource;

//=============================================================================
// DEFINITIONS
//=============================================================================
class BinaryField : public NamedObject, public PrintableObject
{
public:
    BinaryField (DataSource* pDataSource, size_t offset, size_t size);
    BinaryField (DataSource* pDataSource, std::string sName, size_t offset, size_t size);
    
    /**
     * Sets the data source associated with this binary field.
     * 
     * @param pDataSource 
     */
    void setDataSource (DataSource* pDataSource);

    const size_t offset (void) const;
    const size_t size (void) const;
    DataSource* parent (void);

    /**
     * Get the string representation of this object.
     * 
     * @param void 
     * @return std::string 
     */
    std::string getStringRepr (void);  

protected:
    DataSource* m_pParent;
    size_t m_offset;
    size_t m_size;
};


//=============================================================================
#endif //BINARY_FIELD_H_
