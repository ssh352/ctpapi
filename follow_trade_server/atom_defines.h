#ifndef FOLLOW_TRADE_SERVER_ATOM_DEFINES_H
#define FOLLOW_TRADE_SERVER_ATOM_DEFINES_H
#include "common/api_struct.h"
#include "websocket_typedef.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

using QueryStrategyRntOrderAtom = caf::atom_constant<caf::atom("qrto")>;
using QueryStrategyOrderIDMapAtom = caf::atom_constant<caf::atom("qid")>;
using InsertStrategyOrderIDAtom = caf::atom_constant<caf::atom("sgyid")>;

using SubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;
using LimitOrderAtom = caf::atom_constant<caf::atom("limit")>;
using CancelOrderAtom = caf::atom_constant<caf::atom("cancel")>;
using RtnOrderAtom = caf::atom_constant<caf::atom("rto")>;
using QueryInverstorPositionAtom = caf::atom_constant<caf::atom("qip")>;


using CTPReqLogin = caf::atom_constant<caf::atom("lg")>;
using CTPRspLogin = caf::atom_constant<caf::atom("rsplg")>;
using CTPReqQryInvestorPositionsAtom = caf::atom_constant<caf::atom("qryipa")>;
using CTPRspQryInvestorPositionsAtom = caf::atom_constant<caf::atom("rspqip")>;
using CTPReqHistoryRtnOrdersAtom = caf::atom_constant<caf::atom("reqrto")>;
using CTPSubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;
using CTPRtnOrderAtom = caf::atom_constant<caf::atom("rtnord")>;
using CTPReqSettlementInfoConfirm = caf::atom_constant<caf::atom("reqsttcnf")>;
using CTPRspSettlementInfoConfirm = caf::atom_constant<caf::atom("rspsttcnf")>;
using CTPReqOpenOrderAtom = caf::atom_constant<caf::atom("reqoo")>;
using CTPReqCloseOrderAtom = caf::atom_constant<caf::atom("reqco")>;
using CTPCancelOrderAtom = caf::atom_constant<caf::atom("co")>;


using CTASignalInitAtom = caf::atom_constant<caf::atom("cta_init")>;
using CTASignalRtnOrderAtom = caf::atom_constant<caf::atom("cta_ro")>;
using CTASignalInverstorPositionAtom = caf::atom_constant<caf::atom("cta_ip")>;

using StragetyPortfilioAtom = caf::atom_constant<caf::atom("sp")>;

using InsertStrategyRtnOrder = caf::atom_constant<caf::atom("sqr")>;




using CallOnActorAtom = caf::atom_constant<caf::atom("self")>;
using ConnectAtom = caf::atom_constant<caf::atom("connect")>;
using CheckHistoryRtnOrderIsDoneAtom = caf::atom_constant<caf::atom("check")>;
using ConnectTimeOutAtom = caf::atom_constant<caf::atom("timeout")>;

using ThostFtdcOrderFieldAtom = caf::atom_constant<caf::atom("ftdcro")>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::function<void(void)>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcRspUserLoginField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcRspInfoField>)

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(connection_hdl)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(OrderField)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(boost::shared_ptr<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(boost::shared_ptr<std::vector<InvestorPositionField>>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<boost::shared_ptr<OrderField> >)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::list<boost::shared_ptr<OrderField> >)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(OrderPosition)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderPosition>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CThostFtdcOrderField>)



#endif // FOLLOW_TRADE_SERVER_ATOM_DEFINES_H



