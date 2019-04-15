/*
** $Id: dumpbmp.c 167 2008-02-03 05:42:58Z tangjianbin $
**
** vbfeditor.c: A VBF font editor.
**
** Copyright (C) 2003 FMSoft.
*/

/*
**  This source is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This software is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#include <string.h>
#include "FreeImage.h"
#endif
#include <time.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "dumpbmp.h"

#define IDC_TB_SELF            100
#define IDC_TB_OPEN            110
#define IDC_TB_SAVE            120
#define IDC_TB_PREVIOUS        130
#define IDC_TB_NEXT            140
#define IDC_TB_ZOOMOUT         150
#define IDC_TB_ZOOMIN          160

static FILE *fp_header = NULL, *fp_bmps = NULL, *fp_inc = NULL;
static FILE *fp_loader = NULL, *fp_unloader = NULL;
static int bitmaps_number = 0;

BOOL RLE_ENCODE = FALSE;

#define TB_HEIGHT              16
#define TB_WIDTH               22

//static MYBITMAP my_dib;
//static BITMAP my_bmp;
//static RGB my_pal [256];

#ifdef WIN32
static const char* get_extension (const char* filename)
{
    const char* ext;
	
    ext = strrchr (filename, '.');
	
    if (ext)
        return ext + 1;
	
    return NULL;
}
#endif

void encode_rle (FILE* fp, BYTE* bits, int width, int bytesPerPixel)
{
    int i, count = 0, run;

    while (count < width) {
        run = 1;
        while (((count ++) < width) && (run < 255) && memcmp (bits, bits + bytesPerPixel * run, bytesPerPixel) == 0)
            run ++;

        if (bytesPerPixel < 4)
            fprintf (fp, "0x%02x, ", (BYTE)run);
        else {
            fprintf (fp, "0x%02x, ", (BYTE)run);
            fprintf (fp, "0x%02x, ", 0);
        }

        for (i = 0; i < bytesPerPixel; i++)
            fprintf (fp, "0x%02x, ", bits [i]);

        fprintf (fp, "\n\t\t");

        bits += bytesPerPixel * run;
    }

    if (bytesPerPixel < 4)
        fprintf (fp, "0x00, ");
    else {
        fprintf (fp, "0x00, ");
        fprintf (fp, "0x00, ");
    }
}


int dump_bitmap (BITMAP* bmp, const char* prefix, FILE *fp_b)
{
    int i, j;
    char file [MAX_PATH + 1];
    FILE* fp;
    Uint8* buf;

    //strcpy (file, prefix);
    //strcat (file, ".c");
    sprintf(file, "%s_bmp.c", prefix);

    if ((fp = fopen (file, "w+")) == NULL)
        return -1;

    fprintf (fp, "/* This file is generated by 'dumpbmp', do not edit manually. */\n\n");
    fprintf (fp, "#include <minigui/common.h>\n");
    fprintf (fp, "#include <minigui/gdi.h>\n");
    fprintf (fp, "\n");
    fprintf (fp, "static unsigned char %s_bits [] = {\n", prefix);

    buf = bmp->bmBits;
    for (i = 0; i < bmp->bmHeight; i ++) {
        fprintf (fp, "    ");
        if (RLE_ENCODE) {
            encode_rle (fp, buf, bmp->bmWidth, bmp->bmBytesPerPixel);
            buf += bmp->bmPitch;
        } else {
            for (j = 0; j < bmp->bmPitch; j ++) {
                fprintf (fp, "0x%02x, ", *buf);
                buf++;

                if (j != 0 && (j % 10 == 0))
                    fprintf (fp, "\n        ");
            }
        }
        fprintf (fp, "\n");
    }

    fprintf (fp, "};\n\n\n");

    /* make bitmap structure */

    if (RLE_ENCODE)
        bmp->bmType |= BMP_TYPE_RLE;

    if (!fp_b) {
        fprintf (fp, "static BITMAP _bmpdata_%s_bmp = {\n", prefix);
        fprintf (fp, "    0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", 
                    bmp->bmType, bmp->bmBitsPerPixel, bmp->bmBytesPerPixel, bmp->bmAlpha);
        fprintf (fp, "    0x%08x,\n",
                    bmp->bmColorKey);
#ifdef _FOR_MONOBITMAP
        fprintf (fp, "    0x%08x,\n",
                    bmp->bmColorRep);
#endif
        fprintf (fp, "    0x%08x, 0x%08x, 0x%08x,\n",
                    bmp->bmWidth, bmp->bmHeight, bmp->bmPitch);
        fprintf (fp, "    %s_bits\n", prefix);
        fprintf (fp, "};\n\n");
    }
    else {
        fprintf (fp_b, "{\n");
        fprintf (fp_b, "    0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", 
                    bmp->bmType, bmp->bmBitsPerPixel, bmp->bmBytesPerPixel, bmp->bmAlpha);
        fprintf (fp_b, "    0x%08x,\n",
                    bmp->bmColorKey);
#ifdef _FOR_MONOBITMAP
        fprintf (fp_b, "    0x%08x,\n",
                    bmp->bmColorRep);
#endif
        fprintf (fp_b, "    0x%08x, 0x%08x, 0x%08x,\n",
                    bmp->bmWidth, bmp->bmHeight, bmp->bmPitch);
        fprintf (fp_b, "    %s_bits\n", prefix);
        fprintf (fp_b, "},\n\n");
    }

    fclose (fp);
    return 0;
}

