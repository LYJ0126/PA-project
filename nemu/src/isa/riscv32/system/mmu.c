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
#include <memory/vaddr.h>
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  //return MEM_RET_FAIL;
  vaddr_t pdirbase = cpu.satp << 12;//页目录基址
  uint32_t pgoff = vaddr & 0xfff;//页内偏移
  uint32_t vpn1 = (vaddr >> 22) & 0x3ff;//一级页表索引
  uint32_t vpn0 = (vaddr >> 12) & 0x3ff;//二级页表索引
  uint32_t pdiraddr = pdirbase + vpn1 * 4;//页目录项地址
  word_t pdir = paddr_read(pdiraddr, 4);//页目录项内容
  assert(pdir & 0x1);//页目录项有效
  vaddr_t ptablebase = pdir & 0xfffff000;//页表基址
  uint32_t ptableaddr = ptablebase + vpn0 * 4;//页表项地址
  word_t ptable = paddr_read(ptableaddr, 4);//页表项内容
  assert(ptable & 0x1);//页表项有效
  switch (type)
  {
    case MEM_TYPE_IFETCH: assert(ptable & 0x8); break;//指令访问
    case MEM_TYPE_READ: assert(ptable & 0x2); break;//读访问
    case MEM_TYPE_WRITE: assert(ptable & 0x4); break;//写访问
    default: assert(0); break;
  }
  paddr_t paddr = (ptable & 0xfffff000) | pgoff;//物理地址
  assert(paddr == vaddr);//恒等映射
  return paddr;
}
