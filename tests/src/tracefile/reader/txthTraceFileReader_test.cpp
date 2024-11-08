//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <gtest/gtest.h>
#include "osi-utilities/tracefile/reader/txthTraceFileReader.h"
#include "osi_groundtruth.pb.h"
#include "osi_sensorview.pb.h"

#include <fstream>
#include <filesystem>

class TxthTraceFileReaderTest : public ::testing::Test {
protected:
    osi3::TxthTraceFileReader reader_;
    const std::string test_file_gt_ = "test_gt_.txth";
    const std::string test_file_sv_ = "test_sv_.txth";

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
        std::ofstream file(test_file_gt_);
        file << "version {\n";
        file << "  version_major: 3\n";
        file << "  version_minor: 7\n";
        file << "  version_patch: 0\n";
        file << "}\n";
        file << "timestamp {\n";
        file << "  seconds: 123\n";
        file << "  nanos: 456\n";
        file << "}\n";
        file << "version {\n";  // Second message delimiter
        file << "  version_major: 3\n";
        file << "  version_minor: 7\n";
        file << "  version_patch: 0\n";
        file << "}\n";
        file << "timestamp {\n";
        file << "  seconds: 789\n";
        file << "  nanos: 101112\n";
        file << "}\n";
    }

    void CreateTestSensorViewFile() const {
        std::ofstream file(test_file_sv_);
        file << "version {\n";
        file << "  version_major: 3\n";
        file << "  version_minor: 7\n";
        file << "  version_patch: 0\n";
        file << "}\n";
        file << "timestamp {\n";
        file << "  seconds: 789\n";
        file << "  nanos: 101\n";
        file << "}\n";
    }
};

TEST_F(TxthTraceFileReaderTest, OpenGroundTruthFile) {
    EXPECT_TRUE(reader_.Open(test_file_gt_));
}

TEST_F(TxthTraceFileReaderTest, OpenSensorViewFile) {
    EXPECT_TRUE(reader_.Open(test_file_sv_));
}

TEST_F(TxthTraceFileReaderTest, OpenWithExplicitMessageType) {
    EXPECT_TRUE(reader_.Open(test_file_gt_, osi3::ReaderTopLevelMessage::kGroundTruth));
}

TEST_F(TxthTraceFileReaderTest, ReadGroundTruthMessage) {
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

TEST_F(TxthTraceFileReaderTest, ReadSensorViewMessage) {
    ASSERT_TRUE(reader_.Open(test_file_sv_));
    EXPECT_TRUE(reader_.HasNext());

    const auto result = reader_.ReadMessage();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->message_type, osi3::ReaderTopLevelMessage::kSensorView);

    auto* sv = dynamic_cast<osi3::SensorView*>(result->message.get());
    ASSERT_NE(sv, nullptr);
    EXPECT_EQ(sv->timestamp().seconds(), 789);
    EXPECT_EQ(sv->timestamp().nanos(), 101);
}

TEST_F(TxthTraceFileReaderTest, HasNextReturnsFalseWhenEmpty) {
    ASSERT_TRUE(reader_.Open(test_file_gt_));
    ASSERT_TRUE(reader_.HasNext());
    reader_.ReadMessage();
    reader_.ReadMessage();
    EXPECT_FALSE(reader_.HasNext());
}

TEST_F(TxthTraceFileReaderTest, OpenNonexistentFile) {
    EXPECT_FALSE(reader_.Open("nonexistent_file.txth"));
}

TEST_F(TxthTraceFileReaderTest, OpenInvalidFileExtension) {
    const std::string invalid_file = "invalid.txt";
    {
        std::ofstream file(invalid_file);
        file << "Invalid data";
    }
    EXPECT_FALSE(reader_.Open(invalid_file));
    std::filesystem::remove(invalid_file);
}

TEST_F(TxthTraceFileReaderTest, OpenInvalidMessageType) {
    const std::string invalid_file = "invalid_type.txth";
    {
        std::ofstream file(invalid_file);
        file << "InvalidType:\n";
        file << "some data\n";
    }
    EXPECT_FALSE(reader_.Open(invalid_file));
    std::filesystem::remove(invalid_file);
}

TEST_F(TxthTraceFileReaderTest, ReadEmptyFile) {
    std::string empty_file = "empty.txth";
    {
        std::ofstream file(empty_file);
    }
    ASSERT_TRUE(reader_.Open(empty_file, osi3::ReaderTopLevelMessage::kGroundTruth));
    EXPECT_FALSE(reader_.HasNext());
    std::filesystem::remove(empty_file);
}

TEST_F(TxthTraceFileReaderTest, ReadInvalidMessageFormat) {
    std::string invalid_format_file = "invalid_format_gt_99.txth";
    {
        std::ofstream file(invalid_format_file);
        file << "GroundTruth:\n";
        file << "invalid protobuf format\n";
    }
    ASSERT_TRUE(reader_.Open(invalid_format_file));
    EXPECT_THROW(reader_.ReadMessage(), std::runtime_error);
    std::filesystem::remove(invalid_format_file);
}
