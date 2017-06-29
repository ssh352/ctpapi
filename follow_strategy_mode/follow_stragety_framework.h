#ifndef FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
#define FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H

class FollowStrategyFramework {
 public:
  FollowStrategyFramework();

  void HandleRtnOrder(OrderData rtn_order);

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

 private:
  class StragetyWrap : public FollowStragetyService::Delegate {
   public:
    StragetyWrap(int start_seq) : start_seq_(start_seq) {}

    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           OrderPriceType price_type,
                           double price,
                           int quantity) override;

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            OrderPriceType price_type,
                            double price,
                            int quantity) override;

    virtual void CancelOrder(const std::string& order_no) override;

   private:
    int start_seq_;
  };
  struct StragetyIndex {
    std::string order_id;
    std::string stragety_id;
  };
  std::map<std::string, std::string> master_ids_;
  std::map<std::string, StragetyIndex> slave_order_id_to_stragety_;
  std::vector<boost::shared_ptr<FollowStragetyService> > services_;
  std::map<std::string, std::string> stragety_ids_;
  std::string master_account_id_;
  std::string slave_account_id_;
};

#endif  // FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
