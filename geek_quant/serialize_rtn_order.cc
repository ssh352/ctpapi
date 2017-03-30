#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include "geek_quant/serialize_ctp_trader.h"

/*


void LoadRtnOrder() {
  std::ifstream file("rtn_order_20170327.txt");

  std::vector<CThostFtdcOrderField> orders;
  try {
    boost::archive::text_iarchive ia(file);
    while (true) {
      CThostFtdcOrderField order;
      memset(&order, 0, sizeof(CThostFtdcOrderField));
      ia >> order;
      orders.push_back(order);
      std::cout << orders.size() << "\n";
    }
  } catch (boost::archive::archive_exception& exp) {
    std::cout << "exception " << exp.what() << "\n";
  }

  std::cout << "load\n";
}

std::ofstream MakeFileStream() {
  std::ofstream stream("38030022_rtn_order.csv");
  stream << "OrderNo,"
         << "Instrument,"
         << "OrderDirection,"
         << "OrderStatus,"
         << "OrderPrice,"
         << "Volume\n";
  return stream;
}

void TestSerialize() {
  std::ofstream out_stream = MakeFileStream();
  std::ifstream
file("c:\\Users\\yjqpro\\Desktop\\cta_0322\\38030022_20170327_rtn_order.txt");
  std::vector<CThostFtdcOrderField> orders;
  try {
    boost::archive::text_iarchive ia(file);
    while (true) {
      CThostFtdcOrderField order;
      memset(&order, 0, sizeof(CThostFtdcOrderField));
      ia >> order;
      orders.push_back(order);
    }
  } catch (boost::archive::archive_exception& exp) {
    std::cout << "exception " << exp.what() << "\n";
  }

  CtpOrderDispatcher ctp_order_dispatcher;
  for (auto order : orders) {
    if (auto order_opt = ctp_order_dispatcher.HandleRtnOrder(order)) {
      out_stream << order_opt->order_no << "," << order_opt->instrument << ","
                 << order_opt->order_direction << "," << order_opt->order_status
                 << "," << order_opt->order_price << "," << order_opt->volume
                 << "\n";
    }
  }
}

*/

