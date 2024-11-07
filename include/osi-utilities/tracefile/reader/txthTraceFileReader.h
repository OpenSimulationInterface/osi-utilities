//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#ifndef TXTHTRACEFILEREADER_H
#define TXTHTRACEFILEREADER_H

#include <google/protobuf/text_format.h>

#include <fstream>
#include <functional>

#include "../Reader.h"
#include "osi_groundtruth.pb.h"
#include "osi_hostvehicledata.pb.h"
#include "osi_motionrequest.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"
#include "osi_sensorviewconfiguration.pb.h"
#include "osi_streamingupdate.pb.h"
#include "osi_trafficcommand.pb.h"
#include "osi_trafficcommandupdate.pb.h"
#include "osi_trafficupdate.pb.h"

namespace osi3 {
class TxthTraceFileReader final : public TraceFileReader {
    /**
     * @brief Function type for parsing protobuf TextFormat strings into protobuf objects
     */
    using MessageParserFunc = std::function<std::unique_ptr<google::protobuf::Message>(const std::string&)>;

   public:
    bool Open(const std::string& filename) override;
    bool Open(const std::string& filename, ReaderTopLevelMessage message_type);
    void Close() override;
    bool HasNext() override;
    std::optional<ReadResult> ReadMessage() override;

   private:
    std::ifstream trace_file_;
    MessageParserFunc parser_; /**< Message parsing function */
    std::string line_indicating_msg_start_;
    ReaderTopLevelMessage message_type_{ReaderTopLevelMessage::kUnknown}; /**< Current message type */
    // MessageParserFunc parser_;
    std::string ReadNextMessageFromFile();

    template <typename T>
    std::unique_ptr<google::protobuf::Message> ParseMessage(const std::string& data) {
        auto msg = std::make_unique<T>();
        if (!google::protobuf::TextFormat::ParseFromString(data, msg.get())) {
            throw std::runtime_error("Failed to parse message");
        }
        return std::move(msg);
    }

    template <typename T>
    MessageParserFunc CreateParser() {
        return [this](const std::string& data) { return ParseMessage<T>(data); };
    }

    const std::unordered_map<ReaderTopLevelMessage, MessageParserFunc> kParserMap_ = {{ReaderTopLevelMessage::kGroundTruth, CreateParser<GroundTruth>()},
                                                                                      {ReaderTopLevelMessage::kSensorData, CreateParser<SensorData>()},
                                                                                      {ReaderTopLevelMessage::kSensorView, CreateParser<SensorView>()},
                                                                                      {ReaderTopLevelMessage::kSensorViewConfiguration, CreateParser<SensorViewConfiguration>()},
                                                                                      {ReaderTopLevelMessage::kHostVehicleData, CreateParser<HostVehicleData>()},
                                                                                      {ReaderTopLevelMessage::kTrafficCommand, CreateParser<TrafficCommand>()},
                                                                                      {ReaderTopLevelMessage::kTrafficCommandUpdate, CreateParser<TrafficCommandUpdate>()},
                                                                                      {ReaderTopLevelMessage::kTrafficUpdate, CreateParser<TrafficUpdate>()},
                                                                                      {ReaderTopLevelMessage::kMotionRequest, CreateParser<MotionRequest>()},
                                                                                      {ReaderTopLevelMessage::kStreamingUpdate, CreateParser<StreamingUpdate>()}};
};
}  // namespace osi3

#endif  // TXTHTRACEFILEREADER_H
