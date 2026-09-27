# Week 3 Review — Kernel Modules & Build System

**How to use this:** Mechanism-first, same as Weeks 1-2. Built from the lessons, the questions *you* asked (linking, toolchains, sysroot), and the **Sep 27 mini-drill gaps** (the ⚠️ boxes). Week 3 was taught in fragments between a lot of setup friction, so read the ⚠️ boxes first: they mark what didn't consolidate.

**The through-line of the week:** a module is code built on one machine to run *inside* the kernel of another. Almost every problem traces to one question: **"built for which machine, matching which kernel?"**

**Environment procedure** (flash, boot, SSH, kernel build, load loop) lives in `FLASH.md`. This doc covers the *concepts*.

---

## 1. What a Kernel Module Is

**Definition:** compiled code loaded into the **running kernel** at runtime and unloaded without a reboot. It runs at **ring 0 / EL1**, shares the kernel's address space, and calls kernel functions directly.

- **No isolation:** a bug can panic the whole board.
- **Why modules exist:**
  - load drivers only for hardware that's actually present
  - update a driver without rebooting (essential during development)
  - keep the core kernel image small

**A module is NOT a program:**

| User program | Kernel module |
|---|---|
| has `main()`: runs, then exits | `init` runs once at load, `exit` runs once at unload, no `main` |
| its own process | becomes part of the kernel; runs on behalf of whoever calls it |
| crash → only that process dies | crash → the kernel can panic |
| libc (`printf`, `malloc`) | kernel API (`printk`, `kmalloc`); **no libc exists** |

**Mental model:** a module **registers capabilities, then waits**. `init` tells the kernel "I exist, here's what I can do". After that, module code runs only when invoked (a user opens its device, an interrupt fires). `exit` unregisters everything so it can be removed safely.

### The minimal module

```c
#include <linux/module.h>   // MODULE_*, module_init/exit
#include <linux/kernel.h>   // printk, KERN_* levels
#include <linux/init.h>     // __init, __exit

static int __init hello_init(void)
{
    printk(KERN_INFO "hello: module loaded\n");
    return 0;                        // 0 = success
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "hello: module unloaded\n");
}

module_init(hello_init);             // registers THE init function
module_exit(hello_exit);             // registers THE exit function
MODULE_LICENSE("GPL");               // required
```

- **`#include <linux/...>`**: kernel headers, a separate world from user space. There is no `stdio.h` here.
- **`printk`** writes to the kernel log ring buffer; read it with `dmesg`. `KERN_INFO` is a log-level prefix string glued onto the message (note: **no comma** after it).
- **`module_init` / `module_exit`** make your functions *the* entry and exit points. The function names themselves are arbitrary.
- **`MODULE_LICENSE`** is required. A non-GPL-compatible license **taints** the kernel and blocks access to GPL-only exported symbols. `"Dual BSD/GPL"` (what your old `hello.c` uses) is also GPL-compatible.
- **Module parameters:** your `modinfo` showed `parm: iHowMany:int` and `parm: whom:charp`. Those are declared with `module_param()` and set at load time: `insmod hello.ko whom="world" iHowMany=3`.

> ⚠️ **DRILL GAP (D1) — init return value + errno.**
> - **Kernel convention everywhere: `0` = success, a negative errno = failure.** `-ENOMEM` means "out of memory". Others you'll use constantly: `-EINVAL` (bad argument), `-EBUSY` (resource in use), `-EFAULT` (bad user pointer), `-ENODEV` (no device). `insmod` prints the readable form, e.g. `Cannot allocate memory`.
> - **`main()` vs `init` return value.**
>   - When `main()` returns non-zero, the program has *already run to completion*. The value only *reports* a status to the parent (`echo $?`); nothing is undone.
>   - When `init()` returns non-zero, the kernel **undoes the load**: it frees the module's memory and the module never exists.
>   - In short: `main`'s return *reports*; `init`'s return *decides*.
> - **Consequence (Week 4):** if `init` registered something before failing, *you* must undo it before returning the error, or it leaks. This is the classic `goto`-unwind pattern.

