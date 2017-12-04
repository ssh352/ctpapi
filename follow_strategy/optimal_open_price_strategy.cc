#include "optimal_open_price_strategy.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

OptimalOpenPriceStrategy::OptimalOpenPriceStrategy(
  Delegate* delegate,
  boost::property_tree::ptree* strategy_config,
  boost::log::sources::logger* log) : delegate_(delegate),
  log_(log) {
   for (const auto& pt : *strategy_config) {
      try {
        if (pt.first == "default") {
        default_param_.delayed_open_after_seconds =
            pt.second.get<int>("DelayOpenOrderAfterSeconds");
        default_param_.wait_optimal_open_price_fill_seconds =
            pt.second.get<int>("WaitOptimalOpenPriceFillSeconds");
        default_param_.price_offset = pt.second.get<double>("OptimizeTickSize");
        } else {
        OptimalOpenPriceStrategy::StrategyParam param;
        param.delayed_open_after_seconds =
            pt.second.get<int>("DelayOpenOrderAfterSeconds");
        param.wait_optimal_open_price_fill_seconds =
            pt.second.get<int>("WaitOptimalOpenPriceFillSeconds");
        param.price_offset = pt.second.get<double>("OptimizeTickSize");
        instrument_params_.insert({pt.first, std::move(param)});
        }

      } catch (boost::property_tree::ptree_error& err) {
        std::cout << "Read Confirg File Error:" << pt.first << ":" << err.what()
                  << "\n";
      }
    }
}

std::string OptimalOpenPriceStrategy::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_id_seq_++);
}

int OptimalOpenPriceStrategy::PendingOpenQty(const std::string& instrument,
                                          OrderDirection position_effect_direction) {
  return std::accumulate(
      pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(), 0,
      [&instrument, position_effect_direction](int val,
                                     const InputOrder& input_order) {
        if (input_order.instrument != instrument ||
            input_order.direction != position_effect_direction) {
          return val;
        }
        return val + input_order.qty;
      });
}

void OptimalOpenPriceStrategy::CancelUnfillOpeningOrders(
    const std::string& instrument,
    OrderDirection direciton,
    int leaves_cancel_qty) {
  auto orders = portfolio_.UnfillOpenOrders(instrument, direciton);
  std::sort(orders.begin(), orders.end(),
            [direciton](const auto& l, const auto& r) {
              return direciton == OrderDirection::kBuy
                         ? l->input_price < r->input_price
                         : l->input_price > r->input_price;
            });

  auto for_each_lambda = [=, &leaves_cancel_qty](const auto& order) {
    if (leaves_cancel_qty <= 0) {
      return;
    }
    int cancel_qty = std::min<int>(order->leaves_qty, leaves_cancel_qty);
    if (cancel_qty == order->leaves_qty) {
      delegate_->HandleCancelOrder(order->instrument_id, order->order_id,
                                   order->position_effect_direction);
    } else {
      OrderAction action_order;
      action_order.instrument = order->instrument_id;
      action_order.order_id = order->order_id;
      action_order.old_price = 0.0;
      action_order.new_price = 0.0;
      action_order.old_qty = order->leaves_qty;
      action_order.new_qty = order->leaves_qty - cancel_qty;
      delegate_->HandleActionOrder(std::move(action_order),
                                   order->position_effect_direction);
    }
    // delegate_->HandleCancelOrder(order->instrument_id,order->order_id,
    // order->position_effect_direction, cancel_qty);
    // signal_dispatch_->CancelOrder(order->order_id);

    // int open_qty = order->leaves_qty - cancel_qty;
    // if (open_qty > 0) {
    //  signal_dispatch_->OpenOrder(order->instrument_id, GenerateOrderId(),
    //                              position_direction, order->input_price,
    //                              open_qty);
    //}
    leaves_cancel_qty -= cancel_qty;
  };

  if (direciton == OrderDirection::kBuy) {
    std::for_each(orders.begin(), orders.end(), for_each_lambda);
  } else {
    std::for_each(orders.rbegin(), orders.rend(), for_each_lambda);
  }
}

