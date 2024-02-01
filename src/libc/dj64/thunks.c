#include <assert.h>
#include <djdev64/dj64init.h>
#include <libc/djthunks.h>
#include <dpmi.h>
#include <stddef.h>
#include "plt.h"
#include "thunks_c.h"
#include "thunks_p.h"
#include "thunks_a.h"

static __dpmi_regs s_regs;
static int recur_cnt;

struct udisp {
    dj64dispatch_t *disp;
    const struct elf_ops *eops;
};
#define MAX_HANDLES 10
static struct udisp udisps[MAX_HANDLES];
static const struct dj64_api *dj64api;

void djloudprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    dj64api->print(DJ64_PRINT_LOG, format, vl);
    va_end(vl);
    va_start(vl, format);
    dj64api->print(DJ64_PRINT_TERMINAL, format, vl);
    va_end(vl);
}

uint8_t *djaddr2ptr(uint32_t addr)
{
    return dj64api->addr2ptr(addr);
}

uint32_t djptr2addr(uint8_t *ptr)
{
    return dj64api->ptr2addr(ptr);
}

static int _dj64_call(int libid, int fn, __dpmi_regs *regs, uint8_t *sp,
    dj64dispatch_t *disp)
{
    int len;
    UDWORD res;
    enum DispStat stat;

//    assert(fdpp);

    s_regs = *regs;
    res = (libid ? disp : dj64_thunk_call)(fn, sp, &stat, &len);
    *regs = s_regs;
    if (stat == DISP_NORET)
        return (res == ASM_NORET ? DJ64_RET_NORET : DJ64_RET_ABORT);
    switch (len) {
    case 0:
        break;
    case 1:
        regs->h.al = res;
        break;
    case 2:
        regs->x.ax = res;
        break;
    case 4:
        regs->d.eax = res;
        break;
    default:
//        _fail();
        break;
    }
    return DJ64_RET_OK;
}

static int dj64_call(int handle, int libid, int fn, uint8_t *sp)
{
    int ret;
    struct udisp *u;
    __dpmi_regs *regs = (__dpmi_regs *)sp;
    sp += sizeof(*regs);
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    recur_cnt++;
    ret = _dj64_call(libid, fn, regs, sp, u->disp);
    recur_cnt--;
    return ret;
}

static int dj64_ctrl(int handle, int libid, int fn, uint8_t *sp)
{
//    struct udisp *u;
//    struct dj64_symtab *st;
    __dpmi_regs *regs = (__dpmi_regs *)sp;
    assert(handle < MAX_HANDLES);
//    u = &udisps[handle];
    switch (fn) {
    case DL_SET_SYMTAB: {
        struct udisp *u = &udisps[handle];
        uint32_t addr = regs->d.ebx;
        uint32_t size = regs->d.ecx;
        uint32_t mem_base = regs->d.edx;
        char *elf;
        void *eh;
        int i, ret = 0;
        djloudprintf("addr 0x%x mem_base 0x%x\n", addr, mem_base);
        elf = (char *)djaddr2ptr(addr);
        djloudprintf("data %p(%s)\n", elf, elf);
        eh = u->eops->open(elf, size, mem_base);
        for (i = 0; i < num_athunks; i++) {
            struct athunk *t = &asm_thunks[i];
            uint32_t off = u->eops->getsym(eh, t->name);
            if (off) {
                *t->ptr = djaddr2ptr(off);
            } else {
                djloudprintf("symbol %s not resolved\n", t->name);
                ret = -1;
                break;
            }
        }
        u->eops->close(eh);
        return ret;
    }
    }
    return -1;
}

static dj64cdispatch_t *dops[] = { dj64_call, dj64_ctrl };

dj64cdispatch_t **DJ64_INIT_FN(int handle,
    dj64dispatch_t *disp, const struct elf_ops *ops)
{
    struct udisp *u;
    assert(handle < MAX_HANDLES);
    u = &udisps[handle];
    u->disp = disp;
    u->eops = ops;
    return dops;
}

int DJ64_INIT_ONCE_FN(const struct dj64_api *api, int api_ver)
{
    int ret = 0;
    if (api_ver != DJ64_API_VER)
        return -1;
    if (!dj64api)
        ret++;
    dj64api = api;
    return ret;
}

uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    return 0;
}

uint8_t *clean_stk(size_t len)
{
//    uint8_t *ret = (uint8_t *)so2lin(s_regs.ss, LO_WORD(s_regs.esp));
//    s_regs.esp += len;
//    return ret;
    return NULL;
}

uint32_t alloc_cbk_VOID(_cbk_VOID cbk)
{
    return 0; // TODO
}