### `__init` / `__exit`

- `__init` puts the function into the `.init.text` section. The kernel frees that section after init completes, because init code never runs again. At boot, `dmesg` shows "Freeing unused kernel memory" for built-in code doing the same thing.
- `__exit` code is dropped entirely when the module is built *into* the kernel, since a built-in module can never be unloaded.
- **Danger (interview point):** never call an `__init` function (or touch `__initdata`) from code that runs later, e.g. from `read()`. Its memory is gone, so the kernel oopses. `modpost` warns "section mismatch".

---

## 2. The Two "Match" Requirements

These two requirements are **independent**: different things must match, and the pieces come from different sources.

| | Must match | Controlled by | Where it comes from |
|---|---|---|---|
| **Match #1: toolchain** | the **CPU architecture + ABI** (BBB: ARMv7 32-bit) | `ARCH=arm` + `CROSS_COMPILE` (`CC` in your Makefile) | any vendor's ARM toolchain (ARM, Linaro, bb-kernel's `dl/`) |
| **Match #2: kernel tree** | the **exact running kernel** (`6.18.52-bone54`) | `KDIR` / `KERNEL_DIR` | must come from the board's kernel (bb-kernel tag) |

- **Match #1 fails** → the code can't execute on the CPU (wrong architecture).
- **Match #2 fails** → `insmod` rejects the module (vermagic mismatch). Worse, struct layouts could differ and silently corrupt memory.
- **vermagic is more than a version number.** `6.18.52-bone54 SMP preempt mod_unload ARMv7 thumb2 p2v8` also encodes config options. That's why you build against the *exact configured tree*, not just "some 6.18 source".

### Your toolchain confusion, resolved

