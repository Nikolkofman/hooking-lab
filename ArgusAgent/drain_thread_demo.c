#include <windows.h>
#include <stdio.h>
#include "event_queue.h"

typedef struct {
    EventQueue* queue;
    HANDLE pipe;
    volatile LONG stop;
    uint64_t last_seen_dropped;
} DrainCtx;

static DWORD WINAPI drain_thread(LPVOID param) {
    DrainCtx* ctx = (DrainCtx*)param;
    char buf[EQ_MAX_EVENT_LEN + 2];

    while (!ctx->stop || eq_count(ctx->queue) > 0) {
        // Check for dropped events
        uint64_t dropped = eq_dropped(ctx->queue);
        if (dropped > ctx->last_seen_dropped) {
            char notice[128];
            sprintf(notice, "{\"type\":\"drop_notice\",\"count\":%llu}\n",
                dropped - ctx->last_seen_dropped);
            DWORD written;
            WriteFile(ctx->pipe, notice, (DWORD)strlen(notice), &written, NULL);
            ctx->last_seen_dropped = dropped;
        }

        char event[EQ_MAX_EVENT_LEN + 1];
        if (eq_pop(ctx->queue, event, sizeof(event))) {
            sprintf(buf, "%s\n", event);
            DWORD written;
            WriteFile(ctx->pipe, buf, (DWORD)strlen(buf), &written, NULL);
        }
        else if (!ctx->stop) {
            Sleep(1);
        }
    }
    return 0;
}

int main(void) {
    // Connect to the pipe_server.exe from Milestone 3
    WaitNamedPipeW(L"\\\\.\\pipe\\argus-hello", 5000);
    HANDLE pipe = CreateFileW(L"\\\\.\\pipe\\argus-hello",
        GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (pipe == INVALID_HANDLE_VALUE) {
        printf("[demo] failed to connect to pipe: %lu\n", GetLastError());
        return 1;
    }

    static EventQueue q;
    eq_init(&q);

    DrainCtx ctx = { &q, pipe, 0, 0 };
    HANDLE thread = CreateThread(NULL, 0, drain_thread, &ctx, 0, NULL);

    printf("[main] pushing 10 events\n");
    for (int i = 0; i < 10; i++) {
        char s[64];
        sprintf(s, "{\"event\":%d}", i);
        eq_push(&q, s);
    }

    printf("[main] all events pushed, stopping drain thread\n");
    InterlockedExchange(&ctx.stop, 1);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);

    printf("[main] drain thread exited cleanly\n");
    CloseHandle(pipe);
    eq_destroy(&q);
    printf("[main] done\n");
    return 0;
}