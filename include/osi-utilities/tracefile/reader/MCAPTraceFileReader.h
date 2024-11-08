//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#ifndef OSIUTILITIES_TRACEFILE_READER_MCAPTRACEFILE_READER_H_
#define OSIUTILITIES_TRACEFILE_READER_MCAPTRACEFILE_READER_H_

#include "osi-utilities/tracefile/Reader.h"
#include <mcap/reader.hpp>

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

class MCAPTraceFileReader final : public TraceFileReader {
public:
    bool Open(const std::string& filename) override;
    std::optional<ReadResult> ReadMessage() override;
    void Close() override;
    bool HasNext() override;

private:
    mcap::McapReader mcap_reader_;
    std::unique_ptr<mcap::LinearMessageView> message_view_;  // Cannot copy or move LinearMessageView
    std::unique_ptr<mcap::LinearMessageView::Iterator> message_iterator_;

    bool skip_non_osi_msgs_ = false; // todo add setter

    template <typename T>
    std::unique_ptr<google::protobuf::Message> Deserialize(const mcap::Message& msg) {
        auto output = std::make_unique<T>();
        if (!output->ParseFromArray(msg.data,
                                   msg.dataSize))
        {
            throw std::runtime_error("Error: Failed to deserialize message.");
        }
        return output;
    }

    using DeserializeFunction = std::function<std::unique_ptr<google::protobuf::Message>(const mcap::Message& msg)>;
    const std::map<std::string, std::pair<DeserializeFunction, ReaderTopLevelMessage>> deserializer_map_ = {
        {"osi3.GroundTruth", {[this](const mcap::Message& msg) { return Deserialize<osi3::GroundTruth>(msg); }, ReaderTopLevelMessage::kGroundTruth}},
        {"osi3.SensorData", {[this](const mcap::Message& msg) { return Deserialize<osi3::SensorData>(msg); }, ReaderTopLevelMessage::kSensorData}},
        {"osi3.SensorView", {[this](const mcap::Message& msg) { return Deserialize<osi3::SensorView>(msg); }, ReaderTopLevelMessage::kSensorView}},
        {"osi3.SensorViewConfiguration", {[this](const mcap::Message& msg) { return Deserialize<osi3::SensorViewConfiguration>(msg); }, ReaderTopLevelMessage::kSensorViewConfiguration}},
        {"osi3.HostVehicleData", {[this](const mcap::Message& msg) { return Deserialize<osi3::HostVehicleData>(msg); }, ReaderTopLevelMessage::kHostVehicleData}},
        {"osi3.TrafficCommand", {[this](const mcap::Message& msg) { return Deserialize<osi3::TrafficCommand>(msg); }, ReaderTopLevelMessage::kTrafficCommand}},
        {"osi3.TrafficCommandUpdate", {[this](const mcap::Message& msg) { return Deserialize<osi3::TrafficCommandUpdate>(msg); }, ReaderTopLevelMessage::kTrafficCommandUpdate}},
        {"osi3.TrafficUpdate", {[this](const mcap::Message& msg) { return Deserialize<osi3::TrafficUpdate>(msg); }, ReaderTopLevelMessage::kTrafficUpdate}},
        {"osi3.MotionRequest", {[this](const mcap::Message& msg) { return Deserialize<osi3::MotionRequest>(msg); }, ReaderTopLevelMessage::kMotionRequest}},
        {"osi3.StreamingUpdate", {[this](const mcap::Message& msg) { return Deserialize<osi3::StreamingUpdate>(msg); }, ReaderTopLevelMessage::kStreamingUpdate}}
    };



};

} // namespace osi3
#endif