- **"Why not download the toolchain from the BeagleBone site?"** A compiler has no idea what a BeagleBone is. It only needs to emit the right architecture and ABI. The *board-specific* piece is the kernel tree (Match #2), not the compiler.
- **Linaro vs ARM:** these are two vendors of the same thing. Linaro published ARM Linux toolchains historically (your `gcc-linaro-6.5.0`); ARM took over around 2022 (`arm-gnu-toolchain-*`). The target triples are equivalent. The old Linaro one was rejected only for **age** (GCC 6.5 is too old for a 6.18 kernel), not vendor.
- **Reading a target triple:**
  - `arm-none-linux-gnueabihf`: `arm` = 32-bit ARM, `linux` = builds Linux code, `hf` = hard-float (uses the FPU). ✓ Right for BBB user space.
  - `arm-none-eabi`: **bare-metal** (microcontrollers, no Linux). ✗ The classic wrong pick.
  - `aarch64-*`: **64-bit** ARM. ✗ BBB's Cortex-A8 is 32-bit.
- **The final setup uses bb-kernel's own pinned toolchain.** `version.sh` pins it with `toolchain="gcc_15_arm"`, and `build_kernel.sh` auto-downloads it into `dl/`. Kernel and modules are built with the *same* compiler, so they're consistent by construction.
- **Compiler mismatch warning** ("the compiler differs from the one used to build the kernel"): harmless for a simple module, since it's a warning, not an error. Building both kernel and module with one toolchain removes it.

---

## 3. Linking — why failures happen at load time

**Symbol** = a name (a function or global variable) paired with its **address**. A **symbol table** is a `name → address` lookup table. The running kernel exports its table, and you can read it with `cat /proc/kallsyms | grep printk`.

**Unresolved reference:** `printk`'s code isn't inside your module. At compile time the compiler leaves a **labeled blank** in the call instruction ("fill in printk's address later"). The `.ko` carries a list of these undefined symbols.

**Patching:** `insmod` looks up each name in the running kernel's symbol table and **overwrites the blank with the real address**. The module is now linked into the live kernel. `insmod` is effectively a **dynamic linker operating on the running kernel**.

### Two-stage linking (the question you pushed on)

| | Existence check ("does it exist?") | Address patching |
|---|---|---|
| Statically linked user program | build time | build time |
| Dynamically linked user program (libc.so) | **build time**: the static linker `ld` reads `libc.so`'s exported symbol table (`nm -D`) | **startup**: `ld.so` patches, because ASLR means the address is only known at runtime |
| Kernel module (`.ko`) | **load time** (`insmod`) | **load time** (`insmod`) |

Each half of the work happens **at the earliest moment the needed information exists**. A module's provider (the running kernel on the board) isn't visible on the build machine at all, so both halves wait for `insmod`.

- **Headers vs library.** A header (`stdio.h`, `linux/printk.h`) is a **declaration**: the compiler uses it for type checking. The library or kernel symbol table holds the **definition**: the linker uses it for the existence check.
  - Missing header → compiler error.
  - Missing definition → linker error (for a module: "Unknown symbol" at `insmod`).
  - That gap is why a module can compile cleanly and still fail to load.

> ⚠️ **DRILL GAP (D3) — virtual, not physical.** `insmod` patches in the symbol's **kernel virtual address** (the addresses in `/proc/kallsyms` are virtual). Nothing in linking deals with physical addresses. This is the same seam as W1 #12/#13: **which address space things live in**.

### Cross-compiling user programs: the sysroot

- Cross-compiled user programs need the **target's** libc (ARM), not the host's. It lives in a **sysroot**: a directory on the build machine that mirrors the target's `/usr/lib` and `/usr/include`.
- **Version rule:** the board's runtime libc must be the **same or newer** than the one you built against.
  - Build against 2.31, run on 2.35 → OK.
  - Build against 2.35, run on 2.31 → `GLIBC_2.34 not found`.
- Escape hatches:
  - static linking (`-static`)
  - Yocto SDK: builds the image and a matching sysroot from the same source, so they can't drift
- **Parallel to modules:** the kernel tree *is* the module's "sysroot". It must match the target kernel the same way.

---

## 4. The Kernel Build System (Kbuild)

You don't build a module with a plain `gcc` call. Your Makefile **hands off** to the kernel's own build system:

```makefile
obj-m += hello.o                 # declare: build hello as a MODULE

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CC) modules
```

- **`obj-m += hello.o`**: Kbuild is *declarative*. You say *what kind* of object it is, not how to compile it.
  - `obj-m` → loadable module (`.ko`)
  - `obj-y` → built into the kernel image
  - `obj-$(CONFIG_FOO) += foo.o` → whatever `menuconfig` set (`y`, `m`, or nothing). The whole kernel's drivers are built conditionally this way.

> ⚠️ **DRILL GAP (D4) — what the handoff really does.**
> - **`-C $(KDIR)`** means **change into the kernel tree and run *the kernel's own Makefile* (Kbuild)**. It isn't just "point at the source".
> - **`M=$(PWD)`** means "the external module is *here*; build it and put the results back here".
> - **Why `gcc hello.c -o hello.ko` can't work** — Kbuild supplies three things a bare gcc doesn't have:
>   1. the **exact compiler flags** the kernel was built with (architecture, float ABI, config-dependent defines). Wrong flags → different struct layouts → silent corruption.
>   2. the **kernel include paths** (`linux/...`, no libc).
>   3. the **`modpost`** step, which generates module metadata: vermagic, symbol versions, `MODULE_*` info (what `modinfo` shows).

### The two-pass Makefile pattern (your old `hello_world` Makefile)

```makefile
ifneq (${KERNELRELEASE},)
    obj-m += hello.o                       # pass 2: inside Kbuild
else
    KERNEL_DIR := $(PWD)/../../bb-kernel/KERNEL
    CC := $(PWD)/../../bb-kernel/dl/gcc-15.3.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
default:
	${MAKE} ARCH=arm CROSS_COMPILE=${CC} -C ${KERNEL_DIR} M=${PWD} modules
endif
```

- **Pass 1:** you run `make`. `KERNELRELEASE` is empty, so the `else` branch runs and re-invokes make through Kbuild (`-C`).
- **Pass 2:** Kbuild reads your Makefile again with `KERNELRELEASE` defined, so it only sees `obj-m += hello.o`.
- It's the same handoff as above, written as one self-recursive file.

- **`CROSS_COMPILE` is a prefix**, including the path. Kbuild appends the tool name: `${CC}gcc`, `${CC}ld`, and so on. With an absolute path, nothing needs to be on `PATH`.
- **Recipe lines need real TABs.** Spaces give `missing separator`, and editors sometimes silently convert tabs to spaces.

---

## 5. Host tools vs target code — the `.deb` failure

> ⚠️ **DRILL GAP (D5) — this whole section.**

A cross-build compiles code for **two different machines**:

| What | Runs on | Built by | Examples |
|---|---|---|---|
| **Target code** | the board (ARM) | `CROSS_COMPILE` toolchain | `hello.o`, `hello.ko`, the kernel |
| **Host helper tools** | your build PC (x86), *during* the build | `HOSTCC` (native gcc) | `fixdep`, `modpost`, `kconfig` |

The helper tools are programs the build **executes on your PC**, so they must be x86, even though the output they help produce is ARM.

**What went wrong with the `linux-headers` `.deb`:**
1. The package was built **on the BeagleBone, for the BeagleBone**, so its `scripts/basic/fixdep` was an **ARM binary**.
2. When the x86 PC tried to run it, the kernel refused → **"Exec format error"** (the CPU can't execute that machine code).
3. The package was also a **stripped tree**: it had no `scripts/kconfig/`, so it couldn't regenerate those tools. `modules_prepare` and `make scripts/basic` both triggered `syncconfig`, failed, and **deleted the pre-baked `autoconf.h` / `auto.conf`** on the way out. That's why we had to re-extract twice.
4. Such packages are designed for **building on the target**. They are not suitable for cross-builds from a different architecture.

**Why the `bb-kernel` source build fixed it:** a full source tree builds `fixdep`, `modpost`, and `kconfig` with `HOSTCC` **on your PC, for your PC**. Host tools come out x86, kernel code comes out ARM, and the mismatch becomes impossible. The build also produces a fully prepared tree: config processed, generated headers present.

**One line to remember:** *`CROSS_COMPILE` builds what runs on the target; `HOSTCC` builds what runs on the build machine. A header package built on the target has the host half wrong.*

**Lessons from the recovery:**
- **Don't run `modules_prepare` on a packaged headers tree.** It's already prepared; regenerating destroys it. `modules_prepare` is for raw source trees.
- **Keep your fallback until the replacement is proven.** We didn't delete the old toolchain or `.deb` until `hello.ko` actually loaded.

---

## 6. Loading, Unloading, Inspecting

**`insmod hello.ko`** is the raw loader. It takes an exact file path and:
1. reads the `.ko` into memory
2. **checks vermagic** against the running kernel (Match #2 enforced)
3. **resolves symbols** against the kernel symbol table (unresolved → `Unknown symbol`)
4. allocates kernel memory and links the module in
5. **calls `init()`**: `0` → module is live; negative errno → unload and report the error

`insmod` does **not** resolve dependencies.

**`modprobe hello`** loads by **name** from `/lib/modules/$(uname -r)/`. It **loads dependencies first** using `modules.dep`, so the module must be installed there (`make modules_install`).
- Development: use `insmod` for the file you just built.
- Real drivers with dependencies: use `modprobe`.

**`rmmod hello`** calls `exit()`, then removes the module. It **refuses if the refcount is not 0** (something is using the module, e.g. its device is open). Same refcount discipline as `struct file`.

**Inspection:**

| Command | Shows |
|---|---|
| `lsmod` | loaded modules; columns = name, size in bytes, use count (refcount), dependents |
| `dmesg \| tail` | your `printk` output |
| `modinfo hello.ko` | license, vermagic, dependencies, parameters |
| `cat /proc/modules` | the raw data `lsmod` formats |

**Verify before loading** (you did this; keep the habit):
```bash
modinfo hello.ko | grep vermagic   # must equal the board's uname -r (Match #2)
file hello.ko                      # must say "ELF 32-bit ... ARM" (Match #1)
```

**Your verified run (2026-09-20):**
1. `insmod` → `hello: module loaded on BBB`
2. `lsmod` → `hello 12288 0` (size, refcount 0)
3. `rmmod` → `hello: module unloaded`
4. `lsmod` again → empty

That is the full lifecycle on real hardware.

---

## 7. Environment concepts worth keeping (details in `FLASH.md`)

- **S2 boot override:** the AM335x ROM checks **eMMC before SD**. Holding S2 at power-on pulls a boot-config pin so it checks SD first.
  - Root filesystem on `mmcblk0` = SD card; `mmcblk1` = eMMC.
  - This was a live preview of the Week 9 boot chain: ROM → MLO → U-Boot → kernel → rootfs.
- **`sysconf.txt`:** newer images ship with **no default password** (the old `debian`/`temppwd` was a security hole). A first-boot script applies the file, reboots once, then wipes the plaintext password.
  - Lines starting with `#` are inactive examples. `#root_password=FooBar` means **no** root password, which is the safer choice.
- **`dd` safety:** identify the card by **before/after `lsblk` diff + `RM=1` + dmesg**. Write to the whole disk (`/dev/sdc`, not `sdc1`). `conv=fsync` makes sure the data is on the card before `dd` exits.
- **Ethernet DHCP quirk:** "link up (`carrier=1`, `LOWER_UP`) but no IPv4" plus `DHCPDISCOVER` with no OFFER means DHCP failed, not the cable. Fix: plug the cable in *after* boot.
- **SSH:**
  - `~/.ssh/config` alias (`Host bbb` → `bbb.local`, `User debian`) survives DHCP IP changes via mDNS.
  - **Key auth is asymmetric crypto:** the private key never leaves the host; the board holds only the public key. Same trust model as Secure Boot (Week 9).
  - `ssh bbb 'sudo …'` fails because sudo needs a terminal to prompt for the password. Use `ssh -t` or log in interactively.
  - `apt-get download` fetches a `.deb` without sudo.

---

## Rapid self-test (answer cold before the R2 mock)

1. How is a module different from a program? What does `init`'s return value decide that `main`'s doesn't?
2. What is `-ENOMEM`? What's the kernel's error-return convention? What must `init` do before returning an error?
3. Name the two Matches: what matches what, and which variable controls each.
4. A module compiles but `insmod` says `Unknown symbol`. Why wasn't this caught at build time? What address (virtual or physical) gets patched in?
5. In `make -C $(KDIR) M=$(PWD) modules`, what do `-C` and `M=` do? Name three things Kbuild provides that plain `gcc` can't.
6. Why did the `linux-headers` `.deb` fail with `fixdep: Exec format error`? Why did a source build fix it? (Say "HOSTCC" in your answer.)
7. What does `__init` do? Why is it safe, and what bug happens if you call an `__init` function later?
8. `insmod` vs `modprobe`: when do you *need* `modprobe`? Why can `rmmod` refuse?
9. Before copying a `.ko` to the board, which two commands verify it will load and run, and what should each show?
10. Why can a toolchain come from any vendor, while the kernel tree must come from the board's exact release?

**If 2, 4, 5, or 6 feel slow, those are your drill gaps. Re-read the ⚠️ boxes.**
