#ifndef FOLLOW_TRADE_SERVER_ATOM_DEFINES_H
#define FOLLOW_TRADE_SERVER_ATOM_DEFINES_H
#include "common/api_struct.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"
#include "websocket_typedef.h"

// common atom
using CallOnActorAtom = caf::atom_constant<caf::atom("self")>;

// DBStore atom
using QueryStrategyRntOrderAtom = caf::atom_constant<caf::atom("qrto")>;
using QueryStrategyOrderIDMapAtom = caf::atom_constant<caf::atom("qid")>;
using InsertStrategyOrderIDAtom = caf::atom_constant<caf::atom("sgyid")>;
using QueryStrategyPositionsAtom = caf::atom_constant<caf::atom("qip")>;
using QueryThostFtdcOrderFeildsAtom = caf::atom_constant<caf::atom("qftdcro")>;

using SubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;
using LimitOrderAtom = caf::atom_constant<caf::atom("limit")>;
using CancelOrderAtom = caf::atom_constant<caf::atom("cancel")>;
using RtnOrderAtom = caf::atom_constant<caf::atom("rto")>;
using QueryInverstorPositionAtom = caf::atom_constant<caf::atom("qip")>;

using InsertStrategyRtnOrder = caf::atom_constant<caf::atom("sqr")>;

using ConnectAtom = caf::atom_constant<caf::atom("connect")>;
using CheckHistoryRtnOrderIsDoneAtom = caf::atom_constant<caf::atom("check")>;
using ConnectTimeOutAtom = caf::atom_constant<caf::atom("timeout")>;

using ThostFtdcOrderFieldAtom = caf::atom_constant<caf::atom("ftdcro")>;
using ThostFtdcTradeFieldAtom = caf::atom_constant<caf::atom("ftdctde")>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::function<void(void)>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcRspUserLoginField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcRspInfoField>)

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(connection_hdl)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(OrderField)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(boost::shared_ptr<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(
    boost::shared_ptr<std::vector<InvestorPositionField>>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<boost::shared_ptr<OrderField>>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::list<boost::shared_ptr<OrderField>>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(OrderPosition)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderPosition>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcOrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcTradeField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::chrono::high_resolution_clock::time_point)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<std::shared_ptr<CThostFtdcOrderField>>)
#endif  // FOLLOW_TRADE_SERVER_ATOM_DEFINES_H
