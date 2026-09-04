# Week 1 Review — Process, Syscall & Memory Mechanics

**How to use this:** Read top-to-bottom for a refresh. The ⚠️ boxes are *your* gaps from the Jul 17 mock — if you can answer those cold, you're ready. Everything here is mechanism-level, because that was the diagnosed weak seam (you reason well about *why*; the *precise facts* are what slip).

**The one root gap to fix:** most of your mock wobbles trace to a single seam — *where kernel structures live, and who (hardware vs software) does each step*. Watch for it across sections 1, 3, 4.

---

## 1. Context Switch — what's saved, where

When switching task A → task B, the **full CPU register set** must be saved, not just SP/PC: general-purpose registers, PC, SP, and status/flags. Any of them may hold live state (loop counters, partial results, function args in flight).

**Two different register-save structures — do not confuse them:**

| Structure | Captures | When | Lives in |
|---|---|---|---|
| `pt_regs` | user-space state at kernel entry | every syscall/interrupt/exception | top of the task's **kernel stack** |
| `thread_struct` (ARM: via `cpu_context_save` in `thread_info`) | kernel execution state at context switch | only inside `switch_to()` | inside `task_struct` |

Full A→B switch (A in user space → B resumes in user space) uses **both**: `pt_regs` saved A's user state on kernel entry; `switch_to` saves A's kernel state to `thread_struct`, restores B's; on the way back out, `pt_regs` restores B's user state.

**Where task_struct lives:** kernel-owned slab memory — **not** inside any process, **not** touchable by user code. The scheduler always holds a direct pointer (run queue); it never "searches." `current` = per-CPU shortcut to the running task's `task_struct`, found via `thread_info` at the base of the kernel stack.

