extern "C" {
#ifndef SDL_KIT
#include "./sdl/include/SDL.h"
#define SDL_KIT
#endif
}
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#define HERO_OFFSET 26
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif
#include "Timer.h"

// Settings

#define FRAMES_PER_SECOND 60

// Comment out items to hide

#define ALPHA
//#define GRID
//#define ROADS
#define MAP

enum orientation {
	horizontal,
	vertical
};

struct Fpoint {
	double x;
	double y;
};

// DrawString
// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

// DrawScreen
// obliczenie pozycji w kwadracie jednostkowym
Fpoint PositionInSquare(double worldTime) {

	Fpoint p;
	switch((int)worldTime % 4) {
	case 0:
		p.x = fmod(worldTime, 1.0);
		p.y = 0;
		break;
	case 1:
		p.x = 1;
		p.y = fmod(worldTime, 1.0);
		break;
	case 2:
		p.x = 1 - fmod(worldTime, 1.0);
		p.y = 1;
		break;
	case 3:
		p.x = 0;
		p.y = 1 - fmod(worldTime, 1.0);
		break;
	}
	return p;
}

// Putpixel
//rysuje pixel
void Putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
}

// DrawLine
// rysowanie linii o d≥ugoúci l w pionie bπdü poziomie
void DrawLine(SDL_Surface *screen, double x, double y, int l, orientation o, Uint32 pixel) {
	
	for (int i = 0; i < l; i++) {
		Putpixel(screen, x, y, pixel);
		(o==horizontal) ? x++ : y++;
	}
}

// DrawLine3
// rysowanie linii o gruboúci 3 o dlugosci l w pionie bπdü poziomie
void DrawLine3(SDL_Surface *screen, double x, double y, int l, orientation o, Uint32 pixel) 
{
	for (int i = 0; i < l; i++) 
	{
		if (o == horizontal)
		{
			Putpixel(screen, x, y - 1, pixel);
			Putpixel(screen, x, y + 1, pixel);
		}
		else
		{
			Putpixel(screen, x - 1, y, pixel);
			Putpixel(screen, x + 1, y, pixel);
		}
		Putpixel(screen, x, y, pixel);
		
		(o == horizontal) ? x++ : y++;
	}
}

// DrawSquare
// rysowanie prostokπta o d≥ugoúci boku l
void DrawSquare(SDL_Surface *screen, int x, int y, int l, Uint32 pixel) {
	DrawLine(screen, x, y, l, vertical, pixel);
	DrawLine(screen, x + l, y, l, vertical, pixel);
	DrawLine(screen, x, y, l, horizontal, pixel);
	DrawLine(screen, x, y + l, l, horizontal, pixel);
}

enum BlockType
{
	Empty,
	Road,
	Booster,
	Food,
	LeftTopCurve,
	LeftBottomCurve,
	RightTopCurve,
	RightBottomCurve,
	HorizontalLine,
	VerticalLine
};

// Basic map element
struct Block
{
	int x;
	int y;
	BlockType type;
};

#define BLUE SDL_MapRGB(screen->format, 0x00, 0x00, 0xff)
#define GRAY SDL_MapRGB(screen->format, 0xBE, 0xBE, 0xBE)
#define DARK_GRAY SDL_MapRGB(screen->format, 139, 137, 137)
#define MARGIN_SIZE 10

// Returns block size (length of square wall)
int GetBlockSize(int width, int height)
{
	int sqsize = height / 31;
	if (sqsize * 28 > width)
		sqsize = width / 28;
	return sqsize;
}

