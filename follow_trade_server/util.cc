#include "util.h"
#include <iostream>
#include <windows.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

std::string MakeDataBinaryFileName(const std::string& slave_account,
                                   const std::string& sub_dir) {
  char path_buffer[MAX_PATH] = {0};
  GetModuleFileNameA(NULL, path_buffer, MAX_PATH);
  boost::filesystem::path file(path_buffer);
  boost::filesystem::path dir = file.parent_path();
  dir /= sub_dir;
  if (!boost::filesystem::exists(dir) &&
      !boost::filesystem::create_directory(dir)) {
    std::cout << "create directory error\n";
  }

  auto time = boost::posix_time::second_clock::local_time();
  struct ::tm tm_time = boost::posix_time::to_tm(time);
  std::ostringstream time_pid_stream;
  time_pid_stream.fill('0');
  time_pid_stream << 1900 + tm_time.tm_year << std::setw(2)
                  << 1 + tm_time.tm_mon << std::setw(2) << tm_time.tm_mday
                  << '-' << std::setw(2) << tm_time.tm_hour << std::setw(2)
                  << tm_time.tm_min << std::setw(2) << tm_time.tm_sec;
  const std::string& time_pid_string = time_pid_stream.str();

  return dir.string() + "\\" + slave_account + '-' + time_pid_string + ".bin";
}
