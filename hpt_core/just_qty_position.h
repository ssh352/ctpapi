#ifndef HPT_CORE_JUST_QTY_POSITION_H
#define HPT_CORE_JUST_QTY_POSITION_H

class JustQtyPosition {
public:
  JustQtyPosition();

  JustQtyPosition(int qty);

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
  int qty_;
  int frozen_;
};

#endif // HPT_CORE_JUST_QTY_POSITION_H
