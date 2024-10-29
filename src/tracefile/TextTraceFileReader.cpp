//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/TextTraceFileReader.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/text_format.h"
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

namespace osi3 {
namespace {
template<typename T>
std::unique_ptr<google::protobuf::Message> ParseMessage(const std::string& data) {
    auto msg = std::make_unique<T>();
    const auto res = google::protobuf::TextFormat::ParseFromString(data, msg.get());
    if (!res) {
        throw std::runtime_error("Failed to parse message");
    }
    return std::move(msg);
}

const std::unordered_map<osi3::ReaderTopLevelMessage, osi3::MessageParserFunc> kParserMap = {
    {osi3::ReaderTopLevelMessage::kGroundTruth, [](const std::string& data) { return ParseMessage<osi3::GroundTruth>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorData, [](const std::string& data) { return ParseMessage<osi3::SensorData>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorView, [](const std::string& data) { return ParseMessage<osi3::SensorView>(data); }},
    {osi3::ReaderTopLevelMessage::kSensorViewConfiguration, [](const std::string& data) { return ParseMessage<osi3::SensorViewConfiguration>(data); }},
    {osi3::ReaderTopLevelMessage::kHostVehicleData, [](const std::string& data) { return ParseMessage<osi3::HostVehicleData>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficCommand, [](const std::string& data) { return ParseMessage<osi3::TrafficCommand>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficCommandUpdate, [](const std::string& data) { return ParseMessage<osi3::TrafficCommandUpdate>(data); }},
    {osi3::ReaderTopLevelMessage::kTrafficUpdate, [](const std::string& data) { return ParseMessage<osi3::TrafficUpdate>(data); }},
    {osi3::ReaderTopLevelMessage::kMotionRequest, [](const std::string& data) { return ParseMessage<osi3::MotionRequest>(data); }},
    {osi3::ReaderTopLevelMessage::kStreamingUpdate, [](const std::string& data) { return ParseMessage<osi3::StreamingUpdate>(data); }}
};

} // unnamed namespace


bool TextTraceFileReader::Open(const std::string& filename) {
    if (filename.find(".txth") == std::string::npos) {
        std::cerr << "ERROR: The trace file '" << filename << "' must have a '.txth' extension." << std::endl;
        return false;
    }

    if (!std::filesystem::exists(filename)) {
        std::cerr << "ERROR: The trace file '" << filename << "' does not exist." << std::endl;
        return false;
    }

    if (message_type_ == ReaderTopLevelMessage::kUnknown) {
        for (const auto& [key, value] : kMessageTypeMap) {
            if (filename.find(key) != std::string::npos) {
                message_type_ = value;
                break;
            }
        }
    }

    if (message_type_ == ReaderTopLevelMessage::kUnknown) {
        std::cerr << "ERROR: Unable to determine message type from filename." << std::endl;
        return false;
    }

    parser_ = kParserMap.at(message_type_);

    trace_file_ = std::ifstream(filename);

    // find top-level message delimiter by peeking into the file and assuming the first line
    // will be the pattern to indicate a new message
    std::getline(trace_file_, line_indicating_msg_start_);
    trace_file_.seekg(std::ios_base::beg);

    return trace_file_.is_open();
}

bool TextTraceFileReader::Open(const std::string& filename, const ReaderTopLevelMessage message_type) {
    message_type_ = message_type;
    return Open(filename);
}

void TextTraceFileReader::Close() {
    trace_file_.close();
}

bool TextTraceFileReader::HasNext() {
    return trace_file_ && !trace_file_.eof();
}

std::optional<ReadResult> TextTraceFileReader::ReadMessage() {
    if (!trace_file_) {
        return std::nullopt;
    }

    std::string text_message = ReadNextMessageFromFile();
    if (text_message.empty()) {
        return std::nullopt;
    }

    ReadResult result;
    result.message = parser_(text_message);
    result.message_type = message_type_;
    return result;
}

std::string TextTraceFileReader::ReadNextMessageFromFile() {
    std::string message;
    std::string line;
    uint last_position = 0;

    // make sure the first line starts with line_indicating_msg_start_
    // read everything until:
    //   - 1. the next occurrence of line_indicating_msg_start_ and do not include this line
    //   - 2. the end of the file
    // append content message
    std::getline(trace_file_, line);
    message += line;
    line = "";
    while (!trace_file_.eof() && line != line_indicating_msg_start_) {
        message += line + "\n";
        // Get current position
        last_position = trace_file_.tellg();
        // read next line
        std::getline(trace_file_, line);
    }
    if (!trace_file_.eof()) { // go back to the line before line_indicating_msg_start_
        trace_file_.seekg(last_position ,std::ios_base::beg);
    }

    return message;
}


}