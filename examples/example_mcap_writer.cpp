//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <osi-utilities/tracefile/writer/MCAPTraceFileWriter.h>

#include <filesystem>

#include "osi_sensorview.pb.h"
#include "osi_version.pb.h"

std::string GenerateTempFilePath() {
    const auto path = std::filesystem::temp_directory_path() / "example_mcap.mcap";
    return path.string();
}

int main(int argc, const char** argv) {
    std::cout << "Starting MCAP Writer example:" << std::endl;

    // Create writer and open file
    auto tracefile_writer = osi3::MCAPTraceFileWriter();
    const auto tracefile_path = GenerateTempFilePath();
    std::cout << "Creating tracefile at " << tracefile_path << std::endl;
    tracefile_writer.Open(tracefile_path);

    // add a channel to store some data
    const std::string topic = "Sensor_1_Input";
    const std::unordered_map<std::string, std::string> channel_metadata = {{"description", "This channel contains the output of the sensor 1"}};
    tracefile_writer.AddChannel(topic, osi3::SensorView::descriptor(), channel_metadata);

    // create OSI data to store
    const auto osi_version = osi3::InterfaceVersion::descriptor()->file()->options().GetExtension(osi3::current_interface_version);

    osi3::SensorView sensor_view_1;
    sensor_view_1.mutable_version()->CopyFrom(osi_version);
    sensor_view_1.mutable_sensor_id()->set_value(0);

    auto* const ground_truth_1 = sensor_view_1.mutable_global_ground_truth();
    ground_truth_1->mutable_version()->CopyFrom(osi_version);

    auto* const host_vehicle = ground_truth_1->mutable_moving_object()->Add();
    host_vehicle->mutable_id()->set_value(12);
    host_vehicle->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_SMALL_CAR);
    host_vehicle->mutable_base()->mutable_dimension()->set_length(5);
    host_vehicle->mutable_base()->mutable_dimension()->set_width(2);
    host_vehicle->mutable_base()->mutable_dimension()->set_height(1.5);
    host_vehicle->mutable_base()->mutable_velocity()->set_x(10.0);

    // write the data continuously in a loop
    constexpr double kTimeStepSizeS = 0.1;  // NOLINT
    for (int i = 0; i < 10; ++i) {
        // manipulate the data so not every message is the same
        auto timestamp = sensor_view_1.timestamp().seconds() * 1000000000 + sensor_view_1.timestamp().nanos();
        timestamp += 100000000;
        sensor_view_1.mutable_timestamp()->set_nanos(timestamp % 1000000000);
        sensor_view_1.mutable_timestamp()->set_seconds(timestamp / 1000000000);
        ground_truth_1->mutable_timestamp()->set_nanos(timestamp % 1000000000);
        ground_truth_1->mutable_timestamp()->set_seconds(timestamp / 1000000000);
        const auto old_position = host_vehicle->base().position().x();
        const auto new_position = old_position + host_vehicle->base().velocity().x() + kTimeStepSizeS;
        host_vehicle->mutable_base()->mutable_position()->set_x(new_position);

        // finally write the data using topic
        tracefile_writer.WriteMessage(sensor_view_1, topic);
    }

    tracefile_writer.Close();

    std::cout << "Finished MCAP Writer example" << std::endl;
}