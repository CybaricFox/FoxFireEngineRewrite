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

struct JobContext {
    JobType type{};
    JobPriority priority{};
    ResultFunction entryFunction{};
    ThreadFunction successFunction{};
    ThreadFunction failureFunction{};
    void* params = nullptr;
    unsigned int paramsSize = 0;
    void* result = nullptr;
    unsigned int resultSize = 0;
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
    unsigned int paramsSize = 0;
    void* params = nullptr;
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
    static void storeResult(ThreadFunction callback, unsigned int paramSize, const void* params);
    void processQueue(RingQueue<JobContext>& queue, Mutex& queueMutex);

public:
    bool initialize(unsigned char maxThreadCount, unsigned int typeMasks[]);
    void shutdown();

    void update();
    void submit(JobContext jobContext);
    JobContext createJob(ResultFunction entryFunction, ThreadFunction successFunction, ThreadFunction failureFunction, const void* params, unsigned int paramsSize, unsigned int resultsSize, JobType type = GENERAL_JOB, JobPriority priority = JOB_PRIORITY_NORMAL);
};