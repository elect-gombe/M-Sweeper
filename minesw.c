#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define min(x,y) (x<y?x:y)

/*
 * ***field discription***
 *  0-8   .....numOfBomb
 *  9     .....bombInHere
 *  10-18 .....OpenedBlock(NumOfBomb-10)
 *  20-29 .....flaged here(0-9)
 */
typedef struct{
  int width;
  int height;
  int numOfBombs;
  int **field;
} field_t;/*field*/

typedef struct{
  int x;
  int y;
} vector_t;/*vector*/

vector_t add(vector_t a,vector_t b);/*add two vectors*/
int distSq(vector_t a,vector_t b);/*distance from 2 points*/
vector_t calcMotion(char ch);/*calcMotion*/
void calcCursor(field_t *field,char ch,vector_t *cursor);/*cursor motion*/

bool keyTask(field_t *field,vector_t *cursor,unsigned int time,char key,bool *isStarted);/*key tasks*/
int initValue(field_t *field,int argc,const char **argv);/*initialize value*/
int initGame(field_t *field);/*initialize game*/
void exitGame(field_t *field);/*deinitialize game*/
int setBombs(field_t *field,vector_t *cursor);/*set bombs(cursor requred)*/
int newField(field_t *field);/*make new field*/
void freeField(field_t *field);/*free field*/
int numOfFlags(field_t *field,vector_t *p);/*count around flags*/

void dispMessage(field_t *field);/*start up message*/
void dispElement(int num);/*display Element of map*/
void dispField(field_t *field,vector_t *cursor);/*disp map*/

void open(field_t *field,vector_t p);/*open tile*/
void putFlag(field_t *field,vector_t *p);/*put flag*/
bool isGameClear(field_t *field);/*game clear judgment*/
bool isGameOver(field_t *field);/*game over judgment*/

void gameOverEvent(field_t *field,vector_t *cursor);/*game over events*/
void gameClearEvent(field_t *field,vector_t *cursor,int time);/*game clear events*/

/*Around escape sequence*/
#define COL_RED        1
#define COL_GREEN    2
#define COL_YELLOW    3
#define COL_BLUE    4
#define COL_PINK    5
#define COL_CYAN    6
#define COL_WHITE    7
#define COL_RESET    9

#define SET_BACKGROUND_COL(col)     printf("\033[4%dm",(col))
#define SET_FRONTGROUND_COL(col)    printf("\033[3%dm",(col))
#define REVERCE_BACKGROUND_FRONTGROUND_COL() printf("\033[7m")
#define RESET_SETTING() printf("\033[0m")
#define CLR_SCREEN() printf("\033[2J")
#define SET_CURSOR(x,y) printf("\033[%d;%dH",(y)+1,(x)+1)

/*key settings*/
/*key and move value
 * ex. 4 ...move left 
 */
const struct{
  char key;
  vector_t move;
} KEYS[]={
  {'4',{-1,0}},
  {'8',{0,-1}},
  {'6',{1,0}},
  {'2',{0,1}},
  {'7',{-1,-1}},
  {'9',{1,-1}},
  {'1',{-1,1}},
  {'3',{1,1}},
};

/*number of flags in field*/
int numOfFlagsinField;

int main(int argc,const char **argv){
  char ch;
  field_t field;
  vector_t cursor={0,0};
  bool isStarted = false;
  time_t startTime;

  srand(startTime);

  if(initValue(&field,argc,argv))return EXIT_FAILURE;
 
  dispMessage(&field); 
  if(initGame(&field)!=EXIT_SUCCESS){
    printf("cannot initialize\n");
    return EXIT_FAILURE;
  }

  CLR_SCREEN();
  dispField(&field,&cursor);
  time(&startTime);

  do{
    ch = getchar();
    if(!isStarted){
      startTime = time(NULL);
    }
  }while(keyTask(&field,&cursor,time(NULL)-startTime,ch,&isStarted));

  exitGame(&field);

  return EXIT_SUCCESS;
}

int isNumber(const char *str){
  while(*str&&isdigit(*str++));
  return *str=='\0';
}

