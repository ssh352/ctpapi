#ifndef BACKTESTING_ATOM_DEFINES_H
#define BACKTESTING_ATOM_DEFINES_H
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "common/api_struct.h"
#include "type_defines.h"

using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;
using BeforeTradingAtom = caf::atom_constant<caf::atom("bt")>;
using BeforeCloseMarketAtom = caf::atom_constant<caf::atom("bcm")>;
using CloseMarketNearAtom = caf::atom_constant<caf::atom("cmn")>;
using DaySettleAtom = caf::atom_constant<caf::atom("daysetl")>;
using IdleAtom = caf::atom_constant<caf::atom("idle")>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(boost::property_tree::ptree*)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CostBasis)


#endif // BACKTESTING_ATOM_DEFINES_H