void dump_from_file (HDC hdc, const char* file)
{
    BITMAP  bmp;
    char    path [MAX_PATH + 1];
    char    prefix [LEN_PREFIX + 1];
    char    *tmp;
	//int     flag = 0;
    int     ret;
#ifdef WIN32
	FIBITMAP    *fibitmap;
	const char  *ext;
	FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
	struct _finddata_t c_file; 
	char    filefullname[PATH_MAX+NAME_MAX+1];	
	long    hFile; 
	char    tmp_path[MAX_PATH + 1];
	int     path_flag = 0;
	char    *tmp2;
#endif
	
    strncpy (path, file, MAX_PATH);
    tmp = strrchr (path, '.');
	if (tmp) {
        *tmp = '\0';
	}
#ifdef WIN32
	else {
		path_flag = 1;
		sprintf (tmp_path, "%s", strtok (path, "*"));
		strcpy (path, tmp_path);
	}		
#endif
	tmp = path;	
	while (*tmp) {
		if (*tmp == ' ' || *tmp == '/' || *tmp == '.' || *tmp == '-' || *tmp == '\\')
			*tmp = '_';
		tmp++;
	}
	strncpy (prefix, path, LEN_PREFIX);
#ifdef WIN32
	if (path_flag) {
		char find_path[MAX_PATH + 1];
		memset (find_path, 0, MAX_PATH + 1);
		sprintf (find_path, "%s*.*", tmp_path);
		if( (hFile = _findfirst(find_path, &c_file )) == -1L ) 
			printf( "No files in %s!\n" , tmp_path); 
		else 
		{ 
			_findnext( hFile, &c_file );
			while( _findnext( hFile, &c_file ) == 0 ) 
			{ 
				file = c_file.name;
				memset (filefullname, 0, PATH_MAX+NAME_MAX+1);
				sprintf (filefullname, "%s%s", tmp_path, file);
				if ((ext = get_extension (file)) == NULL)
					continue;
				if (strcmp (ext, "c") == 0)
					continue;
				if(strcmp(ext, "bmp") != 0 && strcmp(ext, "gif") != 0)
				{
					if(strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 )		
						image_format = FIF_JPEG;
					else if (strcmp (ext, "png") ==0)
						image_format = FIF_PNG;
					else if (strcmp (ext, "tiff") == 0)		
						image_format = FIF_TIFF;
					else if (strcmp (ext, "psd") == 0)
						image_format = FIF_PSD;
					
					if(image_format != FIF_UNKNOWN)
					{
						fibitmap = FreeImage_Load (image_format, filefullname, 0);
						printf ("fibitmap = %x\n", fibitmap);
						FreeImage_Save (FIF_BMP, fibitmap, "tmp.bmp", 0);
						FreeImage_Unload (fibitmap);
						flag = 1;
					}
				}
				
				if(flag)
					ret = LoadBitmapFromFile (hdc, &bmp, "tmp.bmp");
				else {
					ret = LoadBitmapFromFile (hdc, &bmp, filefullname);
				}
		
				if (ret == 0) 
				{
					char fp_header_path[PATH_MAX+NAME_MAX+1];
					char *prev = NULL;
					
					if (fp_header) 
					{
						strcpy (fp_header_path, filefullname);
						tmp = fp_header_path;
						while (tmp)
						{
							tmp = strchr (tmp, '\\');
							if (tmp) {
								*tmp ++ = '_';
							    prev = tmp;
							}
						}
						
						tmp = strchr (prev, '.');
						if (tmp) {
							*(++tmp) = 'c';
							*(++tmp) = '\0';
						}
						
						fprintf (fp_inc, "#include \"%s\"\n", fp_header_path);
						tmp = strchr (fp_header_path, '.');
						if (tmp)
							*tmp = '\0';
						bmp.bmType = BMP_TYPE_COLORKEY;
						bmp.bmColorKey = RGB2Pixel (hdc, 248, 13, 240);
						tmp = fp_header_path;

						while (*tmp) {
							if (*tmp == ' ' || *tmp == '/' || *tmp == '.' || *tmp == '-' || *tmp == '\\')
								*tmp = '_';
							tmp++;
						}

						add_bmp_header_entry (fp_header, fp_header_path);
					}
					if (fp_loader) 
					{
						add_bmp_loader_entry (fp_loader, filefullname, fp_header_path);
						add_bmp_unloader_entry (fp_unloader, fp_header_path);
					}
					bitmaps_number ++;
					dump_bitmap (&bmp, fp_header_path, fp_bmps);
					if (!fp_header && !fp_loader)
						add_hash_entry (filefullname, fp_header_path);
				}
				else 
					fprintf (stderr, "Skip file: %s\n", filefullname);
			} 
		}
		remove ("tmp.bmp");	
		return;
	}
#endif

#ifdef WIN32
	if ((ext = get_extension (file)) == NULL) 
		strcpy (ext, "bmp");

	printf ("ext = %s\n", ext);
	if(strcmp(ext, "bmp") != 0 || strcmp(ext, "gif") != 0)
	{
		if(strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 )		
			image_format = FIF_JPEG;
		else if (strcmp (ext, "png") ==0)
			image_format = FIF_PNG;
		else if (strcmp (ext, "tiff") == 0)		
			image_format = FIF_TIFF;
		else if (strcmp (ext, "psd") == 0)
			image_format = FIF_PSD;
		
		if(image_format != FIF_UNKNOWN)
		{
			fibitmap = FreeImage_Load (image_format, file, 0);			
			FreeImage_Save (FIF_BMP, fibitmap, "tmp.bmp", 0);
			FreeImage_Unload (fibitmap);
			flag = 1;
		}
	}

	if(flag) 
	    ret = LoadBitmapFromFile (hdc, &bmp, "tmp.bmp");
	else 		
        ret = LoadBitmapFromFile (hdc, &bmp, file);

#else
	ret = LoadBitmapFromFile (hdc, &bmp, file);
#endif
	
    if (ret == 0) 
	{
        if (fp_header) 
		{
            fprintf (fp_inc, "#include \"%s_bmp.c\"\n", prefix);
            bmp.bmType = BMP_TYPE_COLORKEY;
            bmp.bmColorKey = RGB2Pixel (hdc, 248, 13, 240);
            add_bmp_header_entry (fp_header, prefix);
        }
        if (fp_loader) 
		{
            add_bmp_loader_entry (fp_loader, file, prefix);
            add_bmp_unloader_entry (fp_unloader, prefix);
        }
        bitmaps_number ++;
        //dump_bitmap (&bmp, prefix, fp_bmps);
        dump_bitmap (&bmp, prefix, NULL);
        //if (!fp_header && !fp_loader) 
        {
            //fprintf(stderr, "add_hash_entry file=%s, prefix=%s\n", file, prefix);
            add_hash_entry (file, prefix);
        }
    }
    else 
        fprintf (stderr, "Skip file: %s\n", file);
}

