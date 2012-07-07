#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/string.h"
#include "mruby/proc.h"

static mrb_code iseq_0[] = {
  0x01000006,
  0x0180003d,
  0x02000005,
  0x010000a0,
  0x01400483,
  0x01800340,
  0x01004020,
  0x01000006,
  0x018000bd,
  0x02000005,
  0x010000a0,
  0x0000004a,
};

static mrb_code iseq_1[] = {
  0x02000026,
  0x02000006,
  0x0280003d,
  0x03004001,
  0x0281803e,
  0x030000bd,
  0x0281803e,
  0x03000005,
  0x020000a0,
  0x02000029,
};

void
rbmain(mrb_state *mrb)
{
  int n = mrb->irep_len;
  int idx = n;
  mrb_irep *irep;

  mrb_add_irep(mrb, idx+2);

  irep = mrb->irep[idx] = mrb_malloc(mrb, sizeof(mrb_irep));
  irep->idx = idx++;
  irep->flags = 0 | MRB_ISEQ_NOFREE;
  irep->nlocals = 2;
  irep->nregs = 4;
  irep->ilen = 12;
  irep->iseq = iseq_0;
  irep->slen = 2;
  irep->syms = mrb_malloc(mrb, sizeof(mrb_sym)*2);
  irep->syms[0] = mrb_intern(mrb, "puts");
  irep->syms[1] = mrb_intern(mrb, "times");
  irep->plen = 2;
  irep->pool = mrb_malloc(mrb, sizeof(mrb_value)*2);
  irep->pool[0] = mrb_str_new(mrb, "Hello, mruby on stm32f4", 23);
  irep->pool[1] = mrb_str_new(mrb, "end.", 4);

  irep = mrb->irep[idx] = mrb_malloc(mrb, sizeof(mrb_irep));
  irep->idx = idx++;
  irep->flags = 0 | MRB_ISEQ_NOFREE;
  irep->nlocals = 4;
  irep->nregs = 7;
  irep->ilen = 10;
  irep->iseq = iseq_1;
  irep->slen = 1;
  irep->syms = mrb_malloc(mrb, sizeof(mrb_sym)*1);
  irep->syms[0] = mrb_intern(mrb, "puts");
  irep->plen = 2;
  irep->pool = mrb_malloc(mrb, sizeof(mrb_value)*2);
  irep->pool[0] = mrb_str_new(mrb, "loop: ", 6);
  irep->pool[1] = mrb_str_new(mrb, "", 0);

  mrb->irep_len = idx;

  extern mrb_value mrb_top_self(mrb_state *mrb);
  mrb_run(mrb, mrb_proc_new(mrb, mrb->irep[n]), mrb_top_self(mrb));
}
