#ifndef FOLLOW_TRADE_SERVER_WEBSOCKET_TYPEDEF_H
#define FOLLOW_TRADE_SERVER_WEBSOCKET_TYPEDEF_H

#include "websocketpp/config/asio_no_tls.hpp"

#include "websocketpp/server.hpp"

#include "websocketpp/common/thread.hpp"

typedef websocketpp::server<websocketpp::config::asio> Server;
using websocketpp::connection_hdl;

typedef Server::message_ptr message_ptr;

#endif // FOLLOW_TRADE_SERVER_WEBSOCKET_TYPEDEF_H



