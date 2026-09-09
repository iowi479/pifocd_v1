#include <regex>
#include <stdexcept>
#include "utils.h"
#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace inet;

// INFO: Has to be updated with omnetpp.ini.
static uint64_t periods[4] = {400000, 500000, 500000, 600000};




Flow getPacketFlow(Packet *packet)
{
    if (packet == nullptr) {
        throw std::runtime_error("getPacketFlow(): packet is nullptr");
    }

    packet->getName();

    inet::Ptr<const StreamReq> streamReq = packet->findTag<inet::StreamReq>();

    if (streamReq == nullptr) {
        using namespace omnetpp;
        EV_INFO << "Packet has no StreamReq tag: " << packet->getName() << endl;
        // throw std::out_of_range("No StreamReq tag found!");
        return Flow {0, 100};
    }

    const char *streamName = streamReq->getStreamName();

    if (streamName == nullptr) {
        throw std::out_of_range("No StreamName found!");
    }

    // Expected format: "Stream-(pcp)-(id)"
    // Example: "Stream-(3)-(42)"
    std::regex pattern(R"(Stream-\(([0-7])\)-\(([0-9]+)\))");
    std::cmatch match;

    if (!std::regex_match(streamName, match, pattern)) {
        throw std::out_of_range(
            "StreamName is not fitting: expected 'Stream-(pcp)-(id)'");
    }

    int pcp = std::stoi(match[1].str());
    int id  = std::stoi(match[2].str());

    return Flow{pcp, id};
}


uint64_t simtime_to_nsec(inet::simtime_t t) {
    return (uint64_t) (SIMTIME_DBL(t)*1e9 + 0.5);
}

uint64_t pmp_shapingTransaction(
    inet::Packet *packet,
    std::vector<uint64_t>& arrival_times,
    std::vector<int>& counters)
{
    Flow f = getPacketFlow(packet);

    uint64_t now = simtime_to_nsec(simTime());

    // Resize both vectors if this flow ID is not yet present.
    // New elements are initialized to 0.
    if (f.id >= counters.size()) {
        size_t newSize = f.id + 1;

        counters.resize(newSize, 0);
        arrival_times.resize(newSize, 0);
    }

    if (counters[f.id] == 0) {
        arrival_times[f.id] = now;
    }

    uint64_t rt = arrival_times[f.id]
                + (counters[f.id] * periods[f.id]);

    counters[f.id]++;

    return rt;
}


uint64_t pmp_schedulingTransaction(inet::Packet *packet) {
    struct Flow f = getPacketFlow(packet);

    return f.pcp;
}
