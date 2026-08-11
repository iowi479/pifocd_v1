#pragma once

#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace inet;

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
int getPacketRank(Packet *packet);

/**
 * @brief Convert INET simulation time to nanoseconds.
 *
 * @param t INET simulation time.
 * @return Time expressed in nanoseconds as an unsigned 64-bit integer.
 */
uint64_t simtime_to_nsec(inet::simtime_t t);

/**
 * @brief Example ShapingTransaction "stxn": uses a constant value as releasetime.
 *
 * Current implementation returns a constant value (300000).
 *
 * @param packet Unused.
 * @return Constant value 300000 (nanoseconds).
 */
uint64_t stxn(inet::Packet *packet);

/**
 * @brief Example SchedulingTransaction "txn": retreives pcp as rank from the packet.
 *
 * @param packet Packet to classify.
 * @return The computed rank/pcp in [0; 7], or throws if rank parsing throws.
 * @throw std::out_of_range If getPacketRank throws due to invalid PCP digit.
 */
uint64_t txn(inet::Packet *packet);
