#ifndef LIVE_TRADE_CAF_ATOM_DEFINES_H
#define LIVE_TRADE_CAF_ATOM_DEFINES_H

using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;
using IdleAtom = caf::atom_constant<caf::atom("idle")>;
using CtpConnectAtom = caf::atom_constant<caf::atom("ctpconnect")>;

using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)
// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CancelOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrderSignal)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<TickData>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CTPOrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderPosition>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTAPositionQty)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CancelOrderSignal)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTPEnterOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTPCancelOrder)
#endif // LIVE_TRADE_CAF_ATOM_DEFINES_H



