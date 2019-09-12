//
// Created by armanaxh on ۲۰۱۹/۹/۱۱.
//

#include <stdio.h>
#include <string>
#include <sstream>

namespace patch {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}