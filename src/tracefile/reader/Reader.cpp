//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include "osi-utilities/tracefile/Reader.h"

#include "osi-utilities/tracefile/reader/NativeBinaryTraceFileReader.h"
#include "osi-utilities/tracefile/reader/txthTraceFileReader.h"

std::unique_ptr<osi3::TraceFileReader> createTraceFileReader(const std::string& format) {
    if (format == "mcap") {
        throw std::invalid_argument("Format not implemented yet");
        // return std::make_unique<MCAPTraceFileReader>();
    } else if (format == "osi") {
        return std::make_unique<osi3::NativeBinaryTraceFileReader>();
    } else if (format == "txth") {
        return std::make_unique<osi3::TxthTraceFileReader>();
    } else {
        throw std::invalid_argument("Unsupported format: " + format);
    }
}