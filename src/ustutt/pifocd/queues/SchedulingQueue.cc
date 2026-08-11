#include "SchedulingQueue.h"
#include "../SchedulingTree.h"

int SchedulingQueue::size() const {
    return pq.size();
}

bool SchedulingQueue::isEmpty() const {
    return size() == 0;
}

void SchedulingQueue::clear() {
    while(pq.size() > 0) {
        pq.pop();
    }
};

void SchedulingQueue::push(Packet *packet) {
    Entry entry {};
    uint64_t rank = this->txn(packet);

    // assert pcp range, since we might use this as a ref into the children.
    // This allows us to safely downcast to uint8_t later as well.
    if (rank < 0 || rank > 7) throw cRuntimeError("SchedulingQueue Packet-rank is %lu which is not in [0; 7]", rank);


    // Either add packet or the rank which will point us to the ShapingQueue for pcp=rank.
    // The rank is the same as the ref to a ShapingQueue.
    entry.value = isLeaf ? V{packet} : V{(uint8_t) rank};
    entry.rank = rank;

    EV_INFO << "SchedulingQueue enqueued 1 ref with rank=" << rank << EV_ENDL;

    pq.push(entry);

    // Propagate further if there is still a parent
    if (parent != nullptr) {
        parent->push(packet);
    } else {
        // TODO:
        // Then we are the root
        // Notify that packets are available

    }
}

Packet *SchedulingQueue::pull() {

    EV_INFO << "SchedulingQueue pull" << EV_ENDL;

    Packet *packet;
    Entry head = pq.top();

    // Actually remove the head element.
    pq.pop();

    if(std::holds_alternative<Packet *>(head.value)) {
        if (!isLeaf) throw cRuntimeError("SchedulingQueue is not a leaf");


        // We are a leaf and can return the packet *;
        packet = std::get<Packet *>(head.value);
    } else {
        uint8_t ref = std::get<uint8_t>(head.value);

        // ref always has to point into the children.
        // If this is out-of-bounds, something went wrong.
        if (this->children.size() < ref) throw cRuntimeError("SchedulingQueue ref is pointing out of bounds");


        // This can go on recursively until it reaches the leaf.
        packet = this->st->pullLeaf(ref);
    }

    return packet;
}


Packet *SchedulingQueue::peek() const {

    EV_INFO << "SchedulingQueue peek" << EV_ENDL;

    Packet *packet;
    Entry head = pq.top();


    if(std::holds_alternative<Packet *>(head.value)) {
        if (!isLeaf) throw cRuntimeError("SchedulingQueue is not a leaf");


        // We are a leaf and can return the packet *;
        packet = std::get<Packet *>(head.value);
    } else {
        uint8_t ref = std::get<uint8_t>(head.value);

        // ref always has to point into the children.
        // If this is out-of-bounds, something went wrong.
        if (this->children.size() < ref) throw cRuntimeError("SchedulingQueue ref is pointing out of bounds");


        // This can go on recursively until it reaches the leaf.
        EV_INFO << "SchedulingQueue peek @child=" << (int) ref << EV_ENDL;

        if (!this->children[ref]) throw cRuntimeError("SchedulingQueue ref is not there");


        static thread_local int depth = 0;
            if (++depth > 1000) {
                throw cRuntimeError("peek recursion too deep (node may be cycling)");
            }
        // throw cRuntimeError("test2 length children = %d", children.size());
        packet = this->st->peekLeaf(ref);
    }

    return packet;
}
