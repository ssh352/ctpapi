#ifndef BFT_CORE_DETAIL_MESSAGE_IMPL_H
#define BFT_CORE_DETAIL_MESSAGE_IMPL_H

#define BFT_TUPLE_VALS_DISPATCH(x) \
  case x:                          \
    return tuple_inspect_delegate<x, sizeof...(Ts) - 1>(data_, f)

namespace bft {
namespace detail {
// avoids triggering static asserts when using CAF_TUPLE_VALS_DISPATCH
template <size_t X, size_t Max, class T, class F>
auto tuple_inspect_delegate(T& data, F& f) -> decltype(f(std::get<Max>(data))) {
  return f(std::get<(X < Max ? X : Max)>(data));
}

template <size_t X, size_t N>
struct tup_ptr_access_pos {
  constexpr tup_ptr_access_pos() {
    // nop
  }
};

template <size_t X, size_t N>
constexpr tup_ptr_access_pos<X + 1, N> next(tup_ptr_access_pos<X, N>) {
  return {};
}

struct VoidPtrAccess {
  template <class T>
  const void* operator()(T& x) const noexcept {
    return &x;
  }
};

template <typename... Ts>
class MessageImpl : public Message {
 public:
  template <typename... Us>
  MessageImpl(Us&&... args) : data_(std::forward<Us>(args)...) {}

  using DataType = std::tuple<Ts...>;
  virtual const void* Get(size_t pos) const noexcept override {
    VoidPtrAccess f;
    switch (pos) {
      BFT_TUPLE_VALS_DISPATCH(0);
      BFT_TUPLE_VALS_DISPATCH(1);
      BFT_TUPLE_VALS_DISPATCH(2);
      BFT_TUPLE_VALS_DISPATCH(3);
      BFT_TUPLE_VALS_DISPATCH(4);
      BFT_TUPLE_VALS_DISPATCH(5);
      BFT_TUPLE_VALS_DISPATCH(6);
      BFT_TUPLE_VALS_DISPATCH(7);
      BFT_TUPLE_VALS_DISPATCH(8);
      BFT_TUPLE_VALS_DISPATCH(9);
      default:
        // fall back to recursive dispatch function
        static constexpr size_t max_pos = sizeof...(Ts) - 1;
        tup_ptr_access_pos<(10 < max_pos ? 10 : max_pos), max_pos> first;
        return rec_dispatch(pos, f, first);
    }
  }

  virtual std::type_index TypeIndex() const noexcept override {
    return typeid(DataType);
  }

 private:
  template <class F, size_t N>
  auto rec_dispatch(size_t, F& f, tup_ptr_access_pos<N, N>) const
      -> decltype(f(std::declval<int&>())) {
    return tuple_inspect_delegate<N, N>(data_, f);
  }

  template <class F, size_t X, size_t N>
  auto rec_dispatch(size_t pos, F& f, tup_ptr_access_pos<X, N> token) const
      -> decltype(f(std::declval<int&>())) {
    return pos == X ? tuple_inspect_delegate<X, N>(data_, f)
                    : rec_dispatch(pos, f, next(token));
  }

  DataType data_;
};
}  // namespace detail
}  // namespace bft

#endif  // BFT_CORE_DETAIL_MESSAGE_IMPL_H
