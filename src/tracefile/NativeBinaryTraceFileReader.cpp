//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/NativeBinaryTraceFileReader.h"
#include "google/protobuf/descriptor.pb.h"
#include <sstream>
#include <filesystem>


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