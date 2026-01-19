#include <gtest/gtest.h>

#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

extern "C" {
#include "heapInst/heapInst.h"
#include "heapInstStream.h"
#ifdef HEAPINST_TEST_API
void heap_inst_test_reset(void);
#endif
}

namespace
{

struct RecordingClock {
    uint64_t current = 100;
    static uint64_t Now(void* ctx)
    {
        auto* self = static_cast<RecordingClock*>(ctx);
        return self->current++;
    }
};

struct RecordingLog {
    std::string buffer;
    static void Log(const char* msg, void* ctx)
    {
        auto* self = static_cast<RecordingLog*>(ctx);
        self->buffer.append(msg);
    }
};

class HeapInstTest : public ::testing::Test
{
   protected:
    RecordingClock clock_;
    RecordingLog log_;

    void SetUp() override
    {
#ifdef HEAPINST_TEST_API
        heap_inst_test_reset();
#endif
        test_reset_stream_buffer();
        clock_.current = 100;
        log_.buffer.clear();

        heap_inst_platform_hooks_t hooks = {
            .timestamp_us = &RecordingClock::Now,
            .log = &RecordingLog::Log,
            .lock = nullptr,
            .unlock = nullptr,
            .timestamp_ctx = &clock_,
            .log_ctx = &log_,
            .lock_ctx = nullptr,
            .unlock_ctx = nullptr,
        };

        heap_inst_register_platform_hooks(&hooks);
    }

    void TearDown() override
    {
        heap_inst_flush();
        test_reset_stream_buffer();
    }

    // Helper to get records from the stream buffer
    std::vector<heap_inst_record_t> GetStreamRecords()
    {
        const uint8_t* buf = test_get_stream_buffer();
        size_t size = test_get_stream_buffer_size();
        size_t num_records = size / sizeof(heap_inst_record_t);

        std::vector<heap_inst_record_t> records(num_records);
        if (num_records > 0) {
            std::memcpy(records.data(), buf, num_records * sizeof(heap_inst_record_t));
        }
        return records;
    }
};

}  // namespace

TEST_F(HeapInstTest, InitAddsSingleRecord)
{
    heap_inst_init();
    EXPECT_TRUE(heap_inst_is_initialized());
    EXPECT_EQ(heap_inst_get_buffer_count(), 1u);

    heap_inst_flush();

    auto records = GetStreamRecords();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(records[0].operation, HEAP_OP_INIT);
    EXPECT_EQ(records[0].timestamp_us, 100u);
}

TEST_F(HeapInstTest, RecordsMallocAndFree)
{
    heap_inst_init();
    void* ptr = malloc(16);
    ASSERT_NE(ptr, nullptr);
    free(ptr);

    heap_inst_flush();

    auto records = GetStreamRecords();
    ASSERT_EQ(records.size(), 3u);

    EXPECT_EQ(records[0].operation, HEAP_OP_INIT);
    EXPECT_EQ(records[1].operation, HEAP_OP_MALLOC);
    EXPECT_EQ(records[1].arg1, 16u);
    EXPECT_EQ(records[1].arg2, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
    EXPECT_EQ(records[2].operation, HEAP_OP_FREE);
    EXPECT_EQ(records[2].arg1, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
}

TEST_F(HeapInstTest, FlushesWhenBufferFull)
{
    heap_inst_init();
    size_t capacity = heap_inst_get_buffer_capacity();

    // Fill the buffer to trigger auto-flush
    for (size_t i = 0; i < capacity; ++i) {
        void* ptr = malloc(4);
        (void)ptr;
    }

    heap_inst_flush();

    // Should have all records in the stream buffer
    auto records = GetStreamRecords();
    // capacity records from first flush + 1 from final flush
    EXPECT_EQ(records.size(), capacity + 1);
}

TEST_F(HeapInstTest, FallsBackToTextWhenStreamportFails)
{
    // Set streamport to fail mode BEFORE init
    test_set_stream_fail_mode(0);  // Fail immediately on any write

    heap_inst_init();
    void* ptr = malloc(8);
    (void)ptr;
    heap_inst_flush();

    // Stream buffer should be empty (all writes failed)
    EXPECT_EQ(test_get_stream_buffer_size(), 0u);

    // Should have fallen back to text logging
    ASSERT_NE(log_.buffer.find("HEAP_TRACE_START"), std::string::npos)
        << "log buffer:\n"
        << log_.buffer;
    ASSERT_NE(log_.buffer.find("OP:1"), std::string::npos)  // HEAP_OP_MALLOC
        << "log buffer:\n"
        << log_.buffer;
    EXPECT_EQ(heap_inst_get_buffer_count(), 0u);
}

TEST_F(HeapInstTest, RecordsRealloc)
{
    heap_inst_init();
    void* ptr = malloc(16);
    ASSERT_NE(ptr, nullptr);
    void* new_ptr = realloc(ptr, 32);
    ASSERT_NE(new_ptr, nullptr);
    free(new_ptr);

    heap_inst_flush();

    auto records = GetStreamRecords();
    ASSERT_EQ(records.size(), 4u);

    EXPECT_EQ(records[0].operation, HEAP_OP_INIT);
    EXPECT_EQ(records[1].operation, HEAP_OP_MALLOC);
    EXPECT_EQ(records[2].operation, HEAP_OP_REALLOC);
    EXPECT_EQ(records[2].arg1, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
    EXPECT_EQ(records[2].arg2, 32u);
    EXPECT_EQ(records[2].arg3, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(new_ptr)));
    EXPECT_EQ(records[3].operation, HEAP_OP_FREE);
}

TEST_F(HeapInstTest, TimestampsIncrement)
{
    heap_inst_init();
    void* ptr = malloc(8);
    void* ptr2 = malloc(16);
    (void)ptr;
    (void)ptr2;
    heap_inst_flush();

    auto records = GetStreamRecords();
    ASSERT_EQ(records.size(), 3u);

    // Timestamps should be incrementing (100, 101, 102)
    EXPECT_EQ(records[0].timestamp_us, 100u);
    EXPECT_EQ(records[1].timestamp_us, 101u);
    EXPECT_EQ(records[2].timestamp_us, 102u);
}
