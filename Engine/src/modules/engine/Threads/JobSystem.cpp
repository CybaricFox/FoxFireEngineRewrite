//
// Created by cmorg on 9/29/2026.
//

#include "JobSystem.h"

#include "src/modules/engine/Core/Platform.h"

JobSystem* JobSystem::instance = nullptr;

unsigned int JobSystem::runThread(void *params) {
    const unsigned int index = *static_cast<unsigned int *>(params);
    JobThread* thread = &instance->jobThreads[index];
    const ULong id = thread->thread.id;
    Logger::logDebug("Starting job thread " + toString(index) + ". ID: " + toString(id) + "Type: " + toString(thread->typeMask));

    if (!Platform::createMutex(thread->infoMutex)) {
        Logger::logError("Failed to create job mutex");
        return 0;
    }

    while (true) {
        if (!instance || !instance->bIsRunning || !thread) {
            break;
        }

        if (!Platform::lockMutex(thread->infoMutex)) {
            Logger::logError("Failed to lock job mutex");
        }
        JobContext context = thread->context;
        if (!Platform::unlockMutex(thread->infoMutex)) {
            Logger::logError("Failed to unlock job mutex");
        }

        if (context.entryFunction) {
            const bool result = context.entryFunction(context.params, context.result);

            if (result && context.successFunction) {
                storeResult(context.successFunction, context.result);
            } else if (!result && context.failureFunction) {
                storeResult(context.failureFunction, context.result);
            }

            if (context.params) {
                context.params->destroy();
            }
            if (context.result) {
                context.result->destroy();
            }

            if (!Platform::lockMutex(thread->infoMutex)) {
                Logger::logError("Failed to lock job mutex");
            }
            FF_Memory::ff_clear(&thread->context, sizeof(JobContext));
            if (!Platform::unlockMutex(thread->infoMutex)) {
                Logger::logError("Failed to unlock job mutex");
            }
        }

        if (instance->bIsRunning) {
            //This needs to be changed
            Platform::pauseThread(thread->thread, 10);
        } else {
            break;
        }
    }

    Platform::destroyMutex(thread->infoMutex);
    return 1;
}

void JobSystem::storeResult(const ThreadFunction callback, IThreadParam* params) {
    JobResultEntry entry{};
    entry.callback = callback;

    if (params->getSize() > 0) {
        entry.params = static_cast<IThreadParam *>(FF_Memory::ff_allocate_raw(params->getSize(), JOB, alignof(IThreadParam)));
        params->copyTo(entry.params);
    }

    if (!Platform::lockMutex(instance->resultMutex)) {
        Logger::logError("Failed to lock result mutex while storing a result. This can be a sign of corruption.");
    }
    for (unsigned short i = 0; i < MAX_JOB_RESULTS; i++) {
        if (instance->pendingResults[i].id == INVALID_ID_U16) {
            instance->pendingResults[i] = entry;
            instance->pendingResults[i].id = i;
            break;
        }
    }
    if (!Platform::unlockMutex(instance->resultMutex)) {
        Logger::logError("Failed to unlock result mutex while storing a result. This can be a sign of corruption.");
    }
}

void JobSystem::processQueue(RingQueue<JobContext> &queue, Mutex &queueMutex) {
    while (queue.getLength() > 0) {
        JobContext context{};

        if (!queue.peek()) break;

        bool foundThread = false;
        for (unsigned char i = 0; i < threadCount; i++) {
            JobThread& thread = jobThreads[i];

            if ((thread.typeMask & queue.peek()->type) == 0) continue;

            if (!Platform::lockMutex(thread.infoMutex)) {
                Logger::logError("Failed to lock job mutex");
            }
            if (!thread.context.entryFunction) {
                if (!Platform::lockMutex(queueMutex)) {
                    Logger::logError("Failed to lock queue mutex");
                }
                queue.dequeue(context);
                if (!Platform::unlockMutex(queueMutex)) {
                    Logger::logError("Failed to unlock queue mutex");
                }

                thread.context = context;

                Logger::logDebug("Assigning job to thread " + toString(thread.index));

                foundThread = true;
            }
            if (!Platform::unlockMutex(thread.infoMutex)) {
                Logger::logError("Failed to unlock job mutex");
            }

            if (foundThread) break;
        }

        if (!foundThread) break;
    }
}

