/**@file DataSource.h 
 */
#ifndef DATA_SOURCE_H_
#define DATA_SOURCE_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <string>
#include <vector>

//=============================================================================
// FORWARD DECLARATIONS
//=============================================================================
class BinaryField;

//=============================================================================
// DEFINITIONS
//=============================================================================
/**
 * This object is a container for a data buffer                                         .
 *                                                                                      .
 * The data buffer is allocated and destroyed when this object is allocated or destroyed. 
 * The buffer is copied from the constructor inputs into m_data/m_dataSize.     .
 */
class DataSource {
public:
    DataSource (std::string sData);
    DataSource (const void* pData, size_t dataSize);
    virtual ~DataSource (void);

    /**
     * Returns a const pointer to the data buffer.
     *  
     * @return const uint8_t* 
     */
    virtual const uint8_t* data (void) const;

    /**
     * Returns the size of the data pointer 
     *  
     * @return const size_t 
     */
    virtual const size_t size (void) const;

    /**
     * Returns a count of the number of fields in this data source.
     * 
     * @param void 
     * @return size_t Number of fields
     */
    virtual const size_t getFieldCount (void) const;

    /**
     * Gets the field associated with the index, or NULL if index is 
     * out of range. 
     * 
     * @param index The index into the field vector.
     * @return BinaryField* Field object pointer.
     */
    virtual BinaryField* getField (unsigned int index);

    /**
     * Gets the first available field associated with the field 
     * name.
     * 
     * @param sFieldName The name of the field to retrieve.
     * @return BinaryField* Field object pointer
     */
    virtual BinaryField* getFieldByName (std::string sFieldName);

    /**
     * Gets the field associated with a specific field name.
     */
    virtual BinaryField* operator[] (std::string sFieldName);

    /**
     * Gets the field associated with a specific field index.
     * 
     * @param index 
     * @return BinaryField* 
     */
    virtual BinaryField* operator[] (unsigned int index);

    /**
     * Add a field object to the m_field vector.
     * 
     * @param pField 
     */
    virtual void addField (BinaryField* pField);

protected:
    /**
     * Initializes the underlying buffer and copies initial data to 
     * the buffer.
     * 
     * @param pBuffer Source data used to populate the buffer
     * @param bufferSize Size of the source buffer data
     */
    virtual void initBuffer (const void* pBuffer, const size_t bufferSize);

    /**
     * Parses the data buffer and adds the relevant fields to this 
     * object. This action should only occur once per 
     * initialization.
     *  
     * This routine adds one or more fields to the m_fields vector. 
     */
    virtual void parseBuffer (void);
    
protected:
    size_t m_dataSize;
    uint8_t* m_pData;
    std::vector<BinaryField*> m_fields;
};


//=============================================================================
#endif //DATA_SOURCE_H_
