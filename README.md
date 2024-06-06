# dj64dev development suite

## what is that?
dj64dev is a development suite that allows to cross-build 64bit programs
for DOS. It consists of 2 parts: dj64 tool-chain and djdev64 suite.

### dj64 tool-chain
dj64 is a djgpp-compatible tool-chain that compiles the djgpp-buildable
sources for DOS. But unlike djgpp that produces 32bit code, dj64
produces 64bit code.<br/>
The resulting programs run on the emulated DOS environment, with eg
[dosemu2](https://github.com/dosemu2/dosemu2) emulator. In theory the 64bit
DOS extender can be written to run such programs under the bare-metal
DOS, but the future of DOS is probably in the emulated environments anyway.

### djdev64 suite
djdev64 suite is a set of libraries and headers that are needed to
implement the "DJ64" and "DJ64STUB" DPMI extensions on a DPMI host.<br/>
"DJ64" is an extension that allows the dj64-built programs to access
the 64bit world.<br/>
"DJ64STUB" is an optional DPMI extension that implements a loader for
dj64-built programs. If "DJ64STUB" extension is missing, you need to have
the full loader inside the program's stub.<br/>
[djstub](https://github.com/stsp/djstub/) project provides both loader-less
and loader-enabled stubs, but the default is the loader-less ministub that
relies on a "DJ64STUB" loader inside DPMI host.

"DJ64" extension requires 2 things from DPMI host:
- put the 64bit djdev64 runtime into its address space and forward the
  calls from the DOS programs to that runtime
- make the 32bit calls on 64bit runtime's requests.

While the second task is rather simple, the first one is not.
If you have an asm-written DPMI server without an ability to talk to
C-written code, then you likely can't have dj64 support in it, as writing
the "DJ64" DPMI extension by hands, without using djdev64-provided
runtime, is too difficult or impossible.

In addition to that, dj64-built programs rely on a few DPMI-1.0 functions.
Namely, shared memory functions
[0xd00](https://www.delorie.com/djgpp/doc/dpmi/api/310d00.html),
[0xd01](https://www.delorie.com/djgpp/doc/dpmi/api/310d01.html)
and optionally also "Free Physical Address Mapping" function
[0x801](https://www.delorie.com/djgpp/doc/dpmi/api/310801.html)
which is used to unmap shared memory regions without actually destroying
them. DPMI host is not required to implement such a specific 0x801
functionality, but the shared memory support is mandatory.

## building and installing
First, you need to install [thunk_gen](https://github.com/stsp/thunk_gen/).
Pre-built packages are available
[for ubuntu](https://code.launchpad.net/~stsp-0/+archive/ubuntu/thunk-gen)
and
[for fedora](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2/).<br/>
Then run `make`.<br/>
For installing run `sudo make install`.<br/>
Like gcc should be accompanied with binutils in order to produce executables,
dj64 need to be accompanied with
[djstub](https://github.com/stsp/djstub/)
package for the same purpose. That package installs `djstubify`, `djstrip`
and `djlink` binaries that are needed for the final building steps.

## installing from pre-built packages
For the ubuntu package please visit
[dj64 ppa](https://code.launchpad.net/~stsp-0/+archive/ubuntu/dj64).
Fedora packages are
[here](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2).

## running
The simplest way to get dj64-built programs running is to use
[dosemu2](https://github.com/dosemu2/dosemu2).<br/>
Get the pre-built dosemu2 packages from
[ubuntu ppa](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa)
or from
[copr repo](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2)
or build it from
[sources](https://github.com/dosemu2/dosemu2).
dosemu2 uses the dj64-built command.com called
[comcom64](https://github.com/dosemu2/comcom64/).
You can type `ver` to make sure its the right one, in which case you
are already observing the first dj64-built program in the run. :)

## inspecting
You may want to analyze the structure of the dj64-built files to get
the more detailed view of its architecture. You can use `djstubify -i`
for that task:
```
$ djstubify -i comcom64.exe
dj64 file format
Overlay 0 (i386/ELF DOS payload) at 23368, size 30548
Overlay 1 (x86_64/ELF host payload) at 53916, size 87048
Overlay 2 (x86_64/ELF host debug info) at 140964, size 174936
Overlay name: comcom64.exe
Stub version: 4
Stub flags: 0x0b07
```
As can be seen, the executable consists of 3 overlays. If you use
`djstrip` on it, then only 2 remain. Overlay name is needed for
debugger support, for which we use the GNU debuglink technique.<br/>
Stub flags are used to create the shared memory regions with the
[0xd00](https://www.delorie.com/djgpp/doc/dpmi/api/310d00.html)
DPMI-1.0 function. They are not documented in a DPMI spec, so their
support by the DPMI host for dj64 is actually optional.

We can compare that structure with the regular djgpp-built executable:
```
$ djstubify -i comcom32.exe
exe/djgpp file format
COFF payload at 2048
```
Nothing interesting here, except that we see djgpp uses COFF format
instead of ELF. But what if we re-stub the old executable?
```
$ djstubify comcom32.exe
$ djstubify -i comcom32.exe
dj64 file format
Overlay 0 (i386/COFF DOS payload) at 23368, size 256000
```
Now this executable is identified as having the dj64 file format, but
of course it still has just 1 COFF overlay. Sorry but the conversion
from COFF to ELF is not happening. :) But our loaders support both
COFF and ELF formats, so dj64/COFF combination is also functional,
albeit never produced by the dj64 tool-chain itself.

## building your own program
Well, this is the most tricky part. First, a few preparations should be
made to the source code to make it more portable:

- Inline asm should be moved to the separate assembler files and called
  as a functions.
- Non-portable `movedata()` function should be replaced with the
  [fmemcpy*()](https://github.com/stsp/dj64dev/blob/master/include/sys/fmemcpy.h)
  set of functions that are provided by dj64. Their use is very similar to
  that of `movedata()`, except that pointers are used instead of selectors.
- Use macros like DATA_PTR() and PTR_DATA() to convert between the DOS
  offsets and 64bit pointers. Plain type-casts should now be avoided for
  that purpose.
- You need to slightly re-arrange the registration of realmode callbacks:
```
#ifdef DJ64
static unsigned int mouse_regs;
#else
static __dpmi_regs *mouse_regs;
#endif
...
#ifdef DJ64
    mouse_regs = malloc32(sizeof(__dpmi_regs));
#else
    mouse_regs = (__dpmi_regs *) malloc(sizeof(__dpmi_regs));
#endif
    __dpmi_allocate_real_mode_callback(my_mouse_handler, mouse_regs, &newm);
...
    __dpmi_free_real_mode_callback(&newm);
#ifdef DJ64
    free32(mouse_regs);
#else
    free(mouse_regs);
#endif
```
  In this example we see that the second argument of
  `__dpmi_allocate_real_mode_callback()` was changed from the pointer to
  `unsigned int`. The memory is allocated with `malloc32()` call and freed
  with `free32()` call. This requires a few ifdefs if you want that code to
  be also buildable with djgpp.
- The file named
  [glob_asm.h](https://github.com/dosemu2/comcom64/blob/master/src/glob_asm.h)
  should be created, which lists all the global asm symbols.
- C functions that are called from asm, as well as the asm functions that
  are called from C, should be put to the separate header file, for example
  [asm.h](https://github.com/dosemu2/comcom64/blob/master/src/asm.h).
  In that file you need to define the empty macros with names `ASMCFUNC`
  and `ASMFUNC`, and mark the needed functions with them. `ASMCFUNC` denotes
  the C function called from asm, and `ASMFUNC` denotes the asm function
  called from C.

Next, add this to your makefile, verbatim:
```
TGMK = $(shell pkg-config --variable=makeinc thunk_gen)
ifeq ($(TGMK),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
endif
TFLAGS = -a 4 -p 4
PDHDR = $(SRC)/asm.h
include $(TGMK)
```
to involve thunk_gen into a build process. `TFLAGS` variable sets the
alignment of arguments on stack, and a pointer size. For 32bit
djgpp-compatible sources, both should be set to 4, as in the above snip.
`PDHDR` specifies the
[header file](https://github.com/dosemu2/comcom64/blob/master/src/asm.h)
to generate thunks from.

Next comes the building stage with `dj64-gcc`. It accepts similar options
to gcc. Use `-c` to produce the object files. You should link the object
files with host `gcc` this way:
```
LDFLAGS = $(shell pkg-config --libs dj64) \
  -Wl,-rpath=/usr/local/i386-pc-dj64/lib64 \
  -Wl,-rpath=/usr/i386-pc-dj64/lib64
$(LIB): $(OBJS)
	gcc $(LDFLAGS) $^ -o $@
```
which gives you a shared library, because `$(shell pkg-config --libs dj64)`
adds the shared library flags.

Please note that you need to add a certain thunk files to your project, like
[thunks_a.c](https://github.com/dosemu2/comcom64/blob/master/src/thunks_a.c)
and
[thunks_c.c](https://github.com/dosemu2/comcom64/blob/master/src/thunks_c.c)
. As you can see, you don't need to put too many things there, as these
files include the auto-generated stuff. `thunks_a.c` is needed if you
refrence global asm symbols, and `thunks_c.c` is needed if you call C
functions from asm. If you call asm from C, then you also need `thunks_p.c`,
which is currently not documented.

Please note that the asm files are excluded from the C build. They are
built separately, together with
[plt.S](https://github.com/dosemu2/comcom64/blob/master/src/plt.S),
from which `plt.asm` is produced. Asm files are compiled with the i686 GNU
assembler, or with x86_64 one with `--32` flag. The resulting binaries
are linked with the GNU linker for i686 or x86_64 with these flags:
`-melf_i386 -static`.

Next comes the linking stage with `djlink`.
Lets consider this command line:
```
djlink -d dosemu_comcom64.exe.dbg libcomcom64.so -n comcom64.exe \
-f 0x0b07 -o comcom64.exe comcom64.elf
```
`-d` option sets the debuglink name. It always has the form of
`dosemu-<exe_file>.dbg` if you want to debug your program under dosemu2.<br/>
Next arg is a shared library that we linked with host `gcc`.<br/>
`-n` specifies the exe file name. It should match the `<exe_file>`
part passed to `-d` if you want to be able to use debugger.<br/>
`-f` specifies the so called stub flags word. It is suggested to
leave that to 0x0b07. Or you can omit `-f`, in which case the flags
word is zero, which should also work properly in most cases.<br/>
`-o` specifies the output file.<br/>
Last arg is an ELF file that we linked from an asm files.<br/>

Please note that you can't freely rearrange the `djlink` arguments.
They should be provided in exactly that order, or omitted.
For example if you don't need to use debugger, then you can just do:
```
$ strip libcomcom64.so
$ djlink libcomcom64.so -f 0x0b07 -o comcom64.exe comcom64.elf
```
to get an executable without debug info. You can as well do
`djstrip <exe_file>` to remove the debug info after linking.
Note that even though some `djlink` args were omitted in a non-debug
case, the order of the present ones didn't change.

Once you managed to build the code, you get an executable that you can
run under dosemu2.

## what's unimplemented
- direct calls from C to assembler code (calls from asm to C supported)
- some crt0 overrides (only `_crt0_startup_flags` override is supported)

This functionality is unsupported not because its difficult to implement,
but rather because I am not using it myself. Once it is needed, it can
be trivially added.

## debugging
Debugging with host gdb is supported. The djstub package provides a
`djstrip` binary to strip the debug info from an executable.<br/>
You need to attach gdb to the running instance of dosemu2, or just
run `dosemu -gdb`. Once the dj64-built program is loaded, gdb will
be able to access its symbols.

## so x86-only?
Of course not! This tool-chain is cross-platform. But the resulting
binaries are unfortunately not. If you want to run your program on
x86_64 and aarch64, you need to produce 2 separate executables.
aarch64-built executable will work on aarch64-built dosemu2.

## why would I need that?
Well, maybe you don't. :) If you don't have any djgpp-built project of
yours or you don't want to move it to 64bits, then you don't need to care
about dj64 project. It was written for dosemu2, and while I'd be happy
if someone uses it on its own, this wasn't an initial intention.<br/>
Also if your djgpp-buildable project is well-written and uses some
portable libraries like allegro, then most likely you already have the
native 64bit ports for modern platforms, rather than for DOS. In that
case you also don't need dj64. But maybe you are interested in a host-gdb
debugging and aarch64 support?<br/>
Summing it up, dj64 is a niche project that may not be useful outside
of dosemu2. But I'd like to be wrong on that. :)

## license
dj64 code is derived from djgpp libc sources, so most files are covered
by GPLv2+, see
[copying.dj](https://github.com/stsp/dj64dev/blob/master/copying.dj)
for details. dj64-specific files are covered by GPLv3+, see
[LICENSE](https://github.com/stsp/dj64dev/blob/master/LICENSE).
