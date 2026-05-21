//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "System/PowerEventWatcher.h"

#include <chrono>
#include <systemd/sd-bus.h>
#include <utility>

MPowerEventWatcher::MPowerEventWatcher()
{
    bRunning = false;
    Bus = nullptr;
    PrepareForSleepSlot = nullptr;
}

MPowerEventWatcher::~MPowerEventWatcher()
{
    Stop();
}

void MPowerEventWatcher::SetPrepareForSleepCallback(std::function<void(bool)> Callback)
{
    PrepareForSleepCallback = std::move(Callback);
}

bool MPowerEventWatcher::Start()
{
    if (bRunning)
    {
        return true;
    }

    const int OpenResult = sd_bus_open_system(&Bus);
    if (OpenResult < 0)
    {
        Bus = nullptr;
        return false;
    }

    const int MatchResult = sd_bus_add_match(
        Bus,
        &PrepareForSleepSlot,
        "type='signal',"
        "sender='org.freedesktop.login1',"
        "path='/org/freedesktop/login1',"
        "interface='org.freedesktop.login1.Manager',"
        "member='PrepareForSleep'",
        &MPowerEventWatcher::HandlePrepareForSleepSignal,
        this);

    if (MatchResult < 0)
    {
        CloseBus();
        return false;
    }

    bRunning = true;
    WatcherThread = std::thread(&MPowerEventWatcher::Run, this);

    return true;
}

void MPowerEventWatcher::Stop()
{
    bRunning = false;

    if (WatcherThread.joinable())
    {
        WatcherThread.join();
    }

    CloseBus();
}

int MPowerEventWatcher::HandlePrepareForSleepSignal(sd_bus_message* Message, void* UserData, sd_bus_error* Error)
{
    (void)Error;

    auto* Watcher = static_cast<MPowerEventWatcher*>(UserData);
    if (Watcher == nullptr)
    {
        return 0;
    }

    int bPreparingForSleep = 0;
    const int ReadResult = sd_bus_message_read(Message, "b", &bPreparingForSleep);
    if (ReadResult < 0)
    {
        return 0;
    }

    if (Watcher->PrepareForSleepCallback)
    {
        Watcher->PrepareForSleepCallback(bPreparingForSleep != 0);
    }

    return 0;
}

void MPowerEventWatcher::Run()
{
    while (bRunning)
    {
        const int ProcessResult = sd_bus_process(Bus, nullptr);
        if (ProcessResult < 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (ProcessResult > 0)
        {
            continue;
        }

        sd_bus_wait(Bus, 500000);
    }
}

void MPowerEventWatcher::CloseBus()
{
    if (PrepareForSleepSlot != nullptr)
    {
        PrepareForSleepSlot = sd_bus_slot_unref(PrepareForSleepSlot);
    }

    if (Bus != nullptr)
    {
        Bus = sd_bus_unref(Bus);
    }
}