int make_file_begin (void)
{
   /* make header file begin part */
   if ((fp_header = fopen ("_bitmaps_defs.h", "w+")) == NULL)
       return -1;
   fprintf (fp_header, "/*\n");
   fprintf (fp_header, "    mg-tools edition bitmap resource ID definitions\n");
   fprintf (fp_header, "    This file is generated automatically by image-dumper tool, do not edit.\n");
   fprintf (fp_header, "*/\n\n");
   fprintf (fp_header, "extern BITMAP fhas_bitmaps[];\n\n");

   /* make bitmap struct file begin part */
   if ((fp_bmps = fopen ("_bitmaps_struct.c", "w+")) == NULL)
       return -1;
   fprintf (fp_bmps, "/*\n");
   fprintf (fp_bmps, "    mg-tools edition bitmap resource\n");
   fprintf (fp_bmps, "    This file is generated automatically by image-dumper tool, do not edit.\n");
   fprintf (fp_bmps, "*/\n\n");
   fprintf (fp_bmps, "#include \"_bitmaps_inc.c\"\n\n");
   fprintf (fp_bmps, "BITMAP fhas_bitmaps[] = {\n\n");

   if ((fp_inc = fopen ("_bitmaps_inc.c", "w+")) == NULL)
       return -1;
   fprintf (fp_inc, "/*\n");
   fprintf (fp_inc, "    mg-tools edition bitmap include files\n");
   fprintf (fp_inc, "    This file is generated automatically by image-dumper tool, do not edit.\n");
   fprintf (fp_inc, "*/\n\n");

   return 0;
}

