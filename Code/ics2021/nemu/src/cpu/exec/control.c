#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  decoding.is_jmp=1;   //标识跳转。
  rtl_push(eip);       //将eip入栈。
  rtl_addi(&decoding.jmp_eip,eip,id_dest->val);
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(reti) {
rtl_pop(&decoding.jmp_eip);
decoding.is_jmp = 1;
cpu.esp += id_dest->val;
print_asm("ret");
}


make_EHelper(ret) {
  rtl_pop(&decoding.jmp_eip);//获取跳转位置
  decoding.is_jmp=1;//设置跳转
  print_asm("ret");
}

make_EHelper(call_rm) {
  rtl_push(&decoding.seq_eip);
  decoding.is_jmp = 1;
  decoding.jmp_eip = id_dest->val;

  print_asm("call *%s", id_dest->str);
}