#define CIRCLE_EQUATION (x-orx)*(x-orx)+(y-ory)*(y-ory)
// DrawCurve3
// rysowanie krzywej o grubosci 3 o orientacji type
void DrawCurve3(SDL_Surface *screen, int x0, int y0, int blockSize, BlockType type, Uint32 pixel)
{
	int x_end = x0 + blockSize;
	int y_end = y0 + blockSize;
	int r = blockSize / 2;
	int orx = 0;
	int ory = 0;
	
	switch (type)
	{
		case RightBottomCurve:
			orx = x_end;
			ory = y_end;
			break;
		case LeftBottomCurve:
			orx = x0;
			ory = y_end;
			break;
		case RightTopCurve:
			orx = x_end;
			ory = y0;
			break;
		case LeftTopCurve:
			orx = x0;
			ory = y0;
			break;
	}

	for (int x = x0; x <= x_end; ++x)
	for (int y = y0; y <= y_end; ++y)
	{
		if (CIRCLE_EQUATION > (r - 1)*(r - 1) && CIRCLE_EQUATION < (r + 1)*(r + 1) + blockSize)
			Putpixel(screen, x, y, pixel);
	}

}

// Draws a block at selected origin coords
void DrawBlock(SDL_Surface *screen,int x, int y, int blockSize)
{
	// top horiz
	DrawLine(screen, x, y, blockSize, horizontal, GRAY);
	// bottom horiz
	DrawLine(screen, x, y + blockSize, blockSize, horizontal, GRAY);
	// left vert
	DrawLine(screen, x, y, blockSize, vertical, GRAY);
	// right vert
	DrawLine(screen, x + blockSize, y, blockSize, vertical, GRAY);
}

// Draws the grid shadow
void DrawGrid(SDL_Surface *screen, Block blocks[28][31], int x1, int y1, int x2, int y2, int blockSize)
{
	for (int x = x1, s=0; s < 28; s++, x = x + blockSize)
	{
		for (int y = y1, n = 0; n < 31; n++, y = y + blockSize)
		{
#ifdef GRID
			DrawBlock(screen, x, y, blockSize);
#endif
			Block *bl = &blocks[s][n];
			bl->x = x;
			bl->y = y;
		}
	}
}

// Fill screen with appropriate block types
void PaintGrid(SDL_Surface *screen, Block blocks[28][31], int blockSize)
{
	for (int s = 0; s < 28; s++)
	for (int n = 0; n < 31; n++)
	{
		Block block = blocks[s][n];
		switch (block.type)
		{

#ifdef ROADS
			case Road:
			{
			SDL_Rect rct;
			rct.h = blockSize;
			rct.w = blockSize;
			rct.x = block.x;
			rct.y = block.y;
			SDL_FillRect(screen, &rct, DARK_GRAY);
			break;
			}
#endif

#ifdef MAP
			case HorizontalLine:
				DrawLine3(screen, block.x, block.y + blockSize / 2, blockSize, horizontal, BLUE);
				break;
			case VerticalLine:
				DrawLine3(screen, block.x + blockSize / 2, block.y, blockSize, vertical, BLUE);
				break;
			case RightBottomCurve:
				DrawCurve3(screen, block.x, block.y, blockSize, RightBottomCurve, BLUE);
				break;
			case LeftBottomCurve:
				DrawCurve3(screen, block.x, block.y, blockSize, LeftBottomCurve, BLUE);
				break;
			case RightTopCurve:
				DrawCurve3(screen, block.x, block.y, blockSize, RightTopCurve, BLUE);
				break;
			case LeftTopCurve:
				DrawCurve3(screen, block.x, block.y, blockSize, LeftTopCurve, BLUE);
				break;
#endif
		}
	}
}

void UpdatePaths(SDL_Surface *screen, Block blocks[28][31], int blockSize)
{
	for (int s = 0; s < 28; s++)
	for (int n = 0; n < 31; n++)
	{
		Block block = blocks[s][n];
		if (block.type == Road)
		{
			SDL_Rect rct;
			rct.h = blockSize+6;
			rct.w = blockSize+6;
			rct.x = block.x-3;
			rct.y = block.y-3;
			SDL_FillRect(screen, &rct, 0x00);
		}
	}
}

enum Direction
{
	NONE,
	UP,
	DOWN,
	LEFT,
	RIGHT
};

// Behaviour indicators
enum Behaviour
{
	BLINKY,
	PINKY,
	INKY,
	CLYDE
};

// Sprite abstract class
class Sprite
{
public:
	virtual void show() = 0;

