/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>//isa.h已经包含了isa-def.h。isa.h中有extern CPU_state cpu;这个声明

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.mcause = NO;//存放触发异常的原因到mcause寄存器中
  //printf("mcause: %x\n", cpu.mcause);
  cpu.mepc = epc;//存放触发异常的PC到mepc寄存器中
  //printf("mepc: %x\n", cpu.mepc);
  //printf("mtvec: %x\n", cpu.mtvec);
  return cpu.mtvec;//返回异常入口地址
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
