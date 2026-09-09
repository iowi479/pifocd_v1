#pragma once

#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"


using namespace inet;

struct Flow {
    int pcp;
    int id;
};

/**
 * @brief Compute a scheduling priority rank (normally pcp) for a packet based on its INET stream name.
 *
 * This function retrieves an inet::StreamReq tag on the given packet. If present, it
 * reads the stream name from the tag and maps it to an integer rank:
 * The last char of the stream name (e.g. A0, B1) will be used as the pcp value.
 *
 * Wrong formatted stream names will result in pcp=0 as a fallback;
 *
 * @param packet The packet to classify (expected to potentially carry an inet::StreamReq tag).
 * @return The retrieved rank(pcp) is [0; 7].
 */
Flow getPacketFlow(Packet *packet);

/**
 * @brief Convert INET simulation time to nanoseconds.
 *
 * @param t INET simulation time.
 * @return Time expressed in nanoseconds as an unsigned 64-bit integer.
 */
uint64_t simtime_to_nsec(inet::simtime_t t);


uint64_t pmp_shapingTransaction(
    inet::Packet *packet,
    std::vector<uint64_t>& arrival_times,
    std::vector<int>& counters);

uint64_t pmp_schedulingTransaction(inet::Packet *packet);
