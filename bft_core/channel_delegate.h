#ifndef BFT_CORE_CHANNEL_DELEGATE_H
#define BFT_CORE_CHANNEL_DELEGATE_H
#include <memory>
#include <typeindex>
#include "bft_core/message.h"
#include "bft_core/message_handler.h"

namespace bft {
class ChannelDelegate {
 public:
  virtual void Subscribe(std::type_index type_index,
                         std::unique_ptr<BasedMessageHandler> handler) = 0;
  virtual void Send(std::shared_ptr<Message> message) = 0;
};

}  // namespace bft

#endif  // BFT_CORE_CHANNEL_DELEGATE_H
