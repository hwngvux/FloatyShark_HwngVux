#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <time.h>
#include <cstdlib>
#include <SDL_ttf.h>
#include <sstream>

using namespace std;
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

const int SCREEN_WIDTH = 925;
const int SCREEN_HEIGHT = 600;
const int PIPE_WIDTH = 50;
const int PIPE_GAP =  200;
static const int SHARK_WIDTH = 100;
static const int SHARK_HEIGHT = 60;

bool init();
bool loadMedia();
void close();
void waitUntilMousePressed();

SDL_Window* Window = NULL;
SDL_Renderer* Renderer = NULL;
TTF_Font *Font = NULL;
SDL_Color textColor = { 0, 0, 0 };


class LTexture
{
	public:
		LTexture();
		~LTexture();
		bool loadFromFile( std::string path );
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		void free();
		void setColor( Uint8 red, Uint8 green, Uint8 blue );
		void setBlendMode( SDL_BlendMode blending );
		void setAlpha( Uint8 alpha );
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
		int Width;
		int Height;

	private:
		SDL_Texture* mTexture;
		int mWidth;
		int mHeight;
};

class Shark
{
    public:
		Shark();
		float y = SCREEN_WIDTH/4 + 250;
        float x = SCREEN_HEIGHT/4 - 50;
		void floatUP(float a);
		void handleEvent( SDL_Event& e );
		void move(int &lose);
		void render();

    private:
		float v = 0.00f;
};

class Pipe
{
public:
	Pipe(int posX);
	int x ;
    int space;
	void setPipePosX(int posX);
	void setSpaceStart(int spaceY);
	void render();

private:
	int pipePosX;

};

bool checkCollision(Pipe p, Shark b);
bool Pass(Pipe p,Shark b);

LTexture SharkTexture;
LTexture PipeTexture;
LTexture backGround;
LTexture StartTexture;
LTexture GameOver;
LTexture TextTexture;
LTexture ScoreTexture;


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	free();
	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		cout << "Unable to load image " << path.c_str() << "SDL_image Error:" << IMG_GetError() << endl ;
	}
	else
	{
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0xFF, 0xFF, 0xFF ) );


        newTexture = SDL_CreateTextureFromSurface( Renderer, loadedSurface );
		if( newTexture == NULL )
		{
		    cout << "Unable to create texture from " << path.c_str() << "SDL_image Error:" << SDL_GetError() << endl ;
		}
		else
		{

			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}


		SDL_FreeSurface( loadedSurface );
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{

	free();


	SDL_Surface* textSurface = TTF_RenderText_Solid( Font, textureText.c_str(), textColor );
	if( textSurface == NULL )
	{
		cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
	}
	else
	{

        mTexture = SDL_CreateTextureFromSurface( Renderer, textSurface );
		if( mTexture == NULL )
		{
			cout << "Unable to create texture from rendered text! SDL Error:" <<  SDL_GetError() << endl;
		}
		else
		{

			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}


		SDL_FreeSurface( textSurface );
	}


	return mTexture != NULL;
}