	SDL_Surface *screen;
	SDL_Surface *dirPic[5];

	int x;
	int y;
	int speed;
	Direction dir;
};

// PacMan
class PacMan : public Sprite
{
public:
	Direction delayedDir;
	int delayedTime;

	PacMan()
	{
		dir = LEFT;
		delayedDir = NONE;
		delayedTime = 0;
		speed = 2;
		dirPic[LEFT] = SDL_LoadBMP("pacman_left.bmp");
		dirPic[RIGHT] = SDL_LoadBMP("pacman_right.bmp");
		dirPic[UP] = SDL_LoadBMP("pacman_up.bmp");
		dirPic[DOWN] = SDL_LoadBMP("pacman_down.bmp");

		for (int i = 1; i <= 4; i++)
		{
			if (dirPic[i] == NULL)
				printf("\nAccess violation!");
			else
				SDL_SetColorKey(dirPic[i], SDL_SRCCOLORKEY, 0x000000);
		}
	}

	~PacMan()
	{
		SDL_FreeSurface(dirPic[LEFT]);
		SDL_FreeSurface(dirPic[RIGHT]);
		SDL_FreeSurface(dirPic[UP]);
		SDL_FreeSurface(dirPic[DOWN]);
	}

	void move(Direction dir)
	{
		this->dir = dir;
	}
	virtual void show()
	{
		SDL_Surface *pic;
		switch (this->dir)
		{
			case LEFT:
				pic = dirPic[LEFT];
				break;
			case RIGHT:
				pic = dirPic[RIGHT];
				break;
			case UP:
				pic = dirPic[UP];
				break;
			case DOWN:
				pic = dirPic[DOWN];
				break;
		}
		
		DrawSurface(screen, pic, x, y);
	}
	void setDir(Direction newDirection, int worldTime)
	{
		delayedDir = newDirection;
		delayedTime = worldTime + 1000;
	}
};

typedef const unsigned short cushort;

// Ghost
class Ghost : public Sprite
{
private:
	static cushort NUMBER_OF_GHOSTS = 4;
	static cushort NUMBER_OF_DIRECTIONS = 4;
	Behaviour behaviour;
	SDL_Surface *dirPic[NUMBER_OF_GHOSTS][NUMBER_OF_DIRECTIONS];
public:
	Ghost()
	{
		speed = 1;
		dir = LEFT;
		dirPic[BLINKY][LEFT] = SDL_LoadBMP("blinky.bmp");
		dirPic[PINKY][LEFT] = SDL_LoadBMP("pinky.bmp");
		dirPic[INKY][LEFT] = SDL_LoadBMP("inky.bmp");
		dirPic[CLYDE][LEFT] = SDL_LoadBMP("clyde.bmp");
	}
	void setBehaviour(Behaviour bhv)
	{
		this->behaviour = bhv;
	}

	Behaviour getBehaviour()
	{
		return this->behaviour;
	}

	virtual void show()
	{
		if (dirPic[behaviour][LEFT] == NULL)
			printf("Access violation!");
		else
			DrawSurface(screen, dirPic[behaviour][LEFT], x, y);
	}

	~Ghost()
	{
		SDL_FreeSurface(dirPic[behaviour][LEFT]);
	}
};

struct Path
{
	int x;
	int y;
};

/************************************************************************/
/* Abstract class for basic functionality of maps						*/
/************************************************************************/
/*@author DualCore*/
class PlayableMap
{
public:
	virtual void DrawMap(SDL_Surface *screen, double worldTime) = 0;

	PacMan *pMan;
	Ghost *ghosts[4];
	Block blocks[29][31];
	Path paths[300];
	int height;
	int width;
	int bSize;
	double lastWT;
	double delta;
	
	virtual void setPacMan(PacMan*) = 0;
	virtual void setBlinky(Ghost*) = 0;
	virtual void setPinky(Ghost*) = 0;
	virtual void setInky(Ghost*) = 0;
	virtual void setClyde(Ghost*) = 0;
};

