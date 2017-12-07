#include "util.h"
#include <windows.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

boost::filesystem::path CreateDirectoryIfNoExist(const std::string& sub_dir) {
  //boost::filesystem::path dir{GetExecuableFileDirectoryPath()};
  //dir /= sub_dir;
  boost::filesystem::path dir{sub_dir};
  //dir /= sub_dir;
  if (!boost::filesystem::exists(dir) &&
      !boost::filesystem::create_directory(dir)) {
    std::cout << "create directory error\n";
  }
  return dir;
}

std::string GetExecuableFileDirectoryPath() {
  char path_buffer[MAX_PATH] = {0};
  GetModuleFileNameA(NULL, path_buffer, MAX_PATH);
  boost::filesystem::path file(path_buffer);
  return file.parent_path().string();
}

void ClearUpCTPFolwDirectory(const std::string& flow_path) {
  boost::filesystem::path dir = CreateDirectoryIfNoExist(flow_path);
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator it(dir); it != end_iter; ++it) {
    boost::filesystem::remove_all(it->path());
  }
}
