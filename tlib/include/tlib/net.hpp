//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TLIB_NET_H
#define TLIB_NET_H

#include <expected.hpp>

#include "tlib/net_constants.hpp"
#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

std::expected<size_t> socket_open(socket_domain domain, socket_type type, socket_protocol protocol);
void socket_close(size_t fd);

} // end of namespace tlib

#endif
