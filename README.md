# pvnspack
P/ECE VISUALNOVEL Scripter で使用している独自形式の圧縮ファイルパックツールおよびP/ECE側展開関数

 
## pvnspackの使用方法
コマンドプロンプトで
```
pvnspack ファイル名1 ファイル名2 …
```
と実行すると、各ファイルを圧縮した後、圧縮前のファイル名扱いでパックし、
カレントディレクトリにpvnspack.tmpという圧縮パックファイルを作成します。
ファイル名指定にはワイルドカードも使用できます。

オプションとして、
```
pvnspack -o出力ファイル名 ファイル名1 ファイル名2 …
```
とすると、出力ファイル名を指定できます。

また、格納するファイルに拡張子".pvn"（P/VNSシナリオファイル）があった場合、
パックファイルのファイル名（拡張子まで）は自動的にシナリオファイルと同じになります（sample.pvnならsample.pva）。
この場合、-oオプションを指定しているとエラーになりますので注意して下さい。

パックできるファイルの最大数を128→256に増やしました。もっと必要な場合は「pvnspack.c」の`#define FILE_MAX 256`を増やして下さい。
 
## 展開関数について
```
int unpack( char *pArcData, char *pOutBuff );
```
メモリに配置された圧縮データを展開します。展開先メモリはあらかじめ確保しておく必要があります。
※この関数は内部で呼び出しているので直接使用する必要はありません。
（P/ECE HAND BOOK Vol.2収録『緋色の霧』のソースを改変）

```
BOOL ppack_checkHeader( unsigned char* arcData );
```
メモリに配置された圧縮データのヘッダが正しいかどうか（先頭から4バイトのデータのlong型キャストが0x1c0258か）を返します。
※この関数は内部で呼び出しているので直接使用する必要はありません。

```
long ppack_getExpandSize( unsigned char* arcData );
```
メモリに配置された圧縮データの展開後のサイズ（先頭から001Ch=28バイト目のデータをlong型にキャスト）を返します。
※この関数は内部で呼び出しているので直接使用する必要はありません。

```
unsigned char* ppack_heapUnpack( unsigned char* arcData );
```
メモリに配置された圧縮データを、展開用にヒープ領域を確保して、その領域に展開します。
圧縮データをメモリに配置する際はpceFileReadSct()関数などを使って下さい。

【例】`data = heapUnpack( arcData );`

```
unsigned char* ppack_findPackDataEx( char *fpkName, char *fName );
```
フラッシュメモリに配置されている、『pvnspack』で作成した圧縮パックファイルから、
指定されたファイル名のファイルを、展開用ヒープ領域を確保して、その領域に展開します。
P/ECEの標準パックファイル内のファイルをヒープ領域に読み込むこともできます。

【例】`data = ppack_findPackDataEx( "packfile.pva", "arc.txt" );`

※ppack_heapUnpack()、ppack_findPackDataEx()の両方について、ヒープ領域を確保して展開するので、展開したファイルが不要になった際に
```
pceHeapFree( data );
```
で必ずヒープ領域を開放して下さい。
 
## 展開関数の組み込み方法
\usr\PIECE\sysdev\ku\から、inflate.c、inflate.h、piecezl.hを、作成中プログラムのフォルダにコピーしてきます。
そして関数を使用するプログラムで
```
#include "pvnsunpk.h"
```
と宣言して下さい。
コンパイル時は、makefileの「OBJS=」行にpvnsunpk.o、inflate.oを追加して下さい。

