# work
>DirectX12とC++によるシンプルなゲームアプリケーション  
>制作期間：3か月  
>(C)2024 Motoya Kimura
## 使用技術一覧
### 言語・ライブラリ
<img src="https://img.shields.io/badge/-c++-000000.svg?logo=c%2B%2B&style=for-the-badge">  <img src="https://img.shields.io/badge/-DirectX12-000000.svg?&style=for-the-badge">  <img src="https://img.shields.io/badge/-assimp-000000.svg?&style=for-the-badge">
### ソフトウェア
<img src="https://img.shields.io/badge/-Blender4.2-000000.svg?&style=for-the-badge">  <img src="https://img.shields.io/badge/-Adobe_Illustrator_2025-000000.svg?&style=for-the-badge">  <img src="https://img.shields.io/badge/-CLIP_STUDIO-000000.svg?&style=for-the-badge">  
<br>
## プロジェクトの概要
本プロジェクトはDirectX12とC++を使用したゲームアプリケーションです。  
制限時間内に階段を上り、バイトの遅刻を回避させる簡単なゲームとなっております。  
<br>
本ゲームは、私がバイトに遅刻しそうになった時の感覚を、ゲーム性と交えて皆さんに体験してもらったら楽しいのではないか、という思いから作りました。
是非皆さんもスリルと達成感を味わってみてください。  
<br>
### 注意したこと
ゲームを作るうえでまず注意したことは、ゲームとして違和感のないものを作ることです。
例えば、ウィンドウを動かしたときにプレイヤーの挙動がおかしくなる、カメラがモデルの中に入ってしまう、というようなことが起きると、
プレイしている人の注意がそがれてしまい、ゲームが楽しめなくなってしまいます。そのような事態を防ぐために、例で述べたことだけでなく、レンダリングや衝突判定、モーションの挙動など、
いろいろな違和感を修正するところに注力しました。  
<br>
そして次に、プレイに没頭できるような工夫を凝らすことです。違和感のないものだけ作れてもゲーム性がないと面白くないので、本ゲームではシーンのはじめや移り変わりの部分でエフェクト効果を入れました。
例えば、タイトルシーンからゲームシーンへ移るときは幕が閉じ、ゲームが開始するときに幕が上がります。さらに、幕が上がる間に画面にノイズ効果を挟むことで見て楽しめるような作りになっています。
また、本ゲームでは時間制限を設けることでより緊張感のあるものに仕上げました。
プレイ画面では、右上にタイマーを用意し、時間が経過していく様子が視覚的に見えるようになっています。さらに、残り時間が10秒を切ると、タイマーの文字が赤く変化します。このような工夫により
ゲームへの没入度を高めることに努めました。  
<br>
### 大変だったこと
一番大変だったことはDirectX12を理解し、レンダリング結果を表示させることです。はじめはバッファー、ビュー、ディスクリプタヒープ、ルートシグネチャ、パイプラインステートなどのレンダリングに必須なものの知識が全然なく、
うまく画面に表示することができませんでした。特に、GPUのリソースバインドがうまくいっておらず、ディスクリプタヒープとの対応関係を何度も確認したり、ルートシグネチャの設定を見直したりするところに時間を取られていました。
そのような問題を改善するために、対応関係を手動で数字を打ち込んでいたのを変数を使うことで、修正の際にミスが起こる可能性を抑えることができ、神経を切らして考える手間を省くことができました。これにより、作業の効率性も上がり、
作業の進行度を早めることができました。  
<br>
### 特にみてほしいところ
特にみてほしい部分はプログラムコードです。作業の効率化を図るため、C++でクラスを用いたプログラムに仕上げています。
また、なるべく他者が見ても理解できるようにわかりやすいネーミングをつけることに気を付けました。
また、プログラムには関係ない点でいえば、ゲーム内に登場する文字やタイトルロゴ、またタイマーやスカイスフィアのテクスチャも工夫を凝らしているので注意してみていただけると幸いです。  
<br>
## ゲームタイトル
「やばい！バイトの時間ダッ」  
<br>
## 作品動画
YouTubeで限定公開  
※再配布の関係上、PMXモデルとモーションはGitHub上のものとは異なることがあります。  
※音楽機能はまだ実装しておりません、今後実装していく予定です。  
<br>
## インストール
GitHubのworkプロジェクト内のCodeボタンからzipファイルをダウンロードし、圧縮ファイルを解凍してください。  
圧縮ファイルを解凍すると以下のようなファイルとディレクトリが生成されます。  
<br>
□work-main  
├─assimp-vc143-mtd.dll  
├─main.cpp  
├─SceneManager.h  
├─ShaderHeader.hlsli  
├─Texture.cpp  
├─Texture.h  
├─TitlePeraHeader.hlsli  
├─TitlePeraPixelShader.cso  
├─TitlePeraPixelShader.hlsl  
├─TitlePeraVertexShader.cso  
├─TitlePeraVertexShader.hlsl  
├─TitleScene.cpp  
├─TitleScene.h  
├─VertexShader.cso  
├─VertexShader.hlsl  
├─VMD.cpp  
├─VMD.h  
├─work.exe  
├─work.sln  
├─work.vcxproj  
├─work.vcxproj.filters  
├─Wrapper.cpp  
├─Wrapper.h  
├─GameOverPeraPixelShader.hlsl  
├─GameOverPeraVertexShader.cso  
├─GameOverPeraVertexShader.hlsl  
├─GameOverScene.cpp  
├─GameOverScene.h  
├─GameScene.cpp  
├─GameScene.h  
├─HowToPlayPeraHeader.hlsli  
├─HowToPlayPeraPixelShader.cso  
├─HowToPlayPeraPixelShader.hlsl  
├─HowToPlayPeraVertexShader.cso  
├─HowToPlayPeraVertexShader.hlsl  
├─HowToPlayScene.cpp  
├─HowToPlayScene.h  
├─IKSolver.cpp  
├─IKSolver.h  
├─Keyboard.cpp  
├─Keyboard.h  
├─LICENSE.txt  
├─MenuPeraHeader.hlsli  
├─MenuPeraPixelShader.cso  
├─MenuPeraPixelShader.hlsl  
├─MenuPeraVertexShader.cso  
├─MenuPeraVertexShader.hlsl  
├─MenuScene.cpp  
├─MenuScene.h  
├─Model.cpp  
├─Model.h  
├─ModelRenderer.cpp  
├─ModelRenderer.h  
├─Morph.cpp  
├─Morph.h  
├─MorphManager.cpp  
├─MorphManager.h  
├─NodeManager.cpp  
├─NodeManager.h  
├─Pera.cpp  
├─Pera.h  
├─PeraPixelShader.cso  
├─PeraPixelShader.hlsl  
├─PeraRenderer.cpp  
├─PeraRenderer.h  
├─PeraShaderHeader.hlsli  
├─PeraVertexShader.cso  
├─PeraVertexShader.hlsl  
├─PixelShader.cso  
├─PixelShader.hlsl  
├─PmxModel.cpp  
├─PmxModel.h  
├─Renderer.cpp  
├─Renderer.h  
├─RSM.cpp  
├─RSM.h  
├─Scene.cpp  
├─Scene.h  
├─SceneManager.cpp  
├─SSAO.cpp  
├─SSAO.h  
├─SSAOPixelShader.cso  
├─SSAOPixelShader.hlsl  
├─SSAOShaderHeader.hlsli  
├─SSAOVertexShader.cso  
├─SSAOVertexShader.hlsl  
├─.gitattributes  
├─.gitignore  
├─Application.cpp  
├─Application.h  
├─AssimpModel.cpp  
├─AssimpModel.h  
├─BoneNode.cpp  
├─BoneNode.h  
├─Button.cpp  
├─Button.h  
├─Camera.cpp  
├─Camera.h  
├─ClearPeraHeader.hlsli  
├─ClearPeraPixelShader.cso  
├─ClearPeraPixelShader.hlsl  
├─ClearPeraVertexShader.cso  
├─ClearPeraVertexShader.hlsl  
├─ClearScene.cpp  
├─ClearScene.h  
├─GameOverPeraHeader.hlsli  
├─GameOverPeraPixelShader.cso  
├─□x64  
├─□texture  
├─□vmdData  
├─□work  
├─□modelData  
├─□assimp  
└─□DirectXTex-main  
<br>
## アンインストール
解凍したディレクトリごと削除してください。  
<br>
## 起動方法
work.exeを実行してください。  
<br>
## シーンの構成
本アプリケーションでは以下のシーンがございます。  
<br>
・タイトルシーン  
・遊び方シーン  
・ゲームシーン  
・メニューシーン  
・ゲームオーバーシーン  
・クリアシーン  

