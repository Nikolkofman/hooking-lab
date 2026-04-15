#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==========================================
// WHAT THIS PROGRAM DOES
// ==========================================
// This is an "injector" - it takes our hook DLL and forces it to load
// into the memory of another running process.
//
// Steps:
// 1. Open the target process by its PID (Process ID)
// 2. Allocate memory in that process for the DLL filename
// 3. Write the DLL filename into that memory
// 4. Create a remote thread that loads our DLL using LoadLibraryA
// 5. Wait for the thread to finish
// 6. Clean up

BOOL InjectDLL(DWORD dwPID, const char* szDLLPath) {
    printf("[*] Starting injection process...\n");
    printf("[*] Target PID: %d\n", dwPID);
    printf("[*] DLL Path: %s\n\n", szDLLPath);

    // ==========================================
    // STEP 1: Open the target process
    // ==========================================
    printf("[1] Opening target process...\n");

    HANDLE hProcess = OpenProcess(
        PROCESS_ALL_ACCESS,  // We want full access to the process
        FALSE,               // Don't inherit this handle
        dwPID                // Process ID to open
    );

    if (hProcess == NULL) {
        printf("    [!] ERROR: Failed to open process!\n");
        printf("    [!] Make sure you're running as Administrator\n");
        printf("    [!] Make sure the PID is correct\n");
        return FALSE;
    }
    printf("    [+] Process opened successfully!\n\n");

    // ==========================================
    // STEP 2: Allocate memory in the target process
    // ==========================================
    printf("[2] Allocating memory in target process...\n");

    // We need to allocate space for the DLL path string
    LPVOID pRemoteMemory = VirtualAllocEx(
        hProcess,                           // Which process to allocate in
        NULL,                               // Let Windows choose the address
        strlen(szDLLPath) + 1,              // Size needed for the string
        MEM_COMMIT | MEM_RESERVE,           // Allocate and reserve memory
        PAGE_READWRITE                      // Make it readable and writable
    );

    if (pRemoteMemory == NULL) {
        printf("    [!] ERROR: Failed to allocate memory!\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    printf("    [+] Memory allocated at: 0x%p\n\n", pRemoteMemory);

    // ==========================================
    // STEP 3: Write the DLL path to the allocated memory
    // ==========================================
    printf("[3] Writing DLL path to target process memory...\n");

    if (!WriteProcessMemory(
        hProcess,                           // Which process to write to
        pRemoteMemory,                      // Where to write
        szDLLPath,                          // What to write (the DLL path)
        strlen(szDLLPath) + 1,              // How many bytes to write
        NULL                                // Don't care about bytes written
    )) {
        printf("    [!] ERROR: Failed to write DLL path!\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }
    printf("    [+] DLL path written successfully!\n\n");

    // ==========================================
    // STEP 4: Get the address of LoadLibraryA
    // ==========================================
    printf("[4] Finding LoadLibraryA address...\n");

    // kernel32.dll is the Windows library that contains LoadLibraryA
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

    // LoadLibraryA is a function that loads DLLs
    LPVOID pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");

    if (pLoadLibraryA == NULL) {
        printf("    [!] ERROR: Failed to get LoadLibraryA address!\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }
    printf("    [+] LoadLibraryA found at: 0x%p\n\n", pLoadLibraryA);

    // ==========================================
    // STEP 5: Create a remote thread
    // ==========================================
    printf("[5] Creating remote thread in target process...\n");
    printf("    [*] This thread will call LoadLibraryA(DLL_path)\n");

    // CreateRemoteThread is like CreateThread, but for another process
    HANDLE hThread = CreateRemoteThread(
        hProcess,                                       // Which process
        NULL,                                           // Default security
        0,                                              // Default stack size
        (LPTHREAD_START_ROUTINE)pLoadLibraryA,         // Start at LoadLibraryA
        pRemoteMemory,                                  // Pass the DLL path as parameter
        0,                                              // Run immediately
        NULL                                            // Don't care about thread ID
    );

    if (hThread == NULL) {
        printf("    [!] ERROR: Failed to create remote thread!\n");
        printf("    [!] Make sure you're running as Administrator\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }
    printf("    [+] Remote thread created successfully!\n\n");

    // ==========================================
    // STEP 6: Wait for the thread to finish
    // ==========================================
    printf("[6] Waiting for DLL to load...\n");
    printf("    [*] This should only take a moment...\n\n");

    WaitForSingleObject(hThread, INFINITE);  // Wait forever (until thread finishes)

    printf("    [+] DLL loading complete!\n\n");

    // ==========================================
    // STEP 7: Clean up resources
    // ==========================================
    printf("[7] Cleaning up resources...\n");

    // Free the memory we allocated in the target process
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);

    // Close the thread handle
    CloseHandle(hThread);

    // Close the process handle
    CloseHandle(hProcess);

    printf("    [+] Cleanup complete!\n\n");

    return TRUE;
}

// ==========================================
// MAIN FUNCTION
// ==========================================

int main(int argc, char* argv[]) {
    printf("\n");
    printf("====================================\n");
    printf("    DLL Injection Tool\n");
    printf("====================================\n\n");

    // Check command line arguments
    if (argc != 3) {
        printf("[!] Usage: injector.exe <PID> <DLL_PATH>\n");
        printf("[!] Example: injector.exe 1234 C:\\path\\to\\hooks.dll\n");
        printf("[!] \n");
        printf("[!] <PID> - Process ID of the target program\n");
        printf("[!] <DLL_PATH> - Full path to your hooked DLL\n");
        return 1;
    }

    DWORD dwPID = atoi(argv[1]);           // Parse the PID from command line
    const char* szDLLPath = argv[2];       // Get the DLL path from command line

    // Perform the injection
    if (InjectDLL(dwPID, szDLLPath)) {
        printf("[+] Injection completed successfully!\n");
        printf("[+] The hooked DLL should now be running in the target process\n");
        printf("[+] Watch the console window of the target program for hook alerts!\n\n");
        return 0;
    }
    else {
        printf("[!] Injection failed!\n");
        printf("[!] Please check the error messages above\n\n");
        return 1;
    }
}
