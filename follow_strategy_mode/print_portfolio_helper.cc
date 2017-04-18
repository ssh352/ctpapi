#include "print_portfolio_helper.h"
#include <boost/lexical_cast.hpp>

/*Header file to color text and background in windows console applications
Global variables - textcol,backcol,deftextcol,defbackcol,colorprotect*/

#include <windows.h>
#include <iosfwd>

#ifndef CONCOL
#define CONCOL
enum class concol {
  black = 0,
  dark_blue = 1,
  dark_green = 2,
  dark_aqua,
  dark_cyan = 3,
  dark_red = 4,
  dark_purple = 5,
  dark_pink = 5,
  dark_magenta = 5,
  dark_yellow = 6,
  dark_white = 7,
  gray = 8,
  blue = 9,
  green = 10,
  aqua = 11,
  cyan = 11,
  red = 12,
  purple = 13,
  pink = 13,
  magenta = 13,
  yellow = 14,
  white = 15
};
#endif  // CONCOL

HANDLE std_con_out;
// Standard Output Handle
bool colorprotect = false;
// If colorprotect is true, background and text colors will never be the same
concol textcol, backcol, deftextcol, defbackcol;
/*textcol - current text color
backcol - current back color
deftextcol - original text color
defbackcol - original back color*/

inline void update_colors() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(std_con_out, &csbi);
  textcol = concol(csbi.wAttributes & 15);
  backcol = concol((csbi.wAttributes & 0xf0) >> 4);
}

inline void setcolor(concol textcolor, concol backcolor) {
  if (colorprotect && textcolor == backcolor)
    return;
  textcol = textcolor;
  backcol = backcolor;
  unsigned short wAttributes =
      ((unsigned int)backcol << 4) | (unsigned int)textcol;
  SetConsoleTextAttribute(std_con_out, wAttributes);
}

inline void settextcolor(concol textcolor) {
  if (colorprotect && textcolor == backcol)
    return;
  textcol = textcolor;
  unsigned short wAttributes =
      ((unsigned int)backcol << 4) | (unsigned int)textcol;
  SetConsoleTextAttribute(std_con_out, wAttributes);
}

inline void setbackcolor(concol backcolor) {
  if (colorprotect && textcol == backcolor)
    return;
  backcol = backcolor;
  unsigned short wAttributes =
      ((unsigned int)backcol << 4) | (unsigned int)textcol;
  SetConsoleTextAttribute(std_con_out, wAttributes);
}

inline void concolinit() {
  std_con_out = GetStdHandle(STD_OUTPUT_HANDLE);
  update_colors();
  deftextcol = textcol;
  defbackcol = backcol;
}

template <class elem, class traits>
inline std::basic_ostream<elem, traits>& operator<<(
    std::basic_ostream<elem, traits>& os,
    concol col) {
  os.flush();
  settextcolor(col);
  return os;
}

template <class elem, class traits>
inline std::basic_istream<elem, traits>& operator>>(
    std::basic_istream<elem, traits>& is,
    concol col) {
  std::basic_ostream<elem, traits>* p = is.tie();
  if (p != NULL)
    p->flush();
  settextcolor(col);
  return is;
}

template <typename charT, typename traits = std::char_traits<charT> >
class center_helper {
  std::basic_string<charT, traits> str_;

 public:
  center_helper(std::basic_string<charT, traits> str) : str_(str) {}
  template <typename a, typename b>
  friend std::basic_ostream<a, b>& operator<<(std::basic_ostream<a, b>& s,
                                              const center_helper<a, b>& c);
};

template <typename charT, typename traits = std::char_traits<charT> >
center_helper<charT, traits> centered(std::basic_string<charT, traits> str) {
  return center_helper<charT, traits>(str);
}

// redeclare for std::string directly so we can support anything that implicitly
// converts to std::string
center_helper<std::string::value_type, std::string::traits_type> centered(
    const std::string& str) {
  return center_helper<std::string::value_type, std::string::traits_type>(str);
}

template <typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(
    std::basic_ostream<charT, traits>& s,
    const center_helper<charT, traits>& c) {
  std::streamsize w = s.width();
  if (w > static_cast<std::streamsize>(c.str_.length())) {
    std::streamsize left = (w + c.str_.length()) / 2;
    s.width(left);
    s << c.str_;
    s.width(w - left);
    s << "";
  } else {
    s << c.str_;
  }
  return s;
}

std::string FormatQuantityHelper(int left, int right) {
  return boost::lexical_cast<std::string>(left) + "(" +
         boost::lexical_cast<std::string>(right) + ")";
}

