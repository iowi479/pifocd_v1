#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace inet;


int getPacketRank(Packet *packet) {
    const char *streamId = nullptr;
    inet::Ptr<const StreamReq> streamReq = packet->findTag<inet::StreamReq>();

    if (streamReq != nullptr) {
        streamId = streamReq->getStreamName();
    }

    if (streamId == nullptr) {
        // No Tag available
        throw std::out_of_range("No SteamName found!");
    }

    size_t len = std::strlen(streamId);
    if (len < 1) throw std::out_of_range("StreamName is not fitting: should be: (.*d)");

    char last = streamId[len - 1];
    int pcp = last - '0';

    if (pcp < 0 || pcp > 7) throw std::out_of_range("pcp out of range");

    return pcp;
}

uint64_t simtime_to_nsec(inet::simtime_t t) {
    uint64_t tns;

    tns = (uint64_t) (SIMTIME_DBL(t)*1e9 + 0.5);

    return tns;
}

uint64_t stxn(inet::Packet *packet) {
    uint64_t now = simtime_to_nsec(simTime());
    int rank = getPacketRank(packet);

    static uint64_t a0[2] = {0, 0};
    static int counter[2] = {0, 0};

    // Stream 1: truncnormal(150us, 20us)
    // Stream 2: truncnormal(200us, 20us)
    static uint64_t period[2] = {150000, 200000};
    static uint64_t wcd[2] = {80000,120000};

    if (counter[rank] == 0) {
        a0[rank] = now;
    }

    uint64_t rt = a0[rank] + (counter[rank] * period[rank]) + wcd[rank];

    counter[rank]++;

    return rt;
}

uint64_t txn(inet::Packet *packet) {
    return getPacketRank(packet);
}