void OptimalOpenPriceStrategy::DecreasePendingOpenOrderQty(
    const std::string& instrument,
    OrderDirection position_effect_direction,
    int qty) {
  if (qty <= 0)
    return;
  int leaves_qty = qty;
  pending_delayed_open_order_.sort([position_effect_direction](const auto& l, const auto& r) {
    return position_effect_direction == OrderDirection::kBuy ? (l.price < r.price)
                                             : (l.price > r.price);
  });
  for (auto& input_order : pending_delayed_open_order_) {
    if (input_order.instrument != instrument ||
        input_order.direction != position_effect_direction) {
      continue;
    }
    int minus_qty = std::min<int>(input_order.qty, leaves_qty);
    input_order.qty -= minus_qty;
    leaves_qty -= minus_qty;
    if (leaves_qty <= 0)
      break;
  }

  auto remove_it = std::remove_if(
      pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
      [](const auto& input_order) { return input_order.qty == 0; });

  pending_delayed_open_order_.erase(remove_it,
                                    pending_delayed_open_order_.end());

  pending_delayed_open_order_.sort([](const auto& l, const auto& r) -> bool {
    return l.timestamp < r.timestamp;
  });
}

std::vector<InputOrder>
OptimalOpenPriceStrategy::GetSpecificOrdersInPendingOpenQueue(
    const std::string& instrument,
    OrderDirection direction) {
  std::vector<InputOrder> orders;
  std::copy_if(pending_delayed_open_order_.begin(),
               pending_delayed_open_order_.end(), std::back_inserter(orders),
               [&instrument, direction](const auto& order) {
                 return order.instrument == instrument &&
                        order.direction == direction;
               });
  return orders;
}

void OptimalOpenPriceStrategy::CancelSpecificOpeningOrders(
    const std::string& instrument,
    OrderDirection direction) {
  if (portfolio_.UnfillOpenQty(instrument, direction) > 0) {
    auto orders = portfolio_.UnfillOpenOrders(instrument, direction);
    for (const auto& order : orders) {
      delegate_->HandleCancelOrder(order->instrument_id, order->order_id, order->position_effect_direction);
    }
  }
}

void OptimalOpenPriceStrategy::RemoveSpecificPendingOpenOrders(
    const std::string& instrument,
    OrderDirection position_effect_direction) {
  auto remove_it = std::remove_if(
      pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
      [&instrument, position_effect_direction](const auto& input_order) {
        return input_order.instrument == instrument &&
               input_order.direction == position_effect_direction;
      });

  pending_delayed_open_order_.erase(remove_it,
                                    pending_delayed_open_order_.end());
}

