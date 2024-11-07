//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/Writer.h"

#include "osi-utilities/tracefile/writer/MCAPTraceFileWriter.h"

// todo use enum or something else instead of a string
// todo for the writer use mcap as default if the "string" is empty
// todo makes a writer factory really sense?
std::unique_ptr<osi3::TraceFileWriter> CreateTraceFileWriter(const std::string& format) {
    if (format == "mcap") {
        // throw std::invalid_argument("Format not implemented yet");
        return std::make_unique<osi3::MCAPTraceFileWriter>();
    } else if (format == "osi") {
        throw std::invalid_argument("Format not implemented yet");
        // return std::make_unique<OSIBinaryTraceFileWriter>();
    } else if (format == "txth") {
        throw std::invalid_argument("Format not implemented yet");
        // return std::make_unique<TxtTraceFileWriter>();
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }
}