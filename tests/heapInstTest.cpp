#include <gtest/gtest.h>

#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

extern "C" {
#include "heapInst/heapInst.h"
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

struct RecordingTransport {
    std::vector<std::vector<uint8_t>> writes;
    bool fail_write = false;

    static int Write(const void* data, size_t len, void* ctx)
    {
        auto* self = static_cast<RecordingTransport*>(ctx);
        if (self->fail_write) {
            return -1;
        }
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        self->writes.emplace_back(bytes, bytes + len);
        return static_cast<int>(len);
    }

    static int Flush(void* ctx)
    {
        (void)ctx;
        return 0;
    }

    static int Close(void* ctx)
    {
        (void)ctx;
        return 0;
    }
};

class HeapInstTest : public ::testing::Test
{
   protected:
    RecordingClock clock_;
    RecordingLog log_;
    RecordingTransport transport_;

    void SetUp() override
    {
#ifdef HEAPINST_TEST_API
        heap_inst_test_reset();
#endif
        clock_.current = 100;
        log_.buffer.clear();
        transport_.writes.clear();
        transport_.fail_write = false;
        heap_inst_transport_t t = {
            .write = &RecordingTransport::Write,
            .flush = &RecordingTransport::Flush,
            .close = &RecordingTransport::Close,
            .ctx = &transport_,
        };
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

        heap_inst_register_transport(&t);
        heap_inst_register_platform_hooks(&hooks);
    }

    void TearDown() override { heap_inst_flush(); }
};

}  // namespace

TEST_F(HeapInstTest, InitAddsSingleRecord)
{
    heap_inst_init();
    EXPECT_TRUE(heap_inst_is_initialized());
    EXPECT_EQ(heap_inst_get_buffer_count(), 1u);

    heap_inst_flush();
    ASSERT_EQ(transport_.writes.size(), 1u);
    ASSERT_EQ(transport_.writes[0].size(), sizeof(heap_inst_record_t) * 1);

    heap_inst_record_t rec;
    std::memcpy(&rec, transport_.writes[0].data(), sizeof(rec));
    EXPECT_EQ(rec.operation, HEAP_OP_INIT);
    EXPECT_EQ(rec.timestamp_us, 100u);
}

TEST_F(HeapInstTest, RecordsMallocAndFree)
{
    heap_inst_init();
    void* ptr = heap_inst_malloc(16);
    ASSERT_NE(ptr, nullptr);
    heap_inst_free(ptr);

    heap_inst_flush();
    ASSERT_EQ(transport_.writes.size(), 1u);
    const auto& buf = transport_.writes[0];
    ASSERT_EQ(buf.size(), sizeof(heap_inst_record_t) * 3);

    std::vector<heap_inst_record_t> recs(3);
    std::memcpy(recs.data(), buf.data(), buf.size());
    EXPECT_EQ(recs[0].operation, HEAP_OP_INIT);
    EXPECT_EQ(recs[1].operation, HEAP_OP_MALLOC);
    EXPECT_EQ(recs[1].arg1, 16u);
    EXPECT_EQ(recs[1].arg2, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
    EXPECT_EQ(recs[2].operation, HEAP_OP_FREE);
    EXPECT_EQ(recs[2].arg1, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr)));
}

TEST_F(HeapInstTest, FlushesWhenBufferFull)
{
    heap_inst_init();
    size_t capacity = heap_inst_get_buffer_capacity();

    for (size_t i = 0; i < capacity; ++i) {
        heap_inst_malloc(4);
    }

    heap_inst_flush();

    ASSERT_EQ(transport_.writes.size(), 2u);
    size_t record_size = sizeof(heap_inst_record_t);
    EXPECT_EQ(transport_.writes[0].size(), capacity * record_size);
    EXPECT_EQ(transport_.writes[1].size(), 1 * record_size);
}

TEST_F(HeapInstTest, FallsBackToTextWhenTransportFails)
{
    transport_.fail_write = true;

    heap_inst_init();
    heap_inst_malloc(8);
    heap_inst_flush();

    EXPECT_TRUE(transport_.writes.empty());
    ASSERT_NE(log_.buffer.find("HEAP_TRACE_START"), std::string::npos)
        << "log buffer:\n"
        << log_.buffer;
    ASSERT_NE(log_.buffer.find("OP:1"), std::string::npos)  // HEAP_OP_MALLOC
        << "log buffer:\n"
        << log_.buffer;
    EXPECT_EQ(heap_inst_get_buffer_count(), 0u);
}
