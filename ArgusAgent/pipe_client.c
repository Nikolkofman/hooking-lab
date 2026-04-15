#include <windows.h>
#include <stdio.h>

#define PIPE_NAME L"\\\\.\\pipe\\argus-hello"

int main(void) {
    WaitNamedPipeW(PIPE_NAME, 5000);

    HANDLE h = CreateFileW(PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        printf("[client] connect failed: %lu\n", GetLastError());
        return 1;
    }

    printf("[client] connected\n");

    const char* msg = "{\"api\":\"VirtualAllocEx\",\"pid\":1234}\n";
    DWORD written;
    WriteFile(h, msg, (DWORD)strlen(msg), &written, NULL);
    printf("[client] sent JSON event\n");

    CloseHandle(h);
    printf("[client] done\n");
    return 0;
}