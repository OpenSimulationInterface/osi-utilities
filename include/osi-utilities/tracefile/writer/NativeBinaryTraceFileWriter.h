//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//


#ifndef OSIUTILITIES_TRACEFILE_NATIVEBINARYTRACEFILEWRITER_H_
#define OSIUTILITIES_TRACEFILE_NATIVEBINARYTRACEFILEWRITER_H_

#include "osi-utilities/tracefile/Writer.h"
#include <fstream>

namespace osi3 {

class NativeBinaryTraceFileWriter : public TraceFileWriter {
public:
    bool Open(const std::string& filename) override;
    void Close() override;
    bool SetMetadata(const std::string& /*name*/, const std::unordered_map<std::string, std::string>& /*metadata_entries*/) override { return true; }

    template <typename T>
    bool WriteMessage(T top_level_message);

    template <typename T>
    bool WriteMessage(T /*top_level_message*/, const std::string& /*topic*/) { return false; }

private:
    std::ofstream trace_file_;
    bool file_open_ = false;
};

} // namespace osi3
#endif