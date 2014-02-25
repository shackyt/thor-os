//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_H
#define VFS_H

#include <stat_info.hpp>

//TODO Once userspace is done, integrate parts of disks.hpp here

namespace vfs {

int64_t open(const char* file, size_t flags);
int64_t stat(size_t fd, stat_info& info);
int64_t mkdir(const char* file);
int64_t rm(const char* file);
int64_t read(size_t fd, char* buffer, size_t max);

void close(size_t fd);

} //end of namespace vfs

#endif