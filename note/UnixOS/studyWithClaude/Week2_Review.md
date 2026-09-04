# Week 2 Review — Multithreading & IPC

**How to use this:** Mechanism-first, same as Week 1. Built from the lessons *and* the specific things you questioned/derived, since those are your anchors. The through-line to hold across the whole week: **everything is an fd, and buffers decouple producers from consumers everywhere** (page cache, pipe buffers, socket buffers — same shape).

**The recurring pattern this week:** *check-then-act* appeared five times — plain race, CAS, condition-variable `while`, futex re-check, signalfd setup ordering. When you see "check a shared thing, then act on the result," ask "what closes the gap?"

---

## 1. Instruction-Level Race Conditions

`counter++` is **three instructions**, not one: **load → modify → store** (ARM `ldr/add/str`; x86 `mov/add/mov`). You saw this in your own `race.c` disassembly at `11be/11c5/11c9`.

**Lost update:** two threads each load the same value, each increment their own register copy, each store — two increments produce one effect. Works because each thread/core has its **own registers** (per-thread via pt_regs/thread_struct; per-core physically). The race is a property of **shared memory**, not the instruction sequence — `i` on the private stack (`-0x4(%rbp)`) runs the identical triplet safely.

**Single-core still races:** a timer interrupt can preempt between load and store; the stale register value gets restored and its store overwrites. Rarer window → harder to reproduce → *worse* in practice.

**Fixes:**
- **Atomic** (single variable): `atomic_fetch_add` → compiles to `lock addq` (one instruction, `f0` lock prefix = hold the cache line exclusively for the whole RMW). ARM: `ldrex/strex` retry loop.
- **Mutex** (compound / multi-variable / check-then-act): brackets the triplet so no interleaving is possible.

**Key principle — atomicity doesn't compose:** two atomic ops in sequence are NOT atomic together. An `atomic_int` does NOT fix a check-then-act race (`if (x==0) act();`) — the gap between the atomic load and the act remains. Use CAS or a mutex for check-then-act.

**`volatile` is NOT a concurrency tool.** It forces real memory access (no register caching) but gives (1) no atomicity — `volatile counter++` is still the 3-instruction triplet; (2) no inter-core ordering/visibility (no memory barrier). Its real home: MMIO registers (Week 4+). Confusing volatile with synchronization is a classic mid-level mistake.

**Optimization hides races, doesn't fix them:** `-O2` may hoist `counter` into a register for the whole loop → one giant RMW window instead of 100k small ones → race becomes rare but still there (and racy code is UB — the compiler optimizes assuming no races).

---

## 2. What a pthread Actually Is

**A thread IS a `task_struct`.** No separate "thread object." `clone()` is the general primitive; flags decide what's shared vs copied:

```
fork()           = clone(SIGCHLD)              → share nothing (COW)
pthread_create() = clone(CLONE_VM | CLONE_FILES | CLONE_FS |
                         CLONE_SIGHAND | CLONE_THREAD | ...)
                                               → SHARE mm, fd table,
                                                 signal handlers
```