void MoveSprite(PlayableMap *map, SDL_Surface *screen, Sprite *sprite, Path paths[300], int bSize, int worldTime);

/* Poziom 1 */
class Level1 : public PlayableMap
{
public:
	Level1(SDL_Surface *screen)
	{
		height = screen->h;
		width = screen->w;
		
		bSize = GetBlockSize(width, height);

		int x1 = width / 2 - 14 * bSize, x2 = width / 2 + 14 * bSize;
		int y1 = 0 + MARGIN_SIZE, y2 = height - MARGIN_SIZE;

		DrawGrid(screen, blocks, x1, y1, x2, y2, bSize);

		//----- Plan half of the map -----
		for (int x = 0; x <= 13; x++)
		for (int y = 0; y <= 30; y++)
		{
			if (x > 0 && x<13 && y == 1)
				goto draw;
			else
			if (y > 1 && y < 5 && (x == 1 || x == 6 || x == 12))
				goto draw;
			else
			if (x > 0 && y == 5)
				goto draw;
			else
			if ((x == 1 || x == 6 || x == 9) && y > 5 && y < 8)
				goto draw;
			else
			if (y == 8 && (x > 0 && x < 7 || x > 8 && x < 13))
				goto draw;
			else
			if ((x == 6 || x == 12) && y > 8 && y < 11)
				goto draw;
			else
			if ((x == 6 || x>8) && y == 11)
				goto draw;
			else
			if ((x == 6 || x == 9) && y > 11 && y < 14)
				goto draw;
			else
			if (x < 10 && y == 14)
				goto draw;
			else
			if ((x == 6 || x == 9) && (y > 14 && y < 17 || y>17 && y < 20))
				goto draw;
			else
			if ((x == 6 || x>8) && y == 17)
				goto draw;
			else
			if (x > 0 && x < 13 && y == 20)
				goto draw;
			else
			if (y > 20 && y < 23 && (x == 1 || x == 6 || x == 12))
				goto draw;
			else
			if (y == 23 && (x > 0 && x < 4 || x > 5))
				goto draw;
			else
			if (y > 23 && y < 26 && (x == 3 || x == 6 || x == 9))
				goto draw;
			else
			if (y == 26 && (x > 0 && x < 7 || x > 8 && x < 13))
				goto draw;
			else
			if (y > 26 && y < 29 && (x == 1 || x == 12))
				goto draw;
			else
			if (y == 29 && x>0)
				goto draw;

			if (false)
			{
			draw:
				Block *bl = &blocks[x][y];
				bl->type = Road;
			}
		}
		//--------------------------------

		//----- Mirror the other half -----
		for (int x = 14; x <= 27; x++)
		for (int y = 0; y <= 30; y++)
		{
			blocks[x][y].type = blocks[27 - x][y].type;
		}
		//---------------------------------

#define TOP_MID x][y-1
#define TOP_LEFT x-1][y-1
#define TOP_RIGHT x+1][y-1
#define LEFT x-1][y
#define MID x][y
#define RIGHT x+1][y
#define BOTTOM_MID x][y+1
#define BOTTOM_LEFT x-1][y+1
#define BOTTOM_RIGHT x+1][y+1
		int coll = 0;
		// Raise the walls
		for (int x = 0; x <= 27; x++)
		for (int y = 0; y <= 30; y++)
		{
			if (blocks[x][y].type == Road)
			{
				paths[coll].x = blocks[MID].x;
				paths[coll].y = blocks[MID].y;
				coll++;
				continue;
			}	

			// RightBottomCurve
			if (blocks[BOTTOM_RIGHT].type == Road && blocks[BOTTOM_MID].type != Road && blocks[RIGHT].type != Road)
				blocks[MID].type = RightBottomCurve;
			else
			if (blocks[BOTTOM_RIGHT].type != Road && (x!=0 && blocks[LEFT].type == Road) && blocks[TOP_MID].type == Road)
				blocks[MID].type = RightBottomCurve;
			// ---------------

			// RightTopCurve
			if (blocks[TOP_RIGHT].type == Road && blocks[TOP_MID].type != Road && blocks[RIGHT].type != Road)
				blocks[MID].type = RightTopCurve;
			else
			if (blocks[TOP_RIGHT].type != Road && blocks[LEFT].type == Road && blocks[BOTTOM_MID].type == Road)
				blocks[MID].type = RightTopCurve;
			// --------------

			// LeftBottomCurve
			if ((blocks[BOTTOM_LEFT].type == Road && x!=0) && blocks[BOTTOM_MID].type != Road && blocks[LEFT].type != Road)
				blocks[MID].type = LeftBottomCurve;
			else
			if (blocks[BOTTOM_LEFT].type != Road && blocks[TOP_MID].type == Road && blocks[RIGHT].type == Road)
				blocks[MID].type = LeftBottomCurve;
			// ---------------

			// LeftTopCurve
			if ((blocks[TOP_LEFT].type == Road && x!=0) && blocks[TOP_MID].type != Road && blocks[LEFT].type != Road)
				blocks[MID].type = LeftTopCurve;
			else
			if (blocks[TOP_LEFT].type != Road && blocks[RIGHT].type == Road && blocks[BOTTOM_MID].type == Road)
				blocks[MID].type = LeftTopCurve;
			// ------------

			// HorizontalLines
			if (x == 0 && (blocks[TOP_MID].type == Road || blocks[BOTTOM_MID].type == Road) && blocks[RIGHT].type != Road)
				blocks[MID].type = HorizontalLine;
			else
			if (x == 27 && (blocks[TOP_MID].type == Road || blocks[BOTTOM_MID].type == Road) && blocks[LEFT].type != Road)
				blocks[MID].type = HorizontalLine;
			else
			if ((blocks[TOP_MID].type == Road || blocks[BOTTOM_MID].type == Road) && blocks[LEFT].type != Road && blocks[RIGHT].type != Road)
				blocks[MID].type = HorizontalLine;
			// ----------------

			// VerticalLines
			if (y == 0 && (blocks[LEFT].type == Road || blocks[RIGHT].type == Road) && blocks[BOTTOM_MID].type != Road)
				blocks[MID].type = VerticalLine;
			else
			if (y == 30 && (blocks[LEFT].type == Road || blocks[RIGHT].type == Road) && blocks[TOP_MID].type != Road)
				blocks[MID].type = VerticalLine;
			else
			if ((blocks[LEFT].type == Road && x != 0 || blocks[RIGHT].type == Road && x != 27) && blocks[TOP_MID].type != Road && blocks[BOTTOM_MID].type != Road)
				blocks[MID].type = VerticalLine;
			// --------------
		}

		PaintGrid(screen, blocks, bSize);
#undef LEFT
#undef RIGHT
	}
	
