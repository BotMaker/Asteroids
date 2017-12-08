#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <math.h>  //pi sin cos


// my game library for c++ use one buffer for drawing onto
// the buffer needs to get resize when the window is resize staus=1
// need to make the keyboard buffer for game library staus=1
// max 5 keys to be stored in the key buffer maybe less five
// need to make some sort of list struct that can grow and strink
// need to no how to convert number into strings and strings to numbers

#define WS_GAME (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_MINIMIZEBOX)
                             

#define check_bb_collision_general(x1,y1,w1,h1,x2,y2,w2,h2) (!( ((x1)>=(x2)+(w2)) || ((x2)>=(x1)+(w1)) || \
                                                                ((y1)>=(y2)+(h2)) || ((y2)>=(y1)+(h1)) ))
          
void drawbitmap(HBITMAP image,int x,int y);
void drawspritetran(HBITMAP image,HBITMAP mask,int x,int y,int image_single);
void drawspritetran2(HBITMAP image,HBITMAP mask,int x,int y);
                                                      
int check_pp_collision_normal(HBITMAP spr1, HBITMAP spr2, int x1, int y1, int x2, int y2)
{
 int dx1, dx2, dy1, dy2; //We will use this deltas...
 int fx,fy,sx1,sx2; //Also we will use this starting/final position variables...
 int maxw, maxh; //And also this variables saying what is the maximum width and height...
 BITMAP bm1,bm2;
 
 GetObject(spr1, sizeof(bm1), &bm1);
 GetObject(spr2, sizeof(bm2), &bm2);
 
 //drawbitmap(spr2, x2, y2);
 
 if( !check_bb_collision_general(x1,y1,bm1.bmWidth,bm1.bmHeight, x2,y2,bm2.bmWidth,bm2.bmHeight) ) return 0; //If theres not a bounding box collision, it is impossible to have a pixel perfect collision right? So, we return that theres not collision...
  COLORREF crTransparent=RGB(0,0,0);
  HDC hdcMem, hdcMem2;
  hdcMem = CreateCompatibleDC(0);
  hdcMem2= CreateCompatibleDC(0);
  SelectObject(hdcMem, spr1);
  SelectObject(hdcMem2, spr2);
  
 //First we need to see how much we have to shift the coordinates of the sprites...
 if(x1>x2) {
   dx1=0;	   //don't need to shift sprite 1.
   dx2=x1-x2;  //shift sprite 2 left. Why left? Because we have the sprite 1 being on the right of the sprite 2, so we have to move sprite 2 to the left to do the proper pixel perfect collision...
   } else {
   dx1=x2-x1;  //shift sprite 1 left.
   dx2=0;	   //don't need to shift sprite 2.
   }
 if(y1>y2) {
   dy1=0;
   dy2=y1-y2;  //we need to move this many rows up sprite 2. Why up? Because we have sprite 1 being down of sprite 2, so we have to move sprite 2 up to do the proper pixel perfect collision detection...
   } else {
   dy1=y2-y1;  //we need to move this many rows up sprite 1.
   dy2=0;
   }

 //Then, we have to see how far we have to go, we do this seeing the minimum height and width between the 2 sprites depending in their positions:
 if(bm1.bmWidth-dx1 > bm2.bmWidth-dx2) {
   maxw=bm2.bmWidth-dx2;
   } else {
   maxw=bm1.bmWidth-dx1;
   }
 if(bm1.bmHeight-dy1 > bm2.bmHeight-dy2) {
   maxh=bm2.bmHeight-dy2;
   } else {
   maxh=bm1.bmHeight-dy1;
   }
 maxw--;
 maxh--;

 fy=dy1;
 fx=dx1;
 dy1+=maxh;
 dy2+=maxh;
 sx1=dx1+maxw;
 sx2=dx2+maxw;
 
 for(; dy1>=fy; dy1--,dy2--) { //Go through lines...
  for(dx1=sx1,dx2=sx2; dx1>=fx; dx1--,dx2--) { //Go through the X axis...
	 if((GetPixel(hdcMem,dx1,dy1)!=crTransparent) && (GetPixel(hdcMem2,dx2,dy2)!=crTransparent)) {
       DeleteDC(hdcMem);
       DeleteDC(hdcMem2);
       return 1; //Both sprites don't have transparent color in that position, so, theres a collision and return collision detected!
	 }
  }
 }
 
 DeleteDC(hdcMem);
 DeleteDC(hdcMem2);
 //If we have reached here it means that theres not a collision:
 return 0; //Return no collision.
}                                                                