### MMC対応カーネルでの使用方法
MMC対応カーネルで使用する、つまりMMC上にあるpvnspackで作成した圧縮パックファイルから`ppack_findPackDataEx()`関数でデータをヒープ上に展開する場合、
[MMC対応カーネルのソース](http://www2.plala.or.jp/madoka/Piece_ele/mmc/mmc.htm#DOWNLOAD)に含まれる「mmc_lib.h」「mmc_lib.lib」が必要です。
上記と同じ作業のほか、「pvnsunpk.c」で
```
// #define USE_MMC
```
のコメントを解除して下さい。また、メインプログラムで
```
#include "mmc_lib.h"
```
と宣言し、`pceAppInit()`関数内で`mmcInit()`関数、`pceAppExit()`関数内で`mmcExit()`関数をそれぞれ呼び出して下さい。
コンパイル時は、makefileに以下のようにルールを定めると良いでしょう。
```
mmc : $(OBJS)
$(LD) $(LDFLAGS) -e$(PRGNAME).srf $(OBJS) mmc_api.lib
```

## pvnspackのフォーマット
pvnspackで作成される圧縮パックファイルのフォーマットは、P/ECE標準のパックファイル（標準拡張子「.fpk」）のフォーマットと全く同じです。
但しP/ECE標準のパックファイルはヘッダが
```
DWORD_CHAR('F','P','A','K')→KAPF
```
になっているのに対し、pvnspackで作成される圧縮パックファイルはヘッダが
```
DWORD_CHAR('A','N','V','P')
```
になっています。P/ECE標準のパックファイルについては、tools\filepack\デコーダsrc\filepack.cを参照下さい。
  
## ppackで圧縮展開
ppackは、P/ECE用の実行ファイルである.pexファイルを作成するためのソフトですが、
その他の一般的なファイルも圧縮することができます。
P/ECE HAND BOOK Vol.1の開発Tips「データの圧縮展開を利用したい」に書かれている通り、
```
ppack -e -b1000 test.txt -otest.arc
```
とすると、test.txtがtest.arcに圧縮されます。

これを展開するためのソースコードも開発Tipsに掲載されていますが、使用方法の説明がいささか不親切です。
そんな中、先日P/ECE HAND BOOK 掲示板に、まかべひろし氏による書き込みがありました。
> 圧縮の方法につきましては、Vol.1に書かれている通り、ppack.exeを使用して行います。
> こうしてできあがったファイルを、「圧縮データ」としてメモリに配置します。
> メモリに置く方法ですが、pceFileReadSct() を用いるといいと思います。
> この圧縮データが置かれたアドレスが
> char *pArc;
> であるとします。圧縮したデータはそのままでは使えませんので、使用時に
> 展開することになります。展開したデータを入れるメモリを
> char pData[128];
> とします。128という数字は、元のデータサイズであり、各プログラムに応じて変更してください。
> 圧縮元のサイズより多くメモリを確保することに注意してください。
> 
> Vol.1に記載されているunpack()関数は、
> unpack( pArc, pData );
> のようにして使います。これにより、pArcに格納されている圧縮データを展開し、pDataに書き込みます。

これにより、圧縮データをメモリに配置し、展開後のデータサイズ分だけメモリを確保すれば、展開できるということが分かりました。
あらかじめ展開後のサイズが分かっているデータ（例えば、自分のプログラムでのみ読み込むようなデータ）についてはこれで対応できます。
しかし、展開後のサイズが分からなければ、メモリの確保が上手くいきません。

解決策として、たとえばP/ECE HAND BOOK Vol.2に収録されているゲームでは、独自のファイルパック形式を用いて、
圧縮ファイルと非圧縮ファイルを混在させたパックファイル内に各ファイルの展開後のサイズを保持しています。
圧縮ツールおよび展開関数のソースも添付されているので、参考にするといいでしょう。

さて、圧縮されたファイルをバイナリエディタで解析してみましょう。
他にもいくつかのファイルを圧縮して解析してみたところ、以下に示す特徴を読み取ることができました。

![解析結果](https://github.com/zurachu/pvnspack/blob/master/ppack.gif?raw=true)

これより、先頭から001Ch=28バイト目のデータをlong型にキャストすることで、
展開後のサイズを取得できることが分かります（`ppack_getExpandSize(unsigned char* arcData)`関数）。
つまりは、先ほどの例のような加工を施さなくても圧縮ファイル単体でも適切なサイズのメモリを確保して展開することが可能になります。

## 関連リンク
* [圧縮と展開](http://park17.wakwak.com/~hitode/piece/index.html#plz)（p/wareさん）
ワークメモリなしで高速に展開できるLZSS形式のP/ECE版、『plz』。
* [Autch.net](http://www.autch.net/)（Yui N.さん）
通常ファイル、LZSS圧縮ファイル、zlib圧縮ファイルをパックする独自ファイルパック形式『par』。
