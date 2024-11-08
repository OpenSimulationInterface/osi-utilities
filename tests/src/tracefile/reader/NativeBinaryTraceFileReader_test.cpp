//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <gtest/gtest.h>
#include "osi-utilities/tracefile/reader/NativeBinaryTraceFileReader.h"
#include "osi_groundtruth.pb.h"
#include "osi_sensorview.pb.h"

#include <fstream>
#include <filesystem>

class NativeBinaryTraceFileReaderTest : public ::testing::Test {
protected:
    osi3::NativeBinaryTraceFileReader reader_;
    const std::string test_file_gt_ = "test_gt_.osi";
    const std::string test_file_sv_ = "test_sv_.osi";

    void SetUp() override {
        CreateTestGroundTruthFile();
        CreateTestSensorViewFile();
    }

    void TearDown() override {
        reader_.Close();
        std::filesystem::remove(test_file_gt_);
        std::filesystem::remove(test_file_sv_);
    }

private:
    void CreateTestGroundTruthFile() const {
        std::ofstream file(test_file_gt_, std::ios::binary);
        osi3::GroundTruth gt;
        gt.mutable_timestamp()->set_seconds(123);
        gt.mutable_timestamp()->set_nanos(456);

        std::string serialized = gt.SerializeAsString();
        uint32_t size = serialized.size();

        file.write(reinterpret_cast<char*>(&size), sizeof(size));
        file.write(serialized.data(), size);
    }

    void CreateTestSensorViewFile() const {
        std::ofstream file(test_file_sv_, std::ios::binary);
        osi3::SensorView sv;
        sv.mutable_timestamp()->set_seconds(789);
        sv.mutable_timestamp()->set_nanos(101);

        std::string serialized = sv.SerializeAsString();
        uint32_t size = serialized.size();

        file.write(reinterpret_cast<char*>(&size), sizeof(size));
        file.write(serialized.data(), size);
    }
};

TEST_F(NativeBinaryTraceFileReaderTest, OpenGroundTruthFile) {
    EXPECT_TRUE(reader_.Open(test_file_gt_));
}

TEST_F(NativeBinaryTraceFileReaderTest, OpenSensorViewFile) {
    EXPECT_TRUE(reader_.Open(test_file_sv_));
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadGroundTruthMessage) {
    ASSERT_TRUE(reader_.Open(test_file_gt_));
    EXPECT_TRUE(reader_.HasNext());

    const auto result = reader_.ReadMessage();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->message_type, osi3::ReaderTopLevelMessage::kGroundTruth);

    auto* gt = dynamic_cast<osi3::GroundTruth*>(result->message.get());
    ASSERT_NE(gt, nullptr);
    EXPECT_EQ(gt->timestamp().seconds(), 123);
    EXPECT_EQ(gt->timestamp().nanos(), 456);
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadSensorViewMessage) {
    ASSERT_TRUE(reader_.Open(test_file_sv_));
    EXPECT_TRUE(reader_.HasNext());

    auto result = reader_.ReadMessage();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->message_type, osi3::ReaderTopLevelMessage::kSensorView);

    auto* sv = dynamic_cast<osi3::SensorView*>(result->message.get());
    ASSERT_NE(sv, nullptr);
    EXPECT_EQ(sv->timestamp().seconds(), 789);
    EXPECT_EQ(sv->timestamp().nanos(), 101);
}

TEST_F(NativeBinaryTraceFileReaderTest, HasNextReturnsFalseWhenEmpty) {
    ASSERT_TRUE(reader_.Open(test_file_gt_));
    ASSERT_TRUE(reader_.HasNext());
    reader_.ReadMessage();
    EXPECT_FALSE(reader_.HasNext());
}

TEST_F(NativeBinaryTraceFileReaderTest, OpenNonexistentFile) {
    EXPECT_FALSE(reader_.Open("nonexistent_file.osi"));
}

TEST_F(NativeBinaryTraceFileReaderTest, OpenInvalidFileFormat) {
    std::string invalid_file = "invalid.bin";
    {
        std::ofstream file(invalid_file, std::ios::binary);
        file << "Invalid data";
    }

    EXPECT_FALSE(reader_.Open(invalid_file));
    std::filesystem::remove(invalid_file);

    invalid_file = "invalid_filename.osi";
    {
        std::ofstream file(invalid_file, std::ios::binary);
        file << "Invalid data";
    }
    EXPECT_FALSE(reader_.Open(invalid_file));
    std::filesystem::remove(invalid_file);
}

TEST_F(NativeBinaryTraceFileReaderTest, OpenWithExplicitMessageType) {
    EXPECT_TRUE(reader_.Open(test_file_gt_, osi3::ReaderTopLevelMessage::kGroundTruth));
    EXPECT_TRUE(reader_.Open(test_file_sv_, osi3::ReaderTopLevelMessage::kSensorView));
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadEmptyMessage) {
    std::string empty_file = "empty_sv_99.osi";
    {
        std::ofstream file(empty_file, std::ios::binary);
        uint32_t size = 0;
        file.write(reinterpret_cast<char*>(&size), sizeof(size));
    }

    ASSERT_TRUE(reader_.Open(empty_file));
    EXPECT_THROW(reader_.ReadMessage(), std::runtime_error);
    std::filesystem::remove(empty_file);
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadCorruptedMessageSize) {
    std::string corrupted_file = "corrupted_size_sv_99.osi";
    {
        std::ofstream file(corrupted_file, std::ios::binary);
        uint32_t invalid_size = 0xFFFFFFFF;
        file.write(reinterpret_cast<char*>(&invalid_size), sizeof(invalid_size));
    }

    ASSERT_TRUE(reader_.Open(corrupted_file));
    EXPECT_THROW(reader_.ReadMessage(), std::runtime_error);
    std::filesystem::remove(corrupted_file);
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadCorruptedMessageContent) {
    std::string corrupted_file = "corrupted_content_sv_99.osi";
    {
        std::ofstream file(corrupted_file, std::ios::binary);
        uint32_t size = 100;
        file.write(reinterpret_cast<char*>(&size), sizeof(size));
        // Write fewer data than specified in size
        std::string incomplete_data = "incomplete";
        file.write(incomplete_data.c_str(), incomplete_data.size());
    }

    ASSERT_TRUE(reader_.Open(corrupted_file));
    EXPECT_THROW(reader_.ReadMessage(), std::runtime_error);
    std::filesystem::remove(corrupted_file);
}

TEST_F(NativeBinaryTraceFileReaderTest, ReadMessageAfterClose) {
    ASSERT_TRUE(reader_.Open(test_file_gt_));
    reader_.Close();
    EXPECT_FALSE(reader_.HasNext());
    auto result = reader_.ReadMessage();
    EXPECT_FALSE(result.has_value());
}