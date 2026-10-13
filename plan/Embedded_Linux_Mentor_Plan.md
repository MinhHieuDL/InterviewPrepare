# Embedded Linux Mentorship — Tracking Document

**Goal:** Level up from intermediate embedded C/C++ engineer to job-ready middle/senior embedded software developer, focused on Embedded Linux (kernel → application) and CI/CD for embedded projects.

**Timeline:** ~11 weeks of material, 15+ hrs/week (actual pace slower; that's fine — retention over speed)
**Started:** 2026-06-21
**Reference material:** *The Design of the UNIX Operating System* (Bach) for internals theory, *Linux Device Drivers, 3rd Ed* (LDD3) for hands-on driver development (LDD3 targets kernel 2.6 — APIs are modernized during lessons, e.g. `class_create(name)` since 6.4)

**Companion docs:**
- `FLASH.md` — BBB flash + dev-environment runbook (flash, boot, SSH, kernel build, module load loop)
- `Week1_Review.md`, `Week2_Review.md`, `Week3_Review.md` — per-week study docs with ⚠️ gap boxes and cold self-tests

---

## How to use this document

- This is a **living record**. Update the diagnostic table as topics are revisited and retested.
- Status legend: ✅ Solid | ⚠️ Partial/needs reinforcement | ❌ Gap (not yet built) | 🔁 Reinforced (was a gap, now solid after review)
- Update the table in place — don't just append.

---

## Session-Start Protocol (for Claude)

1. **The user begins each study day with a message starting with "Start".** On that keyword: check today's date against the Review Schedule below, say how many days since the last session, and flag any mini-drill / mock that is due or overdue.
2. The user decides: run the review now, or defer (if deferred, agree a concrete date and record it).
3. After each review: mark it done in the schedule, log results in the Progress Log, update the diagnostic table (fumbled → ⚠️ with note; solid twice → ✅).
4. Pacing rule (user request): **one topic / one sub-mechanism per message**; clear the user's questions before moving on. Never batch several mechanisms into one long message.
5. Drill/mock rule: no teaching during the review; full debrief at the end.

---

## Diagnostic Table (baseline 2026-06-21, updated through R2 mock 2026-10-11)

| # | Area | Status | Notes |
|---|------|--------|-------|
| 1 | Scheduling triggers | ✅ Solid | |
| 2 | Context switch mechanism | ✅ Solid | pt_regs vs thread_struct/cpu_context_save, switch_to, current, CR3/TTBR0 + TLB flush. Graduated R1. |
| 3 | Syscall mechanism | ✅ Solid | ring 3→0 / EL0→EL1, stack switch, page tables unchanged. Graduated R1. |
| 4 | fork() / COW | ✅ **Graduated (R2 Oct 11)** | One 4 KB page copied, parent sees old value — answered cleanly. Refinements: hardware has **no COW bit** (kernel marks PTE read-only; "COW-ness" = VMA writable + PTE read-only, known by software); the new page is a full copy because it replaces the child's whole view of that page; child gets its own mm_struct/page tables pointing at shared pages. |
| 5 | Thread semantics | ✅ Solid | clone flags, shared mm → no CR3 reload. |
| 6 | Virtual memory — why | 🔁 Reinforced | |
| 7 | Buffer cache / write-back | ✅ Solid | |
| 8 | Races — scenario level | ✅ Solid | |
| 9 | Races — instruction level | 🔁 Reinforced (W2) | own race.c disassembly, lock addq, atomicity doesn't compose. |
| 10 | Interrupt vs syscall | ✅ Solid | |
| 11 | Kernel vs user space | ✅ Solid | Graduated R1. |
| 12 | Page faults — who does what | ✅ **Graduated (R2 Oct 11)** | "MMU detects + faults (hardware), kernel handler checks VMA + repairs (software)" stated correctly unprompted. Full depth still Week 7. |
| 13 | fd / struct file / inode | ⚠️ **Scope fixed, 2 errors left** | R2: scope now correct (fd table per-process; open-file entry + inode shared; all in kernel memory, syscall-only access). **Still wrong:** (a) **names swapped** — said "file_struct"; correct: `struct file` = open-file entry (f_pos, f_op, private_data), `struct files_struct` = per-process fd-table container (`task_struct->files`); (b) **shared f_pos consequence** — said forked writes overwrite; correct: shared `struct file` → shared f_pos advanced inside each write → child's BBB lands **after** AAA. Independent open() → separate f_pos both at 0 → **does** overwrite; fix = `O_APPEND`. |
| 14 | Disk block mapping | ⚠️ Partial | Low priority. |
| 15 | Mutex fast path (uncontended = 0 syscalls) | ✅ **Graduated (R2)** | W2 drill miss, now correct with 0/1/2 + CAS/exchange. Slip: said "kernel CAS" — the **thread** executes the atomic in user space. |
| 16 | Condvar while-not-if | ✅ **Graduated (R2)** | Stolen-condition reason correct. Add second reason: spurious wakeups. |
| 17 | Kernel return convention + init unwind | ⚠️ | R2: unwind order + IS_ERR + leak correct; next-insmod analysis correct after probe (new major, then class_create → -EEXIST). **First instinct was "return 0" on failure** (dangerous: kernel keeps half-built module). Rule: **return the negative errno received (`return ret;`)**; never 0 or -1 (in kernel, -1 = -EPERM, misleading). Classic mistake: `if (!cls)` instead of `IS_ERR(cls)`. |
| 18 | Host tools vs target code (HOSTCC vs CROSS_COMPILE) | ⚠️ | W3 drill ❌ → R2 core only ("ARM binary can't run on x86"). Missing: fixdep is a **host helper tool** the build **executes on the PC**, built by **HOSTCC**; target-built headers package ships an ARM one. |
| 19 | Why Kbuild, not bare gcc | ❌ **3rd miss (D4 drill, R2)** | R2 answer gave architecture + "library linking" — both wrong (they already use an ARM gcc; kernel has no library to link). The three real reasons: (1) exact kernel compiler flags/config defines → wrong flags = different struct layouts = silent corruption; (2) configured kernel include paths + generated headers (autoconf.h); (3) **modpost** → .mod.c with vermagic, symbol CRCs, MODULE_* info → without it insmod rejects. **Top drill item.** |
| 20 | Char device open path | ✅ Solid (R2) | major/minor → cdev → fops installed at open (chrdev_open swaps `struct file->f_op`), no re-lookup on read, missing .read → -1/EINVAL with no driver code run. |

### Overall pattern

**Strength:** conceptual reasoning is consistently good; self-corrects well under probing (R2 Q4).

**R2 shows real progress:** 5 items that had been open since July/July-drill graduated (#4, #12, #15, #16, plus #13 scope). The "where things live / who does what" seam is largely fixed.

**What remains is precise facts, not concepts:** `struct file` vs `files_struct` naming; shared-f_pos → append; kernel negative-errno return; HOSTCC vs CROSS_COMPILE; the three reasons Kbuild is required. Pattern: items learned as **background friction** (W3 build errors) consolidate poorly — drill them as explicit topics.

---

## The Plan

**Format per topic:** concept from first principles → user/kernel/hardware/DT role breakdown → code example → hands-on → interview Q&A + pitfalls.

### Phase 1 — User Space Foundations
**Week 1 — Process, Syscall & Memory** ✅ COMPLETE (2026-07-10) — `Week1_Review.md`
**Week 2 — Multithreading & IPC** ✅ COMPLETE (2026-07-17) — `Week2_Review.md`

### Phase 2 — Kernel Fundamentals & First Driver
**Week 3 — Kernel Modules & Build System** ✅ COMPLETE (2026-09-20) — `Week3_Review.md`, `FLASH.md`
- Module concept, the two Matches, Kbuild, linking deep-dive, insmod/modprobe/rmmod
- Hands-on: full BBB bring-up, kernel 6.18.52-bone54 built from bb-kernel source, hello.ko loaded on hardware

**Week 4 — Char Drivers** ← **IN PROGRESS**
- ✅ Topic 1 (Sep 28): device types (char/block/net), device file = numbered signpost, major (driver) / minor (device instance within driver; driver=class, device=object), /proc/devices, `open()` → inode → `def_chr_fops` → `chrdev_open()` → cdev lookup → **swaps `struct file->f_op` to driver fops** → later reads dispatch with no re-lookup. mknod trick (routing by numbers, not name).
- ✅ Topic 2 (Sep 29 – Oct 2): `alloc_chrdev_region` (dynamic, counts down from 254 → got 240), `cdev_init`/`cdev_add` (goes live; call last; `.owner = THIS_MODULE` → module refcount while open), `class_create(name)` (6.4+ API) / `device_create` → sysfs + uevent → devtmpfs/udev create `/dev` node; `goto` reverse unwind; `IS_ERR`/`PTR_ERR`; what each call creates (kernel objects, not files; /proc and /sys are generated views). sysfs purpose (/dev = data path, /sys = control/metadata path; uevent → node; discovery; attributes; topology). **Hands-on on BBB:** hellochar loaded (major 240 minor 0), /dev/hellochar auto-created 0600 root, `cat` → EINVAL with no driver print, clean unload. LED via sysfs (`trigger`, `brightness`; store() callback can return EINVAL; brightness 0 clears trigger).
  - Fix-ups still to do in hellochar: rename to `hellochar.c` / `obj-m += hellochar.o` (module name was still `hello`); move `\n` to end of printk format (stray `)` lines).
- ⬜ **Topic 3 (NEXT):** `file_operations` read/write, `copy_to_user`/`copy_from_user`, `f_pos`, `private_data`, return conventions
- ⬜ Topic 4: full driver end-to-end + user-space test program
- ⬜ Topic 5: locking inside the driver, robust error paths

### Phase 3 — Platform Drivers & Hardware Interaction
**Week 5 — Platform Driver Model & Device Tree** (bb-kernel source tree on host for DT exploration)
**Week 6 — Interrupts & Kernel Synchronization** (build on W1 RCU preview + W2 spinlock trade-off)

### Phase 4 — Memory, Debugging & Observability
**Week 7 — Memory Mapping & DMA** (full page-fault depth)
**Week 8 — Observability & Debugging** (sysfs show/store attributes — previewed with LEDs in W4)

### Phase 5 — Boot & Build Systems
**Week 9 — Boot Chain & Yocto** (S2 boot, MLO/U-Boot, kernel build already previewed in W3)

### Phase 6 — CI/CD for Embedded
**Week 10 — Git, Static Analysis, Unit Testing**
**Week 11 — Build Pipelines & Integration Testing**

---

## Open Items

- [x] Target environment: **BeagleBone Black**, fully working since 2026-09-20.
- [ ] RTOS topics excluded per user request — revisit later if needed.
- [x] Build tooling: CMake for user-space code; bb-kernel-style Makefile for modules.
- [ ] Cleanup (low priority): delete unused `toolchain/arm-gnu-toolchain-13.3.rel1-...` and the broken `kmod/headers/` extraction (keep the .deb).

---

## Review Schedule (Spaced Retrieval Practice)

**Interval rule:** topic completed → mini-drill +2-3 days → mock +1 week → mixed mock +3 weeks → final comprehensive at end.

| Due date | Type | Scope | Status |
|---|---|---|---|
| 2026-07-12 | Mini-drill | Week 1 | ✅ Done — 3/6 solid |
| 2026-07-17 | Mock R1 | Week 1 | ✅ Done — pass w/ reservations |
| 2026-07-20 | Mini-drill | Week 2 | ✅ Done — misses: mutex 0 syscalls, condvar while |
| 2026-09-27 | Mini-drill | Week 3 | ✅ Done — 2 solid (Matches, __init), D3 slip ("physical" addr), D1 errno ⚠️, D4 Kbuild ⚠️, D5 host-vs-target ❌ |
| 2026-10-01 → ran 10-11 | Mock R2 | Weeks 1-3 mixed (+W4 T1) | ✅ Done Oct 11 — **pass at mid level.** Graduated #4, #12, #15, #16, #20, #13-scope. Open: #13 naming + f_pos, #17, #18, #19. |
| **~2026-10-14** | **Targeted mini-drill** | **The 4 open items:** (1) `struct file` vs `files_struct` + shared-f_pos/O_APPEND, (2) kernel return convention + init unwind, (3) HOSTCC vs CROSS_COMPILE (fixdep), (4) three reasons Kbuild is required | ⬜ Pending |
| after W4 complete (+2-3 d) | Mini-drill | Week 4 (char drivers) | ⬜ Pending |
| ~3 weeks after R2 (~early Nov) | Mock R3 | Weeks 1-4 mixed | ⬜ Pending |
| End of plan | Final comprehensive | Everything | ⬜ Pending |

---

## Progress Log

- **2026-06-21**: Initial diagnostic, plan created. BeagleBone chosen.
- **2026-07-10**: Week 1 COMPLETE. Review Schedule system added.
- **2026-07-12**: W1 mini-drill (3/6 solid).
- **2026-07-17**: W1 Mock R1 (pass w/ reservations). `Week1_Review.md` created. Week 2 COMPLETE; `Week2_Review.md` created.
- **2026-07-20**: W2 mini-drill (misses: mutex syscall count, condvar while).
- **2026-09-20**: **Week 3 COMPLETE** — module concepts + full BBB bring-up, kernel built from bb-kernel source, hello.ko loaded on hardware. `FLASH.md` created.
- **2026-09-27**: W3 mini-drill (gaps: errno convention, Kbuild "why", host-vs-target tools, "physical" address slip). `Week3_Review.md` created. Agreed **"Start" keyword** session convention. **Week 4 started.**
- **2026-09-28**: W4 Topic 1 — char device model, major/minor, driver vs device, chrdev_open f_op swap.
- **2026-09-29 → 10-02**: W4 Topic 2 — registration (alloc_chrdev_region, cdev, class/device, uevent → /dev), goto unwind, sysfs purpose; hellochar loaded on BBB; LED driven via sysfs.
- **2026-10-11**: **Mock R2 done** (after 9-day gap; was set for Oct 1). Pass at mid level; 5 long-standing gaps graduated. Remaining: struct file vs files_struct naming + shared-f_pos→append, kernel negative-errno return, HOSTCC vs CROSS_COMPILE, three reasons for Kbuild (3rd miss).
- **2026-10-13**: Tracker updated. **Next:** targeted 4-item mini-drill (~Oct 14), then W4 Topic 3 (read/write, copy_to_user). Also fix hellochar module name + printk `\n`.
