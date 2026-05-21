//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".
#pragma once

#include <atomic>
#include <functional>
#include <thread>

struct sd_bus;
struct sd_bus_slot;
struct sd_bus_message;
struct sd_bus_error;

class MPowerEventWatcher
{
public:
    MPowerEventWatcher();
    ~MPowerEventWatcher();

    void SetPrepareForSleepCallback(std::function<void(bool)> Callback);
    bool Start();
    void Stop();

private:
    static int HandlePrepareForSleepSignal(sd_bus_message* Message, void* UserData, sd_bus_error* Error);

    void Run();
    void CloseBus();

    std::function<void(bool)> PrepareForSleepCallback;
    std::thread WatcherThread;
    std::atomic<bool> bRunning;

    sd_bus* Bus;
    sd_bus_slot* PrepareForSleepSlot;
};
