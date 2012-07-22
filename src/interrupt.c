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
                Interrupt
 ==========================================

*/
#include "interrupt.h"
#include "io.h"

/* ����Ĭ���жϴ����� */
void interrupt_asm_default_handle_for_interrupt() ;

/* idt ����ṹ */
struct interrupt_idt_item_struct{
	unsigned short offset_0_15 ;		// ƫ������0~15λ
	unsigned short seg_selector ;		// ��ѡ���
	unsigned char un_used_0 ;			// ����������Ϊȫ��
	unsigned char saved_1_1_0 : 3 ;		// ����������Ϊ 110
	unsigned char d : 1 ;				// D λ
	unsigned char saved_0 : 1 ;			// ����������Ϊ0
	unsigned char dpl : 2 ;				// ��Ȩλ
	unsigned char p : 1 ;				// P λ
	unsigned short offset_16_31 ;		// ƫ������16~31λ
} ;

/* idt �������ṹ */
struct interrupt_idt_descriptor_struct{
	unsigned short idt_length_limit ;
	struct interrupt_idt_item_struct *idt_addr ;
} ;

/* 8259A ��ʼ������ */
static void interrupt_init_8259A() ;

/* ��ʼ���ж������� IDT */
static void interrupt_init_idt() ;

static struct interrupt_idt_descriptor_struct interrupt_idt_descriptor ;

static struct interrupt_idt_item_struct interrupt_idt[ 256 ] ;

/* �жϳ�ʼ������ */
void interrupt_init()
{
	// ��ʼ�� 8259A �жϿ�����
	interrupt_init_8259A() ;
	
	// ��ʼ���ж������� IDT
	interrupt_init_idt() ;
}

/* 8259A ��ʼ������ */
static void interrupt_init_8259A()
{
	// ���жϼĴ������
	// ���� ICW1 : ʹ�� ICW4����������
	io_write_to_io_port( 0x20 , 0x11 ) ;
	io_write_to_io_port( 0xa0 , 0x11 ) ;
	
	// ���� ICW2���ж���ʼ�Ŵ� 0x20 ��ʼ����һƬ���� 0x28��ʼ���ڶ�Ƭ��
	io_write_to_io_port( 0x21 , 0x20 ) ; 
	io_write_to_io_port( 0xa1 , 0x28 ) ;
	
	// ���� ICW3
	io_write_to_io_port( 0x21 , 0x4 ) ;
	io_write_to_io_port( 0xa1 , 0x2 ) ;
	
	// ���� ICW4
	io_write_to_io_port( 0x21 , 0x1 ) ;
	io_write_to_io_port( 0xa1 , 0x1 ) ;
	
	// �����ж�����λ OCW1 �����������ж����� 
	// ������Ƭ�� IRQ2 ��ӱ�����,�ʲ�������
	io_write_to_io_port( 0x21 , 0xfb ) ;
	io_write_to_io_port( 0xa1 , 0xff ) ;
}

/* ��ʼ���ж����� */
static void interrupt_init_idt()
{
	struct interrupt_idt_item_struct p ;
	p.d = 1 ;
	p.dpl = 0 ;
	p.offset_0_15 = ( unsigned int )interrupt_asm_default_handle_for_interrupt ;
	p.offset_16_31 = ( unsigned int )interrupt_asm_default_handle_for_interrupt ;
	p.p = 1 ;
	p.saved_0 = 0 ;
	p.saved_1_1_0 = 6 ;
	p.seg_selector = 0x8 ; // ʹ��ϵͳ�����
	p.un_used_0 = 0 ;
	
	for( int i = 0 ; i < 256 ; ++i ){
		interrupt_idt[ i ] = p ;
	}
	
	interrupt_idt_descriptor.idt_addr = interrupt_idt ;
	interrupt_idt_descriptor.idt_length_limit = 256 * 8 ;
	
	// �����ж�����
	__asm__( "lidt %0" : "=m"( interrupt_idt_descriptor ) ) ;
}

/* ���ж� */
void interrupt_open_interrupt()
{
	__asm__( "sti" ) ;
}

/* ���ж� */
void interrupt_close_interrupt() 
{
	__asm__( "cli" ) ;
}

/* �жϴ������װ���� */
void interrupt_install_handle_for_interrupt( unsigned char interrupt_number , void *handle_for_interrupt )
{
	/* ���ȹ��ж� */
	interrupt_close_interrupt() ;
	
	/* Ȼ������жϺ���ָ�����жϴ������ľ��ȡ��ԭ���� */
	interrupt_idt[ interrupt_number ].offset_0_15 = ( unsigned int )handle_for_interrupt ;
	interrupt_idt[ interrupt_number ].offset_16_31 = ( unsigned int )handle_for_interrupt >> 16 ;
}

/* �����ж������֣��Զ����жϣ������ֶ����ж� */
void interrupt_set_interrupt_mask_word( unsigned int irq_number , int dose_enable )
{
	interrupt_close_interrupt() ;
	if( irq_number < 0 || irq_number > 15 ){
		/* �������õ������жϺ��� 0~15 ֮�� */
		return ;
	}
	else{
		unsigned char mask_word ;
		if( irq_number < 8 ){
			/* ���õ�һƬ���жϺ� */
			/* ����һƬ���ж������� */
			mask_word = io_read_from_io_port( 0x21 ) ;
		}
		else{
			/* ���ڶ�Ƭ���ж������� */
			mask_word = io_read_from_io_port( 0xa1 ) ;
		}
		switch( irq_number ){
		case 0 :
		case 8:
			if( dose_enable ){
				mask_word &= 0xfe ; // 0xfe = 1111 1110
			}
			else{
				mask_word |= 0x01 ; // 0x00 = 0000 0001
			}
			break ;
		case 1 :
		case 9:
			if( dose_enable ){
				mask_word &= 0xfd ; // 0xfd = 1111 1101
			}
			else{
				mask_word |= 0x02 ; // 0x01 = 0000 0010
			}
			break ;
		case 2 :
		case 10:
			if( dose_enable ){
				mask_word &= 0xfb ; // 0xfb = 1111 1011
			}
			else{
				mask_word |= 0x04 ; // 0x04 = 0000 0100
			}
			break ;
		case 3 :
		case 11:
			if( dose_enable ){
				mask_word &= 0xf7 ; // 0xf7 = 1111 0111
			}
			else{
				mask_word |= 0x08 ; // 0x08 = 0000 1000
			}
			break ;
		case 4 :
		case 12:
			if( dose_enable ){
				mask_word &= 0xef ; // 0xef = 1110 1111
			}
			else{
				mask_word |= 0x10 ; // 0x10 = 0001 0000
			}
			break ;
		case 5 :
		case 13:
			if( dose_enable ){
				mask_word &= 0xdf ; // 0xdf = 1101 1111
			}
			else{
				mask_word |= 0x20 ; // 0x20 = 0010 0000
			}
			break ;
		case 6 :
		case 14:
			if( dose_enable ){
				mask_word &= 0xbf ; // 0xbf = 1011 1111
			}
			else{
				mask_word |= 0x40 ; // 0x40 = 0100 0000
			}
			break ;
		case 7 :
		case 15:
			if( dose_enable ){
				mask_word &= 0x7f ; // 0x7f = 0111 1111
			}
			else{
				mask_word |= 0x80 ; // 0x80 = 1000 0000
			}
			break ;
		}
		if( irq_number < 8 ){
			/* ���͵���Ƭ */
			io_write_to_io_port( 0x21 , mask_word ) ;
		}
		else{
			/* ���͵���Ƭ */
			io_write_to_io_port( 0xa1 , mask_word ) ;
		}
	}
}
