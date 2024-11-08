//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <osi-utilities/tracefile/reader/MCAPTraceFileReader.h>

#include <filesystem>
#include <optional>

#include "osi_groundtruth.pb.h"
#include "osi_hostvehicledata.pb.h"
#include "osi_motionrequest.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"
#include "osi_streamingupdate.pb.h"
#include "osi_trafficcommand.pb.h"
#include "osi_trafficcommandupdate.pb.h"
#include "osi_trafficupdate.pb.h"

template <typename T>
void PrintTimestamp(T msg) {
    auto timestamp = msg->timestamp().seconds() + msg->timestamp().nanos() / 1000000000.0;
    std::cout << "Type: " << msg->descriptor()->full_name() << " Timestamp " << timestamp << "\n";
}

std::string GetTempFilePath() {
    const auto path = std::filesystem::temp_directory_path() / "example_mcap.mcap";
    return path.string();
}

int main(int argc, const char** argv) {
    std::cout << "Starting MCAP Reader example:" << std::endl;

    // Create reader and open file
    auto tracefile_reader = osi3::MCAPTraceFileReader();
    const auto tracefile_path = GetTempFilePath();
    std::cout << "Reading tracefile from " << tracefile_path << std::endl;
    tracefile_reader.Open(tracefile_path);

    // Read messages in a loop until no more messages are available
    while (tracefile_reader.HasNext()) {
        const auto reading_result = tracefile_reader.ReadMessage();
        if (!reading_result) {
            std::cerr << "Error reading message." << std::endl;
            continue;
        }

        // we need to cast the pointer during runtime to allow multiple different trace files to be read
        switch (reading_result->message_type) {
            case osi3::ReaderTopLevelMessage::kGroundTruth: {
                auto* const ground_truth = dynamic_cast<osi3::GroundTruth*>(reading_result->message.get());
                PrintTimestamp(ground_truth);
                break;
            }
            case osi3::ReaderTopLevelMessage::kSensorData: {
                auto* const sensor_data = dynamic_cast<osi3::SensorData*>(reading_result->message.get());
                PrintTimestamp(sensor_data);
                break;
            }
            case osi3::ReaderTopLevelMessage::kSensorView: {
                auto* const sensor_view = dynamic_cast<osi3::SensorView*>(reading_result->message.get());
                PrintTimestamp(sensor_view);
                break;
            }
            case osi3::ReaderTopLevelMessage::kHostVehicleData: {
                auto* const host_vehicle_data = dynamic_cast<osi3::HostVehicleData*>(reading_result->message.get());
                PrintTimestamp(host_vehicle_data);
                break;
            }
            case osi3::ReaderTopLevelMessage::kTrafficCommand: {
                auto* const traffic_command = dynamic_cast<osi3::TrafficCommand*>(reading_result->message.get());
                PrintTimestamp(traffic_command);
                break;
            }
            case osi3::ReaderTopLevelMessage::kTrafficCommandUpdate: {
                auto* const traffic_command_update = dynamic_cast<osi3::TrafficCommandUpdate*>(reading_result->message.get());
                PrintTimestamp(traffic_command_update);
                break;
            }
            case osi3::ReaderTopLevelMessage::kTrafficUpdate: {
                auto* const traffic_update = dynamic_cast<osi3::TrafficUpdate*>(reading_result->message.get());
                PrintTimestamp(traffic_update);
                break;
            }
            case osi3::ReaderTopLevelMessage::kMotionRequest: {
                auto* const motion_request = dynamic_cast<osi3::MotionRequest*>(reading_result->message.get());
                PrintTimestamp(motion_request);
                break;
            }
            case osi3::ReaderTopLevelMessage::kStreamingUpdate: {
                auto* const streaming_update = dynamic_cast<osi3::StreamingUpdate*>(reading_result->message.get());
                PrintTimestamp(streaming_update);
                break;
            }
            default: {
                std::cout << "Could not determine type of message" << std::endl;
                break;
            }
        }
    }

    tracefile_reader.Close();

    std::cout << "Finished MCAP Reader example" << std::endl;
    return 0;
}