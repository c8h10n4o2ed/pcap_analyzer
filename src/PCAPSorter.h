/**@file PCAPSorter.h
 */
#ifndef PCAP_SORT_H_
#define PCAP_SORT_H_
//=============================================================================
//INCLUDES
//=============================================================================
#include <string>
#include <pcrecpp.h>
#include <algorithm>
#include <string.h>
#include <stdio.h>

//=============================================================================
//DEFINITIONS
//=============================================================================
class CAPNumericalCompare {
public:
    template <class Type1, class Type2>
    inline bool operator() (const Type1& a, const Type2 &b) {
        char temp[128];
        std::string &a1 = (std::string&)a;
        std::string &b1 = (std::string&)b;
        std::string a2;
        std::string b2;
        pcrecpp::RE re(".*cap(\\d+)");
        pcrecpp::RE re2("(.*cap).*");
        int i=-1;
        int j=-1;

        re2.FullMatch(a1, &a2);
        re2.FullMatch(b1, &b2);

        if (!re.FullMatch(a1, &i)) {
            i=0;
        }
        if (!re.FullMatch(b1, &j)) {
            j=0;
        }

        sprintf(temp, "%08d", i);
        a2 += std::string(temp);

        sprintf(temp, "%08d", j);
        b2 += std::string(temp);
     
        bool lt = std::less<std::string>()(a2,b2);
        
        return lt;
    }
};


class PCAPNumericalCompare {
public:
    template <class Type1, class Type2>
    inline bool operator() (const Type1& a, const Type2 &b) {
        char temp[128];
        std::string &a1 = (std::string&)a;
        std::string &b1 = (std::string&)b;
        std::string a2;
        std::string b2;
        pcrecpp::RE re(".*pcap(\\d+)");
        pcrecpp::RE re2("(.*pcap).*");
        int i=-1;
        int j=-1;

        re2.FullMatch(a1, &a2);
        re2.FullMatch(b1, &b2);

        if (!re.FullMatch(a1, &i)) {
            i=0;
        }
        if (!re.FullMatch(b1, &j)) {
            j=0;
        }

        sprintf(temp, "%08d", i);
        a2 += std::string(temp);

        sprintf(temp, "%08d", j);
        b2 += std::string(temp);
     
        bool lt = std::less<std::string>()(a2,b2);
        
        return lt;
    }
};

//=============================================================================
#endif //PCAP_SORT_H_
