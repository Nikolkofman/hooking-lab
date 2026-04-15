#pragma once
#include <windows.h>
#include <stdbool.h>

#define IPC_SERVER_MAX_INSTANCES 64
#define ARGUS_MAX_UUID 37
#define ARGUS_MAX_NAME 260
#define ARGUS_MAX_TS   32

typedef struct ArgusIpcServer ArgusIpcServer;

ArgusIpcServer* ipc_server_create(const char* session_id);
void            ipc_server_destroy(ArgusIpcServer* s);
