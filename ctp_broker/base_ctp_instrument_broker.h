#ifndef CTP_BROKER_BASE_CTP_INSTRUMENT_BROKER_H
#define CTP_BROKER_BASE_CTP_INSTRUMENT_BROKER_H


class BaseCTPInstrumentBroker {
public:
  void HandleRtnOrder(const std::shared_ptr<CTPOrderField>& order);

  void HandleInputOrder(const InputOrder& order);

  void HandleCancel(const CancelOrderSignal& cancel);

private:
};

#endif // CTP_BROKER_BASE_CTP_INSTRUMENT_BROKER_H



