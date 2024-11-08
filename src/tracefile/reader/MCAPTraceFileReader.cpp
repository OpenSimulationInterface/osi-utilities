//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/reader/MCAPTraceFileReader.h"

#include <filesystem>

namespace osi3 {

bool MCAPTraceFileReader::Open(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "ERROR: The trace file '" << filename << "' does not exist." << std::endl;
        return false;
    }

    if (const auto status = mcap_reader_.open(filename); !status.ok()) {
        std::cerr << "ERROR: Failed to open MCAP file: " << status.message << std::endl;
        return false;
    }
    message_view_ = std::make_unique<mcap::LinearMessageView>(mcap_reader_.readMessages());
    message_iterator_ = std::make_unique<mcap::LinearMessageView::Iterator>(message_view_->begin());
    return true;
}

std::optional<ReadResult> MCAPTraceFileReader::ReadMessage() {
    // check if ready and if there are messages left
    if (!this->HasNext()) {
        return std::nullopt;
    }

    const auto& msg_view = **message_iterator_;
    const auto msg = msg_view.message;
    const auto channel = msg_view.channel;
    const auto schema = msg_view.schema;

    // mcap can contain more than protobuf messages and also non-osi messages
    // todo check if it makes more sense to use mcap ReadMessageOptions

    // this function only supports osi3 protobuf messages
    if (schema->encoding != "protobuf" || schema->name.substr(0, 5) != "osi3.") {
        // read next message if the user automatically wants to skip non-osi messages
        if (skip_non_osi_msgs_) {
            ++*message_iterator_;
            return ReadMessage();
        }
        throw std::runtime_error("Unsupported message encoding: " + schema->encoding + ". Only OSI3 protobuf is supported.");
    }

    // deserialize message into pre-known osi top-level message
    // use a map based on the schema->name (which is the full protobuf message name according to the osi spec)
    // the map returns the right deserializer function which was instantiated from a template
    // the map also directly returns the top-level message type for the result struct
    ReadResult result;
    auto deserializer_it = deserializer_map_.find(schema->name);
    if (deserializer_it != deserializer_map_.end()) {
        const auto& [deserialize_fn, message_type] = deserializer_it->second;
        result.message = deserialize_fn(msg);
        result.message_type = message_type;
        result.channel_name = channel->topic;
    } else {
        throw std::runtime_error("Unsupported OSI message type: " + schema->name);
    }

    ++*message_iterator_;
    return result;
}

void MCAPTraceFileReader::Close() { mcap_reader_.close(); }

bool MCAPTraceFileReader::HasNext() {
    // not opened yet
    if (!message_iterator_) {
        return false;
    }
    // no more messages
    if (*message_iterator_ == message_view_->end()) {
        return false;
    }
    return true;
}

}  // namespace osi3