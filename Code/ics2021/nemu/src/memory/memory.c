#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (1024 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
  return mmio_read(addr, len, mmio_id);
  }
  else
  {
	return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
	int mmio_id = is_mmio(addr);
    if (mmio_id != -1) {
    mmio_write(addr, len, data, mmio_id);
    }
   else {
      memcpy(guest_to_host(addr), &data, len);
   }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {
        if ((addr&0xFFF)+len>0x1000) {
            /* this is a special case, you can handle it later. */
            int len1,len2;
            len1 = 0x1000-(addr&0xfff);//��ȡǰһҳ��ռ�ÿռ�
            len2 = len - len1;//��ȡ��һҳ��ռ�ÿռ�

            paddr_t addr1 = page_translate(addr,false);//�����ַת��Ϊ�����ַ
            uint32_t data1 = paddr_read(addr1,len1);//��ȡ����

            paddr_t addr2 = page_translate(addr+len1,false);//�����ַת��Ϊ�����ַ
            uint32_t data2 = paddr_read(addr2,len2);//��ȡ����

            //len1<<3��ʾ��ȡdata1��λ��
            uint32_t data = (data2<<(len1<<3))+data1;//��data2�������Ƶ���λ����϶�ȡ�������ݡ�
            return data;
        }
        else {
            paddr_t paddr = page_translate(addr,false);
            return paddr_read(paddr, len);
        }
    }
    else
      return paddr_read(addr,len);
}
void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {
        if ((addr&0xFFF)+len>0x1000) {
            /* this is a special case, you can handle it later. */
            int len1,len2;
            len1 = 0x1000-(addr&0xfff);//��ȡǰһҳ��ռ�ÿռ�
            len2 = len - len1;//��ȡ��һҳ��ռ�ÿռ�

            paddr_t addr1 = page_translate(addr,true);//�����ַת��Ϊ�����ַ
            paddr_write(addr1,len1,data);//д������

            uint32_t data2 = data >> (len1<<3);
            paddr_t addr2 = page_translate(addr+len1,true);
            paddr_write(addr2,len2,data2);
        }
        else {
            paddr_t paddr = page_translate(addr,true);
            return paddr_write(paddr, len, data);
        }
    }
    else
      return paddr_write(addr, len, data);
}

paddr_t page_translate(vaddr_t vaddr,bool isWrite) {

 // Log("addr:0x%x\n",vaddr);
  //cr3��20λ
  vaddr_t CR3 = cpu.cr3.page_directory_base<<12;
 // Log("CR3:0x%x\n",CR3);
  //vaddr��10λ
  vaddr_t dir = (vaddr>>22)*4;
 // Log("dir:0x%x\n",dir);
  //ȡ��cr3�ĸ�20λ��vaddr�ĸ�8λ���
  paddr_t pdAddr = CR3 + dir;
 // Log("pdAddr:0x%x\n",pdAddr);
  //��ȡ
  PDE pd_val;
  pd_val.val = paddr_read(pdAddr,4);
 // Log("pdAddr:0x%x  pd_val0x%x\n",pdAddr,pd_val.val);
 // assert(pd_val.present);

  //��ȡ��20λ
  vaddr_t t1 = pd_val.page_frame<<12;

  //��ȡ�ڶ���ʮλpage
  vaddr_t t2 = ((vaddr>>12)&0x3FF)*4;

  paddr_t ptAddr = t1 + t2;
 // Log("pt_addr:0x%x\n",ptAddr);
  PTE pt_val;
  pt_val.val = paddr_read(ptAddr,4);
 // assert(pt_val.present);
 // Log("pt_addr:0x%x  pt_val:0x%x\n",ptAddr,pt_val.val);
  //��ȡ��20λ
  t1 = pt_val.page_frame<<12;

  //��ȡ���12λ
  t2 = vaddr&0xFFF;

  paddr_t paddr = t1 + t2;
 // Log("paddr:0x%x\n",paddr);

  pd_val.accessed = 1;
  paddr_write(pdAddr,4,pd_val.val);

  if ((pt_val.accessed == 0) || (pt_val.dirty ==0 && isWrite)){
    pt_val.accessed=1;
    pt_val.dirty=1;
  }
  paddr_write(ptAddr,4,pt_val.val);

  return paddr;

}
