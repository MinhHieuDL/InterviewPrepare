# Embedded Linux Mentorship — Tracking Document

**Goal:** Level up from intermediate embedded C/C++ engineer to job-ready middle/senior embedded software developer, focused on Embedded Linux (kernel → application) and CI/CD for embedded projects.

**Timeline:** ~11 weeks, 15+ hrs/week
**Started:** 2026-06-21
**Reference material:** *The Design of the UNIX Operating System* (Bach) for internals theory, *Linux Device Drivers, 3rd Ed* (LDD3) for hands-on driver development (note: LDD3 targets kernel 2.6 — APIs will be modernized to current kernel during lessons)

**Runbooks:** `FLASH.md` — full BBB flash + dev-environment setup procedure (created 2026-09-20 after SD-card-corruption rebuild). Follow it to reproduce the board setup.

---

## How to use this document

- This is a **living record**. Update the diagnostic table as topics are revisited and retested.
- Status legend: ✅ Solid | ⚠️ Partial/needs reinforcement | ❌ Gap (not yet built) | 🔁 Reinforced (was a gap, now solid after review)
- Re-run a mini diagnostic every 2-3 weeks to check retention, not just first-pass understanding. Update the table — don't just append.

---

## Initial Diagnostic Summary (2026-06-21)

A baseline check-in before starting the structured plan, covering core OS/process/memory concepts from Bach. Purpose: find out what's *actually* retained vs. forgotten, and at what depth (policy-level vs. mechanism-level).

