//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/reader/txthTraceFileReader.h"

#include <filesystem>

namespace osi3 {

bool TxthTraceFileReader::Open(const std::string& filename) {
    if (filename.find(".txth") == std::string::npos) {
        std::cerr << "ERROR: The trace file '" << filename << "' must have a '.txth' extension." << std::endl;
        return false;
    }

    if (!std::filesystem::exists(filename)) {
        std::cerr << "ERROR: The trace file '" << filename << "' does not exist." << std::endl;
        return false;
    }

    if (message_type_ == ReaderTopLevelMessage::kUnknown) {
        for (const auto& [key, value] : kFileNameMessageTypeMap) {
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

    parser_ = kParserMap_.at(message_type_);

    trace_file_ = std::ifstream(filename);

    // find top-level message delimiter by peeking into the file and assuming the first line
    // will be the pattern to indicate a new message
    std::getline(trace_file_, line_indicating_msg_start_);
    trace_file_.seekg(std::ios_base::beg);

    return trace_file_.is_open();
}

bool TxthTraceFileReader::Open(const std::string& filename, const ReaderTopLevelMessage message_type) {
    message_type_ = message_type;
    return Open(filename);
}

void TxthTraceFileReader::Close() { trace_file_.close(); }

bool TxthTraceFileReader::HasNext() { return trace_file_ && !trace_file_.eof(); }

std::optional<ReadResult> TxthTraceFileReader::ReadMessage() {
    if (!trace_file_) {
        return std::nullopt;
    }

    const std::string text_message = ReadNextMessageFromFile();
    if (text_message.empty()) {
        return std::nullopt;
    }

    ReadResult result;
    result.message = parser_(text_message);
    result.message_type = message_type_;
    return result;
}

std::string TxthTraceFileReader::ReadNextMessageFromFile() {
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
    if (!trace_file_.eof()) {  // go back to the line before line_indicating_msg_start_
        trace_file_.seekg(last_position, std::ios_base::beg);
    }

    return message;
}

}  // namespace osi3