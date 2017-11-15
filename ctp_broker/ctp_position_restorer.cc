#include "ctp_position_restorer.h"

void CtpPositionRestorer::AddYesterdayPosition(const std::string& instrument,
                                               OrderDirection direction,
                                               int qty) {
  auto container = direction == OrderDirection::kBuy ? &long_ : &short_;
  MakeCTPPositionAmountIfNeeed(container, instrument);
  BOOST_ASSERT(container->at(instrument)->Total() == 0);
  container->at(instrument)->Init(qty, 0, 0, 0);
}

void CtpPositionRestorer::HandleRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  auto ctp_it = ctp_orders_.find(order->order_id, HashCTPOrderField(),
                                 CompareCTPOrderField());
  if (ctp_it == ctp_orders_.end()) {
    ctp_orders_.insert(std::make_unique<CTPOrderField>(*order));
    if (!IsCtpOpenPositionEffect(order->position_effect)) {
      if (order->position_effect_direction == OrderDirection::kBuy) {
        Frozen(&long_, order->instrument, order->qty, order->position_effect);
      } else {
        Frozen(&short_, order->instrument, order->qty, order->position_effect);
      }
    }
  } else if (order->status == OrderStatus::kCanceled) {
    if (!IsCtpOpenPositionEffect((*ctp_it)->position_effect)) {
      // Close
      if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
        Unfrozen(&long_, order->instrument, (*ctp_it)->leaves_qty,
                 (*ctp_it)->position_effect);
      } else {
        Unfrozen(&short_, order->instrument, (*ctp_it)->leaves_qty,
                 (*ctp_it)->position_effect);
      }
    } else {
      auto ctp_it = ctp_orders_.find(order->order_id, HashCTPOrderField(),
                                     CompareCTPOrderField());
      BOOST_ASSERT(ctp_it != ctp_orders_.end());
      (*ctp_it)->status = order->status;
    }
  } else {
  }
}

void CtpPositionRestorer::HandleTraded(const std::string& order_id,
                                       double trading_price,
                                       int trading_qty,
                                       TimeStamp timestamp) {
  auto ctp_it =
      ctp_orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
  BOOST_ASSERT(ctp_it != ctp_orders_.end());
  if (ctp_it == ctp_orders_.end()) {
    return;
  }

  if (IsCtpOpenPositionEffect((*ctp_it)->position_effect)) {
    if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
      OpenTraded(&long_, (*ctp_it)->instrument, trading_qty);
    } else {
      OpenTraded(&short_, (*ctp_it)->instrument, trading_qty);
    }
  } else {
    if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
      CloseTraded(&long_, (*ctp_it)->instrument, trading_qty,
                  (*ctp_it)->position_effect);
    } else {
      CloseTraded(&short_, (*ctp_it)->instrument, trading_qty,
                  (*ctp_it)->position_effect);
    }
  }

  (*ctp_it)->leaves_qty -= trading_qty;
  BOOST_ASSERT((*ctp_it)->leaves_qty >= 0);
}

bool CtpPositionRestorer::IsCtpOpenPositionEffect(
    CTPPositionEffect position_effect) const {
  return position_effect == CTPPositionEffect::kOpen;
}

void CtpPositionRestorer::Frozen(
    std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
    const std::string& instrument,
    int qty,
    CTPPositionEffect position_effect) {
  MakeCTPPositionAmountIfNeeed(container, instrument);
  (*container)[instrument]->Frozen(qty, position_effect);
}

void CtpPositionRestorer::Unfrozen(
    std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
    const std::string& instrument,
    int qty,
    CTPPositionEffect position_effect) {
  BOOST_ASSERT(container->find(instrument) != container->end());
  if (container->find(instrument) != container->end()) {
    (*container)[instrument]->Unfrozen(qty, position_effect);
  }
}

void CtpPositionRestorer::OpenTraded(
    std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
    const std::string& instrument,
    int qty) {
  MakeCTPPositionAmountIfNeeed(container, instrument);
  (*container)[instrument]->OpenTraded(qty);
}

void CtpPositionRestorer::CloseTraded(
    std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
    const std::string& instrument,
    int qty,
    CTPPositionEffect position_effect) {
  BOOST_ASSERT(container->find(instrument) != container->end());
  if (container->find(instrument) != container->end()) {
    (*container)[instrument]->CloseTraded(qty, position_effect);
  }
}

void CtpPositionRestorer::MakeCTPPositionAmountIfNeeed(
    std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
    const std::string& instrument) {
  if (container->find(instrument) == container->end()) {
    container->insert({instrument, std::make_unique<CTPPositionAmount>()});
  }
}

std::vector<CTPPositionField> CtpPositionRestorer::Result() const {
  std::vector<CTPPositionField> result;

  for (const auto& item : long_) {
    BOOST_ASSERT(item.second->Closeable() == item.second->Total());
    result.push_back(CTPPositionField{item.first, OrderDirection::kBuy,
                                      item.second->TodayCloseable(),
                                      item.second->YesterdayCloseable()});
  }

  for (const auto& item : short_) {
    BOOST_ASSERT(item.second->Closeable() == item.second->Total());
    result.push_back(CTPPositionField{item.first, OrderDirection::kSell,
                                      item.second->TodayCloseable(),
                                      item.second->YesterdayCloseable()});
  }

  return result;
}
