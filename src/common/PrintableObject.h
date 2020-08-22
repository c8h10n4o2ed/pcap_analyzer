/**@file PrintableObject.h 
 */
#ifndef PRINTABLE_OBJECT_H_
#define PRINTABLE_OBJECT_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <string>

//=============================================================================
// DEFINITIONS
//=============================================================================
class PrintableObject {
public:
    /**
     * Retrieves a printable string representation of this object.
     * 
     * @param void 
     * @return std::string 
     */
    virtual std::string getStringRepr (void) = 0;
};


//=============================================================================
#endif //PRINTABLE_OBJECT_H_