std::string FormatPortfolio(std::string account_id,
                            std::vector<AccountPortfolio> master_portfolios,
                            std::vector<AccountPortfolio> slave_portfolios,
                            bool fully) {
  std::map<std::pair<std::string, OrderDirection>, AccountPortfolio>
      master_account_portfolio;
  std::set<std::pair<std::string, OrderDirection> > keys;
  for (auto portfolio : master_portfolios) {
    std::pair<std::string, OrderDirection> key = {portfolio.instrument,
                                                  portfolio.direction};
    master_account_portfolio[key] = portfolio;
    keys.insert(key);
  }

  std::map<std::pair<std::string, OrderDirection>, AccountPortfolio>
      slave_account_portfolio;
  for (auto portfolio : slave_portfolios) {
    std::pair<std::string, OrderDirection> key = {portfolio.instrument,
                                                  portfolio.direction};
    slave_account_portfolio[key] = portfolio;
    keys.insert(key);
  }

  std::vector<std::tuple<bool, std::string, OrderDirection, std::string,
                         std::string, std::string> >
      restuls;
  for (auto key : keys) {
    if (master_account_portfolio.find(key) != master_account_portfolio.end() &&
        slave_account_portfolio.find(key) != slave_account_portfolio.end()) {
      bool equal_quantity = master_account_portfolio[key].closeable ==
                                slave_account_portfolio[key].closeable &&
                            master_account_portfolio[key].open ==
                                slave_account_portfolio[key].open &&
                            master_account_portfolio[key].close ==
                                slave_account_portfolio[key].close;

      if (fully || !equal_quantity) {
        restuls.push_back(
            {equal_quantity, key.first, key.second,
             FormatQuantityHelper(slave_account_portfolio[key].closeable,
                                  master_account_portfolio[key].closeable),
             FormatQuantityHelper(slave_account_portfolio[key].open,
                                  master_account_portfolio[key].open),
             FormatQuantityHelper(slave_account_portfolio[key].close,
                                  master_account_portfolio[key].close)});
      }
    } else if (master_account_portfolio.find(key) !=
                   master_account_portfolio.end() &&
               slave_account_portfolio.find(key) ==
                   slave_account_portfolio.end()) {
      //       ss << "|" << std::left << std::setw(10) << key.first
      //          << "|" << std::setw(10) << (key.second == OrderDirection::kBuy
      //          ? "Buy" : "Sell")
      //          << "|" << std::right << std::setw(10) << "0(" +
      //          boost::lexical_cast<std::string>(master_account_portfolio[key].closeable)
      //          + ")"
      //          << "|" << std::setw(10) << "0(" +
      //          boost::lexical_cast<std::string>(master_account_portfolio[key].open)
      //          + ")"
      //          << "|" << std::setw(10) << "0(" +
      //          boost::lexical_cast<std::string>(master_account_portfolio[key].close)
      //          + ")"
      //          << "|\n";
      restuls.push_back(
          {false, key.first, key.second,
           FormatQuantityHelper(0, master_account_portfolio[key].closeable),
           FormatQuantityHelper(0, master_account_portfolio[key].open),
           FormatQuantityHelper(0, master_account_portfolio[key].close)});
    } else {
      restuls.push_back(
          {false, key.first, key.second,
           FormatQuantityHelper(slave_account_portfolio[key].closeable, 0),
           FormatQuantityHelper(slave_account_portfolio[key].open, 0),
           FormatQuantityHelper(slave_account_portfolio[key].close, 0)});
      //       ss << "|" << std::left << std::setw(10) << key.first
      //          << "|" << std::setw(10) << (key.second == OrderDirection::kBuy
      //          ? "Buy" : "Sell")
      //          << "|" << std::right << std::setw(10) <<
      //          boost::lexical_cast<std::string>(slave_account_portfolio[key].closeable)
      //          + "(0)"
      //          << "|" << std::setw(10) <<
      //          boost::lexical_cast<std::string>(slave_account_portfolio[key].open)
      //          + "(0)"
      //          << "|" << std::setw(10) <<
      //          boost::lexical_cast<std::string>(slave_account_portfolio[key].close)
      //          + "(0)"
      //          << "|\n";
    }
    // master_portfolio[] = portfolio;
  }

  std::sort(restuls.begin(), restuls.end(), [](auto l, auto r) {
    return std::make_pair(std::get<1>(l), std::get<2>(l)) <
           std::make_pair(std::get<1>(r), std::get<2>(r));
  });
  std::stringstream ss;
  if (!fully && restuls.empty()) {
    ss << account_id << ":complete sync order.\n";
  } else {
    ss << account_id << ":\n";
    ss << "|" << std::setw(10) << centered("Instrument") << "|" << std::setw(10)
       << centered("Direction") << "|" << std::setw(10) << centered("Closeable")
       << "|" << std::setw(10) << centered("Open") << "|" << std::setw(10)
       << centered("Close") << "|\n";
    for (auto t : restuls) {
      ss << "|" << std::setw(10) << centered(std::get<1>(t)) << "|" << std::setw(10)
         << centered((std::get<2>(t) == OrderDirection::kBuy ? "Buy" : "Sell")) << "|"
         << std::right << std::setw(10) << std::get<3>(t) << "|"
         << std::setw(10) << std::setw(10) << std::get<4>(t) << "|"
         << std::setw(10) << std::setw(10) << std::get<5>(t) << "|\n";
    }
  }

  ss << std::setw(80) << std::setfill('-') << "-\n\n";
  return ss.str();
}