void OptimalOpenPriceStrategy::HandleCanceled(
    const std::shared_ptr<const OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {
  if (rtn_order->position_effect == PositionEffect::kOpen) {
    return;
  }
  auto it = cta_to_strategy_closing_order_id_.find(rtn_order->order_id);
  if (it != cta_to_strategy_closing_order_id_.end()) {
    auto order = portfolio_.GetOrder(it->second);
    BOOST_LOG(*log_)  << boost::log::add_value("quant_timestamp",
                           TimeStampToPtime(last_timestamp_))
    << "处理撤捎订单:" << order->instrument_id;
    delegate_->HandleCancelOrder(
        order->instrument_id, order->order_id, order->position_effect_direction);
    cta_to_strategy_closing_order_id_.erase(it);
  }
}

void OptimalOpenPriceStrategy::HandleClosed(
    const std::shared_ptr<const OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {
  //int closeable_qty = portfolio_.GetPositionCloseableQty(
  //    rtn_order->instrument_id, rtn_order->position_effect_direction);
  BOOST_ASSERT(position_qty.position >= position_qty.frozen);
  int pending_open_order_qty =
      PendingOpenQty(rtn_order->instrument_id, rtn_order->position_effect_direction);
  int opening_order_qty =
      portfolio_.UnfillOpenQty(rtn_order->instrument_id, rtn_order->position_effect_direction);
  if (position_qty.position == 0) {
    RemoveSpecificPendingOpenOrders(rtn_order->instrument_id,
                                    rtn_order->position_effect_direction);
    CancelSpecificOpeningOrders(rtn_order->instrument_id, rtn_order->position_effect_direction);
  } else if (pending_open_order_qty + opening_order_qty >
             position_qty.position) {
    int leaves_cancel_or_unqueue_qty =
        pending_open_order_qty + opening_order_qty - position_qty.position;
    BOOST_ASSERT(leaves_cancel_or_unqueue_qty > 0);
    if (leaves_cancel_or_unqueue_qty > 0) {
      enum class OpenOrderFrom {
        kPendingQueue,
        kUnfillQueue,
      };
      std::vector<std::tuple<OpenOrderFrom, double, int> > orders_from_queue;
      auto pending_open_orders = GetSpecificOrdersInPendingOpenQueue(
          rtn_order->instrument_id, rtn_order->position_effect_direction);
      std::for_each(
          pending_open_orders.begin(), pending_open_orders.end(),
          [&orders_from_queue](const auto& order) {
            orders_from_queue.push_back(std::make_tuple(
                OpenOrderFrom::kPendingQueue, order.price, order.qty));
          });
      auto opening_orders = portfolio_.UnfillOpenOrders(
          rtn_order->instrument_id, rtn_order->position_effect_direction);
      std::for_each(opening_orders.begin(), opening_orders.end(),
                    [&orders_from_queue](const auto& order) {
                      orders_from_queue.push_back(std::make_tuple(
                          OpenOrderFrom::kUnfillQueue, order->input_price,
                          order->leaves_qty));
                    });

      std::sort(orders_from_queue.begin(), orders_from_queue.end(),
                [&rtn_order](const auto& l, const auto& r) {
                  return rtn_order->position_effect_direction == OrderDirection::kBuy
                             ? std::get<1>(l) < std::get<1>(r)
                             : std::get<1>(l) > std::get<1>(r);
                });
      int want_decrease_pending_open_qty = 0;
      int want_cancel_opening_qty = 0;
      for (const auto& item : orders_from_queue) {
        int qty = std::min(std::get<2>(item), leaves_cancel_or_unqueue_qty);

        if (std::get<0>(item) == OpenOrderFrom::kPendingQueue) {
          want_decrease_pending_open_qty += qty;
        } else {
          want_cancel_opening_qty += qty;
        }

        leaves_cancel_or_unqueue_qty -= qty;
        if (leaves_cancel_or_unqueue_qty <= 0) {
          break;
        }
      }
      DecreasePendingOpenOrderQty(rtn_order->instrument_id,
                                  rtn_order->position_effect_direction,
                                  want_decrease_pending_open_qty);
      CancelUnfillOpeningOrders(rtn_order->instrument_id, rtn_order->position_effect_direction,
                                want_cancel_opening_qty);
    }
  } else {
  }
}

void OptimalOpenPriceStrategy::HandleCloseing(
    const std::shared_ptr<const OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {
  int closeable_qty = portfolio_.GetPositionCloseableQty(
      rtn_order->instrument_id, rtn_order->position_effect_direction);
  BOOST_ASSERT(position_qty.position >= position_qty.frozen);
  if (position_qty.position == position_qty.frozen && closeable_qty > 0) {
    // Close All
    std::string order_id = GenerateOrderId();
    cta_to_strategy_closing_order_id_.insert(
        std::make_pair(rtn_order->order_id, order_id));

    BOOST_LOG(*log_)  << 
      boost::log::add_value("quant_timestamp",
                             TimeStampToPtime(last_timestamp_))
      << "CTA全部清仓,发出平仓指令:" 
      << " 订单:" << order_id
      << " 价格:" << rtn_order->input_price
      << " 数量:" << rtn_order->qty;
    delegate_->HandleEnterOrder(
        InputOrder{rtn_order->instrument_id, std::move(order_id),
                   PositionEffect::kClose, rtn_order->direction,
                   rtn_order->input_price, closeable_qty, 0}, rtn_order->position_effect_direction);
  } else if (rtn_order->qty <= closeable_qty) {
    std::string order_id = GenerateOrderId();
    cta_to_strategy_closing_order_id_.insert(
        std::make_pair(rtn_order->order_id, order_id));
    BOOST_LOG(*log_)  << 
      boost::log::add_value("quant_timestamp",
                             TimeStampToPtime(last_timestamp_))
      << "CTA平仓并且数量小于等于策略可平仓量,发出平仓指令:" 
      << " 订单:" << order_id
      << " 价格:" << rtn_order->input_price
      << " 数量:" << rtn_order->qty;
    delegate_->HandleEnterOrder(
        InputOrder{rtn_order->instrument_id, std::move(order_id),
                   PositionEffect::kClose, rtn_order->direction,
                   rtn_order->input_price, rtn_order->qty, 0}, rtn_order->position_effect_direction);
  } else if (closeable_qty > 0) {
    std::string order_id = GenerateOrderId();
    cta_to_strategy_closing_order_id_.insert(
        std::make_pair(rtn_order->order_id, order_id));
    BOOST_LOG(*log_)  << 
      boost::log::add_value("quant_timestamp",
                             TimeStampToPtime(last_timestamp_))
      << "CTA平仓并且数量大于策略可平仓量,发出清仓指令:" 
      << " 订单:" << order_id
      << " 价格:" << rtn_order->input_price
      << " 数量:" << closeable_qty;
    delegate_->HandleEnterOrder(
        InputOrder{rtn_order->instrument_id, std::move(order_id),
                   PositionEffect::kClose, rtn_order->direction,
                   rtn_order->input_price, closeable_qty, 0}, rtn_order->position_effect_direction);
  } else {
  }
}

void OptimalOpenPriceStrategy::HandleOpened(
    const std::shared_ptr<const OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {
  if (GetStrategyParamDealyOpenAfterSeconds(rtn_order->instrument_id,
    ParseProductCodeWithInstrument(rtn_order->instrument_id)) > 0) {
    BOOST_LOG(*log_)  << 
      boost::log::add_value("quant_timestamp",
                             TimeStampToPtime(last_timestamp_))
      << "加入等待开仓对列:" << rtn_order->instrument_id
      << "价格:" << rtn_order->trading_price << "数量:" << rtn_order->trading_qty;
    pending_delayed_open_order_.push_back(InputOrder{
        rtn_order->instrument_id, "",PositionEffect::kOpen,
        rtn_order->position_effect_direction, rtn_order->trading_price, rtn_order->trading_qty,
        rtn_order->update_timestamp});
  } else {
    OpenOptimalPriceOrder(InputOrder{
        rtn_order->instrument_id, "",PositionEffect::kOpen,
        rtn_order->position_effect_direction, rtn_order->trading_price, rtn_order->trading_qty,
        rtn_order->update_timestamp});
  }
}

void OptimalOpenPriceStrategy::HandleOpening(
    const std::shared_ptr<const OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {}

void OptimalOpenPriceStrategy::HandleCTARtnOrderSignal(
    const std::shared_ptr<OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {
  switch (rtn_order->status) {
    case OrderStatus::kActive:
      if (rtn_order->leaves_qty == rtn_order->qty) {
        // New Open/Close
        if (IsOpenOrder(rtn_order->position_effect)) {
          HandleOpening(rtn_order, position_qty);
        } else {
          HandleCloseing(rtn_order, position_qty);
        }
      } else {
        if (IsOpenOrder(rtn_order->position_effect)) {
          HandleOpened(rtn_order, position_qty);
        } else {
          HandleClosed(rtn_order, position_qty);
        }
      }
      break;
    case OrderStatus::kAllFilled: {
      if (IsOpenOrder(rtn_order->position_effect)) {
        HandleOpened(rtn_order, position_qty);
      } else {
        HandleClosed(rtn_order, position_qty);
      }
    } break;
    case OrderStatus::kCanceled:
      HandleCanceled(rtn_order, position_qty);
      break;
    default:

      break;
  }
}

void OptimalOpenPriceStrategy::HandleRtnOrder(
    const std::shared_ptr<OrderField>& rtn_order) {
  if (rtn_order->trading_qty != 0) {
    BOOST_LOG(*log_)
        << boost::log::add_value("quant_timestamp",
                                 TimeStampToPtime(last_timestamp_))
      << "收到订单成交:" << rtn_order->order_id 
      << " 成交数量:" << rtn_order->trading_qty
      << (rtn_order->leaves_qty == 0 ? "全部成交" : " 部份成交");
  }
  if (!optimal_open_price_orders_.empty() && 
    (rtn_order->status == OrderStatus::kAllFilled ||
      rtn_order->status == OrderStatus::kCanceled)) {
    auto it  = std::find_if(optimal_open_price_orders_.begin(), optimal_open_price_orders_.end(),
      [&rtn_order](const auto& order){
      return rtn_order->order_id == order.order_id;
    });

    if (it != optimal_open_price_orders_.end()) {
      optimal_open_price_orders_.erase(it);
    }
  }
  portfolio_.HandleOrder(rtn_order);
}

void OptimalOpenPriceStrategy::HandleTick(const std::shared_ptr<TickData>& tick) {
  last_timestamp_ = tick->tick->timestamp;
  int delayed_open_after_seconds = GetStrategyParamDealyOpenAfterSeconds(
      *tick->instrument,
    ParseProductCodeWithInstrument(*tick->instrument));
  if (delayed_open_after_seconds > 0 && !pending_delayed_open_order_.empty()) {
    auto it_end = std::remove_if(
        pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
        [=](const auto& order) -> bool {
          if (*tick->instrument != order.instrument) {
            return false;
          }
          double price_offset = GetStrategyParamPriceOffset(order.instrument,
            ParseProductCodeWithInstrument(order.instrument));
          // is expire
          if (tick->tick->timestamp >=
              order.timestamp +
                  delayed_open_after_seconds * 1000) {
            double maybe_input_price =
                order.direction == OrderDirection::kBuy
                    ? order.price -
                          price_offset
                    : order.price +
                          price_offset;
            double input_price =
                order.direction == OrderDirection::kBuy
                    ? std::min(order.price, tick->tick->ask_price1)
                    : std::max(order.price, tick->tick->bid_price1);
            BOOST_LOG(*log_)
                << boost::log::add_value("quant_timestamp",
                                         TimeStampToPtime(last_timestamp_))
                << "延迟开仓订单到期 :" << input_price
                << "当前 Tick :" << *(tick->instrument) << ":"
                << " Last:" << tick->tick->last_price << "(" << tick->tick->qty
                << ")"
                << " Bid1:" << tick->tick->bid_price1 << "("
                << tick->tick->bid_qty1 << ")"
                << " Ask1:" << tick->tick->ask_price1 << "("
                << tick->tick->ask_qty1 << ")";
            OpenOptimalPriceOrder(order);
            return true;
          }
          return false;
        });
    pending_delayed_open_order_.erase(it_end,
                                      pending_delayed_open_order_.end());
  }
  if (!optimal_open_price_orders_.empty()) {
    auto it_end = std::remove_if(
        optimal_open_price_orders_.begin(), optimal_open_price_orders_.end(),
        [=](const auto& order) -> bool {
          if (*tick->instrument != order.instrument) {
            return false;
          }
          double price_offset = GetStrategyParamPriceOffset(order.instrument,
            ParseProductCodeWithInstrument(order.instrument));
          int delayed_open_after_seconds = GetStrategyParamWaitOptimalOpenPriceFillSeconds(order.instrument,
            ParseProductCodeWithInstrument(order.instrument));
          // is expire
          if (tick->tick->timestamp >=
              order.timestamp +
                  delayed_open_after_seconds * 1000) {
            double input_price =
                order.direction == OrderDirection::kBuy
                    ? std::min(order.price, tick->tick->ask_price1)
                    : std::max(order.price, tick->tick->bid_price1);
            BOOST_LOG(*log_)
                << boost::log::add_value("quant_timestamp",
                                         TimeStampToPtime(last_timestamp_))
                << "延迟等待最优价到期:" << input_price
                << "当前 Tick :" << *(tick->instrument) << ":"
                << " Last:" << tick->tick->last_price << "(" << tick->tick->qty
                << ")"
                << " Bid1:" << tick->tick->bid_price1 << "("
                << tick->tick->bid_qty1 << ")"
                << " Ask1:" << tick->tick->ask_price1 << "("
                << tick->tick->ask_qty1 << ")";
            delegate_->HandleActionOrder(
                OrderAction{order.order_id, order.instrument,
                           0, 0, order.optimal_price, order.price},
              order.direction);
            return true;
          }
          return false;
          }
        );
    optimal_open_price_orders_.erase(it_end,
                                      optimal_open_price_orders_.end());
  }
  last_tick_ = tick;
}


void OptimalOpenPriceStrategy::HandleNearCloseMarket() {
  auto orders = portfolio_.UnfillCloseOrders();
  for (const auto& order : orders) {
    BOOST_LOG(*log_)
        << boost::log::add_value("quant_timestamp",
                                 TimeStampToPtime(last_timestamp_))
        << "临近收市:" << order->instrument_id << ";" << order->order_id;
    OrderAction action_order;
    action_order.instrument = order->instrument_id;
    action_order.order_id = order->order_id;
    action_order.old_qty = 0;
    action_order.new_qty = 0;
    action_order.old_price = order->input_price;
    action_order.new_price = (order->direction == OrderDirection::kBuy) ?
      last_tick_->tick->ask_price1 : last_tick_->tick->bid_price1;
    delegate_->HandleActionOrder(std::move(action_order), 
      order->position_effect_direction);
  }
}



void OptimalOpenPriceStrategy::InitPosition(const std::vector<OrderPosition>& positions) {
  for (const auto& pos : positions) {
    portfolio_.AddPosition(pos.instrument, pos.order_direction, pos.quantity);
  }
}

void OptimalOpenPriceStrategy::OpenOptimalPriceOrder(InputOrder input_order)
{
  input_order.order_id = GenerateOrderId();
  double optimal_price = GetOptimalOpenPrice(input_order.price,
    GetStrategyParamPriceOffset(input_order.instrument,
      ParseProductCodeWithInstrument(input_order.instrument)),
    input_order.direction);
  double old_price = input_order.price;
  input_order.price = optimal_price;
  delegate_->HandleEnterOrder(input_order, input_order.direction);

  optimal_open_price_orders_.push_back(OptimalOpenOrder{
    input_order.instrument,
    input_order.order_id,
    input_order.direction,
    optimal_price,
    old_price,
    input_order.timestamp });

  BOOST_LOG(*log_) <<
    boost::log::add_value("quant_timestamp",
      TimeStampToPtime(last_timestamp_))
    << "以预设的更优价下单:" << input_order.instrument << "价格:"
    << optimal_price;
}


double OptimalOpenPriceStrategy::GetOptimalOpenPrice(double price, double price_offset, OrderDirection direction) const
{
  return direction == OrderDirection::kBuy ? price - price_offset
         : price + price_offset;
}

double OptimalOpenPriceStrategy::GetStrategyParamPriceOffset(
    const std::string& instrument,
  const std::string& product_code) const
{
  if (instrument_params_.find(instrument) != instrument_params_.end()) {
    return instrument_params_.at(instrument).price_offset;
  }

  if (instrument_params_.find(product_code) != instrument_params_.end()) {
    return instrument_params_.at(product_code).price_offset;
  }


  return default_param_.price_offset;

  //std::string instrument_code = instrument.substr(
  //    0, instrument.find_first_of("0123456789"));
  //boost::algorithm::to_lower(instrument_code);
  //if (instrument_params_.find(instrument_code) == instrument_params_.end()) {
  //  BOOST_LOG(*log_) << "没有找到产品配置:" << instrument;
  //  return 0.0;
  //}

  //return instrument_params_.at(instrument_code).price_offset;
}

int OptimalOpenPriceStrategy::GetStrategyParamDealyOpenAfterSeconds(
  const std::string& instrument,
  const std::string& product_code) const {
  if (instrument_params_.find(instrument) != instrument_params_.end()) {
    return instrument_params_.at(instrument).delayed_open_after_seconds;
  }

  if (instrument_params_.find(product_code) != instrument_params_.end()) {
    return instrument_params_.at(product_code).delayed_open_after_seconds;
  }

  return default_param_.delayed_open_after_seconds;
}

int OptimalOpenPriceStrategy::GetStrategyParamWaitOptimalOpenPriceFillSeconds(
  const std::string& instrument,
  const std::string& product_code) const {

  if (instrument_params_.find(instrument) != instrument_params_.end()) {
    return instrument_params_.at(instrument).wait_optimal_open_price_fill_seconds;
  }

  if (instrument_params_.find(product_code) != instrument_params_.end()) {
    return instrument_params_.at(product_code).wait_optimal_open_price_fill_seconds;
  }

  return default_param_.wait_optimal_open_price_fill_seconds;

  //std::string instrument_code = instrument.substr(
  //    0, instrument.find_first_of("0123456789"));
  //boost::algorithm::to_lower(instrument_code);
  //if (instrument_params_.find(instrument_code) == instrument_params_.end()) {
  //  BOOST_LOG(*log_) << "没有找到产品配置:" << instrument;
  //  return 0.0;
  //}
}

std::string OptimalOpenPriceStrategy::ParseProductCodeWithInstrument(const std::string& instrument) {
  std::string product_code;
  auto it = instrument_code_cache_.find(instrument);
  if (it == instrument_code_cache_.end()) {
      product_code = instrument.substr(
          0, instrument.find_first_of("0123456789"));
      boost::algorithm::to_lower(product_code);
      instrument_code_cache_.insert({instrument, product_code});
  } else {
    product_code = it->second;
  }
  return product_code;
}
