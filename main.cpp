#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "graphics.h"
#include "defs.h"
using namespace std;
 void move (Graphics &graphics, SDL_Event event)
{
    int step = 20;
        switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                graphics.Y-=step;
                break;

            case SDLK_DOWN:
                graphics.Y+=step;
                break;

            case SDLK_LEFT:
                graphics.X-=step;
                break;

            case SDLK_RIGHT:
                graphics.X+=step;
                break;
            }
            if (graphics.X<0) graphics.X=SCREEN_WIDTH-graphics.size;
            if (graphics.Y<0) graphics.Y=SCREEN_HEIGHT-graphics.size;
            if (graphics.X+graphics.size>SCREEN_WIDTH) graphics.X=0;
            if (graphics.Y+graphics.size>SCREEN_HEIGHT) graphics.Y=0;

}
int main(int argc, char* argv[])

{

    Graphics graphics;
    graphics.init();

    ScrollingBackground background;
    background.setTexture(graphics.loadTexture(BACKGROUND_IMG));

    bool quit = false;
    SDL_Event e;
    while( !quit ) {
        while( SDL_PollEvent( &e ) != 0 ) {
            if( e.type == SDL_QUIT ) quit = true;
            else{
                if (e.type == SDL_KEYDOWN) {
                move(graphics, e);

            }
            }
        }

        background.scroll(10);

        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);

        graphics.render(background);


        graphics.box();
        graphics.presentScene();


        SDL_Delay(100);
    }

    SDL_DestroyTexture( background.texture );
    graphics.quit();
    return 0;
}
