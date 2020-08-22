/**@file UnsignedByteContainer.h 
 */
#ifndef UNSIGNED_BYTE_CONTAINER_H_
#define UNSIGNED_BYTE_CONTAINER_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <string>

//=============================================================================
// DEFINITIONS
//=============================================================================

class UnsignedByteContainer {
public:
    UnsignedByteContainer (void);
    UnsignedByteContainer (const UnsignedByteContainer& rhs);
    virtual ~UnsignedByteContainer (void);

    /**
     * Sets the data buffer to a specific value. 
     *  
     * This byte buffer is processed as big-endian. 
     * 
     * @param pData 
     * @param dataSize 
     */
    virtual void setData (
        const uint8_t* pData,
        size_t dataSize
    );

    /**
     * Returns a pointer to the byte container buffer.
     * 
     * @return const uint8_t* 
     */
    virtual const uint8_t* data (void) const;

    /**
     * Returns the size of the byte container in bytes.
     * 
     * @return const size_t 
     */
    virtual const size_t size (void) const;

    /**
     * Compares two byte containers. Returns 0 if they are 
     * identical. Otherwise return -1 or 1 depending on if the 
     * left-hand-side or right-hand-size is larger. 
     * 
     * @param pContainer Right hand side object.
     * @return int Return value
     */
    virtual int compareTo (const UnsignedByteContainer* pContainer) const;

    /**
     * Converts the buffer into a hex string that is returned by 
     * this method. 
     * 
     * @return std::string Hex string
     */
    const std::string toHexString (void) const;

    //=========================================================================
    // OPERATORS
    //=========================================================================
    virtual UnsignedByteContainer& operator= (const UnsignedByteContainer& rhs);
    virtual bool operator== (const UnsignedByteContainer& rhs) const;
    virtual bool operator!= (const UnsignedByteContainer& rhs) const;
    virtual bool operator> (const UnsignedByteContainer& rhs) const;
    virtual bool operator< (const UnsignedByteContainer& rhs) const;

protected:
    virtual void freeBuffer (void);
    virtual bool isPrefixNull (size_t index, size_t count);

    uint8_t* m_pData;
    size_t m_dataSize;
};

//=============================================================================
#endif //UNSIGNED_BYTE_CONTAINER_H_