## 基本操作  
### 全画面表示
ALT + Enter  
<br>
ゲーム画面では以下のような操作が可能です。  
<br>
### キャラ移動：キーボード
・W   ：前  
・A   ：左  
・S   ：後ろ  
・D   ：右  
・SPACE  ：ジャンプ  
・Esc  ：メニュー画面へ（メニュー画面ではEscでゲーム画面に戻ることができます。）  
・Pause  ：ポーズ  
<br>
### 視点移動
・マウス操作  
<br>

タイトル、遊び方、メニュー、ゲームオーバー、クリア画面では以下の操作が可能です。
### 選択
・マウス左クリック  
<br>


## 環境
### 動作環境
OS         ：Windows 11 Home 64bit  
バージョン  ：24H2  
API        ：DirectX12  
CPU        ：Intel(R) Core(TM) i5-14400F  
GPU        ：NVIDIA GeForce RTX 4060  
メモリ      ：16.0 GB  
<br>
### 開発環境
名前        ：Visual Studio 2022、Debug x64  
バージョン  ：17.12.3  
<br>
## ライセンス
このプロジェクトはMITライセンスのもとで公開されています。  
<br>
## クレジット
### ライブラリ
〇DirectX  
Copyright (c) Microsoft Corporation.  
〇assimp  
Copyright (c) 2006-2021, assimp team
All rights reserved.  
<br>
### PMXモデル
〇八重沢なとり
(C)Appland, Inc.  
<br>
### 参考文献
〇「DirectX 12の魔導書」（川野 竜一 著、翔泳社 刊）  
〇「HLSL シェーダーの魔導書」（清原 隆行 著、翔泳社 刊）  
〇「Direct3D 12 ゲームグラフィックス実践ガイド（Pocol 著、技術評論社 刊）  
〇「Reflective Shadow Maps」[Dachsbacher et al. 2005]  
https://dl.acm.org/doi/10.1145/1053427.1053460  
<br>
### 参考サイト
〇「C言語で簡単なシーン管理システム」（GAMEWORKS LAB）https://gameworkslab.jp/2018/10/05/c%E8%A8%80%E8%AA%9E%E3%81%A7%E7%B0%A1%E5%8D%98%E3%81%AA%E3%82%B7%E3%83%BC%E3%83%B3%E7%AE%A1%E7%90%86%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0/  
〇「DirectX12でミクさんを躍らせてみよう」（@dpals39)  
https://qiita.com/dpals39/items/f296a458d4905dfa7341  
<br>



## 制作者
木村元哉
