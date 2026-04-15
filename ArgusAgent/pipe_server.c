#include <windows.h>
#include <stdio.h>

#define PIPE_NAME L"\\\\.\\pipe\\argus-hello"
#define BUF_SIZE 4096

int main(void) {
    HANDLE h = CreateNamedPipeW(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, BUF_SIZE, BUF_SIZE, 0, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        printf("[server] CreateNamedPipeW failed: %lu\n", GetLastError());
        return 1;
    }

    printf("[server] waiting for client...\n");
    ConnectNamedPipe(h, NULL);
    printf("[server] client connected\n");

    char line_buf[BUF_SIZE];
    int line_pos = 0;
    char read_buf[BUF_SIZE];
    DWORD bytes_read;

    while (ReadFile(h, read_buf, sizeof(read_buf) - 1, &bytes_read, NULL) && bytes_read > 0) {
        for (DWORD i = 0; i < bytes_read; i++) {
            if (read_buf[i] == '\n') {
                line_buf[line_pos] = '\0';
                printf("[server] received JSON: %s\n", line_buf);

                // Extract "api" field manually
                char* api = strstr(line_buf, "\"api\":\"");
                if (api) {
                    api += 7;
                    char api_val[256];
                    int j = 0;
                    while (api[j] && api[j] != '"') {
                        api_val[j] = api[j];
                        j++;
                    }
                    api_val[j] = '\0';
                    printf("[server] api = %s\n", api_val);
                }
                line_pos = 0;
            }
            else {
                line_buf[line_pos++] = read_buf[i];
            }
        }
    }

    printf("[server] client disconnected\n");
    DisconnectNamedPipe(h);
    CloseHandle(h);
    return 0;
}