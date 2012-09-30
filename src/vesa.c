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
             VESA Graphic Driver
 ==========================================

*/

#include "vesa.h"
#include "bmp.h"

/****************************************
*  defination                           *
*****************************************/

const int VESA_SCREEN_X_MAX = 800 ;
const int VESA_SCREEN_Y_MAX = 600 ;

static const int VESA_ADDR = 0x80001 ;

unsigned short vesa_compound_rgb( unsigned char r , unsigned char g , unsigned char b )
{
	union{
		unsigned int color ;
		struct{
			unsigned char b : 5 ;
			unsigned char g : 6 ;
			unsigned char r : 5 ;

		};
	}u ;
	u.r = r % 32 ;
	u.g = g % 64 ;
	u.b = b % 32 ;
	return u.color ;
}

void vesa_draw_point( unsigned int x , unsigned int y , unsigned short color )
{
	unsigned int x_max = VESA_SCREEN_X_MAX - 1 ;
	unsigned int y_max = VESA_SCREEN_Y_MAX - 1 ;
	// avoid overflow
	if( x > x_max ) return ;
	if( y > y_max ) return ;
	
	// get the beginning of video card memory
	unsigned short* video = ( unsigned short * )( * ( ( unsigned int *)VESA_ADDR ) ) ;

	// caculuate the offset of this point
	unsigned int offset = y * ( x_max + 1 ) + x ;

	// set the unsigned short, to which the ( video + offset ) is pointed, as color
	*( video + offset ) = color ;
}


unsigned short vesa_get_point_color( unsigned int x , unsigned int y )
{
	unsigned int x_max = VESA_SCREEN_X_MAX - 1 ;
	unsigned int y_max = VESA_SCREEN_Y_MAX - 1 ; 

	// avoid overflow
	if( x > x_max )	x = x_max ;
	if( y > y_max ) y = y_max ;
	
	// get the beginning of video card memory
	unsigned short *video = ( unsigned short * )( * ( ( unsigned int *)VESA_ADDR ) ) ;

	// caculuate the offset of this point
	unsigned int offset = y * ( x_max + 1 ) + x ;
	return *( video + offset ) ;
}


void vesa_clean_screen( unsigned short color )
{
	vesa_fill_rect( 0 , 0 , VESA_SCREEN_X_MAX - 1 , VESA_SCREEN_Y_MAX - 1 , color ) ;
}


void vesa_copy_picture_from_screen( unsigned int x , unsigned int y , unsigned short *object_picture_addr , unsigned int picture_width , unsigned int picture_height )
{
	for( int j = 0 ; j < picture_height ; ++j ){
		for( int i = 0 ; i < picture_width ; ++i ){
			*( object_picture_addr + j * picture_width + i ) = vesa_get_point_color( x + i , y + j ) ;
		}
	}
}

void vesa_copy_picture_to_screen( unsigned int x , unsigned int y , unsigned short *source_picture_addr , unsigned int picture_width , unsigned int picture_height )
{
	for( int j = 0 ; j < picture_height ; ++j ){
		for( int i = 0 ; i < picture_width ; ++i ){
			vesa_draw_point( x + i , y + j , *( source_picture_addr + j * picture_width + i ) ) ;
		}
	}
}

void vesa_draw_x_line( unsigned int y , unsigned int x1 , unsigned int x2 , unsigned short color )
{
	for( int x = x1 ; x < x2 + 1 ; ++x ){
		vesa_draw_point( x , y , color ) ;
	}
}

void vesa_draw_y_line( unsigned int x , unsigned int y1 , unsigned int y2 , unsigned short color )
{
	for( int y = y1 ; y < y2 + 1 ; ++y ){
		vesa_draw_point( x , y , color ) ;
	}
}

void vesa_draw_rect( unsigned int x1 , unsigned int y1 , unsigned int x2 , unsigned y2 , unsigned short color , int dose_fill_it )
{
	vesa_draw_x_line( y1 , x1 , x2 , color ) ;
	vesa_draw_x_line( y2 , x1 , x2 , color ) ;
	vesa_draw_y_line( x1 , y1 , y2 , color ) ;
	vesa_draw_y_line( x2 , y1 , y2 , color ) ;
	
	if( dose_fill_it ){
		vesa_fill_rect( x1 , y1 , x2 , y2 , color ) ;
	}
}

