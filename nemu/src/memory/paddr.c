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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

#ifdef CONFIG_MTRACE_COND
char mringtrace_buf[16][100]={0};   
int mringtrace_count=0;
void print_mringbuf()
{
  printf("mtrace:\n");
  for(int i=0;i<16;i++)
  {
    if(strlen(mringtrace_buf[i])==0)
      break;
    if((i+1)%16==mringtrace_count)
      printf("-->");
    else
      printf("   ");
    printf("%s\n",mringtrace_buf[i]);
  }
  printf("See more mamery trace by print mtrace\n");
}
char mtrace_buf[CONFIG_MTRACE_END-CONFIG_MTRACE_START][100]={0};   
int mtrace_count=0;
void print_mtrace(){
  for(int i=0;i<CONFIG_MTRACE_END-CONFIG_MTRACE_START;i++)
  {
    if(strlen(mtrace_buf[i])==0)
      break;
    printf("%s\n",mtrace_buf[i]);
  }
}
#endif

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
#ifdef CONFIG_MTRACE_COND
  if((CONFIG_MTRACE_START<=mtrace_count)&&(mtrace_count<CONFIG_MTRACE_END))
     sprintf(mtrace_buf[mtrace_count],"read:  addr:%016x len:%02d content:%016lx",addr,len,ret);
  sprintf(mringtrace_buf[mringtrace_count],"read:  addr:%016x len:%02d content:%016lx",addr,len,ret);
  mtrace_count++;
  mringtrace_count=(mringtrace_count+1)%16;
#endif
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
#ifdef CONFIG_MTRACE_COND
  if((CONFIG_MTRACE_START<=mtrace_count)&&(mtrace_count<CONFIG_MTRACE_END))
     sprintf(mtrace_buf[mtrace_count],"read:  addr:%016x len:%02d content:%016lx",addr,len,data);
  sprintf(mringtrace_buf[mringtrace_count],"read:  addr:%016x len:%02d content:%016lx",addr,len,data);
  mtrace_count++;
  mringtrace_count=(mringtrace_count+1)%16;
#endif
}

static void out_of_bound(paddr_t addr) {
  void print_iringbuf();
  print_iringbuf();
  print_mringbuf();
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