	void setPacMan(PacMan *pman)
	{
		lastWT = 0;
		delta = 0;
		pMan = pman;

		// Set PacMan starting position
		Block *pmanstart = &blocks[13][11];
		pMan->x = pmanstart->x;
		pMan->y = pmanstart->y;
		// ---------------------------		
	}
	
	virtual void setBlinky(Ghost *ghs)
	{
		ghosts[BLINKY] = ghs;

		Block *blinkystart = &blocks[14][11];
		ghosts[BLINKY]->x = blinkystart->x;
		ghosts[BLINKY]->y = blinkystart->y;
	}

	virtual void setPinky(Ghost *ghs)
	{
		ghosts[PINKY] = ghs;

		Block *blinkystart = &blocks[14][11];
		ghosts[PINKY]->x = blinkystart->x;
		ghosts[PINKY]->y = blinkystart->y;
	}

	virtual void setInky(Ghost *ghs)
	{
		ghosts[INKY] = ghs;

		Block *blinkystart = &blocks[14][11];
		ghosts[INKY]->x = blinkystart->x;
		ghosts[INKY]->y = blinkystart->y;
	}

	virtual void setClyde(Ghost *ghs)
	{
		ghosts[CLYDE] = ghs;

		Block *blinkystart = &blocks[14][11];
		ghosts[CLYDE]->x = blinkystart->x;
		ghosts[CLYDE]->y = blinkystart->y;
	}
	
