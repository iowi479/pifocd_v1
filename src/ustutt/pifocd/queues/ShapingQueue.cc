#include "ShapingQueue.h"
#include "../SchedulingTree.h"
#include "../utils.h"
#include "inet/common/packet/Packet.h"
#include <omnetpp.h>
#include <optional>

using namespace omnetpp;

void ShapingQueue::push(PIFOPacket packet) {
  Entry sq_entry{};
  Entry pq_entry{};

  uint64_t rt = this->shapingTransaction(packet, this->st->arrival_times,
                                         this->st->counters);

  // WARN: We assume here, that we are a Leaf. In PIFOCD it can't be different.
  // This simplifies the actions drastically here.
  if (!isLeaf)
    throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);

  pq_entry.value = packet;
  pq_entry.rank = rt;

  // This is only needed for the delayed propagation to the root later.
  // We will need the packet to calculate the rank later.
  // This could also be done with more variants and calculating the rank here.
  // This works for now and is simpler.
  sq_entry.value = packet;
  sq_entry.rank = rt;

  pq.push(pq_entry);

  EV_INFO << "PIFOCD: ShapingQueue(id=" << (int)this->id
          << ") enqueued 1 packet with rt=" << rt << EV_ENDL;

  uint64_t now = simtime_to_nsec(simTime());

  EV_INFO << "now=" << now << " rt=" << rt << EV_ENDL;

  if (now >= rt) {
    // rt is already reached. So we can propagate further without putting the
    // element into the queue
    if (parent != nullptr) {
      parent->push(packet);
    }
  } else {
    // insert the element into the queue
    sq.push(sq_entry);
    EV_INFO << "PIFOCD: ShapingQueue(id=" << (int)this->id
            << ") enqueued 1 shaping-ref with rt=" << rt << EV_ENDL;
  }

  this->updated();
}

std::optional<PIFOPacket> ShapingQueue::pull() {
  if (sq.size() < pq.size()) {
    // First Packet of pq is already released from sq since rt < now.
    // So we can just remove the packet.
    Entry pq_entry = pq.top();

    if (!isLeaf)
      throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);
    if (!std::holds_alternative<PIFOPacket>(pq_entry.value))
      throw cRuntimeError(
          "PIFOCD: ShapingQueue(id=%d) pq_entry doesn't hold a Packet *", id);

    PIFOPacket packet = std::get<PIFOPacket>(pq_entry.value);
    pq.pop();

    return packet;
  } else if (pq.size() > 0 && sq.size() == pq.size()) {
    // There are packets. But all are waiting for release by sq.
    // This is the manual pulling of PIFOCD

    Entry pq_entry = pq.top();

    if (!isLeaf)
      throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);
    if (!std::holds_alternative<PIFOPacket>(pq_entry.value))
      throw cRuntimeError(
          "PIFOCD: ShapingQueue(id=%d) pq_entry doesn't hold a Packet *", id);

    PIFOPacket packet = std::get<PIFOPacket>(pq_entry.value);

    pq.pop();
    sq.pop();

    // update since the head of sq changed.
    this->updated();
    return packet;
  } else {
    // something weird. Fall back
    EV_INFO << "PIFOCD: pulling from queue in weird state: sq.size() = " << sq.size()
            << ", pq.size() = " << pq.size() << EV_ENDL;

    return std::nullopt;
  }
}

std::optional<PIFOPacket> ShapingQueue::peek() const {

  if (sq.size() < pq.size()) {
    // First Packet of pq is already released from sq since rt < now.
    // So we can just remove the packet.
    Entry pq_entry = pq.top();

    if (!isLeaf)
      throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);
    if (!std::holds_alternative<PIFOPacket>(pq_entry.value))
      throw cRuntimeError(
          "PIFOCD: ShapingQueue(id=%d) pq_entry doesn't hold a Packet *", id);

    PIFOPacket packet = std::get<PIFOPacket>(pq_entry.value);

    return packet;
  } else if (pq.size() > 0 && sq.size() == pq.size()) {
    // There are packets. But all are waiting for release by sq.
    // This is the manual pulling of PIFOCD

    Entry pq_entry = pq.top();

    if (!isLeaf)
      throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);
    if (!std::holds_alternative<PIFOPacket>(pq_entry.value))
      throw cRuntimeError(
          "PIFOCD: ShapingQueue(id=%d) pq_entry doesn't hold a Packet *", id);

    PIFOPacket packet = std::get<PIFOPacket>(pq_entry.value);

    return packet;
  } else {
    // something weird. Fall back
    EV_INFO << "PIFOCD: peeking at queue in weird state: sq.size() = " << sq.size()
            << ", pq.size() = " << pq.size() << EV_ENDL;

    return std::nullopt;
  }
}

void ShapingQueue::updated() {
  if (sq.size() > 0) {
    Entry head = sq.top();
    uint64_t rt = head.rank;

    this->st->updateTimer(id, rt);
  } else {
    // There are no Packets anymore.
    this->st->updateTimer(id, UINT64_MAX);
  }

  // A update happened.
  // It may be possible, a packet is now ready for pulling.
  this->st->notifyCanPullPacketChanged();
}

void ShapingQueue::wake() {
  uint64_t now = simtime_to_nsec(simTime());
  Entry head;

  EV_INFO << "PIFOCD: ShapingQueue(id=" << (int)this->id << ") got woken @" << now << EV_ENDL;

  // Dequeue all packet-refs which timers expired and forward them.
  while (sq.size() > 0) {
    head = sq.top();
    uint64_t rt = head.rank;

    if (now >= rt) {
      if (!isLeaf)
        throw cRuntimeError("PIFOCD: ShapingQueue(id=%d) is not a leaf", id);

      PIFOPacket packet = std::get<PIFOPacket>(head.value);
      sq.pop();
      if (parent != nullptr) {
        parent->push(packet);
      }
    } else {
      break;
    }
  }

  // The queue updated itself, so handle these changes.
  this->updated();
}
