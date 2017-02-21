#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include "caf/all.hpp"

using LoginAtom = atom_constant<atom("Login")>;
using RtnOrderAtom = atom_constant<atom("RtnOrder")>;

using CtpObserver = caf::typed_actor<caf::reacts_to<OrderPushAtom, std::shared_ptr<CThostFtdcOrderField> > >;


#endif /* CAF_DEFINES_H */
