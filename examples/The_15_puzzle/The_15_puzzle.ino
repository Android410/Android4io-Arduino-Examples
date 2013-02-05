#include <Arduino.h>
#include <Android4io.h>

Android4ioBT android;

int8_t    board[3][16];
int8_t    fontSize;
uint16_t  moves;
uint16_t  seg,currentPos;
char      message[100];

#define   INDEX 		0
#define	  RND_VALUES		1
#define   OBJ_IDS		2

uint8_t alreadyInPlace (uint8_t v)
{
  uint8_t i;

  for (i=0;i < 16;i++) 
    if (v == board[RND_VALUES][i])
      return 1;

  return 0;
}

int8_t getBoardPosFromObjectId(uint8_t oby) {
  int8_t i;

  for (i=0;i<16;i++) 
    if (board[OBJ_IDS][i] == oby)
      return board[INDEX][i] ;

  return -1;
}

int8_t checkIfWon()
{
  int8_t i;

  for (i=0;i<15;i++)
    if (board[RND_VALUES][i] != i + 1)
      return 0;

  return 1;
}

void gameInit()
{
  uint8_t v,i;

  moves=0;

  for (i=0;i < 16;i++)
    board[RND_VALUES][i] = 0xFF;

  for (i=0;i < 16;i++) {
    do {
#ifdef FOR_TEST_ONLY
      if (v < 15)
        v = i + 1;
      else
        v = 0;
#else
      v = random(0,16);
#endif
    } 
    while (alreadyInPlace(v));
    board[RND_VALUES][i] = v;
    board[INDEX][i] = i;
    board[OBJ_IDS][i] = -1;
  }
}

void setup()
{
  uint8_t   i,p;
  uint16_t  x;

  Serial.begin(115200);
#ifdef ARDUSIM
  randomSeed (0);
#else
  randomSeed (analogRead(0));
#endif

  android.begin(&Serial,MANUAL_REDRAW_LANDSCAPE);

  gameInit();
  android.setTextBgrColor();
  x=android.getMaxY()-1;
  seg=(android.getMaxY()-1)/4;
  currentPos = (seg/2) - (seg/8);
  fontSize = (seg/3);

  android.rectangle(0,0,android.getMaxY()-1,android.getMaxY()-1,redColor, NO_FILL, 1);
  android.line(0,seg,x,seg,redColor);
  android.line(0,seg*2,x,seg*2,redColor);
  android.line(0,seg*3,x,seg*3,redColor);

  android.line(seg,0,seg,x,redColor);
  android.line(seg*2,0,seg*2,x,redColor);
  android.line(seg*3,0,seg*3,x,redColor);

  for (i=0;i<16;i++) {
    p = board[RND_VALUES][i];
    if (p==0) continue;
    if((board[OBJ_IDS][i] = android.createObject(0,0,seg-2,seg-2)) < 0) {
      android.setCurrentObject(mainDisplay);
      sprintf(message,"Error creating objects..");
      android.text((seg*4) + (seg/8),(seg/8),message,whiteColor,DEF_BOLD_FONT,fontSize);
      android.redraw();
      while(1);
    } else {
      if (p < 10) {
        android.ch(currentPos,currentPos,p +'0',whiteColor,DEF_BOLD_FONT,fontSize);
      } 
      else {
        char t[3] = {'1',p-10+'0',0 };
        android.text(currentPos,currentPos,t,whiteColor,DEF_BOLD_FONT,fontSize);
      }
      android.moveObject(board[OBJ_IDS][i],(i%4)*seg,(i/4)*seg);
    }
  }
}

void loop()   //main loop
{
  int8_t currentPos,newPos,savedObjID;
  touch_t  t;
  int8_t obj;

  while (!(t=android.getTouch(obj)));

  if(t == OBJECT_TOUCH) {
    currentPos=getBoardPosFromObjectId(obj);

    if ((currentPos - 4) >= 0 && board[RND_VALUES][currentPos - 4] == 0) {
      newPos = currentPos - 4;
    } 
    else if ((currentPos + 4) < 16 && board[RND_VALUES][currentPos + 4] == 0) {
      newPos = currentPos + 4;
    } 
    else if ((currentPos + 1) < 16 && board[RND_VALUES][currentPos + 1] == 0) {
      newPos = currentPos + 1;
    } 
    else if ((currentPos - 1) >= 0 && board[RND_VALUES][currentPos - 1] == 0) {
      newPos = currentPos - 1;
    } 
    else {
      return;
    }

    android.moveObject(obj,(newPos%4)*seg,(newPos/4)*seg);
    board[RND_VALUES][newPos] = board[RND_VALUES][currentPos];
    board[RND_VALUES][currentPos] = 0;
    savedObjID = board[OBJ_IDS][newPos];
    board[OBJ_IDS][newPos] = board[OBJ_IDS][currentPos];
    board[OBJ_IDS][currentPos] = savedObjID;
    
    moves++;
    if (checkIfWon()) {
      sprintf(message,"Won using %d",moves);
      android.text((seg*4) + (seg/8),(seg/8),message,whiteColor,DEF_BOLD_FONT,fontSize);
      android.redraw();
      android.beep(500,600);
      while(1);
    }
    android.setCurrentObject(mainDisplay);
    sprintf(message,"Move num: %d",moves);
    android.text((seg*4) + (seg/8),(seg/8),message,whiteColor,DEF_BOLD_FONT,fontSize);
    android.redraw();
  }
}


