/*fpsconsole.c - A first person shooter engine done in CLI*/
#define _XOPEN_SOURCE_EXTENDED
#include <curses.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include "sglib.h"
#define sinf(x) (float)sin((double)(x))
#define cosf(x) (float)cos((double)(x))

//Global VARS
int nScreenWidth = 120;
int nScreenHeight = 40;
int nWarpSpeed = 100;

//Player coordinates on map and facing angle
double fPlayerX = 8.0f;
double fPlayerY = 8.0f;
double fPlayerA = 0.0f;

int nMapWidth = 16;
int nMapHeight = 16;
char map[16*16];

double fFOV = 3.142159 / 4.0f;
double fDepth = 16.0f;

//Struct for the vector array containing
// distance and dot product to calculate
// corners of tiles + a sort define for SGLIB
struct vec {
  float d;
  float dot;
};
#define CMP_VEC(p1,p2) ((float)((p1.d) - (p2.d)))

//function definitions
void setup();
void map_setup(char arr[]);

//MAIN
int main(void)
{
  setlocale(LC_ALL, "");         //needed for wide chars(Unicode) display
  struct vec {
    float d;
    float dot;
  } p[2*2];
  int pi = 0;                    //p array iterator

  // Screen buffer
  wchar_t screen[nScreenHeight*nScreenWidth];
  // FPS vars
  int t1, t2;
  int frames = 0;
  int fps  = 0;
  struct timespec tstart={0,0}, tend={0,0};
  clock_settime(CLOCK_MONOTONIC, &tstart);
  clock_settime(CLOCK_MONOTONIC, &tend);

  //generate map string
  map_setup(map);

  // SETUP
  setup();

  //Main LOOP
  while(1)
  {
    clock_gettime(CLOCK_MONOTONIC, &tend);
    double fElapsedTime = timespec_diff(tend, tstart);
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    t2 = time(NULL);

    //Input handling
    char cInput = getch();
    if(cInput == 'a')
      fPlayerA -= 0.1f * fElapsedTime;
    if(cInput == 'd')
      fPlayerA += 0.1f * fElapsedTime;
    if(cInput == 'w')
    {
      fPlayerX += cosf(fPlayerA) * 5.0f * fElapsedTime;
      fPlayerY += sinf(fPlayerA) * 5.0f * fElapsedTime;
      if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
      {
        fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;
        fPlayerY -= sinf(fPlayerA) * 5.0f * fElapsedTime;
      }
    }
    if(cInput == 's')
      {
        fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;
        fPlayerY -= sinf(fPlayerA) * 5.0f * fElapsedTime;
        if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
          {
            fPlayerX += cosf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += sinf(fPlayerA) * 5.0f * fElapsedTime;
          }
      }

    for(int x = 0; x < nScreenWidth; x++)
    {
      float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) *fFOV;
      float fDistanceToWall = 0;
      bool bHitWall = false;

      float fEyeX = cosf(fRayAngle);
      float fEyeY = sinf(fRayAngle);
      bool bBoundry = false;

      while(!bHitWall && fDistanceToWall < fDepth)
      {
        fDistanceToWall += 0.1f;
        pi = 0;
        int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
        int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

        //Test if ray out of bounds
        if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
        {
          bHitWall = true;
          fDistanceToWall = fDepth;
        }
        else
        {
          if(map[nTestY * nMapWidth + nTestX] == '#')
          {
            bHitWall = true;
            for(int tx = 0; tx < 2; tx++)
              for(int ty = 0;ty < 2; ty++)
              {
                float vy = (float)nTestY + ty - fPlayerY;
                float vx = (float)nTestX + tx - fPlayerX;
                float dist = sqrt(vx*vx + vy*vy);
                float dot = (fEyeX * vx / dist) + (fEyeY * vy / dist);
                p[pi].d = dist;
                p[pi].dot = dot;
                pi++;
              }

            SGLIB_ARRAY_SINGLE_HEAP_SORT(struct vec, p, 2*2, CMP_VEC);

            float fBound = 0.005;
            if(acos(p[0].dot) < fBound) bBoundry = true;
            if(acos(p[1].dot) < fBound) bBoundry = true;
          }
        }

        //Calculate distance to ceiling and floor
        int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
        int nFloor = nScreenHeight - nCeiling;


        wchar_t sShader = L' ';

        if(fDistanceToWall <= fDepth/4.0f) sShader = L'\u2588';
        else if(fDistanceToWall < fDepth/3.0f) sShader = L'\u2593';
        else if(fDistanceToWall < fDepth/2.0f) sShader = L'\u2592';
        else if(fDistanceToWall < fDepth) sShader = L'\u2591';
        else sShader = L' ';

        if(bBoundry) sShader = L' ';

        //Render to screen buffer
        for(int y = 0; y < nScreenHeight; y++)
        {
          if(y <= nCeiling)
            screen[y*nScreenWidth + x] = L' ';
          else if(y > nCeiling && y <= nFloor)
            screen[y*nScreenWidth + x] = sShader;
          else
          {
            double b = 1.0f - (((double)y - (nScreenHeight / 2.0f)) / ((float)nScreenHeight / 2.0f));
            if(b < 0.25) sShader = L'#';
            else if(b < 0.5) sShader = L'x';
            else if(b < 0.75) sShader = L'.';
            else if(b < 0.9) sShader = L'-';
            else sShader = L' ';
            screen[y*nScreenWidth + x] = sShader;
          }
        }
      }
    }

    //Display
    move(0,0);
    addwstr(screen);
    move(0,0);
    printw("El Time: %f | fps: %d/60 | X: %d Y: %d | Angle: %f",
           fElapsedTime, fps, (int)fPlayerX, (int)fPlayerY, fPlayerA);
    for(int y = 0; y < nMapHeight; y++)
      for(int x = 0; x < nMapWidth; x++)
        mvaddch(y+1,x,map[x+(y*nMapHeight)]);
    mvaddch((int)fPlayerY+1,(int)fPlayerX,'P');
    refresh();

    //Frame count
    frames++;
    if(t2-t1 >= 1)
    {
      fps = frames / 60;
      frames = 0;
      t1 = t2;
    }
  }
  endwin();
  return 0;
}

// --------FUNCS-----------
void setup()
{
  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);
  resizeterm(nScreenHeight, nScreenWidth);
}

void map_setup(char arr[])
{
  char map00[] = "################";
  char map01[] = "#..............#";
  char map02[] = "#.........#....#";
  char map03[] = "#..............#";
  char map04[] = "#..............#";
  char map05[] = "#..............#";
  char map06[] = "#..............#";
  char map07[] = "#..............#";
  char map08[] = "#..............#";
  char map09[] = "#..............#";
  char map10[] = "#..............#";
  char map11[] = "#..............#";
  char map12[] = "#..............#";
  char map13[] = "#.....##########";
  char map14[] = "#..............#";
  char map15[] = "################";
  strcat(map00, map01);
  strcat(map00, map02);
  strcat(map00, map03);
  strcat(map00, map04);
  strcat(map00, map05);
  strcat(map00, map06);
  strcat(map00, map07);
  strcat(map00, map08);
  strcat(map00, map09);
  strcat(map00, map10);
  strcat(map00, map11);
  strcat(map00, map12);
  strcat(map00, map13);
  strcat(map00, map14);
  strcat(map00, map15);
  strcpy(arr, map00);
}
