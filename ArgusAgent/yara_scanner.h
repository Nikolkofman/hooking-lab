#pragma once
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include "ipc_server.h"

typedef enum {
    FINDING_PRIVATE_EXECUTABLE,
    FINDING_PE_HOLLOWING,
    FINDING_PE_IMPLANT,
    FINDING_REFLECTIVE_LOAD,
    FINDING_CODE_CAVE,
    FINDING_MODULE_STOMP,
    FINDING_YARA_MATCH,
    FINDING_ANOMALOUS_THREAD,
} FindingType;

typedef enum {
    SEVERITY_LOW,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH,
    SEVERITY_CRITICAL,
} Severity;

typedef struct {
    uint64_t base_address;
    uint64_t size;
    uint32_t protect;
    uint32_t type;
    uint32_t state;
} MemoryRegion;

typedef struct {
    char reason[256];
    char yara_rule[128];
    char pe_anomaly[128];
} FindingDetail;

typedef struct {
    char        ts[ARGUS_MAX_TS];
    char        session_id[ARGUS_MAX_UUID];
    char        scan_id[ARGUS_MAX_UUID];
    uint32_t    pid;
    char        process_name[ARGUS_MAX_NAME];
    FindingType finding_type;
    Severity    severity;
    MemoryRegion region;
    FindingDetail detail;
} ScanFinding;

#ifdef YARA_AVAILABLE
#include <yara.h>

int yara_load_rules(const char* path, YR_RULES** rules_out);

int yara_scan_process(HANDLE hProcess,
    DWORD pid,
    const char* process_name,
    const char* session_id,
    const char* scan_id,
    YR_RULES* rules,
    ScanFinding* out_findings,
    size_t* out_count,
    size_t max_findings);
#endif