#ifndef _GUARD_FINDNODEINFO_H
#define _GUARD_FINDNODEINFO_H

struct node_info;

#include <functional>
#include <string>
#include "caf/all.hpp"

struct find_relative : std::unary_function<node_info, bool> {
    caf::actor handle;
    find_relative(caf::actor);
    bool operator()(node_info const&) const;
};

struct find_rel_by_ip_port : std::unary_function<node_info, bool> {
    std::string relative_ip;
    int relative_port;
    find_rel_by_ip_port(std::string, int);
    bool operator()(node_info const&) const;
};

#endif