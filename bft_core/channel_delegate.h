#ifndef BFT_CORE_CHANNEL_DELEGATE_H
#define BFT_CORE_CHANNEL_DELEGATE_H
#include <memory>
#include <typeindex>
#include "bft_core/message.h"
#include "bft_core/message_handler.h"

namespace bft {
class ChannelDelegate {
 public:
   // caf::message_handler
  virtual void Subscribe(bft::MessageHandler handler) = 0;

  virtual void Send(Message message) = 0;
};

}  // namespace bft

#endif  // BFT_CORE_CHANNEL_DELEGATE_H
