#pragma once

#include <vector>
#include <queue>
#include <variant>
#include "inet/common/packet/Packet.h"
#include "SchedulingQueue.h"

using namespace inet;


class ShapingQueue : public SchedulingQueue
{
public:
    using ShapingTransaction = std::function<uint64_t(inet::Packet *)>;


protected:
    std::priority_queue<Entry, std::vector<Entry>, Compare> sq;

    uint8_t id;
    ShapingTransaction stxn;


public:
    ShapingQueue(ShapingTransaction stxn, bool isLeaf, uint8_t id)
    : SchedulingQueue(stxn, isLeaf), id(id), stxn(stxn)
    {
        this->sq = {};
    }

    ~ShapingQueue() {};


    void push(Packet *packet) override;
    Packet *pull() override;
    Packet *peek() const override;

    /**
     * Something in the queue updated.
     * It may be that the tree will be informed to reschedule the wake timer.
     */
    void updated();

    /**
     * Wakes up the queue on a timer event.
     * The queue will make sure to handle the wake and forward packet-refs accordingly.
     */
    void wake();
};


