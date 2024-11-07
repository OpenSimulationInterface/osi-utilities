//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//


#ifndef OSIUTILITIES_TRACEFILE_WRITER_TXTHTRACEFILEWRITER_H_
#define OSIUTILITIES_TRACEFILE_WRITER_TXTHTRACEFILEWRITER_H_

#include "../Writer.h"
#include <google/protobuf/text_format.h>
#include <fstream>

namespace osi3 {

class txthTraceFileWriter final : public TraceFileWriter {
public:
    bool Open(const std::string& filename) override;
    void Close() override;
    bool SetMetadata(const std::string& name, const std::unordered_map<std::string, std::string>& metadata_entries) { return false; }

    template <typename T>
    bool WriteMessage(T top_level_message);

private:
    std::ofstream trace_file_;
    bool file_open_{false};
};

} // namespace osi3
#endif