	virtual void DrawMap(SDL_Surface *screen, double worldTime)
	{
		UpdatePaths(screen, blocks, bSize);



		if (worldTime > 1000)
		{
			MoveSprite(this, screen, ghosts[BLINKY], paths, bSize, worldTime);
			MoveSprite(this, screen, ghosts[PINKY], paths, bSize, worldTime);
			MoveSprite(this, screen, ghosts[INKY], paths, bSize, worldTime);
			MoveSprite(this, screen, ghosts[CLYDE], paths, bSize, worldTime);
			MoveSprite(this, screen, pMan, paths, bSize, worldTime);
		}
		else
		{
			pMan->show();
			ghosts[BLINKY]->show();
			ghosts[PINKY]->show();
			ghosts[INKY]->show();
			ghosts[CLYDE]->show();
		}
	}
};

bool IsPointReachable(int x, int y, Path paths[300], int bSize)
{
	for (int i = 0; i < 300; i++)
	if (x == paths[i].x && y >= paths[i].y && y <= paths[i].y + bSize || y == paths[i].y && x >= paths[i].x && x <= paths[i].x + bSize)
		return true;
	
	return false;
}

bool IsAbleToMove(Sprite *sprite, Direction dir, Path paths[300], int bSize)
{
	int orx = sprite->x;
	int ory = sprite->y;
	int nextCellX = orx;
	int nextCellY = ory;
	switch (dir)
	{
		case UP:
			nextCellY -= bSize;
			break;
		case DOWN:
			nextCellY += bSize;
			break;
		case LEFT:
			nextCellX -= bSize;
			break;
		case RIGHT:
			nextCellX += bSize;
			break;
	}
	
	int step = sprite->speed;

	for (int i = 0; i < 300; i++)
	{
		switch (dir)
		{
			case LEFT:
				if (sprite->y == paths[i].y && sprite->x - step >= paths[i].x && sprite->x - step <= paths[i].x + bSize)
					return true;
				break;
			case RIGHT:
				if (sprite->y == paths[i].y && sprite->x + step >= paths[i].x && sprite->x + step <= paths[i].x + bSize)
				if (IsPointReachable(sprite->x + bSize + step, sprite->y, paths, bSize)) // Extreme right
					return true;
				break;
			case UP:
				if (sprite->x == paths[i].x && sprite->y - step >= paths[i].y && sprite->y - step <= paths[i].y + bSize)
					return true;
				break;
			case DOWN:
				if (sprite->x == paths[i].x && sprite->y + step >= paths[i].y && sprite->y + step <= paths[i].y + bSize)
				if (IsPointReachable(sprite->x, sprite->y + bSize + step, paths, bSize)) // Extreme down
					return true;
				break;

		}

	}
	

	return false;

}

Direction getCounterDir(Direction dir)
{
	switch (dir)
	{
	case UP:
		return DOWN;
	case DOWN:
		return UP;
	case LEFT:
		return RIGHT;
	case RIGHT:
		return LEFT;
	}

	return NONE;
}

void MoveSprite(PlayableMap *map, SDL_Surface *screen, Sprite *sprite, Path paths[300], int bSize, int worldTime)
{
	int step = sprite->speed;
	
	if (PacMan *spr = dynamic_cast<PacMan*> (sprite))
	{
		if (worldTime <= spr->delayedTime && spr->delayedDir != NONE && IsAbleToMove(sprite, spr->delayedDir, paths, bSize))
		{
			spr->dir = spr->delayedDir;
			spr->delayedDir = NONE;
		}
	}
	else
	if (Ghost *ghost = dynamic_cast<Ghost*> (sprite))
	{
		Direction currentDir = ghost->dir;
		Direction counterDir = getCounterDir(currentDir);


		Direction possibleDirs[3] = { NONE, NONE, NONE };
		int differentPosibilities = 0;

		for (int i = 1, s = 0; i < 5; i++)
		if ((Direction)i != counterDir && IsAbleToMove(ghost, (Direction)i, paths, bSize))
		{
			possibleDirs[s++] = static_cast <Direction> (i);
			differentPosibilities++;
		}

#ifdef ALPHA
		if (possibleDirs[0] == NONE)
			ghost->dir = counterDir;
		else
#endif
		switch (ghost->getBehaviour())
		{
			case PINKY:
			case INKY:
			case CLYDE:
			case BLINKY:
				int dirNr = rand() % differentPosibilities;
				ghost->dir = possibleDirs[dirNr];
				break;

		}
	}

	if (IsAbleToMove(sprite, sprite->dir, paths, bSize))
	{
		switch (sprite->dir)
		{
			case UP:
				sprite->y -= step;
				break;
			case DOWN:
				sprite->y += step;
				break;
			case LEFT:
				sprite->x -= step;
				break;
			case RIGHT:
				sprite->x += step;
				break;
		}
	}

	sprite->show();
}

