//
// Copyright (c) 2024, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// SPDX-License-Identifier: MPL-2.0
//

#include <gtest/gtest.h>
#include "osi-utilities/tracefile/writer/MCAPTraceFileWriter.h"
#include "osi_groundtruth.pb.h"
#include <filesystem>

class MCAPTraceFileWriterTest : public ::testing::Test {
protected:
    osi3::MCAPTraceFileWriter writer_;
    const std::string test_file_ = "test_output.mcap";

    void TearDown() override {
        writer_.Close();
        std::filesystem::remove(test_file_);
    }
};

TEST_F(MCAPTraceFileWriterTest, OpenCloseFile) {
    EXPECT_TRUE(writer_.Open(test_file_));
    EXPECT_TRUE(std::filesystem::exists(test_file_));
    writer_.Close();
}


TEST_F(MCAPTraceFileWriterTest, WriteMessage) {
    ASSERT_TRUE(writer_.Open(test_file_));

    // Create test message
    osi3::GroundTruth gt;
    gt.mutable_timestamp()->set_seconds(123);
    gt.mutable_timestamp()->set_nanos(456);

    // Add channel for ground truth
    const std::string topic = "/ground_truth";
    writer_.AddChannel(topic, osi3::GroundTruth::descriptor(), {});

    // Write message
    EXPECT_TRUE(writer_.WriteMessage(gt, topic));
}

TEST_F(MCAPTraceFileWriterTest, SetMetadata) {
    ASSERT_TRUE(writer_.Open(test_file_));

    std::unordered_map<std::string, std::string> metadata{
        {"key1", "value1"},
        {"key2", "value2"}
    };

    EXPECT_TRUE(writer_.SetMetadata("test_metadata", metadata));
}

TEST_F(MCAPTraceFileWriterTest, AddChannel) {
    ASSERT_TRUE(writer_.Open(test_file_));

    const std::string topic = "/ground_truth";
    const std::unordered_map<std::string, std::string> channel_metadata{
        {"description", "Test channel"}
    };

    const uint16_t channel_id = writer_.AddChannel(topic,
                                                   osi3::GroundTruth::descriptor(),
                                                   channel_metadata);
    EXPECT_GT(channel_id, 0);

    // Test adding same topic with same schema
    const uint16_t second_id = writer_.AddChannel(topic,
                                                  osi3::GroundTruth::descriptor(),
                                                  channel_metadata);
    EXPECT_EQ(channel_id, second_id);
}

TEST_F(MCAPTraceFileWriterTest, WriteMessageWithoutChannel) {
    ASSERT_TRUE(writer_.Open(test_file_));

    osi3::GroundTruth gt;
    gt.mutable_timestamp()->set_seconds(123);

    // Try writing without adding channel first
    EXPECT_FALSE(writer_.WriteMessage(gt, "/ground_truth"));
}

TEST_F(MCAPTraceFileWriterTest, WriteMessageWithEmptyTopic) {
    ASSERT_TRUE(writer_.Open(test_file_));

    osi3::GroundTruth gt;
    EXPECT_FALSE(writer_.WriteMessage(gt, ""));
}