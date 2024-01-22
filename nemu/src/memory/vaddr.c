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

#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
  /*int flag = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  if(flag == MMU_DIRECT) return paddr_read(addr, len);
  else if(flag == MMU_TRANSLATE) return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_IFETCH), len);
  else assert(0);
  return addr;*/
}

word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
  /*int flag = isa_mmu_check(addr, len, MEM_TYPE_READ);
  if(flag == MMU_DIRECT) return paddr_read(addr, len);
  else if(flag == MMU_TRANSLATE) return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_READ), len);
  else assert(0);
  return addr;*/
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
  /*int flag = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  if(flag == MMU_DIRECT) paddr_write(addr, len, data);
  else if(flag == MMU_TRANSLATE) paddr_write(isa_mmu_translate(addr, len, MEM_TYPE_WRITE), len, data);
  else assert(0);*/
}