/*
 *key tasks
 *return...isContinue?
 */
bool keyTask(field_t *field,vector_t *cursor,unsigned int time,char key,bool *isStarted){
  if(isdigit(key)&&key!='0'&&key!='5'){
    calcCursor(field,key,cursor);
  }

  if(key == '\n'||key=='5'){
    if(!*isStarted){
      *isStarted = true;
      if(setBombs(field,cursor) == EXIT_FAILURE){
	printf("cannot put bombs\n");
	return false;
      }
    }
    if(field->field[cursor->y][cursor->x]<19)
      open(field,*cursor);
    if(isGameOver(field)){
      gameOverEvent(field,cursor);
      return false;
    }
  }
  else if(key == 'e')return false;
  else if(key == 'f'&&*isStarted)putFlag(field,cursor);
  CLR_SCREEN();
  printf("time:%u\n",time);
  dispField(field,cursor);
  if(*isStarted&&isGameClear(field)){
    gameClearEvent(field,cursor,time);
    return false;
  }
  return true;
}

/*initialize value*/
int initValue(field_t *field,int argc,const char **argv){ 
  if(argc == 1){
    field->width = 15;
    field->height = 15;
    field->numOfBombs = 30;
  }
  else if(argc == 4){
    if(isNumber(argv[1]))
      field->width = atoi(argv[1]);
    else{
      printf("%s is NOT correct!!(width)\n",argv[1]);
      return EXIT_FAILURE;
    }
    if(isNumber(argv[2]))
      field->height = atoi(argv[2]);
    else{
      printf("%s is NOT correct!!(height)\n",argv[2]);
      return EXIT_FAILURE;
    }
    if(isNumber(argv[3]))
      field->numOfBombs = atoi(argv[3]);
    else{
      printf("%s is NOT correct!! (bombs)\n",argv[3]);
      return EXIT_FAILURE;
    }
  }
  else{
    printf("parameter not collect!\n\n usage\t[width] [height] [num of bombs]\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/*start message*/
void dispMessage(field_t *field){
  printf("\nsetting\n");
  printf("[%-*s]:%d\n",15,"width of field",field->width);
  printf("[%-*s]:%d\n",15,"height of field",field->height);
  printf("[%-*s]:%d\n",15,"number of bombs",field->numOfBombs);
  printf("If you will edit these elements, you must put the parameter!\n");
  printf("[width] [height] [numOfBomb]\n\n");
  printf("Enter to continue...");
  while(getchar()!='\n');

  printf("Plz confirm key settings...\n");
  printf("[%-*s]...%s.\n",8,"Enter,5","Dig (if put flag, dig around)");
  printf("[%-*s]...%s.\n",8,"F","Put flag");
  printf("[%-*s]...%s.\n",8,"4","Move cursor to the left");
  printf("[%-*s]...%s.\n",8,"8","Move cursor to the up");
  printf("[%-*s]...%s.\n",8,"6","Move cursor to the right");
  printf("[%-*s]...%s.\n",8,"2","Move cursor to the down");
  printf("[%-*s]...%s.\n",8,"7","Move cursor to the upper left");
  printf("[%-*s]...%s.\n",8,"9","Move cursor to the upper right");
  printf("[%-*s]...%s.\n",8,"1","Move cursor to the lower left");
  printf("[%-*s]...%s.\n",8,"3","Move cursor to the lower right");
  printf("[%-*s]...%s.\n",8,"E","Exit game");
 
  printf("\
That it!\n\
Enter to continue...");
  while(getchar()!='\n');
}

/*is game overed?*/
bool isGameOver(field_t *field){
  int x,y;
  for(x=0;x<field->width;x++){
    for(y=0;y<field->height;y++){
      if(field->field[y][x]==19)
	return true;
    }
  }
  return false;
} 

/*is game cleared?*/
bool isGameClear(field_t *field){
  int x,y;
  for(x=0;x<field->width;x++){
    for(y=0;y<field->height;y++){
      if((field->field[y][x]>=20&&field->field[y][x]<=28)||
	 (field->field[y][x]>=0 &&field->field[y][x]<=8))
	return false;
    }
  }
  return true;
}

/*cursor motion calculate*/
vector_t calcMotion(char ch){
  unsigned int i;
  for(i=0;i<sizeof(KEYS)/sizeof(KEYS[0]);i++){
    if(KEYS[i].key == ch){
      return KEYS[i].move;
    }
  }
  return KEYS[0].move;
}

void calcCursor(field_t *field,char ch,vector_t *cursor){
  *cursor = add(*cursor,calcMotion(ch));
  if(field->width <= cursor->x){    cursor -> x = field->width-1;  }
  else if(0 > cursor->x)cursor->x = 0;
  if(field->height <= cursor->y){    cursor -> y = field->height-1;  }
  else if(0 > cursor->y)cursor->y = 0;
}

/*is p in field?*/
bool isInField(field_t *field,vector_t *p){
  return
    field->width  > p->x && 0 <= p->x&&
    field->height > p->y && 0 <= p->y;
}

/*game initialize*/
int initGame(field_t *field){
  int res;

  numOfFlagsinField = 0;
 
  res = newField(field);
  if(res == EXIT_FAILURE){
    return EXIT_FAILURE;
  }

  if(system("stty -echo -icanon min 1 time 0")<0){
    printf("echo setting failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/*exit game*/
void exitGame(field_t *field){
  freeField(field);
  if(system("stty echo -icanon min 1 time 0")<0){
    printf("echo setting failed\n");
  }
}

/*Clear,Over Events*/
void gameClearEvent(field_t *field,vector_t *cursor,int time){
  int x,y;
  for(x=0;x<field->width;x++){
    for(y=0;y<field->height;y++){
      if(field->field[y][x]==9)
	field->field[y][x]+=20;
      else if(field->field[y][x]<9){
	field->field[y][x]+=10;
      }
    }
  }
  dispField(field,cursor);
  printf("Game cleard\nThank you for playing!\n");
  printf("\n%d s\n",time);
}

void gameOverEvent(field_t *field,vector_t *cursor){
  int x,y;
  for(x=0;x<field->width;x++){
    for(y=0;y<field->height;y++){
      if(field->field[y][x]<=9)
	field->field[y][x]+=10;
    }
  }
  dispField(field,cursor);
  printf("|\\/| exploded!!\n死んでしまうとは情けない。。。\n");
}

/*put bombs*/
int setBombs(field_t *field,vector_t *cursor){
  vector_t *candidi;
  size_t numofcandidi;

  unsigned int i = 0;

  numofcandidi = field->height*field->width-4;/*width*height-4が最大数(角)*/

  candidi = (vector_t*)malloc(sizeof(vector_t)*numofcandidi);

  if(candidi == NULL)return EXIT_FAILURE;
  {//calc candidicate
    vector_t p;

    for(p.y=0;p.y<field->height;p.y++){
      for(p.x=0;p.x<field->width;p.x++){
	if(distSq(p,*cursor)>2){
	  candidi[i++]=p;
	}
      }
    }
  }

  numofcandidi = (size_t)i;
  {//sort by random
    vector_t tmp;
    int randnum;
    for(i=0;i<numofcandidi;i++){
      randnum = rand()%numofcandidi;
      tmp = candidi[i];
      candidi[i] = candidi[randnum];
      candidi[randnum] = tmp;
    }
  }

  if(numofcandidi < (unsigned int)field->numOfBombs){
    printf("cannot put |\\/| ...\ncandidi:%lu\n",numofcandidi);
    return EXIT_FAILURE;
  }
  {/*count up around bombs*/
    int j;
    vector_t tmp;
    for(i = 0;i < (unsigned int)field->numOfBombs;i++){
      field->field[candidi[i].y][candidi[i].x] = 9;
      for(j=0;j<(int)(sizeof(KEYS)/sizeof(KEYS[0]));j++){
	tmp = add(KEYS[j].move,candidi[i]);
	if(isInField(field,&tmp)){
	  if(field->field[tmp.y][tmp.x]!=9){
	    field->field[tmp.y][tmp.x]++;
	  }
	}
      }
    }
  }
  free(candidi);

  return EXIT_SUCCESS;
}

/*set flag*/
void putFlag(field_t *field,vector_t *p){
  if(field->field[p->y][p->x] < 10){
    field->field[p->y][p->x]+=20;
    numOfFlagsinField++;
  }
  else if(field->field[p->y][p->x] >= 20&&
	  field->field[p->y][p->x] < 30){
    field->field[p->y][p->x]-=20;
    numOfFlagsinField--;
  }
}

/*open tiles*/
void open(field_t *field,vector_t p){
  vector_t tmp;
  int i;

  if(field->field[p.y][p.x] < 10)
    field->field[p.y][p.x] += 10;/*open tile*/
  if(numOfFlags(field,&p)==field->field[p.y][p.x]-10){
    for(i=0;i<(int)(sizeof(KEYS)/sizeof(KEYS[0]));i++){/*open around*/
      tmp = add(p,KEYS[i].move);
      if(isInField(field,&tmp)){
	if(field->field[tmp.y][tmp.x]<10)/*have not opended yet*/
	  open(field,tmp);
      }
    }
  }
}

/*allocate map*/
int newField(field_t *field){
  unsigned int height = field->height;
 
  field->field = calloc(sizeof(int*),field->height);

  int **fld = field->field;

  if(fld==NULL)return EXIT_FAILURE;
 
  do{
    *fld = calloc(sizeof(int),field->width);
    if(*fld++==NULL)return EXIT_FAILURE;
  }while(--height);

  return EXIT_SUCCESS;
}

void freeField(field_t *field){
  unsigned int height = field->height;
  int **fld = field->field;

  do{
    free(*(fld++));
  }while(--height);

  free(field->field);
}

/*count flags(around p)*/
int numOfFlags(field_t *field,vector_t *p){
  int i;
  int numOfFlags=0;
  vector_t tmp;

  for(i=0;i<(int)(sizeof(KEYS)/sizeof(KEYS[0]));i++){
    tmp = add(*p,KEYS[i].move);
    if(isInField(field,&tmp)){
      if(field->field[tmp.y][tmp.x]>=20){
	numOfFlags++;
      }
    }
  }
  return numOfFlags;
}

/*display maps functions*/
void dispElement(int num){
  if(num < 10){
    printf("#");
  }
  else if(num < 20){
    if(num == 10){printf(" ");}
    else if(num <= 18){
      printf("%d",num-10);
    }
    else if(num == 19){
      SET_BACKGROUND_COL(COL_RED);
      printf("B");
      SET_BACKGROUND_COL(COL_RESET);
    }
  }
  else if(num < 30){
    SET_BACKGROUND_COL(COL_GREEN);
    printf("M");
    SET_BACKGROUND_COL(COL_RESET);
  }
}

/*display field*/
void dispField(field_t *field,vector_t *cursor){
  int x,y;
  int **fld = field->field;

  for(x=0;x<field->width;x++){
    printf("+-");
  }
  printf("+\n");
  for(y=0;y<field->height;y++){
    for(x=0;x<field->width;x++){
      if(cursor->x==x&&y==cursor->y){
	printf("[");
	REVERCE_BACKGROUND_FRONTGROUND_COL();
      }
      else if(cursor->x==x-1&&y==cursor->y)printf("]");
      else printf("|");
      dispElement(fld[y][x]);
      RESET_SETTING();
    }
    if(cursor->x==x-1&&y==cursor->y)printf("]\n");
    else printf("|\n");
  }
  for(x=0;x<field->width;x++){
    printf("+-");
  }
  printf("+\n");
  printf("num of flags %d/%d\n",numOfFlagsinField,field->numOfBombs);
}

/*vector functions*/
/*add two vectors*/
vector_t add(vector_t a,vector_t b){
  vector_t res;

  res.x = a.x+b.x;
  res.y = a.y+b.y;

  return res;
}

/*sqare of distance from a to b*/
int distSq(vector_t a,vector_t b){
  return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y);
}
