/**@file MD5ByteContainer.h 
 */
#ifndef MD5_BYTE_CONTAINER_H_
#define MD5_BYTE_CONTAINER_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include "UnsignedByteContainer.h"
#include <stdint.h>
#include <openssl/md5.h>

//=============================================================================
// DEFINITIONS
//=============================================================================

class MD5ByteContainer : public UnsignedByteContainer {
public:
    MD5ByteContainer (void);
    MD5ByteContainer (const MD5ByteContainer& rhs);
    MD5ByteContainer (uint8_t* pData, size_t dataSize);
    virtual ~MD5ByteContainer (void);

    /**
     * Sets the data to be hashed. The result of the hash is stored 
     * in this byte container. 
     * 
     * @param pData The data to be hashed
     * @param dataSize Size of the data to be hashed
     */
    virtual void setHashData (uint8_t* pData, size_t dataSize);

    virtual MD5ByteContainer& operator= (const MD5ByteContainer& rhs);
    virtual bool operator== (const MD5ByteContainer& rhs) const;
    virtual bool operator!= (const MD5ByteContainer& rhs) const;
    virtual bool operator> (const MD5ByteContainer& rhs) const;
    virtual bool operator< (const MD5ByteContainer& rhs) const;

protected:
    /**
     * Initialize the container.
     * 
     * @param void 
     */
    virtual void init (void);

    /**
     * Update the hash with more data.
     * 
     * @param pData 
     * @param dataSize 
     */
    virtual void update (uint8_t* pData, size_t dataSize);

    /**
     * Finalize the hash
     * 
     * @param void 
     */
    virtual void finalize (void);

protected:
    MD5_CTX m_md5Context;
};


//=============================================================================
#endif //MD5_BYTE_CONTAINER_H_
