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

#include "rc4.h"

static void swap(unsigned char *s, unsigned int i, unsigned int j) {
    unsigned char temp = s[i];
    s[i] = s[j];
    s[j] = temp;
}

/* KSA */
void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data) {
    int i, j;

    for (i = 0; i < 256; i++)
        key->S[i] = i;

    for (i = j = 0; i < 256; i++) {
        j = (j + data[i % len] + key->S[i]) & 255;
        swap(key->S, i, j);
    }

    key->i = key->j = 0;
}

/* PRGA */
static unsigned char rc4_output(RC4_KEY *key) {
    key->i = (key->i + 1) & 255;
    key->j = (key->j + key->S[key->i]) & 255;

    swap(key->S, key->i, key->j);

    return key->S[(key->S[key->i] + key->S[key->j]) & 255];
}

void RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata, unsigned char *outdata) {
    unsigned long n;

    for (n=0; n<len; ++n) {
        outdata[n] = indata[n] ^ rc4_output(key);
    }
}

#if 0
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rc4.c"

void test(int key_len, const unsigned char *key, int data_len, const unsigned char *data) {
    RC4_KEY rc4_key;
    unsigned char out[1024];
    int i;

    assert(data_len < sizeof(out));
    RC4_set_key(&rc4_key, key_len, key);
    RC4(&rc4_key, data_len, data, out);

    for (i=0; i<data_len; ++i) {
        printf("%02X", out[i]);
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
    test(strlen(argv[1]), argv[1], strlen(argv[2]), argv[2]);
    return 0;
}
#endif