void vesa_fill_rect( unsigned int x1 , unsigned int y1 , unsigned int x2 , unsigned int y2 , unsigned int color )
{
	for( int x = x1 ; x < x2 + 1 ; ++x ){
		for( int y = y1 ; y < y2 + 1 ; ++y ){
			vesa_draw_point( x , y , color ) ;
		}
	}
}

// ��ʾӢ��
void vesa_print_english( unsigned int x , unsigned int y , unsigned int pos_in_font , unsigned short color )
{
	unsigned char *english_font_addr = ( unsigned char * )0x30200 + pos_in_font * 16 ; // һ��Ӣ�� 16 �ֽ�
	union{
		unsigned char ch ;
		struct{
			unsigned char ch0 : 1 ;
			unsigned char ch1 : 1 ;
			unsigned char ch2 : 1 ;
			unsigned char ch3 : 1 ;
			unsigned char ch4 : 1 ;
			unsigned char ch5 : 1 ;
			unsigned char ch6 : 1 ;
			unsigned char ch7 : 1 ;
		} ;
	} u ;
	
	for( int j = 0 ; j < 16 ; ++j ){ // һ��Ӣ�� 16 ��
		u.ch = english_font_addr[ j ] ;
		int offset = x ;
		if( u.ch7 )	vesa_draw_point( offset , y , color ) ;
		if( u.ch6 )	vesa_draw_point( offset + 1  , y , color ) ;
		if( u.ch5 )	vesa_draw_point( offset + 2 , y , color ) ;
		if( u.ch4 )	vesa_draw_point( offset + 3 , y , color ) ;
		if( u.ch3 )	vesa_draw_point( offset + 4 , y , color ) ;
		if( u.ch2 )	vesa_draw_point( offset + 5 , y , color ) ;
		if( u.ch1 )	vesa_draw_point( offset + 6 , y , color ) ;
		if( u.ch0 )	vesa_draw_point( offset + 7 , y , color ) ;
		//english_font_addr[ j ]=0;
		y++ ;// move y ON THE SCREEN to the next line
	}
}

int vesa_print_integer( unsigned int x , unsigned int y , unsigned int in_integer , unsigned short color )
{
	int x_location ;
	x_location= x;
	int integer;
	integer = in_integer; 
	do {
		vesa_print_english(x_location,y,integer % 10 +'0',color);
		x_location -= 8;
		integer = integer / 10;
	} while (integer);
	return x_location;
}

void vesa_print_chinese( unsigned int x , unsigned int y , unsigned int pos_in_font , unsigned short color )
{
	unsigned char *chinese_font_addr = ( unsigned char * )0x30000 + pos_in_font * 32 ; // һ������ 32 �ֽ�
	
	union{
		unsigned char ch ;
		struct{
			unsigned char ch0 : 1 ;
			unsigned char ch1 : 1 ;
			unsigned char ch2 : 1 ;
			unsigned char ch3 : 1 ;
			unsigned char ch4 : 1 ;
			unsigned char ch5 : 1 ;
			unsigned char ch6 : 1 ;
			unsigned char ch7 : 1 ;
		} ;
	} u ;
	
	for( int j = 0 ; j < 16 ; ++j ){ // һ������ 16 ��
		for( int i = 0 ; i < 2 ; ++i ){ // ÿ�� 2 ���ֽ� ( 16 λ )
			u.ch = chinese_font_addr[ j * 2 + i ] ;
			int offset = x + i * 8 ; // һ���ֽ� 8 ������
			if( u.ch7 )	vesa_draw_point( offset , y , color ) ;
			if( u.ch6 )	vesa_draw_point( offset + 1  , y , color ) ;
			if( u.ch5 )	vesa_draw_point( offset + 2 , y , color ) ;
			if( u.ch4 )	vesa_draw_point( offset + 3 , y , color ) ;
			if( u.ch3 )	vesa_draw_point( offset + 4 , y , color ) ;
			if( u.ch2 )	vesa_draw_point( offset + 5 , y , color ) ;
			if( u.ch1 )	vesa_draw_point( offset + 6 , y , color ) ;
			if( u.ch0 )	vesa_draw_point( offset + 7 , y , color ) ;
		}
		y++ ; // move y ON THE SCREEN to the next line
	}
}

