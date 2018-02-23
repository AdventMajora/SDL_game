#include <SDL.h>		//SDL
#include <SDL_image.h>	//SDL image
#include <SDL_ttf.h>	//SDL ttf
#include <stdio.h>
#include <string>
#include <iostream>
#include <cmath>

const int GAME_SCALE = 6;

//Screen dimension constants
const int SCREEN_WIDTH = 160*GAME_SCALE;
const int SCREEN_HEIGHT = 144*GAME_SCALE;

bool init();									//Starts up SDL and creates window
bool loadMedia();								//Loads media
void close();									//Frees media and shuts down SDL
SDL_Window* gWindow = NULL;						//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;					//The window renderer
SDL_Surface* gScreenSurface = NULL;				//Current displayed PNG image
TTF_Font* gFont = NULL;							//Globally used font

//Texture wrapper
class LTexture {
	public:
		
		//constructor
		LTexture() {
			//Initialize
			mTexture = NULL;
			mWidth = 0;
			mHeight = 0;
		}

		//destructor
		~LTexture() {
			//Deallocate
			free();
		}

		//renders the texture at point
		void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) {
			//set rendering space and render to screen
			SDL_Rect renderQuad = {
				x,
				y,
				mWidth,
				mHeight
			};

			//set clip rendering dimensions
			if (clip != NULL) {
				renderQuad.w = clip->w;
				renderQuad.h = clip->h;
			}

			//Render to screen
			SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
		}

		bool loadFromFile(std::string path) {
			//Get rid of preexisting texture
			free();

			//The final texture
			SDL_Texture* newTexture = NULL;

			//Load image at specified path
			SDL_Surface* loadedSurface = IMG_Load(path.c_str());
			if (loadedSurface == NULL) {
				printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
			} else {
				//Color key image
				SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

				//Create texture from surface pixels
				newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
				if (newTexture == NULL) {
					printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
				} else {
					//Get image dimensions
					mWidth = loadedSurface->w;
					mHeight = loadedSurface->h;
				}

				//Get rid of old loaded surface
				SDL_FreeSurface(loadedSurface);
			}

			//Return success
			mTexture = newTexture;
			return mTexture != NULL;
		}
		
		bool loadFromRenderedText(std::string textureText, SDL_Color textColor) {
			//Get rid of preexisting texture
			free();

			//Render text surface
			SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
			if (textSurface == NULL) {
				printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
			} else {
				//Create texture from surface pixels
				mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
				if (mTexture == NULL) {
					printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
				} else {
					//Get image dimensions
					mWidth = textSurface->w;
					mHeight = textSurface->h;
				}

				//Get rid of old surface
				SDL_FreeSurface(textSurface);
			}

			//Return success
			return mTexture != NULL;
		}

		void setColor(Uint8 red, Uint8 green, Uint8 blue) {
			//Modulate texture rgb
			SDL_SetTextureColorMod(mTexture, red, green, blue);
		}

		void setBlendMode(SDL_BlendMode blending) {
			//Set blending function
			SDL_SetTextureBlendMode(mTexture, blending);
		}

		void setAlpha(Uint8 alpha) {
			//Modulate texture alpha
			SDL_SetTextureAlphaMod(mTexture, alpha);
		}

		//deallocate texture
		void free() {
			//Free texture if it exists
			if (mTexture != NULL) {
				SDL_DestroyTexture(mTexture);
				mTexture = NULL;
				mWidth = 0;
				mHeight = 0;
			}
		}
		
		//returns image width
		int getWidth() {
			return mWidth;
		}

		//return image height
		int getHeight() {
			return mHeight;
		}

	private:
		SDL_Texture* mTexture;	//the actual hardware texture
		int mWidth;				//image width
		int mHeight;			//image height

};

SDL_Rect gSpriteClips[4];						//scene sprites
LTexture gSpriteSheetTexture;					//sprite sheet
LTexture gTextTexture;							//text


//Inintialize SDL
bool init() {

	bool success = true;	//Initialization flag

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {	//Initialize SDL
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	} else {

		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) {
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL ) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		} else {
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL) {
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			} else {
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					printf("SDL_image could not initialize! SDL_mage Error: %s\n", IMG_GetError());
					success = false;
				}

				//Initialize SDL_ttf
				if (TTF_Init() == -1) {
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

//load in a .png
bool loadMedia() {
	bool success = true;	//Loading success flag

	if (!gSpriteSheetTexture.loadFromFile("dots.png")) {
		printf("Failed to load sprite sheet texture :(\n");
		success = false;
	} else {
		
		//set top left sprite
		gSpriteClips[0].x = 0;
		gSpriteClips[0].y = 0;
		gSpriteClips[0].w = 100;
		gSpriteClips[0].h = 100;
	}

	return success;
}

bool loadMedia_Font() {
	//Loading success flag
	bool success = true;

	//Open the font
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if (gFont == NULL) {
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	} else {
		//Render text
		SDL_Color textColor = { 0, 0, 0 };
		if (!gTextTexture.loadFromRenderedText("Hello World!!", textColor)) {
			printf("Failed to render text texture!\n");
			success = false;
		}
	}

	return success;
}

void close() {
	//Free loaded images
	gSpriteSheetTexture.free();
	gTextTexture.free();

	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main( int argc, char* args[] ) {
	//Start up SDL and create window
	if( !init() ) {
		printf( "Failed to initialize!\n" );
	} else {
		//Load media
		if( !loadMedia_Font() && !loadMedia()) {
			printf( "Failed to load media!\n" );
		} else {	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			SDL_RenderSetScale(gRenderer, GAME_SCALE, GAME_SCALE);

			//While application is running
			while( !quit ) {
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 ) {
					//User requests quit
					if( e.type == SDL_QUIT ) {
						quit = true;
					}
				}

				//clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//render the sprite
				gSpriteSheetTexture.render(0, 0, &gSpriteClips[0]);

				//render the text
				gTextTexture.render(0,0);

				//Update the screen
				SDL_RenderPresent(gRenderer);

				//Apply the PNG image
				SDL_BlitSurface( gScreenSurface, NULL, gScreenSurface, NULL );
			
				//Update the surface
				SDL_UpdateWindowSurface( gWindow );
			}
		}
	}

	std::cin.get();
	//Free resources and close SDL
	close();

	return 0;
}