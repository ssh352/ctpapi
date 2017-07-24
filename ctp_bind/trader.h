#ifndef CTP_BIND_DEMO_TRADER_H
#define CTP_BIND_DEMO_TRADER_H
#include <atomic>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

namespace ctp_bind {
class Trader : public CThostFtdcTraderSpi {
  template <typename Field>
  struct CTPCallbackVisitor : boost::static_visitor<> {
    CTPCallbackVisitor(Field field,
                       CThostFtdcRspInfoField* rsp_info,
                       bool is_last)
        : field_(field), rsp_info_(rsp_info), is_last_(is_last) {}

    template <typename T>
    void operator()(T callback) const {
      std::cout << "";
    }

    void operator()(std::function<void(Field, CThostFtdcRspInfoField*, bool)>
                        callback) const {
      callback(field_, rsp_info_, is_last_);
    }

    Field field_;
    CThostFtdcRspInfoField* rsp_info_;
    bool is_last_;
  };

  struct CTPRspErrorCallbackVisitor : boost::static_visitor<> {
    CTPRspErrorCallbackVisitor(CThostFtdcRspInfoField* rsp_info, bool is_last)
        : rsp_info_(rsp_info), is_last_(is_last) {}

    template <typename Field>
    void operator()(std::function<void(Field, CThostFtdcRspInfoField*, bool)>
                        callback) const {
      callback(NULL, rsp_info_, is_last_);
    }

    CThostFtdcRspInfoField* rsp_info_;
    bool is_last_;
  };

  // virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
  // CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
  typedef boost::variant<
      std::function<
          void(CThostFtdcInputOrderField*, CThostFtdcRspInfoField*, bool)>,
      std::function<void(CThostFtdcInputOrderActionField*,
                         CThostFtdcRspInfoField*,
                         bool)> >
      CTPCallback;

 public:
  void Run() { io_service_.run(); }
  template <typename Fun, typename Field, typename Callback>
  void Request(Fun f, Field* field, Callback cb) {
    // CTPCallback c = cb;
    int request_id = request_id_.fetch_add(1);
    io_service_.post(
        [=](void) { ctp_callbacks_.insert(std::make_pair(request_id, cb)); });
    // boost::bind(f, api_, field, request_id)();
  }

  template <typename Field>
  auto MakeHandleResponse(Field field,
                          CThostFtdcRspInfoField rsp_info,
                          int request_id,
                          bool is_last) {
    return [ =, field{std::move(field)},
             rsp_info{std::move(rsp_info)} ](void) mutable {
      if (ctp_callbacks_.find(request_id) != ctp_callbacks_.end()) {
        boost::apply_visitor(
            CTPCallbackVisitor<Field*>{&field, &rsp_info, is_last},
            ctp_callbacks_[request_id]);
      }
    };
  }

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override {
    io_service_.post(MakeHandleResponse(CThostFtdcInputOrderField(*pInputOrder),
                                        CThostFtdcRspInfoField(*pRspInfo),
                                        nRequestID, bIsLast));
    /*
    io_service_.post([=](void) {
    });
    */
  }

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override {
    io_service_.post(MakeHandleResponse(
        CThostFtdcInputOrderActionField(*pInputOrderAction),
        CThostFtdcRspInfoField(*pRspInfo), nRequestID, bIsLast));
  }

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override {
    CThostFtdcRspInfoField rsp_info(*pRspInfo);
    io_service_.post([ =, rsp_info{std::move(rsp_info)} ](void) mutable {
      if (ctp_callbacks_.find(nRequestID) != ctp_callbacks_.end()) {
        boost::apply_visitor(CTPRspErrorCallbackVisitor(&rsp_info, bIsLast),
                             ctp_callbacks_[nRequestID]);
      }
    });
  }

