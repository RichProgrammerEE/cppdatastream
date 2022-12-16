#include "cppdatastream/Utilities.hpp"

#ifndef _MSC_VER
    #include <cxxabi.h>
#endif

/// @brief Designed to be called as demangleName(typeid(...).name())
std::string demangleName(const std::string& name)
{
#ifndef _MSC_VER
    int status{0};
    return abi::__cxa_demangle(name, 0, 0, &status);
#else
    // Typeid(...).name() is already demangled on windows.
    return name;
#endif
}