const double PI=M_PI;
double sin_loTRUE2up[361];
double cos_loTRUE2up[361];

double x=0,y=0,speed=0;
int dir=0,image_single=0,can_shot=1,alarm0=0;
int destroyable=30,lives=3,score=0;

int offsetx=0,offsety=0;

void sin_cos_table_init()
{
  int i=0,j=360;
  for(j=360; j>0;j--)
  {
    sin_loTRUE2up[i]=sin(j*PI/180);
    cos_loTRUE2up[i]=cos(j*PI/180);
    i++;      
  }
  sin_loTRUE2up[360]=sin_loTRUE2up[0];
  cos_loTRUE2up[360]=cos_loTRUE2up[0];
}

int keyboard_key[5]; //key buffer

HBITMAP player_sprite = NULL;
HBITMAP player_mask=NULL;
HBITMAP player_moveing_sprite = NULL;
HBITMAP player_moveing_mask=NULL;
HBITMAP player_shot_sprite=NULL;
HBITMAP player_shot_mask=NULL;

HBITMAP background0=NULL;

HBITMAP lives_image=NULL;
HBITMAP lives_mask=NULL;

HBITMAP pixel_image=NULL;
HBITMAP pixel_mask=NULL;

HBITMAP rock_sprite=NULL;
HBITMAP rock_mask=NULL;

HBITMAP hbmBuffer =NULL;
HBITMAP hbmOldBuffer=NULL;


HDC hdcBuffer = NULL;
HDC hdc=NULL;

const int ID_TIMER = 1;
int fps=0,frames=0;
int time1=clock();
int mousex=0,mousey=0,mouse_button=0;
int window_right=0,window_bottom=0;
int keydown=0,keyf1=0;
int window_mode=0; //whever to start in fullscreen or window


class PIXEL1_OBJECT
{
public:
      double x,y;
      int dir,speed,index,alarm0;
      void step();
};

const int pixel1_number=400;
PIXEL1_OBJECT pixel1_objects[pixel1_number];
int pixel1_freelist[pixel1_number];


void PIXEL1_OBJECT::step()
{
 
 if (alarm0>0)
 {
   alarm0-=1;
   if (alarm0==0)
   {
    pixel1_freelist[index]=0; 
   }
 }
 
 x+=speed*cos_loTRUE2up[dir];
 y+=speed*sin_loTRUE2up[dir];
 
 if (int(x)<=-46 || int(x)>640 || int(y)<=-46 || int(y)>480)
 {
   pixel1_freelist[index]=0;              
 }

 drawspritetran2(pixel_image,pixel_mask,int(x),int(y));    
}

void create_pixel1_object(double x, double y,int dir,int speed)
{
  int i;
  for (i=0;i<pixel1_number;i++)
  {
    if (pixel1_freelist[i]==0)
    {
      pixel1_freelist[i]=1;
      pixel1_objects[i].x=x;
      pixel1_objects[i].y=y;
      pixel1_objects[i].dir=dir;
      pixel1_objects[i].speed=speed;
      pixel1_objects[i].index=i;
      pixel1_objects[i].alarm0=1+rand()%60;
      break;                           
    }
  }  
}

void pixel1_object_loop()
{
  int i;
  for (i=0;i<pixel1_number;i++)
  {
    if (pixel1_freelist[i]==1)
    {
      pixel1_objects[i].step();
    }
  }
}



/*====================================int rock int================================*/
class ROCK_OBJECT
{
public:
      double x,y;
      int dir,speed,image_single,index,alarm0;
      void step();
};

