#include <stdio.h>
#include <windows.h>
#include "event_queue.h"
#include "ipc_client.h"
#include "ipc_server.h"
#include "scanner.h"
#include "yara_scanner.h"

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== ArgusAgent starting ===\n");

    EventQueue* queue = (EventQueue*)malloc(sizeof(EventQueue));
    eq_init(queue);
    printf("[MAIN] EventQueue initialized\n");

    ArgusIpcServer* server = ipc_server_create("session-001");
    if (!server) {
        printf("[MAIN] Failed to create IPC server\n");
        return 1;
    }
    printf("[MAIN] IPC server started\n");

    ArgusIpcClient* client = ipc_client_create(queue, 5000);
    if (!client) {
        printf("[MAIN] Failed to create IPC client\n");
        return 1;
    }
    printf("[MAIN] IPC client connected: %s\n",
        ipc_client_connected(client) ? "YES" : "NO");

    eq_push(queue, "{\"api\":\"VirtualAllocEx\",\"pid\":1234}");
    printf("[MAIN] Test event pushed to queue\n");

    ScanFinding findings[MAX_FINDINGS];
    size_t count = 0;

    ScanOptions opts = { 0 };
    opts.detect_private_executable = false;
    opts.detect_pe_anomalies = false;
    opts.yara_rules = NULL;

#ifdef YARA_AVAILABLE
    YR_RULES* rules = NULL;
    int yr = yara_load_rules("C:\\Users\\nikol\\source\\repos", &rules);
    printf("[MAIN] YARA rules loaded: %s\n", yr == 0 ? "YES" : "NO");
    opts.yara_rules = rules;
#endif

    DWORD my_pid = GetCurrentProcessId();
    printf("[MAIN] Running scan on self (pid=%lu)...\n", my_pid);
    scanner_run_pid(my_pid, "session-001", "scan-001",
        &opts, findings, &count, MAX_FINDINGS);

    printf("[MAIN] Findings: %zu\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("[FINDING %zu] rule=%s region=0x%llx\n",
            i, findings[i].detail.yara_rule,
            findings[i].region.base_address);
    }

#ifdef YARA_AVAILABLE
    if (rules) yr_rules_destroy(rules);
#endif

    Sleep(2000);

    ipc_client_destroy(client);
    ipc_server_destroy(server);
    eq_destroy(queue);
    free(queue);

    printf("=== ArgusAgent stopped ===\n");
    return 0;
}