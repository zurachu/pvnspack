// 圧縮ファイルパック for P/VNS
// (c)2003-2005 てとら★ぽっと
// Thanks to MIO.H, Naoyuki Sawa, Hiroshi Makabe
// ppackで圧縮したファイルを、元のファイル名扱いでfpkにパックします
//
// 2004/08/19 最大パック数を128→256へ
//            作成失敗時に一時ファイルを消去するように
// 2005/04/09 最大パック数を256→1024へ

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_MAX 1024	// 最大パック可能ファイル数

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
	printf( "圧縮ファイルパック for P/VNS\n" );
	printf( "usage: > pvnspack filename1 ...\n" );
}

// ppackで圧縮
// dstPath - 圧縮先パス
// srcPath - 圧縮元パス
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

	// 引数で指定されたファイルを圧縮
	for( i = 1; i < argc; i++ ) {
		if( !strncmp( argv[i], "-o", 2 ) ) {	// -oオプション（出力ファイル名）の場合
			if( strlen(oFname) ) {
				printf( "出力ファイル名を指定する必要はありません\n" );
				remove_temp();
				return;
			}
			_splitpath( argv[i]+2, NULL, NULL, oFname, oExt );
		}
		else {
			_splitpath( argv[i], NULL, NULL, iFname, iExt );
			_makepath( path, NULL, NULL, iFname, iExt );
			if( strlen(path) > 16 - 1 ) {
				printf( "%s:ファイル名が長すぎます\n", argv[i] );
				remove_temp();
				return;
			}
			if( ( in = fopen( argv[i], "rb" ) ) == NULL ) {
				printf( "%s:ファイルが見つかりません\n", argv[i] );
				remove_temp();
				return;
			}
			fclose( in );
			for( j = 0; j < i - 1; j++ ) {
				if( !strcmp( path, fInfo[j].fname ) ) {
					printf( "%s:ファイル名が重複しています\n", argv[i] );
					remove_temp();
					return;
				}
			}
			if( !strcmp( iExt, ".pvn" ) ) {	// pvaファイルはpvnファイルと同名に
				if( pvn ) {
					printf( "%s:pvnファイルが重複しています\n", argv[i] );
					remove_temp();
					return;
				}
				if( strlen(oFname) ) {
					printf( "出力ファイル名を指定する必要はありません\n" );
					remove_temp();
					return;
				}
				strcpy( oFname, iFname );
				strcpy( oExt, ".pva" );
			}
			strcpy( fInfo[fpInfo.famount].fname, path );	// 
			strcat( path, "_" );				// 圧縮先ファイルの拡張子に"_"を付加
			compress( path, argv[i] );			// ppack圧縮
			fpInfo.famount++;
		}
	}
	if( !strlen(oFname) ) { strcpy( oFname, "pvnspack" ); }

	// fpkファイルヘッダ準備
	offset = sizeof(FILE_PAC_INFO) + sizeof(FILE_INFO)*fpInfo.famount;
	for( i = 0; i < fpInfo.famount; i++ ) {
		fInfo[i].offset = offset;
		// 圧縮ファイルのサイズ取得
		strcpy( path, fInfo[i].fname );
		strcat( path, "_" );
		if( ( in = fopen( path, "rb" ) ) == NULL ) {
			printf( "%s:ファイルを開けません\n", path );
			remove_temp();
			return;
		}
		fseek( in, 0, SEEK_END );
		fInfo[i].size = ftell( in );
		fclose( in );
		offset += fInfo[i].size;
		while( offset % 4 ) { offset++; }	// オフセットを4の倍数に
	}

	// ファイル書き出し
	_makepath( path, NULL, NULL, oFname, oExt );	// fpkファイル名を生成
	if( ( out = fopen( path, "wb" ) ) == NULL ) {
		printf( "%s:ファイルを作成できません\n", path );
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
			printf( "%s:ファイルを開けません\n", path );
			remove_temp();
			return;
		}
		for( k = 0; k < fInfo[i].size; k++ ) {
			fputc( fgetc( in ), out );
		}
		while( fInfo[i].size % 4 ) {	// ずらしたオフセット分だけNULL文字で埋める
			fputc( '\0', out );
			fInfo[i].size++;
		}
		fclose( in );
		remove( path );
	}
	fclose( out );

	printf( "%s%sを出力しました\n", oFname, oExt );
}
