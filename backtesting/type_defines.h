#ifndef BACKTESTING_TYPE_DEFINES_H
#define BACKTESTING_TYPE_DEFINES_H
#include "common/api_struct.h"

using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

#endif // BACKTESTING_TYPE_DEFINES_H