// WinMain
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR str, int show) {
#else
int main(int argc, const char **argv) {
#endif
	int t1, t2, quit;
	double angle, delta, worldTime;
	SDL_Event event;
	SDL_Surface *screen;

	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		printf("SDL init error: %s\n", SDL_GetError());
		return 1;
		};

	screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	// tryb pelnoekranowy
	//screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

	if(screen == NULL) {
		printf("SetVideoMode error: %s\n", SDL_GetError());
		return 1;
		};

	// wy≥πczenie widocznoúci kursora myszy
	//SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp


	// ustawienie w powierzchni charset koloru przezroczystego
	// od tej chwili pixele koloru 0x000000 na tej powierzchni (czarne)
	// bÍdπ przezroczyste


	// Loading basic components
	PlayableMap *map = new Level1(screen);
	PacMan *pMan = new PacMan();
	Ghost ghosts[4];
	pMan->screen = screen;
	for (int i = 0; i < 4; i++) ghosts[i].screen = screen;
	
	// Set ghosts behaviour
	ghosts[BLINKY].setBehaviour(BLINKY);
	ghosts[PINKY].setBehaviour(PINKY);
	ghosts[INKY].setBehaviour(INKY);
	ghosts[CLYDE].setBehaviour(CLYDE);

	// Set starting positions	
	map->setPacMan(pMan);
	map->setBlinky(&ghosts[BLINKY]);
	map->setPinky(&ghosts[PINKY]);
	map->setInky(&ghosts[INKY]);
	map->setClyde(&ghosts[CLYDE]);
	//Keep track of the current frame
	int frame = 0;

	//Load random number generator
	srand(time(NULL));

	//Whether or not to cap the frame rate
	bool cap = true;

	//The frame rate regulator
	Timer fps;

	quit = 0;
	worldTime = 0;
	t1 = SDL_GetTicks();
	while(!quit) 
	{
		t2 = SDL_GetTicks();
		int start_fps = t2;
		//Start the frame timer
		fps.start();

		// delta ticks
		delta = (t2 - t1);
		t1 = t2;

		worldTime += delta;

		map->DrawMap(screen, worldTime);

		// naniesienie wyniku rysowania na rzeczywisty ekran
		SDL_Flip(screen);

		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					if (event.key.keysym.sym == SDLK_UP) pMan->setDir(UP, worldTime);
					else
					if (event.key.keysym.sym == SDLK_DOWN) pMan->setDir(DOWN, worldTime);
					else
					if (event.key.keysym.sym == SDLK_LEFT) pMan->setDir(LEFT, worldTime);
					else
					if (event.key.keysym.sym == SDLK_RIGHT) pMan->setDir(RIGHT, worldTime);
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		//Increment the frame counter
		frame++;

		//Get Frame Delta
		int fdelta = start_fps - SDL_GetTicks();
		//If we want to cap the frame rate
		if ((cap == true) && (fps.get_ticks() < 1000 / FRAMES_PER_SECOND))
		{
			//Sleep the remaining frame time
			SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
		}
	};

	// zwolnienie powierzchni

	

	SDL_Quit();
	return 0;
};