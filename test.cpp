extern "C"{
#include "./sdl/include/SDL.h"
}
#include <stdio.h>
int main()
{
	SDL_Surface *screen;
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		printf("error: %s\n", SDL_GetError());
		return 1;
		};
	SDL_Event event;	
	screen = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	int quit = 0;
	while(!quit) 
	{
		// naniesienie wyniku rysowania na rzeczywisty ekran
		SDL_Flip(screen);

		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				}
			}
	}
	
	SDL_Quit();
	return 0;
}