const int rock_number=40;
ROCK_OBJECT rock_objects[rock_number];
int rock_freelist[rock_number];

void ROCK_OBJECT::step()
{
 
 if (alarm0>0)
 {
   alarm0-=1;
   if (alarm0==0)
   {
     image_single+=1;
     if (image_single>36)
     {
     image_single=0;       
     }
     alarm0=5;
   }
 }
 
 x+=speed*cos_loTRUE2up[dir];
 y+=speed*sin_loTRUE2up[dir];
 

 if (int(x)<=-46)
 {
   x=640;
 }
 
 if (int(y)<=-46)
 {
   y=480;
 }
 
 if (int(x)>640)
 {
   x=-46;
 }
 
 if (int(y)>480)
 {
   y=-46;
 }

 drawspritetran(rock_sprite,rock_mask,int(x),int(y),image_single);    
}

void create_rock_object(double x, double y,int dir,int speed)
{
  int i;
  for (i=0;i<rock_number;i++)
  {
    if (rock_freelist[i]==0)
    {
      rock_freelist[i]=1;
      rock_objects[i].x=x;
      rock_objects[i].y=y;
      rock_objects[i].dir=dir;
      rock_objects[i].speed=speed;
      rock_objects[i].index=i;
      rock_objects[i].alarm0=1;
      break;                           
    }
  }  
}

void rock_object_loop()
{
  int i;
  for (i=0;i<rock_number;i++)
  {
    if (rock_freelist[i]==1)
    {
      rock_objects[i].step();
    }
  }
}
/*====================================end rock end================================*/


HBITMAP CreateBitmap_2(HBITMAP hbmColour, int image_single)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);
	SelectObject(hdcMem, hbmColour);
	hbmMask =CreateCompatibleBitmap(hdcMem,46,46);
	SelectObject(hdcMem2, hbmMask);
	BitBlt(hdcMem2, 0, 0, 46, 46, hdcMem, 46*image_single, 0, SRCCOPY	);	
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);
	return hbmMask;
}


class PLAYER_SHOT_OBJECT
{
public:
      double x,y;
      int dir,speed,image_single,index;
      void step();
};

const int player_shot_number=20;
PLAYER_SHOT_OBJECT player_shot_objects[player_shot_number];
int player_shot_freelist[player_shot_number];


int check_collision_with_rock(HBITMAP myspr,int x,int y,int image_single)
{
  int i;
  HBITMAP hbmtemp1,hbmtemp2;
  hbmtemp1=CreateBitmap_2(myspr, image_single);
  
  for (i=0;i<rock_number;i++)
  {
    if (rock_freelist[i]==1)
    {
      
      hbmtemp2=CreateBitmap_2(rock_sprite, rock_objects[i].image_single);
      if (check_pp_collision_normal(hbmtemp1, hbmtemp2, int(x), int(y), int(rock_objects[i].x),int(rock_objects[i].y) )==1)
      {
       DeleteObject(hbmtemp1);
       DeleteObject(hbmtemp2);
       return i;
      }
      DeleteObject(hbmtemp2);
    }
  }
  DeleteObject(hbmtemp1);
  return -1;    
}

void PLAYER_SHOT_OBJECT::step()
{
 int i=0,collision=0;
 
 x+=speed*cos_loTRUE2up[dir];
 y+=speed*sin_loTRUE2up[dir];

 if (int(x)<=-46 || int(x)>640 || int(y)<=-46 || int(y)>480)
 {
   player_shot_freelist[index]=0;
 }
 collision=check_collision_with_rock(player_shot_sprite,int(x),int(y),image_single);
 if (collision!=-1)
 {
   player_shot_freelist[index]=0;
   rock_freelist[collision]=0;
   score+=10;
   for (i=0; i<100; i++)
   {
     create_pixel1_object(rock_objects[collision].x, rock_objects[collision].y,rand()%360,4+rand()%6);
   }
 }

 drawspritetran(player_shot_sprite,player_shot_mask,int(x),int(y),image_single);    
}