**Each process has TWO stacks:** a user stack (user-mode execution) and a kernel stack (~8KB, kernel-mode execution on that process's behalf). Separate kernel stack per process is required so multiple processes can be inside the kernel simultaneously without corrupting each other's call frames.

**ARM detail you traced:** `thread_struct` on ARM is nearly empty (just fault/debug info) because ARM saves general registers directly onto the kernel stack via the C ABI callee-save convention (`r4–r14` + PC), and stores the real context in `cpu_context_save` inside `thread_info`. Architecture-specific; x86 stores more in `thread_struct`. Same concept, different placement.

---

## 2. Syscall Mechanism + CPU Privilege Rings

**What a syscall does:** `syscall` instruction (x86) / `SVC` (ARM) traps → CPU switches **privilege ring** (3→0 on x86, EL0→EL1 on ARM) and **switches SP** from user stack to kernel stack. User registers get saved to `pt_regs` on the kernel stack.

**What does NOT change:** the **address space / page tables**. Same process, same `mm_struct`, same CR3/TTBR0. (This was the original diagnostic misconception — corrected.) The kernel is mapped into every process's address space already, just locked behind permission bits, so no address-space switch is needed to run kernel code.

**Privilege rings are HARDWARE-enforced**, not software:
- Ring 0 = full access (kernel, drivers). Ring 3 = restricted (user apps). Linux uses only these two.
- Current ring lives in the **CPL field of the CS register**.
- Privileged instructions (`mov cr3`, `cli`/`sti`, `in`/`out`, `hlt`) → **General Protection Fault** if attempted at ring 3.
- Ring can only change through **controlled gates**: the `syscall`/`SVC` instruction forces entry at a **kernel-chosen fixed entry point**. User code can *trigger* a transition but cannot choose *where in the kernel* it lands.

⚠️ **MOCK GAP — privilege boundary (this one you PASSED, keep it sharp):**
A user program jumping to a kernel function's address does NOT gain kernel privilege. The jump alone changes nothing — **the ring level is the gate**. Ring 3 can't access kernel memory (PTEs lack the User bit → fault) and can't run privileged instructions. The **only** sanctioned path for user code to get kernel code running on its behalf is the **syscall** (controlled gate → fixed entry point).

---

## 3. The 3-Layer File Model (fd → open file table → inode)

⚠️ **MOCK GAP #1 — the scope labeling. This is your softest spot. Memorize this table:**

| Layer | What it holds | Scope | fork() behavior |
|---|---|---|---|
| **fd table** | array of pointers; fd number = index | **per-process** (in `files_struct`) | **COPIED** (new array, same pointers) |
| **open file table** (`struct file`) | file offset (`f_pos`), open flags, `f_op`, `private_data`, `*f_inode` | **KERNEL-GLOBAL** | **SHARED** (both fd tables point to the same entry) |
| **inode** | file identity, permissions, size, timestamps, disk block map | **KERNEL-GLOBAL** | shared |

**The contradiction to kill:** the middle layer (`struct file`) is **kernel-global, NOT per-process**. That is *precisely why* it can be shared after fork and why the shared file offset behavior exists. If it were per-process, nothing would be shareable. All three layers live in **kernel memory — user code cannot touch any of them**; user space only holds the integer fd.

**Consequences that follow from the table:**
- fork → child's fd 3 and parent's fd 3 point to the **same `struct file`** → **shared `f_pos`**. Parent reads 100 → child's read continues at 100.
- Independent `open()` of the same path → **separate `struct file`** (own offset) → **same inode**.
- `dup2(old, new)` → new fd points to old's `struct file`, atomically (close-then-relink in one op). Basis of shell redirection.
- Hard link (`ln`) → new directory entry → same inode number, `i_nlink++`. `rm` = `unlink()` (removes a link; data freed when `i_nlink==0` AND no open refs). Symlink (`ln -s`) → separate inode storing a path string.

**Kernel source chain you traced:** `task_struct → files_struct → fdtable → struct file __rcu **fd` (array of pointers). `**fd` = pointer to an array of `struct file *`. Held by pointer (not embedded) so it can grow at runtime via pointer-swap. `__rcu` = RCU-protected (lockless readers, atomic replacement) because fd lookup is extremely read-heavy — foundation for Week 6.

**open()→read() lifecycle (one-line each):** open = path-walk → inode; alloc `struct file` (offset 0, `f_op` from inode); find lowest-free fd via bitmap; `fd[n]=file`; return integer. read(5,…) = `current->files->fdt->fd[5]` (O(1) index, RCU read, bounds-checked → `-EBADF` if invalid); check `f_mode`; dispatch `f_op->read_iter` (polymorphism point — same path serves ext4/socket/your driver); read offset from `f_pos`, blocks from inode; page cache or disk (may block → context switch); `copy_to_user`; `f_pos += n`; return n.

**per-open state (`private_data`):** each `open()` → own `struct file` → own `private_data`. Allocate in `.open`, free in `.release`. Use `f_pos` for simple position; use `private_data` for anything `f_pos` can't express (per-caller config, partial-read buffers, event queues, hardware state). The real "don't use globals" lesson: global state for per-caller data corrupts across callers.

---

## 4. Virtual Memory — MMU, page tables, faults

**Why VM exists (3 reasons):** (1) **isolation/protection** — each process's page table has no PTE reaching another's memory; (2) **fragmentation** — contiguous virtual maps to scattered physical; (3) **overcommit** — demand paging lets total virtual exceed physical RAM.

**MMU + page tables:**
- **MMU** = hardware in the CPU; translates VA→PA on **every** access using page tables; caches results in the **TLB**.
- **Page table** = tree of page-sized arrays. `pgd_t *pgd` in `mm_struct` points to the root. x86_64: 4 levels (PGD→PUD→PMD→PTE), each table = one 4KB page = 512 entries × 8 bytes. The VA's bit-fields ARE the array indices (9 bits each = 512).
- **512 = 4096/8** — entries per table page, NOT per data page. One PTE ↔ one 4KB page. One PTE table covers 2MB.
- **PTE holds** physical page number + permission bits (R/W/X/User/Valid). Permission bits enforce kernel/user separation and read-only protection at the hardware level.

**VMA vs page table (two different systems, two customers):**
- **VMA** (`vm_area_struct`, in `mm_struct`'s maple tree `mm_mt`) = kernel's high-level "this VA range is valid, these perms, this backing." ~20-50 per process. Used by **software only** (page fault handler, mmap, `/proc/self/maps`).
- **Page table** = hardware-readable VA→PA map. Used by the **MMU**. The MMU never sees `mm_struct` or VMAs.
- The **only** software→hardware handoff: `switch_mm` writes `__pa(next->mm->pgd)` into CR3/TTBR0 at context switch. The VA does NOT select the PGD — the **currently-running process** does (its PGD was loaded into CR3 when it was scheduled in). Every VA thereafter walks that one table.

**How CR3 gets loaded (chain):** scheduler picks task → `context_switch()` → `switch_mm_irqs_off(prev->mm, next->mm)` → reads `next->mm->pgd` (a kernel VA) → `__pa()` → `write_cr3()` → `mov %rax,%cr3` (privileged, ring 0). Side effect: **TLB flush** (the hidden cost of switching processes; skipped for thread↔thread of same process → cheaper).

**Lazy page-table construction:** at process creation the kernel builds **VMAs + an almost-empty PGD** — NOT the full VA→PA map. The first access to any page **faults**, and the handler builds the PGD→PUD→PMD→PTE path on demand. `malloc`/`brk`/`mmap` only extend VMAs — physical pages + PTEs appear on first touch. (This is why `ps` shows VSZ ≫ RSS.) One fault handler unifies: demand loading, COW, stack growth, heap growth.

⚠️ **MOCK GAP #2 — MMU-vs-kernel fault roles. Make this reflex:**
On access to an invalid/write-protected PTE:
- **MMU (hardware) DETECTS** the bad access and raises a **page fault exception**. It only translates + checks permissions. It does NOT allocate, copy, or repair anything.
- **Kernel page fault handler (software) REPAIRS:** looks up the VMA (`find_vma`); if no VMA covers it → **SIGSEGV**; if valid → allocate page / swap in / resolve COW / update PTE → resume.
- One-liner: **"MMU detects and faults; kernel repairs."**

**`/proc/self/maps` reading:** each line = one VMA. Format: `start-end perms offset dev inode path`. Perms: `r-xp`=code (shared physical pages across processes), `rw-p`=writable/COW, `---p`=guard page, `rw-s`=shared mapping. `inode 0`/`00:00` = anonymous (heap, stack, internal buffers). `[vdso]` = kernel code mapped into user space for fast syscalls (also holds the signal trampoline). Shared-library code = ONE physical copy mapped into many address spaces; data pages = COW per process.

---

## 5. fork() + COW

**fork() = clone with nothing shared** (COW everything). Returns **twice**: child PID in parent, 0 in child.

**What fork does:** new `task_struct` (new PID); copy `mm_struct` (VMAs) + page-table structure with **all writable pages marked COW / read-only in both parent and child**; copy fd table (pointers → same `struct file`s, refcounts++); set child's return-register to 0 via its `pt_regs`; add child to run queue. **No physical memory copied.** A 2GB-heap process forks in microseconds.

⚠️ **MOCK GAP #3 — COW granularity. The number is ONE PAGE (4KB), not "4 bytes":**
When the child writes 4 bytes to a COW page:
1. MMU sees write to read-only page → **page fault** (MMU detects).
2. Kernel handler sees COW flag → allocates **one new 4KB page** → copies the **whole page** → updates that PTE to writable → resumes (kernel repairs).
3. Memory consumed = **one page (4KB)**, not 4 bytes. Remaining ~2GB stays shared. COW is **per-page** — copying all 2GB on first write would defeat the entire point.
Parent then reads its own (unchanged) copy of that page → sees the **old** value. Parent/child can't communicate via shared variables after fork (COW splits them) — need explicit IPC.

**exec() after fork:** destroys ALL VMAs/COW pages, loads new program's VMAs, fresh stack, jumps to new entry. **fd table SURVIVES exec** (unless `O_CLOEXEC`) — this is what makes shell redirection work. PID unchanged.

**Why fork()+exec() (separation of concerns):** fork sets up the child's environment (redirect fds via dup2, pipes, cwd, limits) while it's a separate process (changes don't touch parent); exec then loads the program. Gives the shell full control over the child's setup before the program starts.

---

## Rapid self-test (answer cold before the Jul 31 mixed interview)

1. Name the two register-save structures in a full A→B switch and what each captures.
2. Syscall: 2 things that change, 2 that don't.
3. fd/`struct file`/inode: which is per-process, which kernel-global? In fork, which is copied vs shared?
4. Where do all three file-layer structures physically live? Can user code touch them?
5. VA→PA: where does the MMU find the page table, who loaded it and when, what happens on an invalid entry — and **who does what** (hardware vs software)?
6. Child writes 4 bytes to a COW page after fork: exactly how much memory is consumed, and which component detects vs repairs?
7. Why can't user code call a kernel function by jumping to its address? What's the only sanctioned entry?

**If any of 3, 4, 5, 6 feel slow — that's the seam. Drill those.**