bool JobSystem::initialize(const unsigned char maxThreadCount, unsigned int typeMasks[]) {
    instance = this;
    bIsRunning = true;
    lowPriorityQueue.initialize(1024);
    mediumPriorityQueue.initialize(1024);
    highPriorityQueue.initialize(1024);
    threadCount = maxThreadCount;

    Logger::logDebug("Id of the main thread is " + toString(Platform::getCurrentThread()));
    Logger::logDebug("Creating " + toString(threadCount) + " job threads.");

    for (unsigned int i = 0; i < threadCount; i++) {
        jobThreads[i].index = i;
        jobThreads[i].typeMask = typeMasks[i];
        if (!Platform::createThread(runThread, &jobThreads[i].index, false, jobThreads[i].thread)) {
            Logger::logFatal("OS failed to create job thead.");
            return false;
        }
    }

    if (!Platform::createMutex(resultMutex)) {
        Logger::logFatal("OS failed to create result mutex.");
        return false;
    }
    if (!Platform::createMutex(lowPriorityMutex)) {
        Logger::logFatal("OS failed to create low priority mutex.");
        return false;
    }
    if (!Platform::createMutex(mediumPriorityMutex)) {
        Logger::logFatal("OS failed to create medium priority mutex.");
        return false;
    }
    if (!Platform::createMutex(highPriorityMutex)) {
        Logger::logFatal("OS failed to create high priority mutex.");
        return false;
    }

    return true;
}

void JobSystem::shutdown() {
    bIsRunning = false;

    for (unsigned char i = 0; i < threadCount; i++) {
        Platform::destroyThread(jobThreads[i].thread);
    }

    lowPriorityQueue.shutdown();
    mediumPriorityQueue.shutdown();
    highPriorityQueue.shutdown();

    Platform::destroyMutex(resultMutex);
    Platform::destroyMutex(lowPriorityMutex);
    Platform::destroyMutex(mediumPriorityMutex);
    Platform::destroyMutex(highPriorityMutex);

    instance = nullptr;
}

void JobSystem::update() {
    if (!bIsRunning) return;

    processQueue(highPriorityQueue, highPriorityMutex);
    processQueue(mediumPriorityQueue, mediumPriorityMutex);
    processQueue(lowPriorityQueue, lowPriorityMutex);

    for (unsigned short i = 0; i < MAX_JOB_RESULTS; i++) {
        if (!Platform::lockMutex(resultMutex)) {
            Logger::logError("Failed to lock result mutex");
        }
        const JobResultEntry entry = pendingResults[i];
        if (!Platform::unlockMutex(resultMutex)) {
            Logger::logError("Failed to unlock result mutex");
        }

        if (entry.id != INVALID_ID_U16) {
            entry.callback(entry.params);

            if (entry.params) {
                entry.params->destroy();
            }

            if (!Platform::lockMutex(resultMutex)) {
                Logger::logError("Failed to lock result mutex");
            }
            FF_Memory::ff_clear(&pendingResults[i], sizeof(JobResultEntry));
            pendingResults[i].id = INVALID_ID_U16;
            if (!Platform::unlockMutex(resultMutex)) {
                Logger::logError("Failed to unlock result mutex");
            }
        }
    }
}

void JobSystem::submit(JobContext jobContext) {
    RingQueue<JobContext>* queue = &mediumPriorityQueue;
    Mutex* mutex = &mediumPriorityMutex;

    //High priority is run immediately.
    if (jobContext.priority == JOB_PRIORITY_HIGH) {
        queue = &highPriorityQueue;
        mutex = &highPriorityMutex;

        for (unsigned char i = 0; i < threadCount; i++) {
            JobThread& thread = jobThreads[i];
            if (jobThreads[i].typeMask & jobContext.type) {
                bool found = false;

                if (!Platform::lockMutex(thread.infoMutex)) {
                    Logger::logError("Failed to lock job thread mutex");
                }
                if (!jobThreads[i].context.entryFunction) {
                    Logger::logDebug("Job immediately submitted on thread " + toString(jobThreads[i].index));
                    jobThreads[i].context = jobContext;
                    found = true;
                }
                if (!Platform::unlockMutex(thread.infoMutex)) {
                    Logger::logError("Failed to unlock job thread mutex");
                }

                if (found) return;
            }
        }
    }

    if (jobContext.priority == JOB_PRIORITY_LOW) {
        queue = &lowPriorityQueue;
        mutex = &lowPriorityMutex;
    }

    if (!Platform::lockMutex(*mutex)) {
        Logger::logError("Failed to lock queue mutex");
    }
    queue->enqueue(&jobContext);
    if (!Platform::unlockMutex(*mutex)) {
        Logger::logError("Failed to unlock queue mutex");
    }
}
