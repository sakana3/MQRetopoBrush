# MQRetopoBrush
メタセコイア向けリトポプラグイン

# Download
[64bit版](https://github.com/sakana3/MQRetopoBrush/releases/download/v1.0.0/MQRetopoBrush_x64.zip)  
[32bit版](https://github.com/sakana3/MQRetopoBrush/releases/download/v1.0.0/MQRetopoBrush_win32.zip)

# How to Use  

- 編集対象は現在編集中のオブジェクトのみ  
- LMBでなでるとスムース。
- スナップがFaceになっているとLockがかかっているオブジェクトにスナップ  
- ブラシには次のタイプがあります  

|名前        |説明          |
|:---------:|:------------:|
|Smooth      |トポロジを滑らかにします。ボーダーエッジ（オープンエッジ）は含みません          |
|SmoothWithBorder      |トポロジを滑らかにします。ボーダーエッジ（オープンエッジ）を含みます         |
|Tweak      |調整ブラシ          |
|Shrink Wrap      |スナップのみを行います          |

- 各ブラシはCtrl、Shift,Altに自由にアサインできます。
- スナップタイプには2種類あります。

|名前        |説明          |
|:---------:|:------------:|
|Normal     |法線方向にスナップします          |
|Closest    |最も近い点にスナップします         |

基本的にはClosestが綺麗にスムースがかかりますがNoramlの方がスナップ先の形状に沿う傾向があります。お好みで
