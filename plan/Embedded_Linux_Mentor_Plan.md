# Embedded Linux Mentorship — Tracking Document

**Goal:** Level up from intermediate embedded C/C++ engineer to job-ready middle/senior embedded software developer, focused on Embedded Linux (kernel → application) and CI/CD for embedded projects.

**Timeline:** ~11 weeks, 15+ hrs/week
**Started:** 2026-06-21
**Reference material:** *The Design of the UNIX Operating System* (Bach) for internals theory, *Linux Device Drivers, 3rd Ed* (LDD3) for hands-on driver development (note: LDD3 targets kernel 2.6 — APIs will be modernized to current kernel during lessons)

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
| 1 | Scheduling triggers (timeslice expiry, blocking, priority) | ✅ Solid | Correctly explained when context switches happen (timeslice expiry, blocking on event/mutex/semaphore, higher-priority process ready) |
| 2 | Context switch mechanism (what's saved, where) | ✅ Solid (W1 built, mock-confirmed) | Full register set → thread_struct / cpu_context_save; pt_regs vs thread_struct distinction; switch_to(); `current`; CR3/TTBR0 + TLB flush cost. Traced real ARM source. **Mock Jul 17: Q1 answered cleanly, led with *why* pt_regs matters — graduated to solid.** |
| 3 | Syscall mechanism | ✅ Solid (mock-confirmed) | Address-space misconception fully corrected: privilege ring 3→0 / EL0→EL1, user→kernel stack switch, page tables unchanged, syscall/SVC as controlled gate, CPL in CS register. Mock Jul 17: clean. |
| 4 | fork() semantics | 🔁 Reinforced — ⚠️ **COW granularity soft** | Model solid: task_struct copy, VMA/PT copy with COW marking, fd table dup, returns twice, fork+exec, exec keeps fd table. **GAP (drill + mock Jul 17): COW copies ONE PAGE (4KB), not "4 bytes" — user answered "4 bytes." Also MMU-detects-vs-kernel-repairs blurred. Re-drill.** |
| 5 | Thread semantics | ✅ Solid (W2 deepened) | W2: thread = task_struct via clone(CLONE_VM/FILES/...); shared mm → no CR3 reload on thread switch; own stack but no inter-thread stack protection. |
| 6 | Virtual memory — why it exists | 🔁 Reinforced | Derived isolation + fragmentation + overcommit independently. MMU, multi-level page tables, PTEs, VMA-vs-page-table, demand paging, lazy PT construction, TLB, /proc/self/maps read in full. |
| 7 | Buffer cache / write-back | ✅ Solid | Reinforced during open/read syscall walkthrough (page cache on read path). |
| 8 | Race conditions — scenario level | ✅ Solid | Constructed valid check-then-act race unprompted at diagnostic; reinforced all through W2. |
| 9 | Race conditions — instruction-level (atomicity) | 🔁 Reinforced (W2) | **Built with real disassembly of user's own race.c**: saw load/add/store triplet at 11be/11c5/11c9, traced lost update, saw `lock addq` fix, understood atomicity-doesn't-compose (derived independently). CAS vs exchange, futex fast/slow path. |
| 10 | Interrupt vs syscall | ✅ Solid | Correctly distinguished hardware- vs software-triggered. |
| 11 | Kernel space vs user space — why | ✅ Solid (mock-confirmed) | Privilege rings HW-enforced; privileged instructions blocked at ring 3; controlled-gate entry; ARM EL0/EL1 + SVC. **Mock Jul 17: closed the "jump to kernel addr" loophole cleanly — jump ≠ privilege, ring is the gate, syscall is the only door. Graduated.** |
| 12 | Page faults | ⚠️ Partial (preview) — **MMU-vs-kernel roles soft** | Preview built W1. **GAP (mock Jul 17): user blurred "MMU allocates/copies" — correct is MMU *detects* + raises fault, kernel handler *repairs*. Full depth Week 7; fix the who-does-what split before then.** |
| 13 | fd / open file table / inode (3-layer model) | 🔁 Reinforced — ⚠️ **scope-labeling wobble** | Model + full open()/read() lifecycle + dup2/redirection + kernel source chain + private_data all built and mostly solid. **GAP (mock Jul 17): under questioning, user labeled the middle layer (struct file) "per-process" then said fork "shares" it — contradiction. Correct: fd table = per-process (copied); struct file = KERNEL-GLOBAL (shared); inode = kernel-global. Also "where do they live" = kernel memory, user can't touch. Re-drill — softest spot.** |
| 14 | Disk block mapping (how FS locates file data on disk) | ⚠️ Partial | Light coverage during read() walkthrough (inode block map / ext4 extents). Lower priority — full depth optional. |

### Overall pattern

**Strength:** Conceptual/policy-level reasoning is genuinely good — consistently correct on *why* a design exists and *when* mechanisms trigger. Recovers gracefully when a fact is missing (reasons toward it instead of bluffing). This is the harder skill to teach and it's present.

**Consistent gap (refined after Jul 17 mock):** Mechanism-level *facts* cluster around ONE seam — **where kernel structures physically live, and who (hardware vs software) performs each step.** Three symptoms of this one root: (a) COW granularity = page not bytes [#4]; (b) MMU-detects-vs-kernel-repairs [#12]; (c) fd/struct-file/inode scope + "they live in kernel memory, user can't touch" [#13]. Fixing the seam fixes all three.

**Implication for the plan:** Don't re-read Bach. Rebuild mechanism-level pieces from scratch with diagrams + code + tracing, reinforce through reuse. The three ⚠️ items above are the declared focus for the next review (~Jul 31, mixed W1+W2).

**Week 1 observation (2026-07-10):** Mechanism-gap diagnosis confirmed; fix working — user actively traces real kernel source, questions examples that don't match expectations, asks for full end-to-end walkthroughs. Keep encouraging tracing + questioning.

**Week 2 observation (2026-07-17):** Same pattern deepened — user compiled/objdumped their own race.c and atomic version, caught a genuinely misleading example (the "global counter" false-sharing framing) and pushed back correctly, independently derived "atomicity doesn't compose" and reinvented the shm+mq zero-copy hybrid and the unlink-and-recreate crash-recovery pattern. Retention strategy strongly confirmed. Requested strict one-topic-at-a-time pacing (flagged after a too-dense IPC intro) — **honor this: never batch multiple sub-mechanisms in one message.**

---

## The Plan

**Format per topic:** concept from first principles → user/kernel/hardware/Device-Tree role breakdown (where relevant) → code example → small hands-on task → interview Q&A + common pitfalls.

### Phase 1 — User Space Foundations (Weeks 1-2)

**Week 1 — Process, Syscall & Memory Mechanics** ✅ **COMPLETE (2026-07-10)** — review doc: `Week1_Review.md`
- ✅ Context switch — pt_regs vs thread_struct, ARM cpu_context_save, current, CR3/TTBR0
- ✅ Syscall mechanism + CPU privilege rings (x86 ring 0/3, ARM EL0/EL1)
- ✅ fd → open file table → inode 3-layer model, open()/read() lifecycle, dup2/redirection, private_data
- ✅ Virtual memory — MMU, multi-level page tables, VMA-vs-page-table, demand paging, TLB, /proc/self/maps
- ✅ fork() from zero — COW, fd sharing, fork+exec
- ✅ Page fault preview (full depth Week 7)

**Week 2 — Multithreading & IPC** ✅ **COMPLETE (2026-07-17)** — review doc: `Week2_Review.md`
- ✅ Instruction-level race conditions (disassembly of race.c, lost update, lock addq, CAS vs exchange)
- ✅ What a pthread is (clone flags; thread = task_struct sharing mm/files; thread-switch cost)
- ✅ Mutex internals (futex fast/slow path, exchange vs CAS asymmetry, 3-state 0/1/2 design)
- ✅ Condition variables + spurious wakeups (while-not-if), RTOS-vs-futex mutex comparison (corrected: both two-path; futex relocates fast path to user space)
- ✅ False sharing (cache lines, MESI, padding/alignment, true vs false sharing)
- ✅ IPC — pipes (refcounts/EOF/dup2 splicing), shared memory (dual PTEs, offsets-not-pointers, pshared/robust mutex, unlink lifecycle + crash recovery), message queues (boundaries, priorities, EMSGSIZE contract, shm+mq hybrid, /proc/sys/fs/mqueue knobs), signals (bitmask, pt_regs hijack, trampoline/sigreturn, self-deadlock, coalescing, signalfd, per-thread masks)
- ✅ Sockets — fd + network stack, TCP lifecycle (socket/bind/listen/accept/connect), accept() returns new fd, listen backlog + queue behavior, two-buffer flow-control model (TX cleared by ACK / RX cleared by read), byte order (htons/ntohs), TIME_WAIT + SO_REUSEADDR, read/write deadlock + event-loop fix, UDP (recvfrom/sendto, datagram boundaries, no backpressure), AF_UNIX (same API, local, unifies IPC landscape)

### Phase 2 — Kernel Fundamentals & First Driver (Weeks 3-4)

**Week 3 — Kernel Modules & Build System**
- `module_init`/`module_exit`, kernel build system, Kbuild/Makefiles, kernel coding conventions
- How `insmod`/`modprobe` actually work
- Clone BeagleBone (TI) kernel tree, set up cross-compile toolchain

**Week 4 — Char Drivers**
- `file_operations`, major/minor numbers
- Write a real char driver end-to-end (e.g. loopback or fake sensor device), test from user space
- *(Target: BeagleBone — confirmed. Prep before this week: cross-compiler, kernel headers matching board's running kernel, SSH/serial access)*

### Phase 3 — Platform Drivers & Hardware Interaction (Weeks 5-6)

**Week 5 — Platform Driver Model & Device Tree**
- Platform driver model, Device Tree bindings, how DT connects to driver `probe()`
- Clear driver/DT/hardware role separation

**Week 6 — Interrupts & Kernel Synchronization**
- Top half/bottom half, tasklets, workqueues, threaded IRQs
- Spinlocks vs mutexes vs RCU — why you can't sleep in a spinlock
- (RCU intro already given W1 via `__rcu` fd array; user-space spinlock trade-off given W2 — build on both)

### Phase 4 — Memory, Debugging & Observability (Weeks 7-8)

**Week 7 — Memory Mapping & DMA**
- `mmap`, `ioremap`, `kmalloc` vs `vmalloc`
- Full page fault mechanism (deferred from Week 1 — W1 built VMA/PTE/demand-paging foundation; also fixes diagnostic #12 MMU-vs-kernel roles)
- DMA basics: coherent vs streaming, DMA-safe memory

**Week 8 — Observability & Debugging**
- sysfs/procfs/debugfs — exposing driver state to user space
- Kernel debugging toolkit: printk/dynamic debug, ftrace, kgdb/kdb, oops/panic analysis
- Common race condition bugs in real driver code
- (Field-diagnosis triad seeded in W2: ps state / /proc/<pid>/wchan / /proc/<pid>/fd for hung processes)

### Phase 5 — Boot Process & Build Systems (Week 9)

**Week 9 — Boot Chain & Yocto**
- Full boot chain: ROM → SPL → bootloader → kernel → rootfs
- U-Boot internals and commands
- Secure Boot chain of trust basics
- Yocto Project fundamentals: layers, recipes, bitbake

### Phase 6 — CI/CD for Embedded (Weeks 10-11)

**Week 10 — Git, Static Analysis, Unit Testing**
- Git workflows for embedded teams (branching, review practices for safety-critical code)
- Static analysis: cppcheck, clang-tidy
- Unit testing for embedded C/C++: Unity/GoogleTest, mocking hardware

**Week 11 — Build Pipelines & Integration Testing**
- Cross-compilation in CI
- CI/CD for Yocto-based projects (build caching, sstate-cache, automated image builds)
- Integration testing strategies, hardware-in-the-loop (HIL) testing concepts

---

## Open Items / Decisions Needed

- [x] **Target environment for Week 4 onward**: real hardware — **BeagleBone**. Decided 2026-06-21.
- [ ] RTOS topics deliberately excluded from this plan per user request (already has practical RTOS experience) — can be revisited later if needed
- [ ] Before Week 3: prepare BeagleBone environment (TI kernel tree, cross-compiler, serial/SSH access)
- [x] Build tooling preference noted: user prefers **CMake** over raw gcc (target_-scoped commands, Threads::Threads, Debug build type for readable disassembly). Reinforce good CMake habits in code exercises.

---

## Review Schedule (Spaced Retrieval Practice)

**Purpose:** Combat forgetting via retrieval practice at expanding intervals. Mini-drill = 10-15 min, 4-6 quick questions. Mock interview = 20-30 min, full interviewer persona with follow-up probing, no teaching until after the session ends.

**Session-start protocol (for Claude):** At the start of EVERY session, check this table. If any review is due (or overdue), inform the user before starting new material. The user decides whether to run the review now or continue with lessons. After each completed review: mark it done here, log results in Progress Log, and update the diagnostic table (fumbled → ⚠️ with note; solid twice in a row → ✅).

**Interval rule for scheduling new reviews:** topic completed → mini-drill at +2-3 days → mock interview at +1 week → mixed mock interview at +3 weeks → final comprehensive interview in Week 11.

| Due date | Type | Scope | Status |
|---|---|---|---|
| 2026-07-12 | Mini-drill | Week 1 | ✅ Done 2026-07-12 — 3/6 solid, 3/6 partial (MMU-vs-kernel fault roles, per-page COW, privilege gates) |
| 2026-07-17 | Mock interview R1 | Week 1 material | ✅ Done 2026-07-17 — pass w/ reservations. Graduated: privilege gates, context switch, syscall. Still ⚠️: COW granularity (#4), MMU-vs-kernel roles (#12), fd/file/inode scope labeling (#13, newly surfaced). |
| ~2026-07-19/20 | Mini-drill | Week 2 (races/atomics, pthreads/clone, mutex/futex, condvars, false sharing, IPC: pipes/shm/mq/signals, sockets) | ⬜ Pending |
| ~2026-07-31 | Mock interview R2 | Weeks 1-2 mixed | ⬜ Pending — **carry-over W1 focus items: (1) fd/struct-file/inode scope + where they live [#13], (2) MMU-detects-vs-kernel-repairs [#12], (3) per-page COW granularity [#4]** + W2 material. |
| Week 11 (end of plan) | Final comprehensive | Everything, full simulation | ⬜ Pending |

*(Add new rows as each week completes. Mark ✅ Done + date when finished. If a review is skipped/deferred, keep it Pending and flag as overdue at next session.)*

---

## Progress Log

*(Update this section as weeks are completed — date, topics actually covered, any deviations from plan, retention check-ins)*

- **2026-06-21**: Initial diagnostic completed, plan created.
- **2026-06-21**: Decided target hardware for Week 4+ : **BeagleBone**. Confirmed update workflow — Claude pulls latest file from project knowledge, edits, hands back; user re-uploads to project.
- **2026-07-10**: **Week 1 COMPLETE.** 4 topics in depth (context switch+syscall+rings, fd 3-layer model + lifecycle, virtual memory, fork/COW). Diagnostic items 2,3,4,6,11,13 → 🔁; 12 → ⚠️ preview; 14 → ⚠️ light. User traced real kernel source on Elixir throughout.
- **2026-07-10**: Added **Review Schedule** system (spaced retrieval). Session-start protocol active.
- **2026-07-12**: **W1 mini-drill done.** 3/6 solid (context switch, syscall changes, fd+fork sharing). 3/6 partial: MMU-vs-kernel fault roles, per-page COW granularity, privilege-gate specifics — flagged as focus for the mock.
- **2026-07-17**: **W1 Mock Interview R1 done** — pass with reservations (mid-to-senior performance). Strong: syscall/read path, privilege boundary (closed the jump-to-kernel-addr loophole cleanly). Graduated to ✅: context switch (#2), syscall (#3), privilege (#11). Still ⚠️ and set as R2 focus: fd/struct-file/inode **scope labeling** (#13 — labeled middle layer per-process then said fork shares it; correct = struct file is kernel-global), **MMU-detects-vs-kernel-repairs** (#12), **per-page COW granularity** (#4 — answered "4 bytes", correct = one 4KB page). Root cause identified: single seam = "where kernel structures live + who does hardware-vs-software step." Created **`Week1_Review.md`** study doc (mechanism-first, ⚠️ gap boxes at the exact weak spots, 7-question cold self-test).
- **2026-07-17**: **Week 2 progress** — completed instruction-level races (own race.c disassembly), pthread/clone model, mutex/futex internals (exchange vs CAS), condition variables + spurious wakeups, false sharing, and all four IPC mechanisms (pipes, shared memory, message queues, signals) in depth. User requested strict one-topic-at-a-time pacing — honor going forward.
- **2026-07-17**: **Week 2 COMPLETE.** Closed with sockets: fd+network-stack model, TCP lifecycle, accept()-returns-new-fd, listen backlog & queue behavior (single-threaded-server starvation traced), two-buffer flow-control (TX/ACK vs RX/read), byte order, TIME_WAIT/SO_REUSEADDR, the classic read/write deadlock + event-loop fix (converges with signalfd → poll/epoll, Week 8), UDP, AF_UNIX unifying local + network IPC under one API. Diagnostic item 5 & 9 solid. Strong session — user derived the mutual-write() deadlock, corrected own misconception that AF_UNIX writes "directly" to peer buffer (both go through kernel buffers; shm is the only true zero-copy). Created **`Week2_Review.md`**. **Next: Week 2 mini-drill (~Jul 19-20), then Week 3 — Kernel Modules & Build System** (clone BeagleBone TI kernel tree, first module).