void LTexture::free()
{

	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{

	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{

	SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{

	SDL_SetTextureAlphaMod( mTexture, alpha );
}


void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{

	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx( Renderer, mTexture, clip, &renderQuad, angle, center, flip );
}

Shark::Shark()
{}

void Shark::floatUP(float a)
{
	v -= a;
}

void Shark::handleEvent( SDL_Event& e )
{
	if( e.type == SDL_MOUSEBUTTONDOWN && e.key.repeat == 0  )
    {
		v = +5;
	}
}

void Shark::move(int &lose)
{
    y += v;
	if (y < 0)
	{
		y = 0;
		lose = 1;
	}
    if( y + SHARK_HEIGHT > SCREEN_HEIGHT )
    {
        y = SCREEN_HEIGHT - SHARK_HEIGHT;
		lose = 1;
    }
}

void Shark::render()
{
	SharkTexture.render(x,y);
}

Pipe::Pipe(int posX = SCREEN_WIDTH)
{
	x = posX;
	space = rand() % (SCREEN_HEIGHT - PIPE_GAP);
}

void Pipe::setPipePosX(int posX)
{
	if (posX + PIPE_WIDTH < 0)
	{
		x = SCREEN_WIDTH;
		space = rand() % (SCREEN_HEIGHT - PIPE_GAP);
	}
	else
		x = posX;
}
void Pipe::setSpaceStart(int spaceY)
{
	space = spaceY;
}
void Pipe::render()
{
	SDL_Rect pipeTop = { x, -1, PIPE_WIDTH, space };
	SDL_Rect pipeBottom = { x, space + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - (space + PIPE_GAP) + 1 };
	SDL_SetRenderDrawColor(Renderer, 0, 0xFF, 0, 0xFF);
	SDL_RenderFillRect(Renderer, &pipeTop);
	SDL_RenderFillRect(Renderer, &pipeBottom);

	SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(Renderer, &pipeTop);
	SDL_RenderDrawRect(Renderer, &pipeBottom);

}


int main( int argc, char* args[] )
{

	if( !init() )
	{
		cout << "Failed to initialize!" << endl ;
	}
	else
	{

		if( !loadMedia() )
		{
			cout << "Failed to load media!" << endl ;
		}
		else
		{
		    StartTexture.render(0,0);
            SDL_RenderPresent(Renderer);
            waitUntilMousePressed();
			bool quit = false;

			SDL_Event e;

			Shark shark;
			Pipe pipe1 = Pipe(2 * SCREEN_WIDTH / 3 - PIPE_WIDTH /3);
			Pipe pipe2 = Pipe(SCREEN_WIDTH);
			Pipe pipe3 = Pipe(4 * SCREEN_WIDTH / 3 + PIPE_WIDTH / 3);

			int speed = 2;
			float a = 0.2;
			int lose = 0;
			int score = 0;
			string s;

			while( !quit )
			{

					while (SDL_PollEvent(&e) != 0)
					{

						if (e.type == SDL_QUIT)
						{
							quit = true;
						}

						shark.handleEvent(e);
					}

					shark.floatUP(a);
					shark.move(lose);

					if (checkCollision(pipe1, shark) == true || checkCollision(pipe2, shark) == true || checkCollision(pipe3, shark) == true)
                    {
                        lose = 1;
                    }
                    else
                    {
                        score ++;
                        s = patch::to_string(score);
                        ScoreTexture.loadFromRenderedText(s,textColor);

                    }

					SDL_RenderClear(Renderer);

					if (lose == 1)
					{
						speed = 0;

						SDL_Delay(1500);
						GameOver.render(0,0);
						TextTexture.render(340,300);
						ScoreTexture.render(510,300);
						SDL_RenderPresent(Renderer);
						waitUntilMousePressed();
						return false;
					}

					backGround.render(0,0);

					pipe1.setPipePosX(pipe1.x - speed);
					pipe1.render();
					pipe2.setPipePosX(pipe2.x - speed);
					pipe2.render();
					pipe3.setPipePosX(pipe3.x - speed);
					pipe3.render();
					shark.render();

                    ScoreTexture.render(40,25);
					SDL_RenderPresent(Renderer);

			}

		}

	}
	//waitUntilMousePressed();
	close();

	return 0;
}

bool checkCollision(Pipe p, Shark b)
{
	SDL_Rect pipeTop = { p.x, -1, PIPE_WIDTH, p.space };
	SDL_Rect pipeBottom = { p.x, p.space + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - (p.space + PIPE_GAP) + 1 };

	SDL_Rect sharkXCollision = { b.x + 2, b.y + 9, SHARK_WIDTH - 1 - 2, SHARK_HEIGHT - 9 - 1 };
	SDL_Rect sharkYCollision = { b.x + 17, b.y, 11, SHARK_HEIGHT };

	int pipeTopLeft = pipeTop.x;
	int pipeTopRight = pipeTop.x + pipeTop.w;
	int pipeTopTop = -1;
	int pipeTopBottom = 0 + pipeTop.h;

	int pipeBottomLeft = pipeBottom.x;
	int pipeBottomRight = pipeBottom.x + pipeBottom.w;
	int pipeBottomTop = pipeBottom.y;
	int pipeBottomBottom = pipeBottom.y + pipeBottom.h;

	int sharkXLeft = sharkXCollision.x;
	int sharkXRight = sharkXCollision.x + sharkXCollision.w;
	int sharkXTop = sharkXCollision.y;
	int sharkXBottom = sharkXCollision.y + sharkXCollision.h;

	if (pipeTopBottom > sharkXTop && pipeTopTop < sharkXBottom && pipeTopRight > sharkXLeft && pipeTopLeft < sharkXRight)
		return true;
	if (pipeBottomBottom > sharkXTop && pipeBottomTop < sharkXBottom && pipeBottomRight > sharkXLeft && pipeBottomLeft < sharkXRight)
		return true;
	return false;
}

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
    auto res = SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN,&window,&renderer);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: "<< SDL_GetError() << endl;
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			cout << "Warning: Linear texture filtering not enabled!" ;
		}

		//Create window
		Window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( Window == NULL )
		{
			cout << "Window could not be created! SDL Error: "<< SDL_GetError() <<endl;
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			Renderer = SDL_CreateRenderer( Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( Renderer == NULL )
			{
				cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( Renderer, 0xFF, 0xFF, 0xFF, 0xFF );

				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					cout <<"SDL_image could not initialize! SDL_image Error: "<< IMG_GetError() <<endl;
					success = false;
				}

				if( TTF_Init() == -1 )
				{
					cout <<"SDL_ttf could not initialize! SDL_ttf Error: "<< TTF_GetError() << endl ;
					success = false;
				}
			}
		}
	}
	return success;
}

bool loadMedia()
{

	bool success = true;
	if( !SharkTexture.loadFromFile( "img/shark1.png" ) )
	{
		success = false;
	}
	if( !backGround.loadFromFile( "img/underwater.png" ) )
	{
		success = false;
	}
	if ( !StartTexture.loadFromFile("img/startScreen2.png"))
    {
        success = false;
    }
    if (!GameOver.loadFromFile("img/gameover2.png"))
    {
        success = false;
    }
    Font = TTF_OpenFont( "IntroRust.otf", 40 );
	if( Font == NULL )
	{
		cout << "Failed to load IntroRust font! SDL_ttf Error:" << TTF_GetError() << endl;
		success = false;
	}
	else
	{
		//Render text
		SDL_Color textColor = { 0, 0, 0 };
		if( !TextTexture.loadFromRenderedText( "Score:", textColor ) )
		{
			cout <<  "Failed to render text texture!" << endl;
			success = false;
        }
	}
	return success;
}

void close()
{

	SharkTexture.free();
	backGround.free();
	StartTexture.free();
	TextTexture.free();

	TTF_CloseFont( Font );
	Font = NULL;

	SDL_DestroyRenderer( Renderer );
	SDL_DestroyWindow( Window );
	Window = NULL;
	Renderer = NULL;

    TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


void waitUntilMousePressed()
{
    SDL_Event e;
    while (true) {
        if ( SDL_WaitEvent(&e) != 0 &&
             (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_QUIT || e.type == SDL_KEYDOWN) )
            return;
        SDL_Delay(100);
    }
}