int make_file_end (void)
{
   fprintf (fp_bmps, "};\n\n");
   fprintf (fp_bmps, "BITMAP* fhbmps = fhas_bitmaps;\n\n");
   fclose (fp_bmps);
   fclose (fp_inc);

   fprintf (fp_header, "#define fhbmp_number    %d\n", bitmaps_number);
   fprintf (fp_header, "\n");
   fclose (fp_header);
   return 0;
}

void make_loader_begin (void)
{
   if ((fp_loader = fopen ("fh_bitmaps_loader.c", "w+")) == NULL)
       return;
   fprintf (fp_loader, "/*\n");
   fprintf (fp_loader, "    Bitmap loading functions.\n");
   fprintf (fp_loader, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_loader, "*/\n\n");

   fprintf (fp_loader, "BITMAP fhas_bitmaps[fhbmp_number];\n\n");

   fprintf (fp_loader, "BOOL fhbmp_load_all (const char *res_path)\n");
   fprintf (fp_loader, "{\n");
   fprintf (fp_loader, "    char buffer[256];\n");
   fprintf (fp_loader, "    char *tmp;\n\n");
   fprintf (fp_loader, "    if (res_path)\n");
   fprintf (fp_loader, "        strcpy(buffer, res_path);\n");
   fprintf (fp_loader, "    else\n");
   fprintf (fp_loader, "        buffer[0] = 0;\n");
   fprintf (fp_loader, "    tmp = buffer + strlen(buffer);\n\n");

   if ((fp_unloader = fopen ("fh_bitmaps_unloader.c", "w+")) == NULL)
       return;
   fprintf (fp_unloader, "/*\n");
   fprintf (fp_unloader, "    Bitmap unloading functions.\n");
   fprintf (fp_unloader, "    This file is generated automatically by dumpbmp tool, do not edit.\n");
   fprintf (fp_unloader, "*/\n\n");

   fprintf (fp_unloader, "void fhbmp_unload_all (void)\n");
   fprintf (fp_unloader, "{\n");
}

void make_loader_end (void)
{
   fprintf (fp_loader, "\n");
   fprintf (fp_loader, "    return TRUE;\n");
   fprintf (fp_loader, "}\n");
   fclose (fp_loader);

   fprintf (fp_unloader, "}\n");
   fprintf (fp_unloader, "\n");
   fclose (fp_unloader);
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