void create_player_shot_object(double x, double y,int dir,int image_single,int speed)
{
  int i;
  for (i=0;i<player_shot_number;i++)
  {
    if (player_shot_freelist[i]==0)
    {
      player_shot_freelist[i]=1;
      player_shot_objects[i].x=x;
      player_shot_objects[i].y=y;
      player_shot_objects[i].dir=dir;
      player_shot_objects[i].image_single=image_single;
      player_shot_objects[i].speed=speed;
      player_shot_objects[i].index=i;
      break;                           
    }
  }  
}

void player_shot_object_loop()
{
  int i;
  for (i=0;i<player_shot_number;i++)
  {
    if (player_shot_freelist[i]==1)
    {
      player_shot_objects[i].step();
    }
  }
}

void add_key(int key)
{
  int i=0,found=0;
  for(i=0;i<5;i+=1)
  {
	if (keyboard_key[i] == key)
	{
	  found=1;
	  break;
	}
  }
  if (found==0)
  {
	for(i=0;i<5;i+=1)
	{
	  if (keyboard_key[i]==0)
	  {
	  keyboard_key[i]=key;
	  break;
	  }
	}
  }
}

void remove_key(int key)
{
  int i=0;
  for(i=0;i<5;i+=1)
  {
	if (keyboard_key[i] == key)
	{
	  keyboard_key[i]=0;
	}
  }
}

int check_key(int key)
{
  int i=0;
  for(i=0;i<5;i+=1)
  {
	if (keyboard_key[i] == key)
	{
	  return 1;
	}
  }
  return 0;
}

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
char szClassName[ ] = "WindowsApp";

int WINAPI WinMain (HINSTANCE hThisInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpszArgument,
					int nFunsterStil)

{
	HWND hwnd;				 /* This is the handle for our window */
	MSG messages;			 /* Here messages to the application are saved */
	WNDCLASSEX wincl;		 /* Data structure for the windowclass */

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;	  /* This function is called by windows */
	wincl.style = CS_DBLCLKS;				  /* Catch double-clicks */
	wincl.cbSize = sizeof (WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;				   /* No menu */
	wincl.cbClsExtra = 0;					   /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;					   /* structure or the window instance */
	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = CreateSolidBrush(RGB(0,0,0));//(HBRUSH) COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx (&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx (
		   0,					/* Extended possibilites for variation */
		   szClassName, 		/* Classname */
		   "Windows App",		/* Title Text */
		   WS_GAME, /* default window */
		   CW_USEDEFAULT,		/* Windows decides the position */
		   CW_USEDEFAULT,		/* where the window ends up on the screen */
		   646, 				/* The programs width */
		   506, 				/* and height in pixels */
		   HWND_DESKTOP,		/* The window is a child-window to desktop */
		   NULL,				/* No menu */
		   hThisInstance,		/* Program Instance handler */
		   NULL 				/* No Window Creation data */
		   );

	ShowWindow (hwnd, nFunsterStil);
	while (GetMessage (&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	return messages.wParam;
}

	
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;
	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);
	SelectObject(hdcMem, hbmColour);
	SelectObject(hdcMem2, hbmMask);
	SetBkColor(hdcMem, crTransparent);
	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);
	return hbmMask;
}

void drawbitmaptran(HBITMAP image,HBITMAP mask,int x,int y)
{
	BITMAP bm;
	HDC hdcMem = CreateCompatibleDC(hdc);
	GetObject(image, sizeof(bm), &bm);
	SelectObject(hdcMem,mask );
	BitBlt(hdcBuffer, x, y, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND);
	SelectObject(hdcMem, image);
	BitBlt(hdcBuffer ,x, y, bm.bmWidth, bm.bmHeight, hdcMem , 0, 0, SRCPAINT);
	DeleteDC(hdcMem);
}

