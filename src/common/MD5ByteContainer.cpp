/**@file MD5ByteContainer.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "MD5ByteContainer.h"

//=============================================================================
// DEFINITIONS
//=============================================================================

MD5ByteContainer::MD5ByteContainer (void) :
    UnsignedByteContainer()
{
}

MD5ByteContainer::MD5ByteContainer (const MD5ByteContainer& rhs) :
    UnsignedByteContainer()
{
    setData(rhs.data(), rhs.size());
}

MD5ByteContainer::MD5ByteContainer (uint8_t* pData, size_t dataSize) :
    UnsignedByteContainer()
{
    setHashData(pData, dataSize);
}

MD5ByteContainer::~MD5ByteContainer (void) {    
}

void MD5ByteContainer::setHashData (
    uint8_t* pData,
    size_t dataSize
) {
    if (!m_pData) {    
        //Allocate buffer for hashed data
        m_pData = new uint8_t [MD5_DIGEST_LENGTH];
        m_dataSize = MD5_DIGEST_LENGTH;
    }

    init();
    update(pData, dataSize);
    finalize();
}

void MD5ByteContainer::init (void) {
    MD5_Init(&m_md5Context);
}

void MD5ByteContainer::update (
    uint8_t* pData,
    size_t dataSize
) {
    MD5_Update(&m_md5Context, (const void*)pData, dataSize);
}

void MD5ByteContainer::finalize (void) {
    MD5_Final(m_pData, &m_md5Context);
}

MD5ByteContainer& MD5ByteContainer::operator= (const MD5ByteContainer& rhs) {
    //freeBuffer();
    setData(rhs.data(), rhs.size());
    return *this;
}

bool MD5ByteContainer::operator== (const MD5ByteContainer& rhs) const {
    return compareTo(&rhs) == 0;    
}

bool MD5ByteContainer::operator!= (const MD5ByteContainer& rhs) const {
    return compareTo(&rhs) != 0;    
}

bool MD5ByteContainer::operator> (const MD5ByteContainer& rhs) const {
    return compareTo(&rhs) == -1;
}

bool MD5ByteContainer::operator< (const MD5ByteContainer& rhs) const {
    return compareTo(&rhs) == 1;
}
//=============================================================================

