//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/NativeBinaryTraceFileReader.h"
#include "google/protobuf/descriptor.pb.h"
#include <sstream>
#include <filesystem>

#include "osi_groundtruth.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"
#include "osi_sensorviewconfiguration.pb.h"
#include "osi_hostvehicledata.pb.h"
#include "osi_trafficcommand.pb.h"
#include "osi_trafficcommandupdate.pb.h"
#include "osi_trafficupdate.pb.h"
#include "osi_motionrequest.pb.h"
#include "osi_streamingupdate.pb.h"


namespace {
template<typename T>
std::unique_ptr<google::protobuf::Message> ParseMessage(const std::vector<char>& data) {
    auto msg = std::make_unique<T>();
    msg->ParseFromArray(data.data(), static_cast<int>(data.size()));
    return std::move(msg);
}

const std::unordered_map<osi3::ReaderTopLevelMessage, osi3::MessageParserFunc> kParserMap = {
    {osi3::ReaderTopLevelMessage::kGroundTruth, [](const std::vector<char>& data) { return ParseMessage<osi3::GroundTruth>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorData, [](const std::vector<char>& data) { return ParseMessage<osi3::SensorData>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorView, [](const std::vector<char>& data) { return ParseMessage<osi3::SensorView>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorViewConfiguration, [](const std::vector<char>& data) { return ParseMessage<osi3::SensorViewConfiguration>(data); }},
    {osi3::ReaderTopLevelMessage::kHostVehicleData, [](const std::vector<char>& data) { return ParseMessage<osi3::HostVehicleData>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficCommand, [](const std::vector<char>& data) { return ParseMessage<osi3::TrafficCommand>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficCommandUpdate, [](const std::vector<char>& data) { return ParseMessage<osi3::TrafficCommandUpdate>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficUpdate, [](const std::vector<char>& data) { return ParseMessage<osi3::TrafficUpdate>(data); }},
    {osi3::ReaderTopLevelMessage::kMotionRequest, [](const std::vector<char>& data) { return ParseMessage<osi3::MotionRequest>(data); }},
    {osi3::ReaderTopLevelMessage::kStreamingUpdate, [](const std::vector<char>& data) { return ParseMessage<osi3::StreamingUpdate>(data); }}
};

} // unnamed namespace

namespace osi3 {


bool NativeBinaryTraceFileReader::Open(const std::string& filename, const ReaderTopLevelMessage message_type) {
    message_type_ = message_type;
    return this->Open(filename);
}

bool NativeBinaryTraceFileReader::Open(const std::string& filename) {
    // check if at least .osi ending is present
    if (filename.find(".osi") == std::string::npos) {
        std::cerr << "ERROR: The trace file '" << filename << "' must have a '.osi' extension." << std::endl;
        return false;
    }

    // check if file exists
    if (!std::filesystem::exists(filename)) {
        std::cerr << "ERROR: The trace file '" << filename << "' does not exist." << std::endl;
        return false;
    }

    // Determine message type based on filename if not specified in advance
    if (message_type_ == ReaderTopLevelMessage::kUnknown) {
        for (const auto& [key, value] : kMessageTypeMap) {
            if (filename.find(key) != std::string::npos) {
                message_type_ = value;
                break;
            }
        }
    }
    // if message_type_ is still unknown, return false
    if (message_type_ == ReaderTopLevelMessage::kUnknown) {
        std::cerr << "ERROR: Unable to determine message type from the filename '" << filename
                  << "'. Please ensure the filename follows the recommended OSI naming conventions as specified in the documentation or specify the message type manually."
                  << std::endl;
        return false;
    }

    parser_ = kParserMap.at(message_type_);

    trace_file_ = std::ifstream(filename, std::ios::binary);
    if (!trace_file_) {
        std::cerr << "ERROR: Failed to open trace file: " << filename << std::endl;
        return false;
    }
    return true;
}



void NativeBinaryTraceFileReader::Close() {
    trace_file_.close();
}

bool NativeBinaryTraceFileReader::HasNext() {
    return (trace_file_ && trace_file_.peek() != EOF);
}

std::optional<ReadResult> NativeBinaryTraceFileReader::ReadMessage() {
    if (!trace_file_) {
        return std::nullopt;
    }
    const auto serialized_msg = ReadNextMessageFromFile();

    if (serialized_msg.empty()) {
        throw std::runtime_error("Failed to read message");
    }


    ReadResult result;
    result.message = parser_(serialized_msg);
    result.message_type = message_type_;

    return result;
}


std::vector<char> NativeBinaryTraceFileReader::ReadNextMessageFromFile() {
    uint32_t message_size = 0;

    if (!trace_file_.read(reinterpret_cast<char*>(&message_size), sizeof(message_size))) {
        throw std::runtime_error("ERROR: Failed to read message size from file.");
    }
    std::vector<char> serialized_msg(message_size);
    if (!trace_file_.read(serialized_msg.data(), message_size)) {
        throw std::runtime_error("ERROR: Failed to read message from file");
    }
    return serialized_msg;
}


} // osi