void drawbitmap(HBITMAP image,int x,int y)
{
  BITMAP bm;
  HDC hdcMem = CreateCompatibleDC(hdc);
  HBITMAP hbmOld =(HBITMAP) SelectObject(hdcMem, image);
  GetObject(image, sizeof(bm), &bm);
  BitBlt(hdcBuffer ,x, y, bm.bmWidth, bm.bmHeight, hdcMem , 0, 0, SRCCOPY);
  SelectObject(hdcMem, hbmOld);
  DeleteDC(hdcMem);
}


void drawrectangle(int x1,int y1,int x2,int y2,COLORREF color1,COLORREF color2, int fill)
{
  HPEN hpen=CreatePen(0,1,color1 );
  SelectObject(hdcBuffer,hpen);
  HBRUSH hbr=CreateSolidBrush( color2 );
  if (fill==1)
  {
  SelectObject(hdcBuffer,hbr);
  }
  else
  {
  SelectObject(hdcBuffer,(HBRUSH)GetStockObject(HOLLOW_BRUSH));
  }
  Rectangle(hdcBuffer,x1,y1,x2,y2);
  DeleteObject(hpen);
  DeleteObject(hbr);
}

void bitmapbig(HBITMAP image,int x, int y)
{
  BITMAP bm;
  bool result=false;
  HDC hdcMem = CreateCompatibleDC(hdc);
  HBITMAP hbmOld =(HBITMAP) SelectObject(hdcMem, image);
  GetObject(image, sizeof(bm), &bm);
  result=StretchBlt(hdcBuffer,x,y,100,100,hdcMem,x,y,bm.bmWidth, bm.bmHeight,SRCCOPY);
  SelectObject(hdcMem, hbmOld);
  DeleteDC(hdcMem);
}

void drawstring(int x, int y, const char *msg)
{
   HFONT hfnt, hOldFont;
   int result=0;
   COLORREF col=0;
   hfnt =(HFONT) GetStockObject(ANSI_VAR_FONT);
   result=SetBkMode(hdcBuffer,1); // 1 or 2
   col=SetTextColor(hdcBuffer,RGB(255,255,255));
	if (hOldFont =(HFONT) SelectObject(hdcBuffer, hfnt)) {
		TextOut(hdcBuffer, x, y, msg, strlen(msg));
		SelectObject(hdc, hOldFont);
	}
	DeleteObject(hfnt);
	DeleteObject(hOldFont);
	result=SetBkMode(hdcBuffer,0); // 1 or 2
	col=SetTextColor(hdcBuffer,RGB(0,0,0));
}

void drawstring2(int x, int y, const char *msg,COLORREF col,int w,int h)
{
   HFONT hfnt, hOldFont;
   int result=0;
   //hfnt =(HFONT) GetStockObject(ANSI_VAR_FONT);
   HFONT myfont;
   myfont=CreateFont(h,w,0,0,0,0,0,0,DEFAULT_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
   DEFAULT_QUALITY,(DEFAULT_PITCH|FF_DONTCARE),"Arial");
   result=SetBkMode(hdcBuffer,1); // 1 or 2
   col=SetTextColor(hdcBuffer,col);
	if (hOldFont =(HFONT) SelectObject(hdcBuffer, myfont)) {
		TextOut(hdcBuffer, x, y, msg, strlen(msg));
		SelectObject(hdc, hOldFont);
	}
	//DeleteObject(hfnt);
	DeleteObject(hOldFont);
	DeleteObject(myfont);
	result=SetBkMode(hdcBuffer,0); // 1 or 2
	col=SetTextColor(hdcBuffer,RGB(0,0,0));
}


void drawspritetran(HBITMAP image,HBITMAP mask,int x,int y,int image_single)
{
	BITMAP bm;
	HDC hdcMem = CreateCompatibleDC(hdc);
	GetObject(image, sizeof(bm), &bm);
	SelectObject(hdcMem,mask );
	if (image_single==36)
	{
      image_single=0;                     
    }
	BitBlt(hdcBuffer, x, y, 46, 46, hdcMem,  (46*image_single) ,0, SRCAND);
	SelectObject(hdcMem, image);
	BitBlt(hdcBuffer ,x, y, 46, 46, hdcMem , (46*image_single) , 0, SRCPAINT);
	DeleteDC(hdcMem);
}

