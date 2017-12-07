#ifndef LIVE_TRADE_COMMON_UTIL_H
#define LIVE_TRADE_COMMON_UTIL_H
#include <string>

std::string DateTimeSubfix();

std::string MakeFileNameWithDateTimeSubfix(const std::string& dir,
                                     const std::string& file_name,
                                     const std::string& extension);


std::string GetExecuableFileDirectoryPath();

void ClearUpCTPFolwDirectory(const std::string& flow_path);


#endif  // LIVE_TRADE_COMMON_UTIL_H
