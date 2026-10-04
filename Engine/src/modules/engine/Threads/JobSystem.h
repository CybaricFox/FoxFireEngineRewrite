//
// Created by cmorg on 9/29/2026.
//

#pragma once
#include "Mutex.h"
#include "Thread.h"
#include "src/modules/engine/Memory/RingQueue.h"

/**
 *  @file JobSystem.h
 *  @layer Engine
 *  @module Threads
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/29/2026
 *
 *  @copyright (c) 2026
 */

/**
 * @brief Max number of job results that can be stored at a given time.
 */
#define MAX_JOB_RESULTS 512

enum JobType {
    GENERAL_JOB = 0x02, //Does not matter what thread this runs on.
    RESOURCE_LOAD_JOB = 0x04, //Resource loading should be done on the same thread.
    GPU_JOB = 0x08, //Threads used by the render system.
};

enum JobPriority {
    JOB_PRIORITY_LOW,
    JOB_PRIORITY_NORMAL,
    JOB_PRIORITY_HIGH
};

struct IThreadParam {
    virtual ~IThreadParam() = default;
    virtual void copyTo(IThreadParam* destination) const = 0;
    virtual void destroy() = 0;
    virtual ULong getSize() = 0;
};

template <typename T>
struct ThreadParam : IThreadParam{
    void copyTo(IThreadParam *destination) const override {
        std::construct_at(static_cast<T *>(destination), static_cast<const T&>(*this));
    }
    void destroy() override {
        std::destroy_at(static_cast<T *>(this));
    }
    ULong getSize() override {
        return sizeof(T);
    }
};

struct JobContext {
    JobType type{};
    JobPriority priority{};
    ResultFunction entryFunction{};
    ThreadFunction successFunction{};
    ThreadFunction failureFunction{};
    IThreadParam* params = nullptr;
    IThreadParam* result = nullptr;
};

struct JobThread {
    unsigned char index = INVALID_ID_U8;
    Thread thread{};
    JobContext context{};
    Mutex infoMutex{};
    unsigned int typeMask = 0;
};

struct JobResultEntry {
    unsigned short id = INVALID_ID_U16;
    ThreadFunction callback{};
    IThreadParam* params = nullptr;
};

class JobSystem {
private:
    ThreadFunction startFunction{};
    ThreadFunction finishFunction{};
    bool bIsRunning = false;
    unsigned char threadCount = 0;
    JobThread jobThreads[32]{};
    RingQueue<JobContext> lowPriorityQueue{};
    RingQueue<JobContext> mediumPriorityQueue{};
    RingQueue<JobContext> highPriorityQueue{};
    Mutex lowPriorityMutex{};
    Mutex mediumPriorityMutex{};
    Mutex highPriorityMutex{};
    JobResultEntry pendingResults[MAX_JOB_RESULTS]{};
    Mutex resultMutex{};
    static JobSystem* instance;

    static unsigned int runThread(void* params);
    static void storeResult(ThreadFunction callback, IThreadParam *params);
    void processQueue(RingQueue<JobContext>& queue, Mutex& queueMutex);

public:
    bool initialize(unsigned char maxThreadCount, unsigned int typeMasks[]);
    void shutdown();

    static JobSystem& getInstance() {return *instance;}

    void update();
    void submit(JobContext jobContext);

    template <typename P, typename R>
    requires (std::same_as<P, void> || std::derived_from<P, IThreadParam>) && (std::same_as<R, void> || std::derived_from<R, IThreadParam>)
    JobContext createJob(const ResultFunction entryFunction, const ThreadFunction successFunction, const ThreadFunction failureFunction, P* params, const JobType type = GENERAL_JOB, const JobPriority priority = JOB_PRIORITY_NORMAL) {
        JobContext context{};
        context.entryFunction = entryFunction;
        context.successFunction = successFunction;
        context.failureFunction = failureFunction;
        context.type = type;
        context.priority = priority;

        if constexpr (!std::is_void_v<P>) {
            context.params = static_cast<P*>(FF_Memory::ff_allocate(sizeof(P), JOB, alignof(P)));
            std::construct_at(static_cast<P*>(context.params), *params);
        }
        if constexpr (!std::is_void_v<R>) {
            context.result = static_cast<R *>(FF_Memory::ff_allocate(sizeof(R), JOB, alignof(R)));
            std::construct_at(static_cast<R*>(context.result));
        }

        return context;
    }
};