void drawspritetran2(HBITMAP image,HBITMAP mask,int x,int y)
{
	BITMAP bm;
	HDC hdcMem = CreateCompatibleDC(hdc);
	GetObject(image, sizeof(bm), &bm);
	SelectObject(hdcMem,mask );
	BitBlt(hdcBuffer, x, y, bm.bmWidth, bm.bmHeight, hdcMem,  0 ,0, SRCAND);
	SelectObject(hdcMem, image);
	BitBlt(hdcBuffer ,x, y, bm.bmWidth, bm.bmHeight, hdcMem , 0 , 0, SRCPAINT);
	DeleteDC(hdcMem);
}


void set_fullscreen(HWND hwnd)
{
  MoveWindow( hwnd,0 ,0, 640, 480, 1 ); 
  SetWindowLong(hwnd,GWL_STYLE, WS_POPUP) ;
  ShowWindow( hwnd, SW_MAXIMIZE );
  offsetx=int( (GetSystemMetrics(SM_CXSCREEN)/2)-320);
  offsety=int( (GetSystemMetrics(SM_CYSCREEN)/2)-240);
  window_mode=1;  
}

void set_window(HWND hwnd)
{
   
   SetWindowLong(hwnd,GWL_STYLE, WS_GAME) ;
   ShowWindow( hwnd, SW_NORMAL); 
   MoveWindow( hwnd, (GetSystemMetrics(SM_CXSCREEN)/2)-320 ,(GetSystemMetrics(SM_CYSCREEN)/2)-240
   , 646, 506, 1 ); 
   offsetx=0;
   offsety=0;
   window_mode=0;
}

void switch_window(HWND hwnd)
{
  if (window_mode==1)
  {
   set_fullscreen(hwnd);
   window_mode=0;
  }
  else
  {
   set_window(hwnd);
   window_mode=1;  
  }
}


