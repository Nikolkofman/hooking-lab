# Windows Hooking Lab

Practice material on Windows user-mode API hooking, written while working on the ArgusEDR endpoint-detection project (Hackeriot program, BGU, 2026). These are the same monitoring techniques that endpoint-detection tools rely on to observe what a process is doing.

## Contents

- **`api-hooking-examples/`** — two worked examples of API hooking, with notes in `README-detours.md`:
  - `CustomTrampoline/` — a hook built with a hand-written trampoline.
  - `DetoursLibHooking/` — the same idea using Microsoft's [Detours](https://github.com/microsoft/Detours) library.
- **`HookChallengeSolution/`** — a course exercise: a DLL that uses Detours to intercept a Win32 call and observe its arguments.
- **`injector/`** — a helper that loads the hook DLL into a target process, which is how such a monitoring DLL gets attached.
- **`ArgusAgent/`** — an early version of the ArgusEDR agent: an event queue, named-pipe IPC between an agent and a server, and scanning with YARA and PE-sieve.

## Building

Each folder is a Visual Studio 2022 solution (`.sln`). Open it and build the x64 configuration. The Detours examples use the bundled library files under `api-hooking-examples/DetoursLibHooking/`.

## Note

This is coursework and self-study, kept for reference. The code is written for learning the defensive side — understanding these techniques is what makes it possible to detect them.
