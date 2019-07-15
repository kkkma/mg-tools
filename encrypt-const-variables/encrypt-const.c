///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
** This file is part of mg-tools, a collection of programs to convert
** and maintain the resource for MiniGUI.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <assert.h>
#include <string.h>

static unsigned char table[] = {
    0x16, 0xfe, 0xaf, 0x06, 0xd6, 0x9d, 0x54, 0x98,
    0x77, 0x36, 0xc1, 0xc7, 0x29, 0xf8, 0x64, 0x93,
    0x2e, 0x6b, 0x2a, 0x3c, 0x12, 0x13, 0x23, 0xc9,
    0xe2, 0xa3, 0xbd, 0xbe, 0xac, 0xdf, 0x91, 0xe5,
    0xdc, 0x4a, 0xc0, 0xb8, 0xd3, 0x2c, 0x61, 0x60,
    0x5b, 0x32, 0x37, 0x7e, 0xb7, 0x8e, 0x2b, 0x0b,
    0x19, 0x50, 0x40, 0xf3, 0x5f, 0x5d, 0x0d, 0x59,
    0x21, 0xb1, 0xf6, 0xb5, 0xe3, 0xb3, 0x94, 0xa6,
    0x31, 0x78, 0x84, 0xcc, 0x9c, 0x1b, 0x7b, 0x2f,
    0x46, 0xa9, 0x9b, 0xda, 0xee, 0xd0, 0xc5, 0x7f,
    0x4f, 0xc2, 0xa0, 0x9f, 0xe8, 0xb6, 0x3e, 0xbc,
    0xf5, 0xae, 0x1c, 0x7d, 0xca, 0xf2, 0x65, 0xe0,
    0x5a, 0x33, 0xfc, 0x3f, 0x49, 0x6d, 0xa1, 0x83,
    0xcb, 0x4d, 0x0f, 0xf4, 0x55, 0xec, 0x8a, 0x04,
    0x03, 0x7c, 0x86, 0x0e, 0xe4, 0xb4, 0x30, 0x99,
    0xa2, 0xdb, 0x43, 0x8d, 0x92, 0x11, 0xef, 0x8f,
    0x3a, 0xfb, 0xc6, 0x74, 0x0c, 0xb2, 0x97, 0x57,
    0x48, 0x69, 0x70, 0x15, 0x22, 0x67, 0x18, 0x24,
    0xf0, 0x82, 0xe7, 0x44, 0x9e, 0x53, 0x26, 0x81,
    0x1f, 0xc8, 0x63, 0x96, 0x00, 0x20, 0x4e, 0xe1,
    0x45, 0xd2, 0x75, 0x4c, 0x1d, 0x95, 0xbf, 0x52,
    0x05, 0xd8, 0x09, 0x1e, 0x73, 0x1a, 0xd5, 0xab,
    0x76, 0x08, 0xaa, 0xc4, 0x85, 0xb9, 0xd7, 0x8c,
    0x02, 0x58, 0x4b, 0x47, 0x89, 0xdd, 0x62, 0xa5,
    0xbb, 0x51, 0x10, 0xeb, 0x80, 0x79, 0xce, 0xfa,
    0x87, 0xf1, 0xcf, 0xfd, 0x6a, 0x07, 0x2d, 0x38,
    0xde, 0x41, 0x90, 0xb0, 0x25, 0x5e, 0xf7, 0xe6,
    0x42, 0x9a, 0xe9, 0xed, 0x6c, 0xc3, 0x88, 0xd1,
    0x6f, 0xd9, 0x7a, 0x0a, 0x68, 0x3d, 0x34, 0xcd,
    0xff, 0x28, 0x3b, 0x35, 0x01, 0xea, 0x39, 0x8b,
    0x14, 0x27, 0x56, 0xd4, 0x17, 0x72, 0xad, 0xa4,
    0xf9, 0x5c, 0x66, 0xa8, 0x6e, 0xba, 0xa7, 0x71,
};

unsigned char decrypt_char(unsigned int ch) {
    return table[(ch-17)%256];
}

unsigned char encrypt_char(unsigned char ch) {
    int i;
    for (i=0; i<256 && table[i] != ch; ++i)
        ;
    assert(table[i] == ch);
    return (i + 17) % 256;
}

void encrypt(void *_data, int len) {
    unsigned char *data = (unsigned char *)_data;
    int i;
    for (i=0; i<len; ++i) {
        data[i] = encrypt_char(data[i]);
    }
}

void decrypt(void *_data, int len) {
    unsigned char *data = (unsigned char *)_data;
    int i;
    for (i=0; i<len; ++i) {
        data[i] = decrypt_char(data[i]);
    }
}

int main(int argc, const char *argv[]) {
    assert(argc == 3);
    if (strcmp(argv[1], "int") == 0) { // int
        unsigned int value, orig;
        sscanf(argv[2], "%x", &value);
        orig = value;
        encrypt(&value, sizeof(value));
        printf("0x%08X /* 0x%08X */\n", value, orig);
        decrypt(&value, sizeof(value));
        /* printf("value=0x%08X\n", value); */
        assert(value == orig);
    }else{ // string
        int i;
        unsigned char *s = (unsigned char *)strdup(argv[2]);
        size_t len = strlen((char *)s);
        encrypt(s, len);
        printf("[%d] = \"", len);
        for (i=0; i<len; ++i) {
            printf("\\x%02x", s[i]);
        }
        printf("\" /* \"%s\" */", argv[2]);

        decrypt(s, len);
        s[len] = 0;

        printf("\n%s\n", s);
        assert(strcmp((char *)s, argv[2]) == 0);
    }
    return 0;
}
