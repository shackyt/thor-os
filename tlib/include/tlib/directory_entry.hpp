//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_DIRECTORY_ENTRY_HPP
#define USER_DIRECTORY_ENTRY_HPP

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, vfs) {

struct directory_entry {
    size_t type;
    size_t offset_next;
    size_t length;
    char name; //First char
};

} // end of namespace tlib

#endif
