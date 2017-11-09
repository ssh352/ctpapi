#ifndef BACKTESTING_ATOM_DEFINES_H
#define BACKTESTING_ATOM_DEFINES_H

using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;
using BeforeTradingAtom = caf::atom_constant<caf::atom("bt")>;
using BeforeCloseMarketAtom = caf::atom_constant<caf::atom("bcm")>;
using CloseMarketNearAtom = caf::atom_constant<caf::atom("cmn")>;
using DaySettleAtom = caf::atom_constant<caf::atom("daysetl")>;
using IdleAtom = caf::atom_constant<caf::atom("idle")>;

#endif // BACKTESTING_ATOM_DEFINES_H



