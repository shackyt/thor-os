//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TLIB_CONFIG_HPP
#define TLIB_CONFIG_HPP

namespace tlib {

constexpr bool is_thor_program(){
#ifdef THOR_PROGRAM
    return true;
#else
    return false;
#endif
}

constexpr bool is_thor_lib(){
#ifdef THOR_TLIB
    return true;
#else
    return false;
#endif
}

} // end of namespace tlib

#define ASSERT_ONLY_THOR_PROGRAM static_assert(tlib::is_thor_program() || tlib::is_thor_lib(), __FILE__ " can only be used in Thor programs");

#ifdef THOR_TLIB
#define THOR_NAMESPACE_NAME(LIB_NS,THOR_NS) LIB_NS
#elif defined(THOR_PROGRAM)
#define THOR_NAMESPACE_NAME(LIB_NS,THOR_NS) LIB_NS
#else
#define THOR_NAMESPACE_NAME(LIB_NS,THOR_NS) THOR_NS
#endif

#define THOR_NAMESPACE(LIB_NS,THOR_NS) namespace THOR_NAMESPACE_NAME(LIB_NS,THOR_NS)

#endif
