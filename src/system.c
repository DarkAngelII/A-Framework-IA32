/*
Copyright (C) 2012 LeZiZi Studio
 
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
                Kernel HCL
 ==========================================

*/
#include "interrupt.h"
#include "mouse.h"
#include "keyboard.h"

/* ���� gdt ���ʼ������ */
static void system_init_gdt() ;


/* GDT����ṹ */
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

/* GDT��ṹ */
struct system_gdt_struct{ 
	struct system_gdt_item_struct null_seg ;		// �նΣ�Intel����
	struct system_gdt_item_struct system_code_seg ;	// ϵͳ�����
	struct system_gdt_item_struct system_date_seg ;	// ϵͳ���ݶ�  
} ;

/* GDT �������ṹ */
struct system_gdt_descriptor_struct{ 
	unsigned short gdt_length_limit ;
	struct system_gdt_struct *gdt_addr ;
} ;

/* ���� gdt �� */
static struct system_gdt_struct system_gdt ;

/* ���� gdt ������ */
static struct system_gdt_descriptor_struct system_gdt_descriptor ;

/* ϵͳ��ʼ�� */
void system_init()  
{
	// �����Ժ������ gdt ����������Ŀ,���,�˴�Ӧ���³�ʼ�� gdt ��,�Ա��� gdt ��λ��
	system_init_gdt() ;
	
	// ��ʼ���ж�
	interrupt_init() ;
	
	// ��ʼ������
	keyboard_init() ;
	
	// ��ʼ�� mouse
	mouse_init() ;
}

/* ��д gdt �� */
static void system_init_gdt()
{  
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
	system_gdt.system_date_seg = system_gdt.system_code_seg ;
	system_gdt.system_date_seg.type = 0x92 ;
	
	// ��ʼ�� gdt ��������
	system_gdt_descriptor.gdt_addr = &system_gdt ;
	system_gdt_descriptor.gdt_length_limit = 0xffff ;
	
	// ��Ƕ������� gdt ��������
	__asm__( "lgdt %0" : "=m"( system_gdt_descriptor ) ) ;
}