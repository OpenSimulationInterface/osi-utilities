//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <osi-utilities/tracefile/reader/NativeBinaryTraceFileReader.h>
#include <osi-utilities/tracefile/writer/MCAPTraceFileWriter.h>

#include <filesystem>

#include "osi_groundtruth.pb.h"
#include "osi_hostvehicledata.pb.h"
#include "osi_motionrequest.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"
#include "osi_streamingupdate.pb.h"
#include "osi_trafficcommand.pb.h"
#include "osi_trafficcommandupdate.pb.h"
#include "osi_trafficupdate.pb.h"

const std::unordered_map<osi3::ReaderTopLevelMessage, const google::protobuf::Descriptor*> kMessageTypeToDescriptor = {
    {osi3::ReaderTopLevelMessage::kGroundTruth, osi3::GroundTruth::descriptor()},
    {osi3::ReaderTopLevelMessage::kSensorData, osi3::SensorData::descriptor()},
    {osi3::ReaderTopLevelMessage::kSensorView, osi3::SensorView::descriptor()},
    {osi3::ReaderTopLevelMessage::kHostVehicleData, osi3::HostVehicleData::descriptor()},
    {osi3::ReaderTopLevelMessage::kTrafficCommand, osi3::TrafficCommand::descriptor()},
    {osi3::ReaderTopLevelMessage::kTrafficCommandUpdate, osi3::TrafficCommandUpdate::descriptor()},
    {osi3::ReaderTopLevelMessage::kTrafficUpdate, osi3::TrafficUpdate::descriptor()},
    {osi3::ReaderTopLevelMessage::kMotionRequest, osi3::MotionRequest::descriptor()},
    {osi3::ReaderTopLevelMessage::kStreamingUpdate, osi3::StreamingUpdate::descriptor()},
};

const google::protobuf::Descriptor* GetDescriptorForMessageType(const osi3::ReaderTopLevelMessage messageType) {
    if (const auto iterator = kMessageTypeToDescriptor.find(messageType); iterator != kMessageTypeToDescriptor.end()) {
        return iterator->second;
    }
    throw std::runtime_error("Unknown message type");
}

template <typename T>
void WriteTypedMessage(const std::optional<osi3::ReadResult>& read_result, osi3::MCAPTraceFileWriter& writer, const std::string& topic) {
    writer.WriteMessage(*static_cast<T*>(read_result->message.get()), topic);
}

void ProcessMessage(const std::optional<osi3::ReadResult>& read_result, osi3::MCAPTraceFileWriter& writer) {
    const std::string topic = "ConvertedTrace";
    switch (read_result->message_type) {
        case osi3::ReaderTopLevelMessage::kGroundTruth:
            WriteTypedMessage<osi3::GroundTruth>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kSensorData:
            WriteTypedMessage<osi3::SensorData>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kSensorView:
            WriteTypedMessage<osi3::SensorView>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kHostVehicleData:
            WriteTypedMessage<osi3::HostVehicleData>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kTrafficCommand:
            WriteTypedMessage<osi3::TrafficCommand>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kTrafficCommandUpdate:
            WriteTypedMessage<osi3::TrafficCommandUpdate>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kTrafficUpdate:
            WriteTypedMessage<osi3::TrafficUpdate>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kMotionRequest:
            WriteTypedMessage<osi3::MotionRequest>(read_result, writer, topic);
            break;
        case osi3::ReaderTopLevelMessage::kStreamingUpdate:
            WriteTypedMessage<osi3::StreamingUpdate>(read_result, writer, topic);
            break;
        default:
            std::cout << "Could not determine type of message" << std::endl;
            break;
    }
}

struct ProgramOptions {
    std::string input_file_path;
    std::string output_file_path;
    osi3::ReaderTopLevelMessage message_type = osi3::ReaderTopLevelMessage::kUnknown;
};

const std::unordered_map<std::string, osi3::ReaderTopLevelMessage> kValidTypes = {
    {"GroundTruth", osi3::ReaderTopLevelMessage::kGroundTruth},        {"SensorData", osi3::ReaderTopLevelMessage::kSensorData},
    {"SensorView", osi3::ReaderTopLevelMessage::kSensorView},          {"HostVehicleData", osi3::ReaderTopLevelMessage::kHostVehicleData},
    {"TrafficCommand", osi3::ReaderTopLevelMessage::kTrafficCommand},  {"TrafficCommandUpdate", osi3::ReaderTopLevelMessage::kTrafficCommandUpdate},
    {"TrafficUpdate", osi3::ReaderTopLevelMessage::kTrafficUpdate},    {"MotionRequest", osi3::ReaderTopLevelMessage::kMotionRequest},
    {"StreamingUpdate", osi3::ReaderTopLevelMessage::kStreamingUpdate}};

void printHelp() {
    std::cout << "Usage: convert_osi2mcap <input_file> <output_file> [--input-type <message_type>]\n\n"
              << "Arguments:\n"
              << "  input_file              Path to the input OSI trace file\n"
              << "  output_file             Path to the output MCAP file\n"
              << "  --input-type <message_type>   Optional: Specify input message type if not stated in filename\n\n"
              << "Valid message types:\n";
    for (const auto& [type, _] : kValidTypes) {
        std::cout << "  " << type << "\n";
    }
}

std::optional<ProgramOptions> parseArgs(const int argc, const char** argv) {
    if (argc < 3 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        printHelp();
        return std::nullopt;
    }

    ProgramOptions options;
    options.input_file_path = argv[1];
    options.output_file_path = argv[2];
    options.message_type = osi3::ReaderTopLevelMessage::kUnknown;

    for (int i = 3; i < argc; i++) {
        if (std::string arg = argv[i]; arg == "--input-type" && i + 1 < argc) {
            const std::string type_str = argv[++i];
            auto types_it = kValidTypes.find(type_str);
            if (types_it == kValidTypes.end()) {
                std::cerr << "Error: Invalid message type '" << type_str << "'\n\n";
                printHelp();
                return std::nullopt;
            }
            options.message_type = types_it->second;
        }
    }

    return options;
}

int main(const int argc, const char** argv) {
    const auto options = parseArgs(argc, argv);
    if (!options) {
        return 1;
    }

    std::cout << "Input file: " << options->input_file_path << std::endl;
    std::cout << "Output file: " << options->output_file_path << std::endl;

    auto tracefile_reader = osi3::NativeBinaryTraceFileReader();
    if (!tracefile_reader.Open(options->input_file_path, options->message_type)) {
        std::cerr << "ERROR: Could not open input file " << options->input_file_path << std::endl;
        return 1;
    }

    auto tracefile_writer = osi3::MCAPTraceFileWriter();
    if (!tracefile_writer.Open(options->output_file_path)) {
        std::cerr << "ERROR: Could not open output file " << options->output_file_path << std::endl;
        return 1;
    }

    const google::protobuf::Descriptor* descriptor = nullptr;
    try {
        descriptor = GetDescriptorForMessageType(tracefile_reader.GetMessageType());
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    if (!descriptor) {
        std::cerr << "ERROR: Failed to get message descriptor" << std::endl;
        return 1;
    }

    tracefile_writer.AddChannel("ConvertedTrace", descriptor);

    while (tracefile_reader.HasNext()) {
        std::cout << "reading next message\n";
        auto reading_result = tracefile_reader.ReadMessage();
        ProcessMessage(reading_result, tracefile_writer);
    }
    std::cout << "Finished native binary to mcap converter" << std::endl;
    return 0;
}