void game_loop(HWND hwnd)
{
 RECT rc;
 int time2=0,i=0;
 GetClientRect(hwnd, &rc);

 time2=clock();
 
 if (check_key(VK_ESCAPE)==1)
 {
   SendMessage(hwnd,WM_DESTROY,0,0);                        
 }
 
 if (check_key(VK_F4)==0 && keyf1==1)
 {
 keyf1=0;
 }
 if (check_key(VK_F4)==1 && keyf1==0)
 {
   //set_window(hwnd);
   keyf1=1;                       
   switch_window(hwnd);
 }

 if (time2>(time1*1000))
 {
 fps=frames;
 time1+=1;
 frames=0;
 }
 
 if (check_key(VK_UP)==1)
 {
   if (speed<8)
   {
   speed+=0.2;
   }
 }
 else
 {
   if (speed>0)
   {
    speed-=0.1;
    if (speed<0) speed=0;
   }    
 }
 
 int collision=0;
 
 if (check_key(VK_DOWN)==0 && keydown==1)
 {
   keydown=0;                          
 }
 
 if (check_key(VK_DOWN)==1 && keydown==0)
 {              
   keydown=1;
   while (collision!=-1)
   {
    x=rand()%600;
    y=rand()%440;
    if (speed==0)
    {
     collision=check_collision_with_rock(player_sprite,int(x),int(y),image_single);
    }
    else
    {
     collision=check_collision_with_rock(player_moveing_sprite,int(x),int(y),image_single);
    }
   }   
 }
 
 /*
 if (check_key(VK_RIGHT)==1)
 {
   dir-=2;
   if (dir<0)
   {
    dir=360;         
   }                        
 }
 
 if (check_key(VK_LEFT)==1)
 {
   dir+=2;
   if (dir>360)
   {
    dir=0;         
   }                        
 }
 */
 
 if (check_key(VK_RIGHT)==1)
 {
   image_single-=1;
   if (image_single<0)
   {
    image_single=36;         
   }                        
 }
 
 if (check_key(VK_LEFT)==1)
 {
   image_single+=1;
   if (image_single>36)
   {
    image_single=0;         
   }                        
 }
 
 //image_single=int(dir/10);
 
 if (destroyable>0)
 {
   destroyable-=1;                  
 }
 
 dir=image_single*10;
 x+=speed*cos_loTRUE2up[dir];
 y+=speed*sin_loTRUE2up[dir];
 
 if (int(x)<=-46)
 {
   x=640;
 }
 
 if (int(y)<=-46)
 {
   y=480;
 }
 
 if (int(x)>640)
 {
   x=-46;
 }
 
 if (int(y)>480)
 {
   y=-46;
 }
 
 if (alarm0>0)
 {
   alarm0-=1;
   if (alarm0==0)
   {
   can_shot=1;              
   }
 }
 
 if (check_key(VK_SPACE)==1)
 {
   if (can_shot==1)
   {
   create_player_shot_object(x, y, dir,image_single,8);
   can_shot=0;
   alarm0=10;
   }
 }
 
 if (speed==0)
 {
   collision=check_collision_with_rock(player_sprite,int(x),int(y),image_single);
 }
 else
 {
   collision=check_collision_with_rock(player_moveing_sprite,int(x),int(y),image_single);
 }

 if ((collision!=-1) && (destroyable==0) )
 {
   rock_freelist[collision]=0;
   //score+=10;
   x=(640/2)-23; //set player starting x
   y=(480/2)-23; 
   destroyable=60;
   speed=0;
   image_single=0;
   lives-=1;
   for (i=0; i<100; i++)
   {
     create_pixel1_object(rock_objects[collision].x, rock_objects[collision].y,rand()%360,4+rand()%6);
   }                        
 }
 
 pixel1_object_loop();
 rock_object_loop();
 player_shot_object_loop();
 if (destroyable>0)
 {
  COLORREF col=0;
  int result=0;
  result=SetBkMode(hdcBuffer,1); // 1 or 2
  col=SetTextColor(hdcBuffer,RGB(255,255,255));
  //result=SetBkMode(hdcBuffer,0); // 1 or 2
  //col=SetTextColor(hdcBuffer,RGB(0,0,0)); 
                   
 }
 
 if (speed==0)
 {
  drawspritetran(player_sprite,player_mask,int(x),int(y),image_single);
 }
 else
 {
  drawspritetran(player_moveing_sprite,player_moveing_mask, int(x),int(y),image_single);   
 }
 
 for (i=0; i<lives;i++)
 {
    drawspritetran2(lives_image,lives_mask,(80+(20*i)),454);
 }
 char string1[100];
 
 drawstring2(18, 458, "Lives:",RGB(255,255,0),12,16);
 sprintf(string1,"Score:%d",score);
 drawstring2(498, 458, string1, RGB(255,255,0),12,16);
 frames+=1;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)				  /* handle the messages */
	{
		case WM_CREATE: //init the game system
		 {
		  hdc = GetDC(hwnd);
		  hdcBuffer = CreateCompatibleDC(hdc);
		  hbmBuffer = CreateCompatibleBitmap(hdc, 640, 480);
		  hbmOldBuffer =(HBITMAP) SelectObject(hdcBuffer, hbmBuffer);
		  background0=(HBITMAP)LoadImage(NULL, "background0.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(background0 == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
		    
          pixel_image=(HBITMAP)LoadImage(NULL, "pixel.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(pixel_image == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          pixel_mask= CreateBitmapMask(pixel_image, RGB(255,0,255));
		    
		  lives_image=(HBITMAP)LoadImage(NULL, "lives.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(lives_image == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          lives_mask= CreateBitmapMask(lives_image, RGB(255,0,255));
          
		  player_sprite=(HBITMAP)LoadImage(NULL, "player.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(player_sprite == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          player_mask= CreateBitmapMask(player_sprite, RGB(255,0,255));
          
          player_moveing_sprite=(HBITMAP)LoadImage(NULL, "player_moveing.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(player_moveing_sprite == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          player_moveing_mask= CreateBitmapMask(player_moveing_sprite, RGB(255,0,255));
          
          player_shot_sprite=(HBITMAP)LoadImage(NULL, "player_shot.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(player_shot_sprite == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          player_shot_mask= CreateBitmapMask(player_shot_sprite, RGB(255,0,255));
          
          rock_sprite=(HBITMAP)LoadImage(NULL, "rock.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
		  if(rock_sprite == NULL)
		    MessageBox(hwnd, "Could not load bitmap!", "Error", MB_OK | MB_ICONEXCLAMATION);
          rock_mask= CreateBitmapMask(rock_sprite, RGB(255,0,255));
          
		  int ret = SetTimer(hwnd, ID_TIMER, 31, NULL);
		  if(ret == 0)
		   MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);
	      
          srand( (unsigned)clock(  ) ); // init Rand() function
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          create_rock_object(300, 220, rand() %360 , 2+rand() %3);
          x=(640/2)-23; //set player starting x
          y=(480/2)-23; 
          destroyable=60;
          speed=0;
          image_single=0;
          
	      sin_cos_table_init();
	      switch_window(hwnd);
		  
		  
		  }
		  break;
		  case WM_TIMER:
		  {
		   BITMAP bm;
		   GetObject(hbmBuffer, sizeof(bm), &bm);
		   drawbitmap(background0,0,0);
		   //drawrectangle(0,0,bm.bmWidth ,bm.bmHeight,RGB(0,0,0),RGB(0,0,0),1);
		   //drawrectangle(0,0,bm.bmWidth ,bm.bmHeight,RGB(255,255,255),RGB(255,255,255),1);
		   game_loop(hwnd);
		   //char string1[100];
		   //sprintf(string1,"fps %d  sec %d",fps,time1);
		   SetWindowTextA(hwnd,"Asteroids Written By Gareth Mc Daid In C++ 2008  (GDI)");
		   BitBlt(hdc, offsetx, offsety, bm.bmWidth ,bm.bmHeight , hdcBuffer, 0, 0, SRCCOPY);
		  }
		  break;
		  case WM_KEYUP:
		  {
			  remove_key(wParam);
		  }
		  break;
		  case WM_KEYDOWN:
		  {
			  add_key(wParam);
		  }
		  break;
		  case WM_SIZE:
		  {
			//DeleteDC(hdcBuffer);
			//DeleteObject(hbmBuffer);
			//DeleteObject(hbmOldBuffer);
			//hdcBuffer = CreateCompatibleDC(hdc);
			//hbmBuffer = CreateCompatibleBitmap(hdc, LOWORD(lParam), HIWORD(lParam));
			//hbmOldBuffer =(HBITMAP) SelectObject(hdcBuffer, hbmBuffer);
			//window_right=LOWORD(lParam);
			//window_bottom=HIWORD(lParam);
		  }
		  break;
		  case	WM_MOUSEMOVE:
		  {
			mousex = lParam & 0xffff;
			mousey = lParam >> 16;
		  }
		  break;
		  case WM_LBUTTONDOWN:
		  {

		  }
		  break;
		  case WM_DESTROY:
		  {
			DeleteObject(player_sprite);
			DeleteObject(player_mask);
			DeleteObject(player_shot_sprite);
			DeleteObject(player_shot_mask);
			DeleteObject(rock_sprite);
			DeleteObject(rock_mask);
			DeleteObject(lives_image);
			DeleteObject(lives_mask);
			DeleteObject(pixel_image);
			DeleteObject(pixel_mask);
			DeleteObject(background0);
			KillTimer(hwnd, ID_TIMER);
			DeleteDC(hdc);
			DeleteDC(hdcBuffer);
			DeleteObject(hbmBuffer);
			DeleteObject(hbmOldBuffer);
			PostQuitMessage (0);
		  }
		  break;

		default:
			return DefWindowProc (hwnd, message, wParam, lParam);
	}
	return 0;
}
