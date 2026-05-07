#pragma once

#include "connection.hpp"
#include "router.hpp"

namespace tinyhttp {

void handle_connection(Connection conn, const Router& router);

} // namespace tinyhttp