void RtnOrderSerializeToCsvFile(const std::string& in_file,
                                const std::string& out_file) {
  std::ofstream out(out_file);
  out << "BrokerID,"
      << "InvestorID,"
      << "InstrumentID,"
      << "OrderRef,"
      << "UserID,"
      << "OrderPriceType,"
      << "Direction,"
      << "CombOffsetFlag,"
      << "CombHedgeFlag,"
      << "LimitPrice,"
      << "VolumeTotalOriginal,"
      << "TimeCondition,"
      << "GTDDate,"
      << "VolumeCondition,"
      << "MinVolume,"
      << "ContingentCondition,"
      << "StopPrice,"
      << "ForceCloseReason,"
      << "IsAutoSuspend,"
      << "BusinessUnit,"
      << "RequestID,"
      << "OrderLocalID,"
      << "ExchangeID,"
      << "ParticipantID,"
      << "ClientID,"
      << "ExchangeInstID,"
      << "TraderID,"
      << "InstallID,"
      << "OrderSubmitStatus,"
      << "NotifySequence,"
      << "TradingDay,"
      << "SettlementID,"
      << "OrderSysID,"
      << "OrderSource,"
      << "OrderStatus,"
      << "OrderType,"
      << "VolumeTraded,"
      << "VolumeTotal,"
      << "InsertDate,"
      << "InsertTime,"
      << "ActiveTime,"
      << "SuspendTime,"
      << "UpdateTime,"
      << "CancelTime,"
      << "ActiveTraderID,"
      << "ClearingPartID,"
      << "SequenceNo,"
      << "FrontID,"
      << "SessionID,"
      << "UserProductInfo,"
      << "StatusMsg,"
      << "UserForceClose,"
      << "ActiveUserID,"
      << "BrokerOrderSeq,"
      << "RelativeOrderSysID,"
      << "ZCETotalTradedVolume,"
      << "IsSwapOrder,"
      << "BranchID,"
      << "InvestUnitID,"
      << "AccountID,"
      << "CurrencyID,"
      << "IPAddress,"
      << "MacAddress"
      << "\n";

  std::ifstream in(in_file);

  std::vector<CThostFtdcOrderField> orders;
  try {
    boost::archive::text_iarchive ia(in);
    while (true) {
      CThostFtdcOrderField order;
      memset(&order, 0, sizeof(CThostFtdcOrderField));
      ia >> order;
      orders.push_back(std::move(order));
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }

  for (auto order : orders) {
    out << boost::lexical_cast<std::string>(order.BrokerID) << ","
        << boost::lexical_cast<std::string>(order.InvestorID) << ","
        << boost::lexical_cast<std::string>(order.InstrumentID) << ","
        << boost::lexical_cast<std::string>(order.OrderRef) << ","
        << boost::lexical_cast<std::string>(order.UserID) << ","
        << boost::lexical_cast<std::string>(order.OrderPriceType) << ","
        << boost::lexical_cast<std::string>(order.Direction) << ","
        << boost::lexical_cast<std::string>(order.CombOffsetFlag) << ","
        << boost::lexical_cast<std::string>(order.CombHedgeFlag) << ","
        << boost::lexical_cast<std::string>(order.LimitPrice) << ","
        << boost::lexical_cast<std::string>(order.VolumeTotalOriginal) << ","
        << boost::lexical_cast<std::string>(order.TimeCondition) << ","
        << boost::lexical_cast<std::string>(order.GTDDate) << ","
        << boost::lexical_cast<std::string>(order.VolumeCondition) << ","
        << boost::lexical_cast<std::string>(order.MinVolume) << ","
        << boost::lexical_cast<std::string>(order.ContingentCondition) << ","
        << boost::lexical_cast<std::string>(order.StopPrice) << ","
        << boost::lexical_cast<std::string>(order.ForceCloseReason) << ","
        << boost::lexical_cast<std::string>(order.IsAutoSuspend) << ","
        << boost::lexical_cast<std::string>(order.BusinessUnit) << ","
        << boost::lexical_cast<std::string>(order.RequestID) << ","
        << boost::lexical_cast<std::string>(order.OrderLocalID) << ","
        << boost::lexical_cast<std::string>(order.ExchangeID) << ","
        << boost::lexical_cast<std::string>(order.ParticipantID) << ","
        << boost::lexical_cast<std::string>(order.ClientID) << ","
        << boost::lexical_cast<std::string>(order.ExchangeInstID) << ","
        << boost::lexical_cast<std::string>(order.TraderID) << ","
        << boost::lexical_cast<std::string>(order.InstallID) << ","
        << boost::lexical_cast<std::string>(order.OrderSubmitStatus) << ","
        << boost::lexical_cast<std::string>(order.NotifySequence) << ","
        << boost::lexical_cast<std::string>(order.TradingDay) << ","
        << boost::lexical_cast<std::string>(order.SettlementID) << ","
        << boost::lexical_cast<std::string>(order.OrderSysID) << ","
        << boost::lexical_cast<std::string>(order.OrderSource) << ","
        << boost::lexical_cast<std::string>(order.OrderStatus) << ","
        << boost::lexical_cast<std::string>(order.OrderType) << ","
        << boost::lexical_cast<std::string>(order.VolumeTraded) << ","
        << boost::lexical_cast<std::string>(order.VolumeTotal) << ","
        << boost::lexical_cast<std::string>(order.InsertDate) << ","
        << boost::lexical_cast<std::string>(order.InsertTime) << ","
        << boost::lexical_cast<std::string>(order.ActiveTime) << ","
        << boost::lexical_cast<std::string>(order.SuspendTime) << ","
        << boost::lexical_cast<std::string>(order.UpdateTime) << ","
        << boost::lexical_cast<std::string>(order.CancelTime) << ","
        << boost::lexical_cast<std::string>(order.ActiveTraderID) << ","
        << boost::lexical_cast<std::string>(order.ClearingPartID) << ","
        << boost::lexical_cast<std::string>(order.SequenceNo) << ","
        << boost::lexical_cast<std::string>(order.FrontID) << ","
        << boost::lexical_cast<std::string>(order.SessionID) << ","
        << boost::lexical_cast<std::string>(order.UserProductInfo) << ","
        << boost::lexical_cast<std::string>(order.StatusMsg) << ","
        << boost::lexical_cast<std::string>(order.UserForceClose) << ","
        << boost::lexical_cast<std::string>(order.ActiveUserID) << ","
        << boost::lexical_cast<std::string>(order.BrokerOrderSeq) << ","
        << boost::lexical_cast<std::string>(order.RelativeOrderSysID) << ","
        << boost::lexical_cast<std::string>(order.ZCETotalTradedVolume) << ","
        << boost::lexical_cast<std::string>(order.IsSwapOrder) << ","
        << boost::lexical_cast<std::string>(order.BranchID) << ","
        << boost::lexical_cast<std::string>(order.InvestUnitID) << ","
        << boost::lexical_cast<std::string>(order.AccountID) << ","
        << boost::lexical_cast<std::string>(order.CurrencyID) << ","
        << boost::lexical_cast<std::string>(order.IPAddress) << ","
        << boost::lexical_cast<std::string>(order.MacAddress) << "\n";
  }
}

void ErrRtnOrderInsertSerializeToCsvFile(const std::string& in_file,
                                         const std::string& out_file) {
  std::ofstream out(out_file);
  out << "BrokerID,"
      << "InvestorID,"
      << "InstrumentID,"
      << "OrderRef,"
      << "UserID,"
      << "OrderPriceType,"
      << "Direction,"
      << "CombOffsetFlag,"
      << "CombHedgeFlag,"
      << "LimitPrice,"
      << "VolumeTotalOriginal,"
      << "TimeCondition,"
      << "GTDDate,"
      << "VolumeCondition,"
      << "MinVolume,"
      << "ContingentCondition,"
      << "StopPrice,"
      << "ForceCloseReason,"
      << "IsAutoSuspend,"
      << "BusinessUnit,"
      << "RequestID,"
      << "UserForceClose,"
      << "IsSwapOrder,"
      << "ExchangeID,"
      << "InvestUnitID,"
      << "AccountID,"
      << "CurrencyID,"
      << "ClientID,"
      << "IPAddress,"
      << "MacAddress,"
      << "ErrorID,"
      << "ErrorMsg"
      << "\n";

  std::ifstream in(in_file);

  std::vector<std::pair<CThostFtdcInputOrderField, CThostFtdcRspInfoField> >
      orders;
  try {
    boost::archive::text_iarchive ia(in);
    while (true) {
      CThostFtdcInputOrderField order = {0};
      CThostFtdcRspInfoField err = {0};
      ia >> order;
      ia >> err;
      orders.push_back(std::make_pair(order, err));
    }
  } catch (boost::archive::archive_exception&) {
    std::cout << "Done\n";
  }

  for (auto item : orders) {
    out << boost::lexical_cast<std::string>(item.first.BrokerID) << ","
        << boost::lexical_cast<std::string>(item.first.InvestorID) << ","
        << boost::lexical_cast<std::string>(item.first.InstrumentID) << ","
        << boost::lexical_cast<std::string>(item.first.OrderRef) << ","
        << boost::lexical_cast<std::string>(item.first.UserID) << ","
        << boost::lexical_cast<std::string>(item.first.OrderPriceType) << ","
        << boost::lexical_cast<std::string>(item.first.Direction) << ","
        << boost::lexical_cast<std::string>(item.first.CombOffsetFlag) << ","
        << boost::lexical_cast<std::string>(item.first.CombHedgeFlag) << ","
        << boost::lexical_cast<std::string>(item.first.LimitPrice) << ","
        << boost::lexical_cast<std::string>(item.first.VolumeTotalOriginal)
        << "," << boost::lexical_cast<std::string>(item.first.TimeCondition)
        << "," << boost::lexical_cast<std::string>(item.first.GTDDate) << ","
        << boost::lexical_cast<std::string>(item.first.VolumeCondition) << ","
        << boost::lexical_cast<std::string>(item.first.MinVolume) << ","
        << boost::lexical_cast<std::string>(item.first.ContingentCondition)
        << "," << boost::lexical_cast<std::string>(item.first.StopPrice) << ","
        << boost::lexical_cast<std::string>(item.first.ForceCloseReason) << ","
        << boost::lexical_cast<std::string>(item.first.IsAutoSuspend) << ","
        << boost::lexical_cast<std::string>(item.first.BusinessUnit) << ","
        << boost::lexical_cast<std::string>(item.first.RequestID) << ","
        << boost::lexical_cast<std::string>(item.first.UserForceClose) << ","
        << boost::lexical_cast<std::string>(item.first.IsSwapOrder) << ","
        << boost::lexical_cast<std::string>(item.first.ExchangeID) << ","
        << boost::lexical_cast<std::string>(item.first.InvestUnitID) << ","
        << boost::lexical_cast<std::string>(item.first.AccountID) << ","
        << boost::lexical_cast<std::string>(item.first.CurrencyID) << ","
        << boost::lexical_cast<std::string>(item.first.ClientID) << ","
        << boost::lexical_cast<std::string>(item.first.IPAddress) << ","
        << boost::lexical_cast<std::string>(item.first.MacAddress) << ","
        << boost::lexical_cast<std::string>(item.second.ErrorID) << ","
        << boost::lexical_cast<std::string>(item.second.ErrorMsg) << "\n";
  }
}

void InverstorPositionSerializeToCsvFile(const std::string& in_file,
                                         const std::string& out_file) {
  std::ifstream in(in_file);
  std::vector<CThostFtdcInvestorPositionField> positions;
  try {
    boost::archive::text_iarchive ia(in);
    while (true) {
      CThostFtdcInvestorPositionField pos;
      ia >> pos;
      positions.push_back(std::move(pos));
    }
  } catch (boost::archive::archive_exception&) {
    std::cout << "Done\n";
  }

  std::ofstream out(out_file);

  out << "InstrumentID,"
      << "BrokerID,"
      << "InvestorID,"
      << "PosiDirection,"
      << "HedgeFlag,"
      << "PositionDate,"
      << "YdPosition,"
      << "Position,"
      << "LongFrozen,"
      << "ShortFrozen,"
      << "LongFrozenAmount,"
      << "ShortFrozenAmount,"
      << "OpenVolume,"
      << "CloseVolume,"
      << "OpenAmount,"
      << "CloseAmount,"
      << "PositionCost,"
      << "PreMargin,"
      << "UseMargin,"
      << "FrozenMargin,"
      << "FrozenCash,"
      << "FrozenCommission,"
      << "CashIn,"
      << "Commission,"
      << "CloseProfit,"
      << "PositionProfit,"
      << "PreSettlementPrice,"
      << "SettlementPrice,"
      << "TradingDay,"
      << "SettlementID,"
      << "OpenCost,"
      << "ExchangeMargin,"
      << "CombPosition,"
      << "CombLongFrozen,"
      << "CombShortFrozen,"
      << "CloseProfitByDate,"
      << "CloseProfitByTrade,"
      << "TodayPosition,"
      << "MarginRateByMoney,"
      << "MarginRateByVolume,"
      << "StrikeFrozen,"
      << "StrikeFrozenAmount,"
      << "AbandonFrozen"
      << "\n";

  for (auto pos : positions) {
    out << boost::lexical_cast<std::string>(pos.InstrumentID) << ","
        << boost::lexical_cast<std::string>(pos.BrokerID) << ","
        << boost::lexical_cast<std::string>(pos.InvestorID) << ","
        << boost::lexical_cast<std::string>(pos.PosiDirection) << ","
        << boost::lexical_cast<std::string>(pos.HedgeFlag) << ","
        << boost::lexical_cast<std::string>(pos.PositionDate) << ","
        << boost::lexical_cast<std::string>(pos.YdPosition) << ","
        << boost::lexical_cast<std::string>(pos.Position) << ","
        << boost::lexical_cast<std::string>(pos.LongFrozen) << ","
        << boost::lexical_cast<std::string>(pos.ShortFrozen) << ","
        << boost::lexical_cast<std::string>(pos.LongFrozenAmount) << ","
        << boost::lexical_cast<std::string>(pos.ShortFrozenAmount) << ","
        << boost::lexical_cast<std::string>(pos.OpenVolume) << ","
        << boost::lexical_cast<std::string>(pos.CloseVolume) << ","
        << boost::lexical_cast<std::string>(pos.OpenAmount) << ","
        << boost::lexical_cast<std::string>(pos.CloseAmount) << ","
        << boost::lexical_cast<std::string>(pos.PositionCost) << ","
        << boost::lexical_cast<std::string>(pos.PreMargin) << ","
        << boost::lexical_cast<std::string>(pos.UseMargin) << ","
        << boost::lexical_cast<std::string>(pos.FrozenMargin) << ","
        << boost::lexical_cast<std::string>(pos.FrozenCash) << ","
        << boost::lexical_cast<std::string>(pos.FrozenCommission) << ","
        << boost::lexical_cast<std::string>(pos.CashIn) << ","
        << boost::lexical_cast<std::string>(pos.Commission) << ","
        << boost::lexical_cast<std::string>(pos.CloseProfit) << ","
        << boost::lexical_cast<std::string>(pos.PositionProfit) << ","
        << boost::lexical_cast<std::string>(pos.PreSettlementPrice) << ","
        << boost::lexical_cast<std::string>(pos.SettlementPrice) << ","
        << boost::lexical_cast<std::string>(pos.TradingDay) << ","
        << boost::lexical_cast<std::string>(pos.SettlementID) << ","
        << boost::lexical_cast<std::string>(pos.OpenCost) << ","
        << boost::lexical_cast<std::string>(pos.ExchangeMargin) << ","
        << boost::lexical_cast<std::string>(pos.CombPosition) << ","
        << boost::lexical_cast<std::string>(pos.CombLongFrozen) << ","
        << boost::lexical_cast<std::string>(pos.CombShortFrozen) << ","
        << boost::lexical_cast<std::string>(pos.CloseProfitByDate) << ","
        << boost::lexical_cast<std::string>(pos.CloseProfitByTrade) << ","
        << boost::lexical_cast<std::string>(pos.TodayPosition) << ","
        << boost::lexical_cast<std::string>(pos.MarginRateByMoney) << ","
        << boost::lexical_cast<std::string>(pos.MarginRateByVolume) << ","
        << boost::lexical_cast<std::string>(pos.StrikeFrozen) << ","
        << boost::lexical_cast<std::string>(pos.StrikeFrozenAmount) << ","
        << boost::lexical_cast<std::string>(pos.AbandonFrozen) << "\n";
  }
}

int main(int argc, char* argv[]) {
  std::string path = "c:\\Users\\yjqpro\\Desktop\\20170330_day\\";
  std::string prefix = "120350655_20170330_day_";
  // RtnOrderSerializeToCsvFile(
  //     path + prefix + "rtn_order.txt",
  //     path + prefix + "rtn_order.csv");
  // RtnOrderSerializeToCsvFile(
  //     path + "38030022_20170327_night_rtn_order.txt",
  //     path + "38030022_20170327_night_rtn_order.csv");

  // ErrRtnOrderInsertSerializeToCsvFile(
  //    path + "053861_20170327_night_err_rtn_order_insert.txt",
  //    path + "053861_20170327_night_err_rtn_order_insert.csv");

  // InverstorPositionSerializeToCsvFile(
  //     path + "053861_20170327_night_inverstor_position_file.txt",
  //     path + "053861_20170327_night_inverstor_position_file.csv");

  // ErrRtnOrderInsertSerializeToCsvFile(
  //     "c:\\Users\\yjqpro\\Desktop\\cta_0322\\053861_20170327_err_rtn_order_"
  //     "insert.txt",
  //     "c:\\Users\\yjqpro\\Desktop\\cta_0322\\053861_20170327_err_rtn_order_"
  //     "insert.csv");

  // SerializeCtpTrader* ctp = new SerializeCtpTrader("38030022_20170330_day");
  // ctp->LoginServer("tcp://59.42.241.91:41205", "9080", "38030022", "140616");

  // SerializeCtpTrader* ctp = new SerializeCtpTrader("053861_20170327_night_");
  // ctp->LoginServer("tcp://180.168.146.187:10000", "9999", "053861",
  //                  "Cj12345678");

  SerializeCtpTrader* ctp = new SerializeCtpTrader("120350655_20170330_day");
  ctp->LoginServer("tcp://ctp1-front3.citicsf.com:41205", "66666", "120350655",
                   "140616");

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  // TestSerialize();
  return 0;
}
