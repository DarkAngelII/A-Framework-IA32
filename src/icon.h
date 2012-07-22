#ifndef _ICON_H_
#define _ICON_H_

/************************************************
* ֧�ֱ�׼λͼ��ʽ����ֻ�����һҳͼ            *
************************************************/

struct icon_icon_dir_entry_struct{
	char width ;
	char height ;
	char color_count ;
	char reserved_0 ;
	short color_plane ;
	short word_count ;
	int byte_in_resource ;
	int image_offset ;
} ;

struct icon_icon_dir_struct{
	short reserved_0 ;
	short resource_type ;
	short picture_count ;
	struct icon_icon_dir_entry_struct icon_dir_entry ;
} ;
#endif