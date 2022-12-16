#ifndef CPPDS_DATASTREAM_H
#define CPPDS_DATASTREAM_H

#pragma once

#include "cppdatastream/Utilities.hpp"

#ifdef _MSC_VER
    #pragma warning(push, 1)
#endif
#include "cppdatastream/external/queues/blockingconcurrentqueue.h"
#include "cppdatastream/external/queues/concurrentqueue.h"
#include "cppdatastream/external/queues/readerwriterqueue.h"
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#ifdef WIN32
    #define NOMINMAX
    #undef min
    #undef max
#endif

#include <boost/fiber/all.hpp>

#include <any>

namespace mc = moodycamel;

class MetaData
{
public:
    explicit MetaData() = default;
};

class SharedDataBlock
{
public:
    SharedDataBlock() = default;

    virtual ~SharedDataBlock() = default;

    template <typename T>
    auto data() const -> const T&
    {
        return *std::any_cast<T>(_data.get());
    }

    bool isEndOfProcessing() const { return _eop; }

protected:
    /// Pointer to core data type
    std::shared_ptr<std::any> _data;
    /// @brief End of processing
    bool _eop{false};
};

class WritableDataBlock : public SharedDataBlock
{
public:
    WritableDataBlock() = default;

    virtual ~WritableDataBlock() = default;

    void setEndOfProcessing() { this->_eop = true; }

    template <typename T>
    auto data() const -> std::shared_ptr<T>
    {
        return _data;
    }

    void setData(const auto& data) { _data = std::make_shared<std::any>(std::any(data)); }

    auto asShared() const -> SharedDataBlock { return *this; }
};

class DataStream
{
public:
    DataStream() = default;

    virtual ~DataStream() { logDtor("DataStream DTOR: {}\n", className()); }

    const std::string className() const { return demangleName(typeid(*this).name()); }

    /// @brief Connects the provided DataStream to downstream processing pipeline. Takes ownership
    /// of DataStream.
    /// @returns A pointer to the downstream DataStream
    DataStream* connect(std::unique_ptr<DataStream>&& downstream)
    {
        _downstream = std::move(downstream);
        return _downstream.get();
    }

    /// @brief Retrieve the downstream DataStream
    DataStream* downStream() const { return _downstream.get(); }

    /// @brief Push data downstream for processing
    virtual void pushData(const SharedDataBlock& sdb)
    {
        if(!_downstream)
            return;
        // Let the downstream DataStream process the data block.
        auto processed = _downstream->processData(sdb);
        // Push the processed block downstream
        _downstream->pushData(processed);
    }

    /// @brief Function that must be implemented by derived classes. Function provides a
    ///     SharedDataBlock that must be processed and return a SharedDataBlock of specified type.
    virtual auto processData(const SharedDataBlock& sdb) -> SharedDataBlock = 0;

protected:
    /// @brief Metadata describing the DataStream
    std::shared_ptr<MetaData> _meta;
    /// @brief Downstream DataStream
    std::unique_ptr<DataStream> _downstream;
};

class DataStreamThreadedBuffer : public DataStream
{
public:
    explicit DataStreamThreadedBuffer(size_t maxBlocks = 100, bool blockOnFull = false)
        : _queue(maxBlocks), _block_on_full(blockOnFull), _thread(&DataStreamThreadedBuffer::run, this)
    {}

    ~DataStreamThreadedBuffer() { _thread.join(); }

    virtual void pushData(const SharedDataBlock& sdb) override
    {
        // Push data into the queue
        if(!_block_on_full) {
            // Busy wait...
            while(!_queue.try_enqueue(sdb)) {
                // logWarn("SharedBlockThreadedBuffer skiped data because buffer is full\n");
            }
        } else {
            // Block
            _queue.enqueue(sdb);
        }
    }

protected:
    virtual auto processData(const SharedDataBlock& sdb) -> SharedDataBlock override final
    {
        return _downstream->processData(sdb);
    }

private:
    void run()
    {
        SharedDataBlock sdb;
        _queue.wait_dequeue(sdb);
        while(!sdb.isEndOfProcessing()) {
            if(_downstream) {
                sdb = this->processData(sdb);
                _downstream->pushData(sdb);
            }
            _queue.wait_dequeue(sdb);
        }
    }

    /// @brief SPSC thread-safe queue used to buffer SharedDataBlocks
    mc::BlockingConcurrentQueue<SharedDataBlock> _queue;
    /// Should the thread buffer block the input thread when the queue is full?
    std::atomic<bool> _block_on_full{false};
    /// Thread used to push blocks downstream
    std::thread _thread;
};

#endif