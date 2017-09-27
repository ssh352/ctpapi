#pragma once
#include <memory>
#include "common/api_struct.h"

std::shared_ptr<TickData> MakeTick(std::string instrument,
                                   double last_price,
                                   int qty,
                                   TimeStamp timestamp);
