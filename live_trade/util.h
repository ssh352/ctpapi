#ifndef FOLLOW_TRADE_SERVER_UTIL_H
#define FOLLOW_TRADE_SERVER_UTIL_H
#include <string>

std::string GetExecuableFileDirectoryPath();

void ClearUpCTPFolwDirectory(const std::string& flow_path);

#endif  // FOLLOW_TRADE_SERVER_UTIL_H
