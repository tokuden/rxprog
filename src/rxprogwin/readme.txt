                    SP6JTAG ソフトウェア・簡易マニュアル
                       (C) Copyright 2009 特殊電子回路

■本ソフトウェアの概要
SP6JTAGソフトウェアは、DOSプロンプトからSpartan-6に書き込みするためのソフトウェアです。

●対応デバイス
XILINX: Spartan-6

●可能な操作
  ・書き込み
  ・消去(シャットダウン)

●準備
本ソフトウェアを使用する前にNP1027 特電Spartan-6評価ボードをご用意ください。
sp6jtag.exeとsystem.bixを任意のフォルダにコピーしてください。

■操作方法
●コマンド体系
SP6JTAG は、DOSプロンプトから以下のように入力して起動します。

sp6jtag -command [bitファイル名]

書き込みを行う場合には、データファイル名を指定してください。
XILINX FPGAの場合は拡張子は.bitです。

●コマンド一覧
コマンドには以下のものがあります
  -detect  JTAGチェーン上のデバイスを自動認識します
  -bypass  そのデバイスに対しては何も操作しません
  -auto    消去・書き込み・ベリファイを行います
  -write   デバイスにファイルを書き込みます
  -erase   デバイスを消去します

●デバイスの自動認識を行う
jwriter -detect
と入力すると、デバイスの自動認識が行えます。
JTAGチェーンの動作チェックなどにお使い下さい。

●書き込み方法
JTAGチェーンに１個のデバイスが接続されていて、そのデバイスに書き込む場合は
  jwriter -auto データファイル名
と記述します。

■記述例
・Spartan-6基板上のFPGAに、main.bitを書き込む
sp6jtag -auto main.bit

・Spartan-6基板上のFPGAが認識できるかどうか試す
sp6jtag -detect
