
                               JNetHack
                                   
         Copyright 1993-1997  I.Numata, S.Miyashita, N.Hamada
                   2002-2003  K.Shirakata


                             INSTALL NOTE
                                   
    まず，ドキュメントに目を通せ．全てはそれからだ．
                                -- K.E. Aganimot, Science & Technology より

    必要な情報がすべてドキュメントに書いてあるとはかぎらない．
                                -- N.N. Adamah, the Tao of NetHack より

0)

JNetHackは，NetHackに対するソースパッチと，各機種用のバイナリの二種類
の形態で配布されています．ソースからコンパイルするのは少々面倒ですの
で，特に理由がなければバイナリを利用することをお勧めします．

JNetHackをコンパイルする場合は，まずオリジナルの英語版を一度コンパイ
ルしてみることを強くお勧めします．その方が間違いや問題の発見がしやす
くなるでしょう．NetHackのコンパイルは決して単純ではありません，注意深
く一歩一歩，問題を解決して行くのが大切です．

次のファイルにはいかにして Nethack を構築するかが書かれています．

        sys/msdos/Install.dos (MS-DOS)
        sys/unix/Install.unx (UNIX)
        sys/winnt/Install.nt (MS Windows)

より正確にコンパイルするため熟読することをお勧めします．また次のファ
イルには JNetHackのコンパイル方法が書かれています．詳しくはそちらの
ファイルを参考にしてください．

	japanese/Install.dos
	japanese/Install.lnx
	japanese/Install.nt
	japanese/Install.unx

                                NOTICE

    注意書きには時として有益な情報もあるが
    その殆どが無益なくだらない情報だ．
                                        -- I. Atamun, タバーンな人生より

1)環境設定について

UNIXでの環境設定ファイルは .nethackrcより先に .jnethackrcを見に行きま
す．このため英語版のと共存が可能です．また新たに追加された環境変数と
して，`kcode'があります．値は jis,sjis,eucを取ります．

OPTIONS=kcode:jis

のようにして，環境設定ファイルの一番最初に設定してください．以後の環境
の漢字コードは設定した値になります．

2)
JNetHackで追加されたコマンドとして，「`」があります．キーを押す毎に
オリジナルモードと日本語モードが切りかわります．オリジナルモードでは，
'i','I',';','/'で表示される物体名や怪物名が英語になります．またアイテ
ムは識別しないとオリジナルの名前になりません．


                           ABOUT BUG REPORT

    特定できるバグは，たいしたバグではない．何がバグかわからないの
    が問題なのだ．
                                -- Otihorih, Creatures in Palace より


3.4 対応は http://jnethack.sourceforge.jp/ で行っています．
コメント等はこちらに頂けるとありがたいです．
