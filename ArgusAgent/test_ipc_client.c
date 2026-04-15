#include <windows.h>
#include <stdio.h>
#include "event_queue.h"
#include "ipc_client.h"

int main(void) {
    // Start pipe_server.exe first in another terminal!

    static EventQueue q;
    eq_init(&q);

    printf("[test] connecting (retry window = 3000 ms)...\n");
    ArgusIpcClient *c = ipc_client_create(&q, 3000);

    if (!c) {
        printf("[test] failed to connect - is pipe_server.exe running?\n");
        return 1;
    }

    printf("[test] connected. pushing 5 events...\n");
    for (int i = 0; i < 5; i++) {
        char s[64];
        sprintf(s, "{\"test\":\"retry\",\"n\":%d}", i);
        eq_push(&q, s);
    }

    Sleep(200);

    ipc_client_destroy(c);
    printf("[test] client destroyed cleanly\n");

    eq_destroy(&q);
    return 0;
}