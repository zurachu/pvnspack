// ���k�t�@�C���p�b�N for P/VNS
// (c)2003-2005 �ĂƂ灚�ۂ���
// Thanks to MIO.H, Naoyuki Sawa, Hiroshi Makabe
// ppack�ň��k�����t�@�C�����A���̃t�@�C����������fpk�Ƀp�b�N���܂�
//
// 2004/08/19 �ő�p�b�N����128��256��
//            �쐬���s���Ɉꎞ�t�@�C������������悤��
// 2005/04/09 �ő�p�b�N����256��1024��

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MAX 1024	// �ő�p�b�N�\�t�@�C����

extern int main_cmd( int argc, char *argv[] );	// ppack.c

typedef struct {
	unsigned long head;
	long          famount;
} FILE_PAC_INFO;
typedef struct {
	char          fname[16];
	unsigned long offset;
	unsigned long size;
} FILE_INFO;

FILE_PAC_INFO	fpInfo;
FILE_INFO		fInfo[FILE_MAX];

void usage()
{
	printf( "���k�t�@�C���p�b�N for P/VNS\n" );
	printf( "usage: > pvnspack filename1 ...\n" );
}

// ppack�ň��k
// dstPath - ���k��p�X
// srcPath - ���k���p�X
void compress( char* dstPath, char* srcPath )
{
	char  str[][128] = { "pvnspack", "-e", "-b1", "-o" };
	char* arg[5] = { str[0], str[1], str[2], srcPath, str[3] };

	strcat( str[3], dstPath );
	printf( "%s:", srcPath );
	main_cmd( 5, arg );
}

void remove_temp()
{
	long i;
	char path[_MAX_PATH];

	for(i = 0; i < fpInfo.famount; i++) {
		strcpy(path, fInfo[i].fname);
		strcat(path, "_");
		remove(path);
	}
}

void main( int argc, char* argv[] )
{
	FILE *in, *out;
	char path[_MAX_PATH];
	char iFname[_MAX_FNAME];
	char oFname[_MAX_FNAME];
	char iExt[_MAX_EXT];
	char oExt[_MAX_EXT];
	unsigned long offset;
	int i, j;
	unsigned long k;
	int pvn = 0;

	if( argc < 2 ) { usage(); return; }
	
	memset( &fInfo, 0, sizeof(FILE_INFO) * FILE_MAX );
	memcpy( &fpInfo.head, "PVNA", 4 );
	fpInfo.famount = 0;
	*oFname = '\0';

	// �����Ŏw�肳�ꂽ�t�@�C�������k
	for( i = 1; i < argc; i++ ) {
		if( !strncmp( argv[i], "-o", 2 ) ) {	// -o�I�v�V�����i�o�̓t�@�C�����j�̏ꍇ
			if( strlen(oFname) ) {
				printf( "�o�̓t�@�C�������w�肷��K�v�͂���܂���\n" );
				remove_temp();
				return;
			}
			_splitpath( argv[i]+2, NULL, NULL, oFname, oExt );
		}
		else {
			_splitpath( argv[i], NULL, NULL, iFname, iExt );
			_makepath( path, NULL, NULL, iFname, iExt );
			if( strlen(path) > 16 - 1 ) {
				printf( "%s:�t�@�C�������������܂�\n", argv[i] );
				remove_temp();
				return;
			}
			if( ( in = fopen( argv[i], "rb" ) ) == NULL ) {
				printf( "%s:�t�@�C����������܂���\n", argv[i] );
				remove_temp();
				return;
			}
			fclose( in );
			for( j = 0; j < i - 1; j++ ) {
				if( !strcmp( path, fInfo[j].fname ) ) {
					printf( "%s:�t�@�C�������d�����Ă��܂�\n", argv[i] );
					remove_temp();
					return;
				}
			}
			if( !strcmp( iExt, ".pvn" ) ) {	// pva�t�@�C����pvn�t�@�C���Ɠ�����
				if( pvn ) {
					printf( "%s:pvn�t�@�C�����d�����Ă��܂�\n", argv[i] );
					remove_temp();
					return;
				}
				if( strlen(oFname) ) {
					printf( "�o�̓t�@�C�������w�肷��K�v�͂���܂���\n" );
					remove_temp();
					return;
				}
				strcpy( oFname, iFname );
				strcpy( oExt, ".pva" );
			}
			strcpy( fInfo[fpInfo.famount].fname, path );	// 
			strcat( path, "_" );				// ���k��t�@�C���̊g���q��"_"��t��
			compress( path, argv[i] );			// ppack���k
			fpInfo.famount++;
		}
	}
	if( !strlen(oFname) ) { strcpy( oFname, "pvnspack" ); }

	// fpk�t�@�C���w�b�_����
	offset = sizeof(FILE_PAC_INFO) + sizeof(FILE_INFO)*fpInfo.famount;
	for( i = 0; i < fpInfo.famount; i++ ) {
		fInfo[i].offset = offset;
		// ���k�t�@�C���̃T�C�Y�擾
		strcpy( path, fInfo[i].fname );
		strcat( path, "_" );
		if( ( in = fopen( path, "rb" ) ) == NULL ) {
			printf( "%s:�t�@�C�����J���܂���\n", path );
			remove_temp();
			return;
		}
		fseek( in, 0, SEEK_END );
		fInfo[i].size = ftell( in );
		fclose( in );
		offset += fInfo[i].size;
		while( offset % 4 ) { offset++; }	// �I�t�Z�b�g��4�̔{����
	}

	// �t�@�C�������o��
	_makepath( path, NULL, NULL, oFname, oExt );	// fpk�t�@�C�����𐶐�
	if( ( out = fopen( path, "wb" ) ) == NULL ) {
		printf( "%s:�t�@�C�����쐬�ł��܂���\n", path );
		remove_temp();
		return;
	}
	fwrite( &fpInfo, 1, sizeof(FILE_PAC_INFO), out );
	for( i = 0; i < fpInfo.famount; i++ ) {
		fwrite( &fInfo[i], 1, sizeof(FILE_INFO), out );
	}
	for( i = 0; i < fpInfo.famount; i++ ) {
		strcpy( path, fInfo[i].fname );
		strcat( path, "_" );		
		if( ( in = fopen( path, "rb" ) ) == NULL ) {
			printf( "%s:�t�@�C�����J���܂���\n", path );
			remove_temp();
			return;
		}
		for( k = 0; k < fInfo[i].size; k++ ) {
			fputc( fgetc( in ), out );
		}
		while( fInfo[i].size % 4 ) {	// ���炵���I�t�Z�b�g������NULL�����Ŗ��߂�
			fputc( '\0', out );
			fInfo[i].size++;
		}
		fclose( in );
		remove( path );
	}
	fclose( out );

	printf( "%s%s���o�͂��܂���\n", oFname, oExt );
}