  virtual void OnRspAuthenticate(
      CThostFtdcRspAuthenticateField* pRspAuthenticateField,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override;

  virtual void OnRspUserPasswordUpdate(
      CThostFtdcUserPasswordUpdateField* pUserPasswordUpdate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspTradingAccountPasswordUpdate(
      CThostFtdcTradingAccountPasswordUpdateField*
          pTradingAccountPasswordUpdate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField* pParkedOrder,
                                      CThostFtdcRspInfoField* pRspInfo,
                                      int nRequestID,
                                      bool bIsLast) override;

  virtual void OnRspParkedOrderAction(
      CThostFtdcParkedOrderActionField* pParkedOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQueryMaxOrderVolume(
      CThostFtdcQueryMaxOrderVolumeField* pQueryMaxOrderVolume,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspSettlementInfoConfirm(
      CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspRemoveParkedOrder(
      CThostFtdcRemoveParkedOrderField* pRemoveParkedOrder,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspRemoveParkedOrderAction(
      CThostFtdcRemoveParkedOrderActionField* pRemoveParkedOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspExecOrderInsert(
      CThostFtdcInputExecOrderField* pInputExecOrder,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspExecOrderAction(
      CThostFtdcInputExecOrderActionField* pInputExecOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspForQuoteInsert(CThostFtdcInputForQuoteField* pInputForQuote,
                                   CThostFtdcRspInfoField* pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) override;

  virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField* pInputQuote,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspQuoteAction(
      CThostFtdcInputQuoteActionField* pInputQuoteAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspBatchOrderAction(
      CThostFtdcInputBatchOrderActionField* pInputBatchOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspCombActionInsert(
      CThostFtdcInputCombActionField* pInputCombAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast) override;

  virtual void OnRspQryTrade(CThostFtdcTradeField* pTrade,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast) override;

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryTradingAccount(
      CThostFtdcTradingAccountField* pTradingAccount,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryInvestor(CThostFtdcInvestorField* pInvestor,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField* pTradingCode,
                                   CThostFtdcRspInfoField* pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) override;

  virtual void OnRspQryInstrumentMarginRate(
      CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryInstrumentCommissionRate(
      CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryExchange(CThostFtdcExchangeField* pExchange,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspQryProduct(CThostFtdcProductField* pProduct,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override;

  virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                  CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) override;

  virtual void OnRspQryDepthMarketData(
      CThostFtdcDepthMarketDataField* pDepthMarketData,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQrySettlementInfo(
      CThostFtdcSettlementInfoField* pSettlementInfo,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryTransferBank(CThostFtdcTransferBankField* pTransferBank,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) override;

  virtual void OnRspQryInvestorPositionDetail(
      CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryNotice(CThostFtdcNoticeField* pNotice,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspQrySettlementInfoConfirm(
      CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryInvestorPositionCombineDetail(
      CThostFtdcInvestorPositionCombineDetailField*
          pInvestorPositionCombineDetail,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryCFMMCTradingAccountKey(
      CThostFtdcCFMMCTradingAccountKeyField* pCFMMCTradingAccountKey,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryEWarrantOffset(
      CThostFtdcEWarrantOffsetField* pEWarrantOffset,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryInvestorProductGroupMargin(
      CThostFtdcInvestorProductGroupMarginField* pInvestorProductGroupMargin,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryExchangeMarginRate(
      CThostFtdcExchangeMarginRateField* pExchangeMarginRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryExchangeMarginRateAdjust(
      CThostFtdcExchangeMarginRateAdjustField* pExchangeMarginRateAdjust,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryExchangeRate(CThostFtdcExchangeRateField* pExchangeRate,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) override;

  virtual void OnRspQrySecAgentACIDMap(
      CThostFtdcSecAgentACIDMapField* pSecAgentACIDMap,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryProductExchRate(
      CThostFtdcProductExchRateField* pProductExchRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryProductGroup(CThostFtdcProductGroupField* pProductGroup,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) override;

  virtual void OnRspQryMMInstrumentCommissionRate(
      CThostFtdcMMInstrumentCommissionRateField* pMMInstrumentCommissionRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryMMOptionInstrCommRate(
      CThostFtdcMMOptionInstrCommRateField* pMMOptionInstrCommRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryInstrumentOrderCommRate(
      CThostFtdcInstrumentOrderCommRateField* pInstrumentOrderCommRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryOptionInstrTradeCost(
      CThostFtdcOptionInstrTradeCostField* pOptionInstrTradeCost,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryOptionInstrCommRate(
      CThostFtdcOptionInstrCommRateField* pOptionInstrCommRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryExecOrder(CThostFtdcExecOrderField* pExecOrder,
                                 CThostFtdcRspInfoField* pRspInfo,
                                 int nRequestID,
                                 bool bIsLast) override;

  virtual void OnRspQryForQuote(CThostFtdcForQuoteField* pForQuote,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspQryQuote(CThostFtdcQuoteField* pQuote,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast) override;

  virtual void OnRspQryCombInstrumentGuard(
      CThostFtdcCombInstrumentGuardField* pCombInstrumentGuard,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryCombAction(CThostFtdcCombActionField* pCombAction,
                                  CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) override;

  virtual void OnRspQryTransferSerial(
      CThostFtdcTransferSerialField* pTransferSerial,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryAccountregister(
      CThostFtdcAccountregisterField* pAccountregister,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryContractBank(CThostFtdcContractBankField* pContractBank,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) override;

  virtual void OnRspQryParkedOrder(CThostFtdcParkedOrderField* pParkedOrder,
                                   CThostFtdcRspInfoField* pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) override;

  virtual void OnRspQryParkedOrderAction(
      CThostFtdcParkedOrderActionField* pParkedOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryTradingNotice(
      CThostFtdcTradingNoticeField* pTradingNotice,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryBrokerTradingParams(
      CThostFtdcBrokerTradingParamsField* pBrokerTradingParams,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryBrokerTradingAlgos(
      CThostFtdcBrokerTradingAlgosField* pBrokerTradingAlgos,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQueryCFMMCTradingAccountToken(
      CThostFtdcQueryCFMMCTradingAccountTokenField*
          pQueryCFMMCTradingAccountToken,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspFromBankToFutureByFuture(
      CThostFtdcReqTransferField* pReqTransfer,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspFromFutureToBankByFuture(
      CThostFtdcReqTransferField* pReqTransfer,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQueryBankAccountMoneyByFuture(
      CThostFtdcReqQueryAccountField* pReqQueryAccount,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

 private:
  std::atomic<int> request_id_ = 0;
  std::map<int, CTPCallback> ctp_callbacks_;
  CThostFtdcTraderApi* api_;
  boost::asio::io_service io_service_;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_TRADER_H
