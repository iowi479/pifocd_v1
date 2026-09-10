#pragma once

#include "SchedulingQueue.h"
#include "inet/common/packet/Packet.h"
#include <queue>
#include <string>
#include <variant>
#include <vector>

using namespace inet;

class ShapingQueue : public SchedulingQueue {
public:
  using ShapingTransaction = std::function<uint64_t(
      string &streamName, std::vector<uint64_t> &, std::vector<int> &)>;

protected:
  std::priority_queue<Entry, std::vector<Entry>, Compare> sq;

  uint8_t id;
  ShapingTransaction shapingTransaction;

public:
  ShapingQueue(ShapingTransaction sptxn, SchedulingTransaction sdtxn,
               bool isLeaf, uint8_t id)
      : SchedulingQueue(sdtxn, isLeaf), id(id), shapingTransaction(sptxn) {
    this->sq = {};
  }

  ~ShapingQueue() {};

  void push(int packet_id, string &streamname) override;
  packet_id pull() override;
  packet_id peek() const override;

  /**
   * Something in the queue updated.
   * It may be that the tree will be informed to reschedule the wake timer.
   */
  void updated();

  /**
   * Wakes up the queue on a timer event.
   * The queue will make sure to handle the wake and forward packet-refs
   * accordingly.
   */
  void wake();
};
