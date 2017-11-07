#ifndef HPT_CORE_JUST_QTY_POSITION_H
#define HPT_CORE_JUST_QTY_POSITION_H

class JustQtyPosition {
public:
  void OpenTraded(int traded_qty);

  void CloseTraded(int traded_qty);

  void Freeze(int qty);

  void Unfreeze(int qty);

  int Closeable() const;

  int qty() const;

  int frozen() const {
    return frozen_;
  }
private:
  int qty_ = 0;
  int frozen_ = 0;
};

#endif // HPT_CORE_JUST_QTY_POSITION_H
