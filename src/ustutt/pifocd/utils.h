#pragma once

#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
#include <string>

using namespace inet;

struct Flow {
  int pcp;
  int id;
};

struct PIFOPacket {
  int packet_id;
  std::string streamName;
  Flow flow;
};

std::string getStreamNameFromPacket(inet::Packet *packet);

Flow getPacketFlow(std::string &streamName);

uint64_t pmp_shapingTransaction(PIFOPacket p,
                                std::vector<uint64_t> &arrival_times,
                                std::vector<int> &counters);

uint64_t pmp_schedulingTransaction(PIFOPacket p);

uint64_t rgp_shapingTransaction(PIFOPacket p,
                                std::vector<uint64_t> &arrival_times,
                                std::vector<int> &counters);

uint64_t rgp_schedulingTransaction(PIFOPacket p);

uint64_t simtime_to_nsec(inet::simtime_t t);
