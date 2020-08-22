/**@file UnsignedByteContainer.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "UnsignedByteContainer.h"
#include "Logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//=============================================================================
// IMPLEMENTATION
//=============================================================================
UnsignedByteContainer::UnsignedByteContainer (void) :
    m_pData(NULL),
    m_dataSize(0)
{
}

UnsignedByteContainer::UnsignedByteContainer (const UnsignedByteContainer& rhs) :
    m_pData(NULL),
    m_dataSize(0)
{
    setData(rhs.data(), rhs.size());
}

UnsignedByteContainer::~UnsignedByteContainer (void)
{
    freeBuffer();
}

void UnsignedByteContainer::setData (
    const uint8_t* pData,
    size_t dataSize
) {
    freeBuffer();

    m_pData = new uint8_t [dataSize];
    m_dataSize = dataSize;

    if (!m_pData) {
        PrintSimpleLogMessage(LEVEL_DEBUG, "Unable to allocate byte buffer!");
    }

    memcpy(m_pData, pData, dataSize);
}

const uint8_t* UnsignedByteContainer::data (void) const {
    return (const uint8_t*)m_pData;
}

const size_t UnsignedByteContainer::size (void) const {
    return m_dataSize;
}

int UnsignedByteContainer::compareTo (
    const UnsignedByteContainer* pContainer
) const {
    int retValue = 0;
    size_t rhsSize = pContainer->size();
    size_t lhsSize = size();
    
    if (rhsSize > lhsSize) {
        //!pContainer->isPrefixNull(0, rhsSize - lhsSize)) {
        
        //For now assume that if one byte buffer is larger
        //and the prefix bytes are not zero that it is larger
        retValue = 1;
    } else if (rhsSize < lhsSize) {
               //!isPrefixNull(0, lhsSize - rhsSize)){
               
        //For now assume that if one byte buffer is larger
        //and the prefix bytes are not zero that it is larger
        retValue = -1;
    } else if (rhsSize == lhsSize) {
        //For equal size buffers just compare the bytes directly
        const uint8_t* rhsDataCursor = pContainer->data();
        const uint8_t* lhsDataCursor = data();    
        int i;

        for (i=0; i<lhsSize; i++) {
            if (*rhsDataCursor > *lhsDataCursor) {
                retValue = 1;
                goto Exit;
            } else if (*rhsDataCursor < *lhsDataCursor) {
                retValue = -1;
                goto Exit;
            }

            //Increment cursors
            rhsDataCursor++;
            lhsDataCursor++;
        }

        retValue = 0;
    }

Exit:
    return retValue;
}

void UnsignedByteContainer::freeBuffer (void) {
    if (m_pData) {
        delete m_pData;
        m_pData = NULL;
    }
}

bool UnsignedByteContainer::isPrefixNull (
    size_t index,
    size_t count
) {
    //Assume that the prefix is NULL at the beginning
    bool retValue = true;
    int i;

    //Iterate over the data starting at index
    //for count bytes. The first time a non-null
    //byte is found this routine returns false indicating
    //that the prefix is not null.
    for (i=0; i<count; i++) {
        if (m_pData[index + i]) {
            retValue = false;
            break;
        }
    }

    return retValue;
}

const std::string UnsignedByteContainer::toHexString (void) const {
    std::string retValue = std::string("");
    char tmpBuf[16];
    const uint8_t* pData = data();
    int i;

    for (i=0; i<size(); i++) {
        snprintf(tmpBuf, sizeof(tmpBuf), "%02x", pData[i]);
        retValue += std::string(tmpBuf);
    }

    return retValue;
}

UnsignedByteContainer& UnsignedByteContainer::operator= (const UnsignedByteContainer& rhs) {
    setData(rhs.data(), rhs.size());
    return *this;
}

bool UnsignedByteContainer::operator== (const UnsignedByteContainer& rhs) const {
    return compareTo(&rhs) == 0;    
}

bool UnsignedByteContainer::operator!= (const UnsignedByteContainer& rhs) const {
    return compareTo(&rhs) != 0;    
}

bool UnsignedByteContainer::operator> (const UnsignedByteContainer& rhs) const {
    return compareTo(&rhs) == -1;
}

bool UnsignedByteContainer::operator< (const UnsignedByteContainer& rhs) const {
    return compareTo(&rhs) == 1;
}
//=============================================================================
