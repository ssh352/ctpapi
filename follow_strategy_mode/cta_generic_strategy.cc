#include "cta_generic_strategy.h"

CTAGenericStrategy::CTAGenericStrategy(std::string strategy_id)
    : strategy_id_(std::move(strategy_id)) {}

void CTAGenericStrategy::Subscribe(
    StrategyEnterOrderObservable::Observer* observer) {
  observer_ = observer;
}

void CTAGenericStrategy::OpenOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   OrderDirection direction,
                                   double price,
                                   int quantity) {
  observer_->OpenOrder(strategy_id_, instrument, order_id, direction, price,
                       quantity);
}

void CTAGenericStrategy::CloseOrder(const std::string& instrument,
                                    const std::string& order_id,
                                    OrderDirection direction,
                                    PositionEffect position_effect,
                                    double price,
                                    int quantity) {
  observer_->CloseOrder(strategy_id_, instrument, order_id, direction,
                        position_effect, price, quantity);
}

void CTAGenericStrategy::CancelOrder(const std::string& order_id) {
  observer_->CancelOrder(strategy_id_, order_id);
}
