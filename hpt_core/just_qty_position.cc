#include "just_qty_position.h"

int JustQtyPosition::Closeable() const {
  return qty_ - frozen_;
}

int JustQtyPosition::qty() const {
  return qty_;
}

void JustQtyPosition::Unfreeze(int qty) {
  frozen_ -= qty;
}

void JustQtyPosition::Freeze(int qty) {
  frozen_ += qty;
}

void JustQtyPosition::CloseTraded(int traded_qty) {
  qty_ -= traded_qty;
  frozen_ -= traded_qty;
}

void JustQtyPosition::OpenTraded(int traded_qty) {
  qty_ += traded_qty;
}
