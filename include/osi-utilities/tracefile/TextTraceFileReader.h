//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#ifndef TXTHTRACEFILEREADER_H
#define TXTHTRACEFILEREADER_H

#include "Reader.h"
#include <fstream>
#include <functional>

namespace osi3 {

/**
 * @brief Function type for parsing protobuf TextFormat strings into protobuf objects
 */
using MessageParserFunc = std::function<std::unique_ptr<google::protobuf::Message>(const std::string&)>;


class TextTraceFileReader final : public TraceFileReader {
public:
    bool Open(const std::string& filename) override;
    bool Open(const std::string& filename, const ReaderTopLevelMessage message_type);
    void Close() override;
    bool HasNext() override;
    std::optional<ReadResult> ReadMessage() override;

private:
    std::ifstream trace_file_;
    MessageParserFunc parser_;                         /**< Message parsing function */
    std::string line_indicating_msg_start_="";
    ReaderTopLevelMessage message_type_{ReaderTopLevelMessage::kUnknown};  /**< Current message type */
    //MessageParserFunc parser_;
    std::string ReadNextMessageFromFile();
};

} // namespace osi3


#endif //TXTHTRACEFILEREADER_H
