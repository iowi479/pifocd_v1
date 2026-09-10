#include "utils.h"
#include "inet/common/packet/Packet.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
#include <regex>
#include <stdexcept>
#include <string>

using namespace inet;

// INFO: Has to be updated with omnetpp.ini.
static uint64_t periods[4] = {400000, 500000, 500000, 600000};

std::string getStreamNameFromPacket(inet::Packet *packet) {

  if (packet == nullptr) {
    throw std::runtime_error("PIFOCD: getPacketFlow(): packet is nullptr");
  }

  packet->getName();

  inet::Ptr<const StreamReq> streamReq = packet->findTag<inet::StreamReq>();

  if (streamReq == nullptr) {
    using namespace omnetpp;
    EV_INFO << "PIFOCD: Packet has no StreamReq tag: " << packet->getName() << endl;
    throw std::out_of_range("PIFOCD: No StreamReq tag found!");
  }

  const char *streamName = streamReq->getStreamName();

  if (streamName == nullptr) {
    throw std::out_of_range("PIFOCD: No StreamName found!");
  }

  std::string streamNameStr(streamName);

  return streamNameStr;
}

Flow getPacketFlow(std::string &streamName) {
  // Expected format: "Stream-(pcp)-(id)"
  // Example: "Stream-(3)-(42)"
  std::regex pattern(R"(Stream-\(([0-7])\)-\(([0-9]+)\))");
  std::smatch match;

  if (!std::regex_match(streamName, match, pattern)) {
    throw std::out_of_range(
        "PIFOCD: StreamName is not fitting: expected 'Stream-(pcp)-(id)'");
  }

  int pcp = std::stoi(match[1].str());
  int id = std::stoi(match[2].str());

  return Flow{pcp, id};
}

uint64_t pmp_shapingTransaction(PIFOPacket p,
                                std::vector<uint64_t> &arrival_times,
                                std::vector<int> &counters) {

  uint64_t now = simtime_to_nsec(simTime());

  if (p.flow.id >= counters.size()) {
    // Resize the vectors to accommodate the new flow ID
    size_t newSize = p.flow.id + 1;

    counters.resize(newSize, 0);
    arrival_times.resize(newSize, 0);
  }

  if (counters[p.flow.id] == 0) {
    arrival_times[p.flow.id] = now;
  }

  uint64_t rt =
      arrival_times[p.flow.id] + (counters[p.flow.id] * periods[p.flow.id]);

  counters[p.flow.id]++;

  return rt;
}

uint64_t pmp_schedulingTransaction(PIFOPacket p) { return p.flow.pcp; }

uint64_t rgp_shapingTransaction(PIFOPacket p,
                                std::vector<uint64_t> &arrival_times,
                                std::vector<int> &counters) {

  uint64_t now = simtime_to_nsec(simTime());

  if (p.flow.id >= counters.size()) {
    // Resize the vectors to accommodate the new flow ID
    size_t newSize = p.flow.id + 1;

    counters.resize(newSize, 0);
    arrival_times.resize(newSize, 0);
  }

  if (counters[p.flow.id] == 0) {
    arrival_times[p.flow.id] = now;
  }

  uint64_t rt =
      arrival_times[p.flow.id] + (counters[p.flow.id] * periods[p.flow.id]);

  counters[p.flow.id]++;

  return rt;
}

uint64_t rgp_schedulingTransaction(PIFOPacket p) { return p.flow.pcp; }

uint64_t simtime_to_nsec(inet::simtime_t t) {
  return (uint64_t)(SIMTIME_DBL(t) * 1e9 + 0.5);
}
