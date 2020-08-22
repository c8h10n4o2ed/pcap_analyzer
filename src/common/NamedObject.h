/**@file NamedObject.h 
 */
#ifndef NAMED_OBJECT_H_
#define NAMED_OBJECT_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <string>

//=============================================================================
// DEFINITIONS
//=============================================================================

class NamedObject {
public:
    NamedObject (std::string sName);

    /**
     * Gets the name associated with this object.
     * 
     * @return const std::string&
     */
    const std::string& getName (void) const;

protected:
    std::string m_sName;
};

//=============================================================================
#endif //NAMED_OBJECT_H_
