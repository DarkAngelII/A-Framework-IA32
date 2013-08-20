/*
Copyright (C) 2013 LeZiZi Studio
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 ==========================================
           Memory Interrupt Handler
 ==========================================

*/

#include "interrupt.h"
#include "vesa.h"
#include "kernel.h"
#include "memory.h"

/****************************************
*  declaration                          *
*****************************************/

void memory_asm_handler_for_page_fault_interrupt();

void memory_handler_for_page_fault_interrupt() ;

/****************************************
*  defination                           *
*****************************************/

static const int MEMORY_PAGE_FAULT_INTERRUPT_NUMBER = 0x0E ;
unsigned int *page_directory = (unsigned int*) 0x900000; //��ʱ���������Զ��������λ��
unsigned int *first_page_table = (unsigned int*) 0x9C0000;

/****************************************
*  implementation                       *
*****************************************/


/* ���� gdt ���ʼ������ 
static void system_init_gdt() ;


/* GDT����ṹ 
struct system_gdt_item_struct{
	unsigned short seg_length_limit_0_15 ;		// ����0~15λ
	unsigned short seg_base_addr_0_15 ;			// ��ַ�� 0~15��λ
	unsigned char  seg_base_addr_16_23 ;		// ��ַ��16~23λ
	unsigned char  type    : 4 ;				// typeλ
	unsigned char  s       : 1 ;				// Sλ
	unsigned char  dpl     : 2 ;				// ��Ȩλ
	unsigned char  p       : 1 ;				// Pλ
	unsigned char  seg_length_limit_16_19 : 4 ;	// ���޵�16~19λ
	unsigned char  avl     : 1 ;				// AVLλ
	unsigned char  saved_0 : 1 ;				// ����λ������Ϊ0
	unsigned char  d_or_b  : 1 ;				// D/Bλ
	unsigned char  g       : 1 ;				// Gλ
	unsigned char  seg_base_addr_24_31 ;		// ��ַ��24~31λ
} ;

/* GDT��ṹ 
struct system_gdt_struct{ 
	struct system_gdt_item_struct null_seg ;		// �նΣ�Intel����
	struct system_gdt_item_struct system_code_seg ;	// ϵͳ�����
	struct system_gdt_item_struct system_data_seg ;	// ϵͳ���ݶ�  
} ;

/* GDT �������ṹ 
struct system_gdt_descriptor_struct{ 
	unsigned short gdt_length_limit ;
	struct system_gdt_struct *gdt_addr ;
} ;

/* ���� gdt �� 
static struct system_gdt_struct system_gdt ;

/* ���� gdt ������ 
static struct system_gdt_descriptor_struct system_gdt_descriptor ;

/* ��д gdt �� 
static void system_init_gdt()
{  
 	// �����Ժ������ gdt ����������Ŀ,���,�˴�Ӧ���³�ʼ�� gdt ��,�Ա��� gdt ��λ��
	// ��ʼ��ϵͳ�����
	struct system_gdt_item_struct *p = &system_gdt.system_code_seg ;
	p->avl = 0 ;
	p->d_or_b = 1 ;
	p->dpl = 0 ;
	p->g = 1 ;
	p->p = 1 ;
	p->s = 1 ;
	p->saved_0 = 0 ;
	p->seg_base_addr_0_15 = 0 ;
	p->seg_base_addr_16_23 = 0 ;
	p->seg_base_addr_24_31 = 0 ;
	p->seg_length_limit_0_15 = 0xffff ;
	p->seg_length_limit_16_19 = 0xff ;
	p->type = 0x9a ;
	
	// ��ʼ��ϵͳ���ݶ�
	system_gdt.system_data_seg = system_gdt.system_code_seg ;
	system_gdt.system_data_seg.type = 0x92 ;
	
	// ��ʼ�� gdt ��������
	system_gdt_descriptor.gdt_addr = &system_gdt ;
	system_gdt_descriptor.gdt_length_limit = 0xffff ;
	
	// ��Ƕ������� gdt ��������
	__asm__( "lgdt %0" : "=m"( system_gdt_descriptor ) ) ;
}
*/

void memory_mmu_paging_init()
{
	// install interrupt handler
	interrupt_install_handle_for_interrupt( MEMORY_PAGE_FAULT_INTERRUPT_NUMBER , memory_asm_handler_for_page_fault_interrupt ) ;
	
	page_directory = (unsigned int*) 0xA000000; //��ʱ���������Զ��������λ��
	first_page_table = (unsigned int*) 0xAC0000;
	
	// holds the physical address where we want to start mapping these pages to.
	// ��ʱ��������1024��ҳ�渲��4MB�ڴ�

	unsigned int address = 0; 
	for(unsigned int id_in_dir=0; id_in_dir<1024; id_in_dir++)//������Ҫ 4MB����
	{
		first_page_table += 1024;
		for(unsigned int i = 0; i < 1024; i++)
		{
		    first_page_table[i] = address | 7; // ����: supervisor level, read/write, present.
		    address = address + 4096; //advance the address to the next page boundary
		}
		page_directory[id_in_dir] = first_page_table; 
		page_directory[id_in_dir] |= 7;// attributes: supervisor level, read/write, present
	}
	//�ƻ���ʵ��
	first_page_table =(unsigned int*)  0xAC0000;
	for(unsigned int id_in_dir=1024*2; id_in_dir<1024*800; id_in_dir++)
		first_page_table[id_in_dir] = 0 | 2;
	//�ƻ���ʵ��

	// enable paging
	__asm__("mov %0, %%cr3":: "b"(page_directory));
	unsigned int cr0;
 	__asm__("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	cr0 &= 0xFFFEFFFF; //disable write protection
	__asm__("mov %0, %%cr0":: "b"(cr0));
	//__asm__("int $0x0e");// reserve for A-Framewrok action exchanging.
}

void memory_handler_for_page_fault_interrupt()
{
	unsigned int color = vesa_compound_rgb( 255 , 0 , 255  ) ;
	const char string[]  = "  (DEMO) ERROR OF PAGEFAULT HAS OCCURRED!          "; 
	//when using this, -fno-stack-protector must be included in the Makefile
	for( int i = 0 ; i < 52 ; ++i ){
		vesa_print_english(303+i*8 , 502 , string[i] , color ) ;
	}
	while(1);
	
}