void vesa_show_bmp_picture( unsigned int x , unsigned int y , void *bmp_addr , unsigned short mask_color , int dose_use_mask_color )
{ 
	// windows 16-bit bmp (1:5:5:5) is supported
	struct bmp_bmp_head_struct *bmp_head = ( struct bmp_bmp_head_struct * )bmp_addr;
	int width = bmp_head->info_head.width ;
	int height = bmp_head->info_head.height ;
	
	// �������洢ÿ�����ɫ�ʵ���Ϣ���ڵ�λ��
	unsigned short *color = ( unsigned short * )( ( unsigned int )bmp_addr + bmp_head->offset ) ;

	// ����һ�е��ֽ��������� 4 �ı�������ˣ������ȼ���ÿ����Ҫ����������� 2 ����Ϊÿ�����������ֽ�
	int fill_length = width * 2 % 4 / 2 ; 
	
	// bmp �Ĵ��˳���Ǵ��µ��ϣ�������
	for( int i = height - 1 ; i >= 0 ; --i ){
		for( int j = 0 ; j < width ; ++j ){
			// ȡ��ÿ�����ɫ����Ϣ
			// ���� windows Ĭ�ϵ��� 555 ��ʽ���� pyos �õ��� 565 ��ʽ������Ƚ���һ��ת��
			unsigned short temp_color = vesa_change_color_form_555_to_565( *color ) ;
			if( !dose_use_mask_color || temp_color != mask_color ){
				// ����ÿ����
				vesa_draw_point( x + j , y + i , temp_color ) ;
			}
			++color ;
		}
		// ���
		color += fill_length ;
	}
}


// ɫ��ת���������� 5:5:5 ��ʽת���ɱ�׼�� 5:6:5 ��ʽ
unsigned short vesa_change_color_form_555_to_565( unsigned short color_form_555 )
{
	// �� 555 �м��ɫ�� * 2��Ϊ����ɫ�����㣬������� 0����ĩλ���� 1
	union{
		unsigned int color ;
		struct{
			unsigned char b : 5 ;
			unsigned char g : 6 ;
			unsigned char r : 5 ;
		} ;
	} u1 ;
	
	union{
		unsigned int color ;
		struct{
			unsigned char b : 5 ;
			unsigned char g : 5 ;
			unsigned char r : 5 ;
			unsigned char alpha : 1 ;  // alpha ͨ���������ﲻ��
		} ;
	} u2 ;
	
	u2.color = color_form_555 ;
	u1.r = u2.r ;
	if( u2.g ){
		u1.g = ( int )u2.g * 2 + 1 ;
	}
	else{
		u1.g = 0 ;
	}
	u1.b = u2.b ;
	return u1.color ;

}


// --------------------���Ƿָ���---------------------
// ������ֱ�ӻ�ͼ���������ǻ�������ͼ���Ƽ�ʹ������� vesa_draw_buffer

void vesa_draw_buffer ( unsigned short *buffer_ADDR, unsigned int buffer_width , unsigned int buffer_height, unsigned int x , unsigned int y ,  unsigned short mask_color , int use_mask_color )
{
	for( int j = 0 ; j < buffer_height ; ++j ){
		for( int i = 0 ; i < buffer_width ; ++i ){
			// read from buffer
			unsigned short temp_color =  *( buffer_ADDR + j * buffer_width + i ); 
			// process mask
			if( !use_mask_color || temp_color != mask_color ){
				// draw point
				vesa_draw_point( x + i , y + j , temp_color ) ;
			}
		}
	}
}