//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/Writer.h"

#include "osi-utilities/tracefile/writer/MCAPTraceFileWriter.h"
#include "osi-utilities/tracefile/writer/NativeBinaryTraceFileWriter.h"
#include "osi-utilities/tracefile/writer/txthTraceFileWriter.h"

// todo use enum or something else instead of a string
// todo for the writer use mcap as default if the "string" is empty
// todo makes a writer factory really sense?
std::unique_ptr<osi3::TraceFileWriter> CreateTraceFileWriter(const std::string& format) {
    if (format == "mcap") {
        return std::make_unique<osi3::MCAPTraceFileWriter>();
    } else if (format == "osi") {
        return std::make_unique<osi3::NativeBinaryTraceFileWriter>();
    } else if (format == "txth") {
        return std::make_unique<osi3::TxthTraceFileWriter>();
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }
}