| # | Area | Status | Notes |
|---|------|--------|-------|
| 1 | Scheduling triggers (timeslice expiry, blocking, priority) | ✅ Solid | Correctly explained when context switches happen |
| 2 | Context switch mechanism (what's saved, where) | ✅ Solid (W1 built, mock-confirmed) | Full register set → thread_struct / cpu_context_save; pt_regs vs thread_struct; switch_to(); current; CR3/TTBR0 + TLB flush cost. Mock Jul 17: clean, graduated. |
| 3 | Syscall mechanism | ✅ Solid (mock-confirmed) | Address-space misconception fully corrected: ring 3→0 / EL0→EL1, user→kernel stack, page tables unchanged, syscall/SVC gate, CPL in CS. Mock Jul 17: clean. |
| 4 | fork() semantics | 🔁 Reinforced — ⚠️ **COW granularity soft** | Model solid. **GAP (drill + mock Jul 17): COW copies ONE PAGE (4KB), not "4 bytes." Also MMU-detects-vs-kernel-repairs blurred. Re-drill.** |
| 5 | Thread semantics | ✅ Solid (W2 deepened) | thread = task_struct via clone(CLONE_VM/FILES); shared mm → no CR3 reload; own stack but no inter-thread stack protection. |
| 6 | Virtual memory — why it exists | 🔁 Reinforced | Isolation + fragmentation + overcommit derived independently. MMU, multi-level page tables, PTEs, VMA-vs-page-table, demand paging, TLB, /proc/self/maps. |
| 7 | Buffer cache / write-back | ✅ Solid | Reinforced during open/read syscall walkthrough. |
| 8 | Race conditions — scenario level | ✅ Solid | Constructed valid check-then-act race unprompted; reinforced through W2. |
| 9 | Race conditions — instruction-level (atomicity) | 🔁 Reinforced (W2) | Built with real disassembly of user's own race.c; lost update, lock addq, atomicity-doesn't-compose (derived independently). CAS vs exchange, futex. |
| 10 | Interrupt vs syscall | ✅ Solid | Correctly distinguished hardware- vs software-triggered. |
| 11 | Kernel space vs user space — why | ✅ Solid (mock-confirmed) | Privilege rings HW-enforced; controlled-gate entry; ARM EL0/EL1 + SVC. Mock Jul 17: closed the jump-to-kernel-addr loophole cleanly. Graduated. |
| 12 | Page faults | ⚠️ Partial (preview) — **MMU-vs-kernel roles soft** | **GAP (mock Jul 17): user blurred "MMU allocates/copies" — correct is MMU *detects* + faults, kernel handler *repairs*. Full depth Week 7.** |
| 13 | fd / open file table / inode (3-layer model) | 🔁 Reinforced — ⚠️ **scope-labeling wobble** | **GAP (mock Jul 17): middle layer (struct file) mislabeled "per-process" then said fork "shares" it — contradiction. Correct: fd table = per-process (copied); struct file = KERNEL-GLOBAL (shared); inode = kernel-global. All live in kernel memory, user can't touch. Re-drill — softest spot.** |
| 14 | Disk block mapping | ⚠️ Partial | Light coverage during read() walkthrough. Lower priority. |

### Overall pattern

**Strength:** Conceptual/policy-level reasoning is genuinely good — consistently correct on *why* a design exists. Recovers gracefully when a fact is missing (reasons toward it instead of bluffing).

**Consistent gap (refined after Jul 17 mock):** Mechanism-level *facts* cluster around ONE seam — **where kernel structures physically live, and who (hardware vs software) performs each step.** Three symptoms: (a) COW granularity = page not bytes [#4]; (b) MMU-detects-vs-kernel-repairs [#12]; (c) fd/struct-file/inode scope + "they live in kernel memory" [#13]. Fixing the seam fixes all three. **These three are the declared focus for the ~Jul 31 mixed R2 interview.**

**Retention strategy (confirmed working):** user traces real kernel source, questions examples that don't match expectations, derives conclusions independently. Requested strict one-topic-at-a-time pacing — **honor this: never batch multiple sub-mechanisms in one message.**

---

## The Plan

**Format per topic:** concept from first principles → user/kernel/hardware/Device-Tree role breakdown (where relevant) → code example → small hands-on task → interview Q&A + common pitfalls.

### Phase 1 — User Space Foundations (Weeks 1-2)

**Week 1 — Process, Syscall & Memory Mechanics** ✅ **COMPLETE (2026-07-10)** — review doc: `Week1_Review.md`
- ✅ Context switch, syscall + privilege rings, fd 3-layer model + lifecycle, virtual memory, fork/COW, page-fault preview

**Week 2 — Multithreading & IPC** ✅ **COMPLETE (2026-07-17)** — review doc: `Week2_Review.md`
- ✅ Instruction-level races, pthread/clone model, mutex/futex internals, condition variables, false sharing
- ✅ IPC: pipes, shared memory, message queues, signals — all in depth
- ✅ Sockets — TCP lifecycle, flow-control two-buffer model, byte order, deadlock+fix, UDP, AF_UNIX

### Phase 2 — Kernel Fundamentals & First Driver (Weeks 3-4)

**Week 3 — Kernel Modules & Build System** ✅ **COMPLETE (2026-09-20)** — runbook: `FLASH.md`
- ✅ What a kernel module is (loadable ring-0 code; init/exit not main; registers-then-waits; printk not printf; no libc; __init/__exit memory reclaim; MODULE_LICENSE)
- ✅ The two "Match" requirements: **Match #1** toolchain↔chip (ARM hardfloat, = CROSS_COMPILE); **Match #2** kernel tree↔running kernel (vermagic, = KDIR)
- ✅ Kbuild / build system: obj-m, KDIR handoff (-C KDIR M=PWD modules), the ifneq(KERNELRELEASE) two-pass Makefile pattern, cross-compile vars
- ✅ Linking deep-dive: symbols, symbol tables (/proc/kallsyms), unresolved references, insmod as dynamic linker patching against kernel symbol table; two-stage linking (existence check vs address patching), why deferred to load time; sysroot concept for cross-compiling user programs
- ✅ How loading works: insmod (raw, by file, no dep resolution) vs modprobe (by name, resolves deps) vs rmmod (refcount-gated); lsmod/dmesg/modinfo/proc/modules
- ✅ **HANDS-ON (major): full BBB environment bring-up from scratch** — flashed official Debian 13 / kernel 6.18.52-bone54 SD image; S2 boot override; serial (minicom 115200) + SSH (ssh bbb alias, key auth); cross-compiled the ENTIRE 6.18.52-bone54 kernel from bb-kernel source (Robert Nelson scripts, auto-provisioned GCC 15 toolchain); built + deployed + **loaded first cross-compiled module (hello.ko) on real hardware** — verified insmod→hello_init, lsmod, rmmod→hello_exit via dmesg. All Matches verified via modinfo (vermagic 6.18.52-bone54) + file (ARM 32-bit).
- Notes/gotchas captured in FLASH.md: linux-headers .deb is a stripped tree (missing scripts/kconfig, ships ARM host-tools → "fixdep: Exec format error" on x86) — use bb-kernel SOURCE build instead; never Ctrl-C toolchain download (corrupts file); ethernet DHCP wants cable plugged AFTER boot; module vermagic must == board uname -r.

**Week 4 — Char Drivers** ← **NEXT**
- `file_operations`, major/minor numbers
- Write a real char driver end-to-end (loopback or fake sensor device), test from user space
- Environment is READY (board booted, cross-compile + module load loop proven working 2026-09-20). scull skeleton already present in kmod/scull.

### Phase 3 — Platform Drivers & Hardware Interaction (Weeks 5-6)

**Week 5 — Platform Driver Model & Device Tree**
- Platform driver model, Device Tree bindings, how DT connects to driver `probe()`
- (bb-kernel full source tree now on host — useful for DT exploration)

**Week 6 — Interrupts & Kernel Synchronization**
- Top half/bottom half, tasklets, workqueues, threaded IRQs
- Spinlocks vs mutexes vs RCU — why you can't sleep in a spinlock
- (RCU intro given W1 via `__rcu` fd array; user-space spinlock trade-off given W2 — build on both)

### Phase 4 — Memory, Debugging & Observability (Weeks 7-8)

**Week 7 — Memory Mapping & DMA**
- `mmap`, `ioremap`, `kmalloc` vs `vmalloc`
- Full page fault mechanism (deferred from Week 1; also fixes diagnostic #12 MMU-vs-kernel roles)
- DMA basics: coherent vs streaming, DMA-safe memory

**Week 8 — Observability & Debugging**
- sysfs/procfs/debugfs — exposing driver state to user space
- Kernel debugging toolkit: printk/dynamic debug, ftrace, kgdb/kdb, oops/panic analysis
- (Field-diagnosis triad seeded in W2: ps state / /proc/<pid>/wchan / /proc/<pid>/fd)

### Phase 5 — Boot Process & Build Systems (Week 9)

**Week 9 — Boot Chain & Yocto**
- Full boot chain: ROM → SPL → bootloader → kernel → rootfs (PARTIALLY PREVIEWED in W3 hands-on: S2 boot override, MLO/u-boot, kernel build)
- U-Boot internals and commands
- Secure Boot chain of trust basics
- Yocto Project fundamentals: layers, recipes, bitbake
- (Note: user has bb-kernel + rootfs + u-boot parts on disk for manual SD assembly — good raw material for the hands-on boot-chain lesson)

### Phase 6 — CI/CD for Embedded (Weeks 10-11)

**Week 10 — Git, Static Analysis, Unit Testing**
- Git workflows for embedded teams; static analysis (cppcheck, clang-tidy); unit testing (Unity/GoogleTest, mocking hardware)

**Week 11 — Build Pipelines & Integration Testing**
- Cross-compilation in CI; CI/CD for Yocto-based projects; integration testing; HIL testing concepts

---

## Open Items / Decisions Needed

- [x] **Target environment for Week 4 onward**: real hardware — **BeagleBone Black**. Decided 2026-06-21. **Environment fully working as of 2026-09-20** (flashed, booted, SSH, cross-compile + module load loop proven).
- [ ] RTOS topics deliberately excluded per user request (has practical RTOS experience) — revisit later if needed
- [x] Build tooling: user prefers **CMake** for user-space code; kernel modules use the bb-kernel-style Makefile.
- [ ] Toolchain cleanup deferred: the manually-downloaded `toolchain/arm-gnu-toolchain-13.3.rel1-...` (unused now — bb-kernel uses its own GCC 15 in dl/) and the broken `kmod/headers/` .deb extraction can be deleted to reclaim space. Keep the .deb file. (Low priority; do when convenient.)

---

## Review Schedule (Spaced Retrieval Practice)

**Purpose:** Combat forgetting via retrieval practice at expanding intervals. Mini-drill = 10-15 min, 4-6 quick questions. Mock interview = 20-30 min, full interviewer persona, no teaching until after.

**Session-start protocol (for Claude):** At the start of EVERY session, check this table. If any review is due/overdue, inform the user before starting new material. User decides run-or-defer. After each review: mark done, log results, update diagnostic table.

**Interval rule:** topic completed → mini-drill +2-3 days → mock interview +1 week → mixed interview +3 weeks → final comprehensive in Week 11.

| Due date | Type | Scope | Status |
|---|---|---|---|
| 2026-07-12 | Mini-drill | Week 1 | ✅ Done — 3/6 solid, 3/6 partial (MMU-vs-kernel, per-page COW, privilege gates) |
| 2026-07-17 | Mock interview R1 | Week 1 | ✅ Done — pass w/ reservations. Graduated: privilege gates, context switch, syscall. Still ⚠️: #4, #12, #13. |
| 2026-07-20 | Mini-drill | Week 2 | ✅ Done ~Jul 20 — 3 solid (D1 races, D5 shm offsets, D6 TCP connect), 1 label-slip (D1 "process" vs "thread"), 2 misses: **uncontended-mutex syscall count (answered "one", correct = ZERO)** and **condvar while-not-if (gap)**. Added to R2 focus. |
| ~2026-08-10 | Mock interview R2 | Weeks 1-2-3 mixed | ⬜ Pending — **W1 carry-over: (1) fd/struct-file/inode scope [#13], (2) MMU-vs-kernel repairs [#12], (3) per-page COW [#4]. W2 carry-over: (4) uncontended mutex = 0 syscalls, (5) condvar while-not-if. Plus W3 module/build basics.** (Date pushed from ~Jul 31 due to the long W3 hands-on setup session.) |
| Week 11 (end of plan) | Final comprehensive | Everything | ⬜ Pending |

**Pending to schedule:** Week 3 mini-drill (module concepts, the two Matches, Kbuild, insmod/modprobe/rmmod) — ~2-3 days after next session, once W3 conceptual material has had time to settle (note: W3 was heavily hands-on; the conceptual bits were taught around a lot of setup friction and were re-anchored at session end).

---

## Progress Log

- **2026-06-21**: Initial diagnostic completed, plan created. BeagleBone chosen for W4+. Update workflow confirmed.
- **2026-07-10**: **Week 1 COMPLETE.** Added Review Schedule system.
- **2026-07-12**: W1 mini-drill done (3/6 solid).
- **2026-07-17**: W1 Mock R1 done (pass w/ reservations; #4/#12/#13 → R2 focus). Created `Week1_Review.md`.
- **2026-07-17**: **Week 2 COMPLETE.** All IPC + sockets. Created `Week2_Review.md`. User derived write/write deadlock and self-corrected AF_UNIX misconception.
- **2026-07-20**: W2 mini-drill done. Misses: uncontended-mutex syscall count (0, not 1) and condvar while-not-if → added to R2 focus.
- **2026-09-20**: **Week 3 COMPLETE — major hands-on session.** Taught: what a module is (init/exit vs main, registers-then-waits, printk/no-libc, __init reclaim, MODULE_LICENSE), the two Match requirements (toolchain↔chip, kernel-tree↔running-kernel), Kbuild handoff + ifneq(KERNELRELEASE) Makefile, deep linking dive (symbols/symbol-table/unresolved-refs, insmod as dynamic linker, two-stage linking + sysroot), and load mechanics (insmod vs modprobe vs rmmod, lsmod/dmesg/modinfo). **Full BBB bring-up from scratch** (old SD card had corrupted): flashed official Debian 13 kernel-6.18.52-bone54 image → S2 boot → serial+SSH (ssh bbb, key auth) → **cross-compiled the entire 6.18.52-bone54 kernel from bb-kernel source** (after the linux-headers .deb path failed with fixdep Exec-format-error; the source build self-provisions GCC 15 and builds x86 host tools correctly) → **built, deployed, and LOADED first module hello.ko on real hardware** (insmod→hello_init, lsmod refcount 0, rmmod→hello_exit, all seen in dmesg; Matches verified via modinfo vermagic + file ARM). Created **`FLASH.md`** runbook capturing the whole procedure so an SD corruption never costs a session again. Session was long and setup-heavy; conceptual material was re-anchored near the end (user flagged losing track — re-taught module basics + confirmed via 2 checks). **Next: Week 4 — Char Drivers** (environment now proven; kmod/scull skeleton present). Also pending: W3 mini-drill, and the ~Aug 10 mixed R2 interview.
