# MQRetopoBrush
メタセコイア向けリトポサポートプラグイン

# Download
[64bit版](https://github.com/sakana3/MQRetopoBrush/releases/download/v1.0.0/MQRetopoBrush_x64.zip)  
[32bit版](https://github.com/sakana3/MQRetopoBrush/releases/download/v1.0.0/MQRetopoBrush_win32.zip)

# How to Use  

## 基本操作
- 編集対象は現在編集中のオブジェクトのみ  
- LMBでなでるとスムース。
- スナップが面になっているとLockがかかっているオブジェクトにスナップ  
- Shift + ホイールでブラシサイズ変更
- Ctrl + ホイールでブラシ強度変更

## ブラシタイプ

|名前        |説明          |
|:---------:|:------------|
|Smooth      |トポロジを滑らかにします。        |
|Smooth No Border      |トポロジを滑らかにします。ボーダーエッジ（オープンエッジ）は含みません          |
|Tweak      |調整ブラシ          |
|Shrink Wrap      |スナップのみを行います          |

各ブラシはCtrl、Shift,Altに自由にアサインできます。
## スナップタイプ

|名前        |説明          |
|:---------:|:------------|
|Closest    |最も近い点にスナップします         |
|Normal     |法線方向にスナップします          |

基本的にはClosestの方が綺麗にスムースがかかりますがNoramlの方がスナップ先の形状に沿う傾向があります。お好みで

## フォールオフタイプ

|名前        |説明          |
|:---------:|:------------|
|Curve　     |滑らかに減衰します          |
|Const    |減衰しません         |
|Liner    |線形に減衰します         |

Curveだけでいいと思います。
