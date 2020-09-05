/*fpsconsole.c - A first person shooter engine done in CLI*/
#define _XOPEN_SOURCE_EXTENDED
#include <curses.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
int nScreenWidth = 120;
int nScreenHeight = 40;
int nWarpSpeed = 100;

double fPlayerX = 8.0f;
double fPlayerY = 8.0f;
double fPlayerA = 0.0f;

int nMapWidth = 16;
int nMapHeight = 16;
char map[16*16];
double fFOV = 3.142159 / 4.0f;
double fDepth = 16.0f;

void setup();
void map_setup(char arr[]);

int main(void)
{
  setlocale(LC_ALL, "");
  wchar_t screen[nScreenHeight*nScreenWidth+1];
  double tp1 = clock();
  double tp2 = clock();
  int frames = 0;
  int t1 = time(NULL);
  int t2 = time(NULL);
  int fps  = 0;
  //generate map string
  map_setup(map);
  // Ncurses SETUP
  setup();

  while(1)
  {
    tp2 = clock();
    double fElapsedTime = ((tp2 - tp1) / CLOCKS_PER_SEC);
    tp1 = tp2;
    t2 = time(NULL);

    //Input handling
    char cInput = getch();
    if(cInput == 'a')
      fPlayerA -= 0.1f * fElapsedTime * nWarpSpeed;
    if(cInput == 'd')
      fPlayerA += 0.1f * fElapsedTime * nWarpSpeed;
    if(cInput == 'w')
    {
      fPlayerX += sin(fPlayerA) * 5.0f * fElapsedTime * nWarpSpeed;
      fPlayerY += cos(fPlayerA) * 5.0f * fElapsedTime * nWarpSpeed;
    }
    if(cInput == 's')
      {
        fPlayerX -= sin(fPlayerA) * 5.0f * fElapsedTime * nWarpSpeed;
        fPlayerY -= cos(fPlayerA) * 5.0f * fElapsedTime * nWarpSpeed;
      }

    for(int x = 0; x < nScreenWidth; x++)
    {
      double fRayAngle = (fPlayerA - fFOV / 2.0f) + ((double)x / (double)nScreenWidth) *fFOV;
      double fDistanceToWall = 0;
      bool bHitWall = false;

      double fEyeX = sin(fRayAngle);
      double fEyeY = cos(fRayAngle);

      while(!bHitWall && fDistanceToWall < fDepth)
      {
        fDistanceToWall += 0.1f;

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
          }
        }

        //Calculate distance to ceiling and floor
        int nCeiling = (double)(nScreenHeight / 2.0) - nScreenHeight / ((double)fDistanceToWall);
        int nFloor = nScreenHeight - nCeiling;


        short sShader = ' ';

        if(fDistanceToWall <= fDepth/4.0f) sShader = L'\u2588';
        else if(fDistanceToWall < fDepth/3.0f) sShader = L'\u2593';
        else if(fDistanceToWall < fDepth/2.0f) sShader = L'\u2592';
        else if(fDistanceToWall < fDepth) sShader = L'\u2591';
        else sShader = ' ';

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
    screen[nScreenHeight*nScreenWidth-1] = '\0';
    move(0,0);
    addwstr(screen);
    move(0,0);
    printw("El Time: %f | fps: %d/60 ", fElapsedTime, fps);
    refresh();
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
  char map02[] = "#..............#";
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
  char map13[] = "#..............#";
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
