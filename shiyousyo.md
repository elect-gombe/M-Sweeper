# えむすいーぱ
## ゲーム概要
フィールドの中にはいくつかの「えむ」が潜んでいる。その「えむ」を除去すべく「えむすいーぱ」は立ち上がった。フィールドの「えむ」以外のところをすべて開けばゲームクリアだ。「えむ」は開いてはならない。しかし、そのまわりに「えむ」がいくつあるのかの情報は数字で表示される。そのヒントとともに「えむフラグ」をたてて世界を平和に導け！

## 操作方法
### カーソル移動
```
789 ↖↑↗
4.6 ←.→
123 ↙↓↘
```
右と左が対応していて斜め移動もできる。カーソルは[]で囲われていて、その中は選択中のセルである。
カーソル位置を掘る場合、5もしくは[Enter]で掘れる。
Fキーでそこを「えむ」と、断定する。そこの箇所に「えむ」が存在すると仮定して堀り進められる点に注意する。

## プログラム構造
### マップ構造
マップは以下の構造をしている。
```c
typedef struct{
  int width;
  int height;
  int numOfBombs;
  int **field;
} field_t;/*field*/
```
マップの情報にはマップサイズと「えむ」の量、フィールド情報で構成される。フィールド情報には以下の状態番号が振られる。
0-8まではそこの回りにいくつ「えむ」が存在するのかをしめす。そこのところに「えむ」は存在しない。「えむ」が存在する場所には9が与えられる。10-18までは実際に開いた項を示す。20-29まではフラグの箇所を示す。誤ったところにもえむフラグは存在できるものとする。

### マップ生成手順
マップ生成は初手落ちしないように救済が施されている。GNOMEのマインスイーパと同様に、初手の位置が確定した瞬間に、その位置のまわり９マスを除いたすべてのところを「えむ」の置ける候補とする。その候補を配列に格納し、ランダムにシャッフルしたのちにそのうちの先頭からn個を「えむ」としてセットすることで生成することができる。爆弾の数は「えむ」をおいた瞬間にそのまわりのセルをカウントアップすることで、のちに全マップのスキャンする手間が減る。以上の手順を簡単に実装した結果、この関数は40行を越えてしまった。

### 「えむ」のまわりを掘る手順
「えむ」のまわりを掘る手順としては、以下の手順とする。(x,y)を起点とする。
１、まず(x,y)の座標にフラグが立っていないか否かを確認する
２、フラグが立っていなければその座標を開く。
３、この時、この開いた座標から得られる数字が、まわり8方向のフラグの数と一致するかを確認する。
４、一致した場合そのまわり８方向を再帰的に掘る。

### ゲームそれぞれの進行度合いと終了条件
#### ゲーム開始
ゲームが開始される時は最初の一手が決まった時である。この段階からタイムの計測を開始する。実際の実装では、始まるまで0にリセットして擬似的に始まっていないように見せかけている。
#### ゲームクリア
フィールドの中で、すべてのマスが開かれればゲームクリアである。
#### ゲームオーバー
フィールドの中で誤って「えむ」を開き、その結果フィールドに最低一つ以上の開かれた「えむ」が存在した場合にゲームオーバーとする。

### 画面制御あたり
画面制御まわりはエスケープシーケンスをつかって無理やり制御している。
たとえば`<Esc>[2J`で画面をクリアできる。`<Esc>[3<color number>m`で画面の文字の色を変更する
```c
SET_BACKGROUND_COL(col)			背景色をcolに変更
SET_FRONTGROUND_COL(col)		文字色をcolに変更
REVERCE_BACKGROUND_FRONTGROUND_COL()	文字色と背景色を交換
RESET_SETTING				設定をリセットする
CLR_SCREEN				スクリーンをクリアする
SET_CURSOR(x,y)				カーソルを(x,y)に移動する
```


### 関数
ベクトルの型を用意した。足し算などを使ってカーソル移動などを提供する。
```c
vector_t add(vector_t a,vector_t b)				二つのベクトルを加算する
int distSq(vector_t a,vector_t b)				二点間の距離の二乗を求める
vector_t calcMotion(char ch)					移動量を求める
void calcCursor(field_t *field,char ch,vector_t *cursor)	次のカーソル位置を求める
```
キー操作の関数である。
`bool keyTask(field_t *field,vector_t *cursor,unsigned int time,char key,bool *isStarted)`
キータスクを行い、終了時にはtrueを返す。この関数ではキー操作に対応する動作をする関数にジャンプすることが目的であるが、終了判定も行う。
```c
int initValue(field_t *field,int argc,const char **argv);/*initialize value*/
int initGame(field_t *field);/*initialize game*/
void exitGame(field_t *field);/*deinitialize game*/
int setBombs(field_t *field,vector_t *cursor);/*set bombs(cursor requred)*/
int newField(field_t *field);/*make new field*/
void freeField(field_t *field);/*free field*/
```
値の初期化としては、コマンドライン引数を解析し、その結果に応じた処理を行う。
ゲームの初期化ではフィールドの生成、ターミナルの設定を行う。
ゲームの終了の際には、ターミナルの設定のリセット、その他フィールドの開放等を行う
setBombsでは爆弾の設置を行う。その際、カーソル位置のまわり８マス以内に爆弾を設置することはない。
newFieldでは新しくフィールドを生成する。
freeFieldではフィールドを開放する。

```c
void dispMessage(field_t *field);/*start up message*/
void dispElement(int num);/*display Element of map*/
void dispField(field_t *field,vector_t *cursor);/*disp map*/
```
表示関数である。最初のメッセージ、それぞれの要素の表示、フィールドの表示関数である。

```c
void open(field_t *field,vector_t p);/*open tile*/
void putFlag(field_t *field,vector_t *p);/*put flag*/
int numOfFlags(field_t *field,vector_t *p);/*count around flags*/
```
フィールド操作関数である。open関数では、pを開く。putFlagではpにフラグを設置する。numofFlagsではまわりのフラグをカウントする。これはopenに使用する。

```c
bool isGameClear(field_t *field);/*game clear judgment*/
bool isGameOver(field_t *field);/*game over judgment*/
void gameOverEvent(field_t *field,vector_t *cursor);/*game over events*/
void gameClearEvent(field_t *field,vector_t *cursor,int time);/*game clear events*/
```
ゲームイベントの処理関数である。ゲームクリアかどうかなどを判定したり、その時の動作を定義する。
