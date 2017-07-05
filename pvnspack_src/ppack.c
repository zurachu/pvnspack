
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              System Programs
//
//
// PIECE TOOLS : ppack : Ver 1.00
//
// Copyright (C)2001 AUQAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
//
// Coded by MIO.H (OeRSTED)
//
// Comments:
//
//	PPACK ファイル・パッケージャ
//
//	生成された srfファイル(Piece用プログラム)をPiece格納形式に変換します。
//	圧縮には zlib を使います。
//	オプションで単なるバイナリ・データをアドレスを指定してパックする機能
//	もあります。
//
//  v1.00 2001.11.09 MIO.H
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define __PCEKN__
#include "piece.h"

#pragma comment( lib, "zlib.lib" )


static char *infname;
static char *outfname;

static unsigned long bin_adrs = 0;

// crc.c
unsigned long calcrc( unsigned char *c, unsigned n );
void calcrc_init( void );


static char usage[] =
	"PPACK ... PIECE File Packager ver1.00 by MIO.H\n"
	"usage:\n"
	"  for encode PPACK -e [options] InFile -oPackFile\n"
	"  for decode PPACK -d [options] PackFile -oOutputFile\n"
	"  for test   PPACK -t [options] PackFile\n"
	"options:\n"
	"   -v      : verbose\n"
	"   -bhhhh  : BinbaryMode\n"
	"   -rnnn   : force store name\n"
	"   -k      : keywait\n"
	"           : ------ only encode\n"
	"   -mN     : control method\n"
	"   -p<path>: data path\n"
	"   -n<name>: caption\n"
	"   -i<file>: icon image file\n"
	;

static char cmd_mode;

int fverbose;
int fwait;

int fmethod = 0;

char *store_name;
char DataPath[_MAX_PATH];
char FileName[25];
char IconName[256];

unsigned char databuff[1024*1024];
unsigned long topadrs;
unsigned long endadrs;

#define OUTBUFSIZ sizeof(databuff)


static unsigned long read_mlong( unsigned char *mem )
{
	return (((((mem[0]<<8)+mem[1])<<8)+mem[2])<<8)+mem[3];
}

static unsigned short read_mshort( unsigned char *mem )
{
	return (mem[0]<<8)+mem[1];
}

int readfile_srf( FILE *fp )
{
	unsigned long last = -1L, ent = -1L;
	unsigned char tmp[64];
	unsigned long next;
	int m = 100;

	fseek(fp, 0, SEEK_SET);

	if ( fread((char *)tmp, 1, 16, fp) != 16 ) return 1;

	if ( (read_mshort(tmp) | 8)!=0x000e ) return 1;

	printf( "\n" );

	next = read_mlong( tmp+8 );

	while ( next ) {
		unsigned long len;

		//printf( "[%x]", next);
		fseek(fp, next, SEEK_SET);

		if ( fread(tmp, 1, 44, fp) != 44 ) break;

		next = read_mlong( tmp );

		len = read_mlong( tmp+38 );

		if ( len ) {
			unsigned long adr = read_mlong( tmp+10 );
			unsigned long pos = read_mlong( tmp+34 );
			if ( pos ) {
				//printf( "  %06x %06x %06x\n", adr, len, pos );
				printf( "  %06x-%06x\n", adr, adr+len-1 );
				fseek(fp, pos, SEEK_SET);
				if ( !topadrs ) topadrs = adr;
				if ( adr < topadrs ) {
					printf( "address back\n" );
					return 1;
				}
				if ( adr + len - topadrs >= sizeof(databuff) ) {
					printf( "buff over\n" );
					return 1;
				}
				fread( databuff + adr - topadrs, 1, len, fp );
				adr += len;
				if ( endadrs < adr ) endadrs = adr;
			}
		}
	}

	//if ( ent >= 0 ) printf( "Entry address = %06lx\n", ent );
	return 0;
}





unsigned zlbencode( unsigned char *inptr, unsigned size, unsigned char *code );
unsigned zlbdecode( unsigned char *inptr, unsigned size, unsigned char *data );

