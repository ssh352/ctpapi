#ifndef JSON_H
#define JSON_H
#include <string>
#include <fstream>
#include "ctpapi/ThostFtdcUserApiStruct.h"

void ToJson(std::ofstream& os, CThostFtdcOrderField& order);
void ToJson(std::ofstream& os, CThostFtdcTradeField& order);

#endif // _DEBUG
