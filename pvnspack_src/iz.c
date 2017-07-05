
#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"               /* /usr/local/include/zlib.h */

#define OUTBUFSIZ (512*1024)

typedef unsigned char u_char;

unsigned zlbencode( u_char *inptr, unsigned size, u_char *code )
{
	z_stream z;                     /* ���C�u�����Ƃ��Ƃ肷�邽�߂̍\���� */
    int status;

    /* ���ׂẴ������Ǘ������C�u�����ɔC���� */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
#if 0
    /* ������ */
    /* ��2�����͈��k�̓x���B0�`9 �͈̔͂̐����ŁC0 �͖����k */
    /* Z_DEFAULT_COMPRESSION (= 6) ���W�� */
    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
#else
    if (deflateInit2_(&z, 6, Z_DEFLATED, 10, /*DEF_MEM_LEVEL=*/8,
			 Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream)) != Z_OK) {
#endif

        fprintf(stderr, "deflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.avail_in = 0;		/* ���̓o�b�t�@���̃f�[�^�̃o�C�g�� */
    z.next_in = inptr;		/* ���̓|�C���^����̓o�b�t�@�̐擪�� */
    z.avail_in = size;		/* �f�[�^��ǂݍ��� */
    z.next_out = code;		/* �o�̓|�C���^ */
    z.avail_out = OUTBUFSIZ;	/* �o�̓o�b�t�@�̃T�C�Y */

    /* �ʏ�� deflate() �̑�2������ Z_NO_FLUSH �ɂ��ČĂяo�� */

    while (1) {
        status = deflate(&z, Z_FINISH); /* ���k���� */
        if (status == Z_STREAM_END) break; /* ���� */
        if (status != Z_OK) {   /* �G���[ */
            fprintf(stderr, "deflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
    }

    /* ��n�� */
    if (deflateEnd(&z) != Z_OK) {
        fprintf(stderr, "deflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

	return OUTBUFSIZ - z.avail_out;
}

unsigned zlbdecode( u_char *inptr, unsigned size, u_char *data )
{
	z_stream z;                     /* ���C�u�����Ƃ��Ƃ肷�邽�߂̍\���� */
    int status;

    /* ���ׂẴ������Ǘ������C�u�����ɔC���� */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* ������ */
    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.next_in = inptr;  /* ���̓|�C���^�����ɖ߂� */
    z.avail_in = size; /* �f�[�^��ǂ� */
    z.next_out = data;        /* �o�̓|�C���^ */
    z.avail_out = OUTBUFSIZ;    /* �o�̓o�b�t�@�c�� */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        status = inflate(&z, Z_NO_FLUSH); /* �W�J */
        if (status == Z_STREAM_END) break; /* ���� */
        if (status != Z_OK) {   /* �G���[ */
            fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
    }

    /* ��n�� */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

	return OUTBUFSIZ - z.avail_out;
}