void arcs( void )
{
	unsigned len = endadrs - topadrs;
	unsigned char *orgbuff = malloc( len );

	memcpy( orgbuff, databuff, len );

	calcrc_init();
	((unsigned long *)databuff)[0] = len;
	((unsigned long *)databuff)[1] = calcrc( orgbuff, len );

	len = zlbencode( orgbuff, len, databuff+8 );

	free( orgbuff );

	endadrs = topadrs + 8 + len;
}


int readfile( char *infile )
{
	int err = 1;
	FILE *fp;

	memset( databuff, 0xff, sizeof(databuff) );
	topadrs = 0;
	endadrs = 0;

	fp = fopen( infile, "rb" );
	if ( fp != NULL ) {
		if ( !bin_adrs ) {
			err = readfile_srf( fp );
		}
		else {
			topadrs = bin_adrs;
			endadrs = topadrs + fread( databuff, 1, sizeof(databuff), fp );
			err = 0;
		}
		fclose( fp );
	}

	return err;
}


void copyicon( FILE *fp )
{
	FILE *ifp = fopen( IconName, "rb" );

	if ( ifp ) {
		unsigned char icon[256];
		fread( icon, 1, sizeof(icon), ifp );
		fclose( ifp );
		fwrite( icon, 1, sizeof(icon), fp );
	}
}


void encode_pack( char *infile, char *outfile )
{
	FILE *fp = fopen( outfile, "wb" );

	if ( fp && !readfile( infile ) ) {
		pffsFileHEADER fh;
		int fnc = strlen( FileName );
		int icc = ( *IconName ) ? 256 : 0;
		fnc = ( fnc + 1 + 3 ) & ~3;
		arcs();
		calcrc_init();
		printf( "%x - %x\n", topadrs, endadrs );
		fh.mark = 'X';
		fh.type = PFFS_FT_EXE2;
		fh.ofs_data = sizeof(fh)+fnc+icc;
		fh.ofs_name = sizeof(fh);
		fh.ofs_icon = icc ? sizeof(fh)+fnc : 0;
		fh.top_adrs = topadrs;
		fh.length = endadrs - topadrs;
		fh.crc32 = calcrc( databuff, endadrs - topadrs );
		fwrite( &fh, 1, sizeof(fh), fp );
		fwrite( FileName, 1, fnc, fp );
		if ( icc ) copyicon( fp );
		fwrite( databuff, 1, fh.length, fp );
	}

	if ( fp ) fclose( fp );
}

void decode_pack( char *infile, char *outfile )
{
}









static void AdjPath( char *p )
{
	if ( *p ) {
		p += strlen( p );
		if ( p[-1] != '/' && p[-1] != '\\' ) {
			*p++ = '/';
			*p = '\0';
		}
	}
}


void params( char *p )
{
	if ( *p == '-' ) {
		switch ( p[1] ) {
			case 't':
			case 'd':
			case 'e':
				cmd_mode = p[1];
				break;
			case 'o':
				outfname = p+2;
				break;
			case 'v':
				fverbose = isdigit( p[2] ) ? atoi( p+2 ) : 1;
				break;
			case 'k':
				fwait = 1;
				break;
			case 'r':
				store_name = p+2;
				break;
			case 'm':
				fmethod |= (1 << atoi(p+2));
				break;
			case 'b':
				sscanf( p+2, "%lx", &bin_adrs );
				break;
			case 'p':
				strncpy( DataPath, p+2, sizeof(DataPath) );
				AdjPath( DataPath );
				break;
			case 'n':
				strncpy( FileName, p+2, sizeof(FileName) );
				break;
			case 'i':
				strncpy( IconName, p+2, sizeof(IconName) );
				break;
		}
	}
	else {
		infname = p;
	}
}



int main_cmd( int argc, char *argv[] )
{
	int i;

	for ( i = 1; i < argc; i++ ) params( argv[i] );

	switch ( cmd_mode ) {
		case 'd':
			if ( outfname == NULL ) outfname = "nul";
			decode_pack( infname, outfname );
			break;
		case 't':
			decode_pack( infname, NULL );
			break;
		case 'e':
			if ( outfname == NULL ) outfname = "tmp.out";
			encode_pack( infname, outfname );
			break;
		default:
			printf( usage );
			exit( 1 );
	}

	return 0;
}