Same scheduler, same object; difference is only which pointers in the task_struct point to shared vs private structures. Consequences, all mechanistic:
- **"Threads share memory"** = their task_structs point to the *same* `mm_struct` → same page tables → same CR3/TTBR0. A thread↔thread switch **skips the CR3 reload** (kernel checks `prev->mm == next->mm`) → **no TLB flush** → this is *why* thread switches are cheaper than process switches.
- **"Own stack"** = a separate stack VMA per thread inside the *shared* address space. But no protection between them — a stray pointer in thread A can corrupt thread B's stack (same page table, no ownership concept). Convention, not enforcement — unlike processes (separate page tables → hardware-enforced isolation).
- **PID vs TID:** every task_struct has a unique `pid`; userspace "process PID" = **TGID** (first thread's pid). `getpid()` = TGID (shared); `gettid()` = the individual task pid.

---

## 3. Mutex Internals (futex)

A pthread mutex = **an atomic int in user memory** + kernel help only when contended. States: `0`=unlocked, `1`=locked no waiters, `2`=locked with waiters.

**Fast path (uncontended) — pure user space, ZERO syscalls:**
- lock: CAS `0→1` succeeds → done (~20ns).
- unlock: exchange → `0`; old value was `1` (no waiters) → done, no syscall.

**Slow path (contended):** lock's CAS fails → set state `2` → `futex(FUTEX_WAIT)` syscall → kernel puts task on a wait queue, sleeps it. unlock sees old value `2` → `futex(FUTEX_WAKE)`.

**exchange vs CAS (you asked this):**
- **CAS** (compare-exchange) = *conditional* write: write only if current==expected, else fail untouched. Used by **lock** because locking is conditional ("take it IF free") and the loser must *know* it lost.
- **exchange** = *unconditional* write, returns old value. Used by **unlock** because releasing is unconditional (owner's right) — it just needs to *learn* the old state (was it `1` or `2`? → wake or not). Returning the old value is the whole point; a plain store would destroy the "were there waiters" info.
- Rule: **every atomic primitive fuses a read+write into one indivisible step.** exchange = read+write; CAS = read+compare+conditional-write; fetch_add = read+modify+write.

**Cost:** uncontended ~20-50ns (two atomics); contended ~2-10µs (two syscalls + two context switches). → keep critical sections short; don't fear uncontended mutexes; spin only if wait ≪ sleep/wake round-trip.

**RTOS vs futex (corrected):** BOTH are two-path (fast uncontended / slow blocking). The futex's achievement is **relocating the fast path entirely into user space**, which only matters *because* Linux has an expensive privilege boundary. An RTOS (flat address space, no ring crossing) neither needs nor benefits from this — its "call into scheduler" is just a cheap function call. Also: RTOS mutexes have priority inheritance by default; Linux pthread mutexes need `PTHREAD_PRIO_INHERIT` opt-in.

---

## 4. Condition Variables + Spurious Wakeups

A mutex answers "may I touch the data?"; a condvar answers "wait until the data is in the state I need." It's a **waiting room** attached to a condition.

```c
pthread_mutex_lock(&m);
while (!condition)              // ★ WHILE, never IF
    pthread_cond_wait(&cv, &m); // atomically: unlock m + sleep;
                               // re-lock m before returning
/* act on condition — safe, holding m */
pthread_mutex_unlock(&m);
```

**Why `cond_wait` takes the mutex:** it must **atomically** "unlock + sleep" — otherwise a signal firing in the gap between "unlock" and "sleep" is lost forever (lost wakeup). Same check-then-act fix as futex, one level up.

**Why `while` not `if` — two reasons:**
1. **Stolen condition:** between the wake and re-acquiring the mutex, another thread may consume the condition. Re-check under the lock.
2. **Spurious wakeups:** POSIX explicitly permits `cond_wait` to return with no signal. Allowed because correct code needs the `while` for reason 1 anyway, so requiring precise wakeups would buy nothing and cost performance.

The wake is only a **hint**; truth is verified under the mutex. `if` + cond_wait = bug.

---

## 5. False Sharing

**Caches move data in fixed 64-byte lines** (x86 and Cortex-A), not bytes/variables. **Cache coherency (MESI):** to write a line, a core must own it exclusively → writing invalidates every other core's copy of that line. Coherency tracks **lines, not variables**.

**False sharing:** two threads write two *different* variables that happen to sit in the *same 64-byte line* → the line ping-pongs between cores' caches on every write → 5-50× slowdown. Code is **100% correct** (coherency guarantees correctness) — it's purely a performance bug, and adding cores makes it *worse*.

- **True sharing:** same variable → needs synchronization (correctness). Reduce only by sharing less.
- **False sharing:** same line, different variables → needs layout separation (performance). Fix: pad/align hot per-thread data to 64B (`alignas(64)`, `__attribute__((aligned(64)))`; kernel: `____cacheline_aligned`), or group fields by writer, or make hot data thread-local and merge rarely.
- Find it: `perf c2c` (HITM storms on one line touched by two functions).
- BeagleBone Black (single-core Cortex-A8) doesn't show it; multi-core boards do — "works on the bench, collapses on the quad-core."

---

## 6. IPC — Deliberate Holes in the Isolation Wall

Processes are isolated by default (separate page tables). IPC = kernel-provided openings. Threads share by default and must *discipline*; processes isolate by default and must *opt in*.

### 6a. Pipes
Kernel RAM buffer (~64KB) + **two fds** (`fds[0]` read, `fds[1]` write). Pairs with `fork()` via **fd-table inheritance** — child's copied fd table points to the *same* `struct file` entries (refcounts++). Anonymous pipe has no name → only reachable by inheritance (`mkfifo` adds a filesystem name for unrelated processes).
- **Blocking built in:** read-on-empty sleeps, write-on-full sleeps → data + sync in one primitive.
- **EOF rule:** `read()` returns 0 only when buffer empty AND **write-end refcount == 0** (all writers closed everywhere). → must close unused ends after fork, or "all writers closed" never happens → reader blocks forever (**forgotten-close deadlock**; you traced it: a process holding its own unused write-end starves its own read).
- **dup2 splicing:** `dup2(fds[1], STDOUT_FILENO)` makes fd 1 point at the pipe's `struct file` → `cmd1 | cmd2` = pipe + fork×2 + dup2 + exec (the mechanism you dissected: shell is the fd distribution hub; `close()` drops *my* reference, never the object; refcount reaches 0 only when the last holder closes).
- Writes ≤ `PIPE_BUF` (4096) are atomic.

### 6b. Shared Memory
Kernel writes PTEs in **both** processes' page tables pointing at the **same physical pages**. After setup: plain loads/stores, **zero copies, zero syscalls** — fastest IPC because it's *not* IPC, just memory.
- API: `shm_open` (→ fd, tmpfs object at `/dev/shm`) → `ftruncate` (size) → `mmap(MAP_SHARED)` (writes the PTEs). `MAP_SHARED` = anti-COW (writes go to real shared pages; `MAP_PRIVATE` = COW, what libc data uses → `rw-p` vs `rw-s` in maps).
- **Zero-copy = eliminating the private buffers on both sides**, not a faster transfer. Producer writes directly into shared pages; consumer reads them in place. (Note: sockets/pipes always copy through kernel buffers — shm is the *only* true no-copy mechanism, because normally user↔user memory can't be touched directly.)
- **Never store raw pointers in shm** — the region sits at *different VAs* in each process; a pointer is a VA = a coordinate in one process's page table. B dereferencing A's stored pointer → walks B's page table → SIGSEGV (if unmapped) or silent corruption (if some other VMA covers it). **Store offsets from region base; rebase locally.** (Network echo: htons/byte-order is the same lesson — data crossing a boundary needs a representation both sides agree on.)
- **Sync is your problem** — it's threads again (same races, cache lines, false sharing). Put locks *inside* the region with `PTHREAD_PROCESS_SHARED` (the futex must key by **physical page + offset**, not VA, since the lock word is at different VAs per process).
- **Lifetime:** persists until `shm_unlink` or reboot (it's a tmpfs file). Orphans leak `/dev/shm`. Crash while holding a mutex → poisoned lock **survives process restart** (state lives in the named object, unlike a pipe whose state dies with its holders).
  - **Crash recovery** (you designed/derived these): (1) **unlink-and-recreate** on startup — simple, loses state, watch that `unlink` removes the *name* not live mappings; (2) **robust mutex** (`PTHREAD_MUTEX_ROBUST`) → next locker gets `EOWNERDEAD`, repairs state, calls `pthread_mutex_consistent()`; (3) **lock-free layout** (SPSC ring, atomic indices — nothing to orphan).
  - **Live rescue** (no robust mutex, already wedged): `rm /dev/shm/<name>` **then kill the stuck process** **then restart** — order matters (rm before restart, else the new instance re-opens the poisoned object; kill to free the orphaned mapping). A wedged process can't be un-wedged, only replaced. This manual recipe = unlink-and-recreate done by a supervisor.
  - Field diagnosis: `ps` state `S`; `/proc/<pid>/wchan` = `futex_wait`; `/proc/<pid>/maps` shows `/dev/shm/x (deleted)`.

### 6c. Message Queues
Kernel-mediated like pipes (2 copies), but **message-oriented**. Closest Linux relative of RTOS (ThreadX/FreeRTOS) queues.
- `mq_open` (named, `/name`), `mq_send`/`mq_receive`, unlink like shm.
- **Boundaries preserved:** each `mq_receive` returns exactly one whole message (pipes = byte stream, framing is your job).
- **Priority ordering:** highest priority dequeued first (0-31). Risk: **starvation** if high-prio floods. Priorities = urgency classes, must be sparse.
- **Priority does NOT help admission when full:** a full queue blocks even a high-prio sender — nothing is evicted. → **priority inversion, MQ edition** (urgent sender waits on consumer drain rate). Fix: separate `/control` queue, or O_NONBLOCK+escalate. (RTOS-instinct trap: high priority does NOT preempt into a full queue.)
- **EMSGSIZE contract:** `mq_receive` fails if your buffer < `mq_msgsize`, **regardless of the actual waiting message's size**. Because boundary-preservation requires "your buffer must hold ANY legal message" — a statically checkable contract, deterministic. (Contrast pipe `read(fd,buf,64)` with 30 bytes available → returns 30; "up to N of the stream.") Principle: **fail on the contract, not on the traffic.**
- **sysctl knobs** (`/proc/sys/fs/mqueue/`, you inspected these): `msg_max` = ceiling on `mq_maxmsg` you may request (limit on the limit — not the count itself); `msgsize_max` = ceiling on `mq_msgsize` (default 8192 → rejects a 1MB-message design → the kernel enforcing "queues carry commands, not payloads"); `queues_max` = system-wide queue count. Deployment config; mismatched values = "works on dev, EINVAL in the field."
- **shm+mq hybrid (you designed this):** data plane = shm (zero-copy), control plane = mq carrying `{offset, length, seq#}` (offsets not pointers!). A return queue recycling free slots → full buffer ring. The camera/DSP/DPDK pattern.

### 6d. Signals
Not a data channel — a **user-space interrupt**. Async notification only.
- **Model:** sending sets ONE bit in the target's pending bitmask in `task_struct` + wakes it. **Standard signals don't queue** — 5 sends before delivery = the bit is set, extras vanish (**coalescing**). Carries 1 bit: "this kind of event happened ≥1×."
- **Delivery = pt_regs hijack** (your Week 1 machinery): checked on **return-to-user** (any syscall/interrupt/timer-tick exit). Kernel copies the interrupted user context into a **signal frame** on the user stack, rewrites pt_regs (PC→handler, return-addr→trampoline), returns to user → **handler runs at RING 3, user space** (never kernel — a user-chosen function at ring 0 would break all security). Handler's `ret` → trampoline → `sigreturn` syscall → kernel restores the original context from the frame → seamless resume.
  - **Handler is user space; the two kernel round-trips only bracket it.** `sigreturn` exists because restoring the full register set + signal mask is privileged. Trampoline = tiny stub (in `[vdso]`) whose only job is the `sigreturn` syscall; the kernel pre-aimed the handler's return address at it.
- **Self-deadlock — the gap within one thread:** a handler fires between any two main-flow instructions. If main flow holds a lock (e.g. mid-`malloc`/`printf` internal lock) and the handler calls the same non-reentrant function → same thread waits for a lock only it can release → freeze. **No mutex can fix it** (one thread, both sides). → handlers must be **async-signal-safe** only (`write` yes; `printf`/`malloc` no).
- **Pro pattern:** handler sets `volatile sig_atomic_t flag = 1;` and returns; main loop acts. This is the **one legitimate concurrency use of volatile** (same thread/core → no atomicity/ordering issue; only the compiler-caching problem, which volatile solves).
- **EINTR trap:** a signal interrupting a blocked slow syscall → syscall returns -1 with `errno==EINTR` (interruption, not failure). Naive `if (n<0) die()` → daemon dies on first signal. Idiom: `if (errno==EINTR) continue;`. `SA_RESTART` auto-restarts *most* (not all) syscalls — retry idiom still mandatory.
- **signalfd (modern):** `sigprocmask(SIG_BLOCK)` the set, then `signalfd()` → signals arrive as **readable data** on an fd → no handler, no hijack, no safety rules, no EINTR → drop into a `poll()` loop with other fds. Also delivers sender pid/uid (richer than a handler's int). Caveat: **block before creating the fd**; multithreaded → block in all threads (mask is per-thread; process-directed signal goes to any thread not blocking it → block-then-spawn so the mask inherits) and drain in one place.

### IPC decision axes
data volume? boundaries needed? who synchronizes? related processes or not? persist across crashes?
- pipe: stream, related (or FIFO), built-in sync, simplest
- shm: zero-copy bulk, you synchronize, persists (design for crash)
- mq: discrete prioritized commands, built-in sync
- signal: lifecycle/notification only
- socket: client-server, local OR network (see §7)

---

## 7. Sockets — the universal IPC

**A socket is an fd with the network stack behind it** (`f_op` → socket ops instead of a filesystem). Same fd table, same read/write/close, same fork-sharing via refcounted `struct file`, same poll-ability. Only new thing: network stack, not filesystem.

**Coordinates** (replace the shared name): (1) **family/domain** — AF_INET (IPv4), AF_INET6, AF_UNIX (local); (2) **type** — SOCK_STREAM (TCP: reliable, ordered, byte-stream, connection) vs SOCK_DGRAM (UDP: best-effort, unordered, datagram, connectionless); (3) **address** — IP + port. A TCP connection is identified by the **4-tuple** (src IP, src port, dst IP, dst port) → how one server port serves thousands of clients.

**STREAM vs DGRAM decision:** TCP when correctness matters (config, commands, request/response — the default). UDP when latest-beats-complete or you can't afford connection overhead (live sensor streams, video/audio, discovery) — add **sequence numbers** to detect loss/reorder (detect, don't recover — same idea as the shm+mq seq#). Echoes of Week 2: TCP = byte stream = pipe's framing problem over the network; UDP = datagrams = message-queue boundaries + EMSGSIZE-style "buffer must hold it, else truncated."

**TCP lifecycle (server and client are asymmetric):**
```
SERVER: socket → bind → listen → accept (loop) → read/write → close
CLIENT: socket →              →              connect → write/read → close
```
- **`accept()` returns a NEW fd** per client. The listening socket is a *factory* — it keeps listening; each accept hands back a distinct connection fd (distinguished by the 4-tuple). One listen fd, many connection fds.
- **Handshakes are done by the kernel's TCP stack**, not your code. `connect` blocks the client, `accept` blocks the server, through the 3-way handshake (SYN/SYN-ACK/ACK). `close` (last fd → refcount 0) triggers the FIN teardown handshake.
- **No `bind` on the client** — kernel auto-assigns an ephemeral source port (that's the src-port half of the 4-tuple).

**listen backlog + queue (single-threaded starvation, dissected):** the kernel completes handshakes into a **queue of depth `backlog`** WITHOUT the app calling accept. So while a single-threaded server is busy handling client A in-line: client B's `connect()` **succeeds immediately** (kernel did the handshake, B parked in the queue) — B is *connected-but-unserved*, not blocked. B's app-level requests get no response until the server loops back to accept (~later). When the queue fills (backlog reached), new SYNs are dropped → *those* clients' `connect()` blocks then `ETIMEDOUT`. Lesson: an in-line server silently accepts then starves clients; real servers fork/thread/event-loop to return to accept immediately.

**Two-buffer flow-control model (you asked this thoroughly):** every connection has kernel **RX and TX buffers** on each side, allocated when established (not when accepted).
- Data B writes lands in the server's **RX buffer** at the TCP layer, ACKed by the server's **kernel** automatically — **independent of whether the app ever `read()`s it**. `write()` success / TCP ACK says nothing about the app processing it (false-confidence bug).
- **TX buffer cleared by ACKs** (kernel-to-kernel, about reliability/retransmission) — NOT by the peer's reply. **RX buffer cleared by the app `read()`ing** (about consumption).
- **Backpressure chain:** slow reader → RX fills → kernel advertises zero window → sender's TX fills → sender's `write()` **blocks**. Same block-on-full as a pipe, stretched over the wire. What un-blocks the sender is the peer **`read()`ing** (draining RX) — not replying. (UDP has no such backpressure: full RX buffer → datagrams silently **dropped**.)

**The read/write deadlock (you derived it):** single-threaded server echoing; client floods and never reads echoes → client RX fills → server TX fills → server blocks in `write()` → server stops `read()`ing → server RX fills → client blocks in `write()` too. **Both blocked in write(), each needing the other to read() — circular wait, true deadlock.** Cause: blocking read + blocking write in one thread. Fix: **non-blocking I/O + event loop (select/poll/epoll)** — never stuck in one direction. This is the same architecture as signalfd → **all fds (sockets, signalfd, pipes, timerfd) drained in one poll loop** (Week 8 epoll).

**Gotchas:**
- **Byte order:** networks use big-endian ("network order"); your host is usually little-endian. `htons`/`htonl` (host→network, ports/addresses), `ntohs`/`ntohl` (reverse, on received data). Forgetting → "works between two x86 boxes, breaks talking to a big-endian device." Same lesson as offsets-not-pointers: cross-boundary data needs an agreed representation.
- **`struct sockaddr_in` + cast to `struct sockaddr *`** = C's crude polymorphism, dispatched by `sin_family`. Always `memset` to zero first. `inet_pton` (text→binary IP, in), `inet_ntop` (binary→text, out).
- **TIME_WAIT + SO_REUSEADDR:** the side that closes first parks the connection ~60s (so stray old packets can't corrupt a new connection on the same 4-tuple) → restarting a server fails `bind()` with "Address already in use". `SO_REUSEADDR` lets you bind anyway — set on essentially every server socket.
- **`read()` returns 0 = clean disconnect** (peer's FIN, EOF — same as pipe); `< 0` = error.
- **`accept`'s `&client`/`&clen`:** kernel writes back the peer's IP/port. The fd already separates clients' data; `&client` is for the *app's* logic (logging, access control, rate-limiting per source). `clen` is value-result.
- **connect errno diagnostics:** `ECONNREFUSED` = host up, nothing listening (RST); `ETIMEDOUT` = no response (host down/unreachable or queue full). Different causes, distinguishable.

**AF_UNIX — sockets unify the IPC landscape:** change `AF_INET`→`AF_UNIX` and `sockaddr_in`→`sockaddr_un` (addressed by a filesystem path); *all other code identical*. Local-only, so the kernel **skips**: the 3-way handshake, checksums, IP routing / TCP header & sequence/ACK/window processing, and the network device layer — all unnecessary when both processes are in one kernel with no unreliable medium. Faster than TCP-over-loopback; standard for local client-server (Docker, X11, systemd, D-Bus).
- **Correction to bank:** AF_UNIX still copies through **kernel buffers** (user→kernel→user) — it does NOT write directly into the peer's memory (that would violate isolation). The difference vs AF_INET is *how much protocol machinery wraps the copy*, not "direct vs buffered." **shm is the only true zero-copy** mechanism.
- **Design rule:** local client-server that might go cross-machine later → **AF_UNIX now** (trivial port to AF_INET). Pure local bulk throughput → shm. Simple stream between related processes → pipe. Discrete prioritized commands → mq.

---

## Rapid self-test (answer cold before the Jul 31 mixed interview)

1. Why is `counter++` unsafe but `i++` on a stack local safe, given both compile to load/modify/store?
2. `pthread_create` shares what with the creator, via which clone flags? Why is a thread↔thread context switch cheaper than process↔process?
3. Uncontended mutex lock+unlock: how many syscalls, and why? Why does lock use CAS but unlock use exchange?
4. Why must a condvar wait loop use `while`, not `if`? Give both reasons.
5. Two threads increment two *different* variables, no sharing, yet it's slow. What is it, why, and the fix?
6. Pipe: why must you close unused ends after fork? Trace the deadlock if you don't.
7. shm: why can't you store pointers in it? What do you store instead? A crash left a mutex locked in shm — three ways to handle it.
8. mq: `mq_receive` with a 64-byte buffer, queue `mq_msgsize`=256, waiting message is 30 bytes — succeed or fail, and why?
9. Signal handler: why must it be async-signal-safe? Why can't a mutex fix a handler self-deadlock? What's signalfd and why prefer it?
10. Single-threaded echo server, client floods and never reads: trace the deadlock through all four buffers. What's the fix and why is it the same architecture as signalfd?
11. AF_UNIX vs AF_INET: what does the kernel skip locally? Does AF_UNIX write directly to the peer's memory? (Trap.)
12. TCP: `accept()` returns what? While a single-threaded server is busy, does a new client's `connect()` block, fail, or succeed? Why?
