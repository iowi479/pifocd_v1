#include <vector>
#include <omnetpp.h>
#include "inet/common/ModuleAccess.h"
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
#include "utils.h"
#include "SchedulingTree.h"

using namespace omnetpp;

Define_Module(SchedulingTree);


void SchedulingTree::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        producer.reference(inputGate, false);
        collector.reference(outputGate, false);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
        if (producer != nullptr)
            producer.handleCanPushPacketChanged();
    }

    this->wakeMsg = new cMessage("wake");

}

void SchedulingTree::pushPacket(Packet *packet, const cGate *gate) {
    Enter_Method("pushPacket");
    // Takes ownership of the packet.
    // Now the previous owner will no longer dictate its lifetime.
    // We keep ownership until we transmit the packet further.
    take(packet);

    Flow f = getPacketFlow(packet);

    EV_INFO << "Pushing packet (pcp=" << f.pcp << ") " << EV_FIELD(packet) << EV_ENDL;

    this->leafs[f.pcp].push(packet);

    this->notifyCanPullPacketChanged();
}


void SchedulingTree::notifyCanPullPacketChanged() {
    if (collector != nullptr && getNumPackets() > 0){
        EV_INFO << "SchedulingTree notifyCanPullPacketChanged" << EV_ENDL;
        collector.handleCanPullPacketChanged();
    }
}

Packet *SchedulingTree::pullPacket(const cGate *gate) {
    Enter_Method("pullPacket");

    Packet *packet = nullptr;
    if (root.size() > 0) {
        packet = root.pull();

        if (collector != nullptr)
            animatePullPacket(packet, outputGate, collector.getReferencedGate());
    }

    return packet;
}

Packet *SchedulingTree::canPullPacket(const cGate *gate) const {
    // INFO:
    // This seems as it is not being used by omnet to determine if a packet can be pulled.
    // It seems as it only uses the getNumPackets() function to check whether this is non-zero.

    // TODO: this needs to be fixed
    EV_INFO << "SchedulingTree canPullPacket" << EV_ENDL;
    return root.peek();
}


int SchedulingTree::getNumPackets() const {
    // This seems to determine if omnet will pull a packet from this construct.
    // If this is non-empty a packet will be pulled.

    //EV_INFO << "SchedulingTree getNumPackets = " << root.size() << EV_ENDL;
    return root.size();
}



void SchedulingTree::handleMessage(omnetpp::cMessage *msg) {
    if (msg == wakeMsg) {
        uint64_t now = simtime_to_nsec(simTime());

        EV_INFO << "SchedulingTree: Woken @" << now << "ns" << EV_ENDL;

        for (size_t i = 0; i < timers.size(); ++i) {
            uint64_t ts = timers[i];

            if (ts <= now) {
                // Reset timer to not wake this again.
                timers[i] = UINT64_MAX;

                this->leafs[i].wake();
            }
        }

        // Timers where updated in wake.
        // Now: reschedule the timer:
        // this->scheduleWake();

        return;
    }
}

/*
 * Reschedule WakeTimer based on the this->timers.
 */
void SchedulingTree::scheduleWake() {
    cancelEvent(wakeMsg);

    uint64_t now = simtime_to_nsec(simTime());

    uint64_t minTs= timers[0];
    EV_INFO << "timers: " << EV_ENDL;

    for (uint64_t x : timers) {
        EV_INFO << x << ", ";
        minTs = std::min(minTs, x);
    }
    EV_INFO << EV_ENDL;

    // This would cause issues since the message would not arrive.
    if (now > minTs) {
    // if (now >= minTs) {
        throw cRuntimeError("New Waketime is not in the future now=%lu, minTs=%lu", now, minTs);
    }

    if (minTs == UINT64_MAX) {
        EV_INFO << "SchedulingTree: No next Wake since no timer requested" << EV_ENDL;
        return;
    }

    simtime_t t = SimTime(minTs, SIMTIME_NS);

    scheduleAt(t , wakeMsg);
    EV_INFO << "SchedulingTree: Next Wake @" << minTs << "ns" << EV_ENDL;
}

void SchedulingTree::updateTimer(uint8_t id, uint64_t rt) {
    this->timers[id] = rt;
    this->scheduleWake();
}




Packet *SchedulingTree::peekLeaf(uint8_t id) const {
    return this->leafs[id].peek();
}

Packet *SchedulingTree::pullLeaf(uint8_t id) {
    return this->leafs[id].pull();
}






















