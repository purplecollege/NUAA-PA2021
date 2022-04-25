#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  vaddr_t idt_addr;
  GateDesc gateDesc;

  rtl_push((rtlreg_t*)&cpu.eflags);
  rtl_push((rtlreg_t*)&cpu.cs);
  rtl_push((rtlreg_t*)&ret_addr);
  
  idt_addr = cpu.idtr.base + NO * 8;
  *(uint32_t *)&gateDesc = vaddr_read(idt_addr, 4);
  *((uint32_t *)&gateDesc + 1) = vaddr_read(idt_addr + 4, 4);

  decoding.is_jmp = 1;
  decoding.jmp_eip = (gateDesc.offset_31_16 << 16) | (gateDesc.offset_15_0 & 0xffff);
}

void dev_raise_intr() {
}
