#include <iostream>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

//! Status code type.
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef uint8_t StatusCode;

const StatusCode STATUS_CODE_SUCCESS = 0;
const StatusCode STATUS_CODE_FAILURE = 1;

typedef std::string String;


//! Return if result of expression tests true using operator against statusCode
#define ACTIV_RETURN_IF(statusCode, operator, expression)															\
do																												\
{																												\
if (expression operator statusCode)																			\
	return;																									\
}																												\
while (false)

// ---------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------

class Window;

class Surface
{
public:
	friend Window;
	Surface();
	virtual ~Surface();
	SDL_Texture *Get();

	uint16_t GetWidth() const;
	uint16_t GetHeight() const;

protected:
	SDL_Texture	*m_pTexture;
	uint16_t	m_width;
	uint16_t	m_height;

};

Surface::Surface() :
m_pTexture(0),
m_width(0),
m_height(0)
{
}

Surface::~Surface()
{
	if (0 != m_pTexture)
		SDL_DestroyTexture(m_pTexture);
}

SDL_Texture *Surface::Get()
{
	return m_pTexture;
}

uint16_t Surface::GetWidth() const
{
	return m_width;
}

uint16_t Surface::GetHeight() const
{
	return m_height;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------

class Window
{
public:
	Window();
	~Window();

	StatusCode Create(const String &name, const uint32_t width, const uint32_t height);
	StatusCode UpdateWindowSurface();
	StatusCode BlitSurface(const Surface *pSurface);

	SDL_Window *GetWindow();

private:
	SDL_Window		*m_pWindow;
};

Window::Window() :
m_pWindow(0)
{
}

Window::~Window()
{
	if (0 != m_pWindow)
		SDL_DestroyWindow(m_pWindow);
}

SDL_Window *Window::GetWindow()
{
	return m_pWindow;
}

StatusCode Window::Create(const String &name, const uint32_t width, const uint32_t height)
{
	if (0 != m_pWindow)
		return STATUS_CODE_FAILURE;

	//Create window
	m_pWindow = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

	if (0 == m_pWindow)
	{
		std::cout << "Could not create window, error - " << SDL_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	return STATUS_CODE_SUCCESS;
}

StatusCode Window::UpdateWindowSurface()
{
	//Update the surface
	if (0 != SDL_UpdateWindowSurface(m_pWindow))
	{
		std::cout << "Could not update window surface, error - " << SDL_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	return STATUS_CODE_SUCCESS;
}

StatusCode Window::BlitSurface(const Surface *pSurface)
{
	//Apply the image
	//if (0 != SDL_BlitSurface(pSurface->m_pSurface, NULL, m_pSurface, NULL))
	//{
	//	std::cout << "Failed to blit surface, error - " << SDL_GetError() << std::endl;
	//	return STATUS_CODE_FAILURE;
	//}

	return STATUS_CODE_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------

class Renderer
{
public:
	Renderer(Window &window);
	~Renderer();

	StatusCode Create();
	void Clear();
	void Present();

	void SetColour(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha);

	void Copy(Surface &surface, SDL_Rect *pSrcRet = 0, SDL_Rect * pDstRect = 0);
	void SetViewPort(const SDL_Rect &topRightViewport);

	SDL_Renderer * GetRenderer();

private:
	Window			&m_window;
	SDL_Renderer	*m_pRenderer;
};

Renderer::Renderer(Window &window) :
m_window(window),
m_pRenderer(0)
{
}

Renderer::~Renderer()
{
	if (0 != m_pRenderer)
		SDL_DestroyRenderer(m_pRenderer);
}

SDL_Renderer *Renderer::GetRenderer()
{
	return m_pRenderer;
}

void Renderer::Copy(Surface &surface, SDL_Rect *pSrcRet, SDL_Rect * pDstRect)
{
	//Render texture to screen
	SDL_RenderCopy(m_pRenderer, surface.Get(), pSrcRet, pDstRect);
}

StatusCode Renderer::Create()
{
	//Get window surface
	m_pRenderer = SDL_CreateRenderer(m_window.GetWindow(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (0 == m_pRenderer)
	{
		std::cout << "Renderer could not be created, error - " << SDL_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(m_pRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	return STATUS_CODE_SUCCESS;
}

void Renderer::Clear()
{
	SDL_RenderClear(m_pRenderer);
}

void Renderer::Present()
{
	SDL_RenderPresent(m_pRenderer);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void Renderer::SetColour(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha)
{
	SDL_SetRenderDrawColor(m_pRenderer, red, green, blue, alpha);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void Renderer::SetViewPort(const SDL_Rect &topRightViewport)
{
	SDL_RenderSetViewport(m_pRenderer, &topRightViewport);
}

// ---------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------

class Image : public Surface
{
public:
	StatusCode LoadText(SDL_Renderer *pRenderer, TTF_Font *pFont, const String &text, SDL_Color textColor);
	StatusCode LoadPng(const String &path, SDL_Renderer *pRenderer);
	StatusCode LoadBmp(const String &path, SDL_Renderer *pRenderer);
	void Render(Renderer &renderer, const uint32_t x, const uint32_t y, SDL_Rect *pClip = 0);
};

StatusCode Image::LoadPng(const String &path, SDL_Renderer *pRenderer)
{
	if (0 != m_pTexture)
		return STATUS_CODE_FAILURE;

	SDL_Surface *pSurface = IMG_Load(path.c_str());

	if (0 == pSurface)
	{
		std::cout << "Unable to load image <" << path << ">, error - " << IMG_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	//Color key image
	(void)SDL_SetColorKey(pSurface, SDL_TRUE, SDL_MapRGB(pSurface->format, 0, 0xFF, 0xFF));

	//Create texture from surface pixels
	m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

	if (0 == m_pTexture)
	{
		std::cout << "Unable to create texture <" << path << ">, error - " << SDL_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	m_width = pSurface->w;
	m_height = pSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface(pSurface);

	return STATUS_CODE_SUCCESS;
}

StatusCode LoadBmp(const String &path, SDL_Renderer *pRenderer)
{
	return STATUS_CODE_SUCCESS;
}

StatusCode Image::LoadText(SDL_Renderer *pRenderer, TTF_Font *pFont, const String &text, SDL_Color textColor)
{
	if (0 != m_pTexture)
		return STATUS_CODE_FAILURE;

	SDL_Surface *pSurface = TTF_RenderText_Solid(pFont, text.c_str(), textColor);
	if (0 == pSurface)
	{
		std::cout << "Unable to render text surface, error - " << TTF_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	//Create texture from surface pixels
	m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
	if (0 == m_pTexture)
	{
		std::cout << "Unable to create texture from rendered text, error - " << SDL_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	m_width = pSurface->w;
	m_height = pSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface(pSurface);

	return STATUS_CODE_SUCCESS;
}

void Image::Render(Renderer &renderer, const uint32_t x, const uint32_t y, SDL_Rect *pClip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, GetWidth(), GetHeight() };

	if (0 != pClip)
	{
		renderQuad.w = pClip->w;
		renderQuad.h = pClip->h;
	}

	renderer.Copy(*this, pClip, &renderQuad);
}

class TileSet
{
public:
	TileSet();

	uint32_t	m_firstGid;
	String		m_name;
	uint32_t	m_tileWidth = 32;
	uint32_t	m_tileHeight = 32;
	uint32_t	m_tileCount;
	uint32_t	m_columns = 11;
	String		m_source;
	uint32_t	m_width;
	uint32_t	m_height;

	SDL_Rect GetTile(const uint32_t gid);
};

TileSet::TileSet()
{
}

SDL_Rect TileSet::GetTile(const uint32_t gid)
{
	SDL_Rect rect;

	rect.x = (gid % m_columns) * m_tileWidth;
	rect.y = (gid / m_columns) * m_tileHeight;

	rect.h = m_tileHeight;
	rect.w = m_tileWidth;

	return rect;
}

class Font
{
public:
	Font();
	~Font();

	StatusCode Create(const String &path, const uint32_t size);

	TTF_Font *Get() const;

private:
	TTF_Font *m_pFont;
};

Font::Font() :
m_pFont(0)
{
}

Font::~Font()
{
	if (0 != m_pFont)
		TTF_CloseFont(m_pFont);
}

StatusCode Font::Create(const String &path, const uint32_t size)
{
	m_pFont = TTF_OpenFont(path.c_str(), size);

	if (0 == m_pFont)
	{
		std::cout << "Failed to laod fon, error - " << TTF_GetError() << std::endl;
		return STATUS_CODE_FAILURE;
	}

	return STATUS_CODE_SUCCESS;
}

TTF_Font *Font::Get() const
{
	return m_pFont;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------

void Load()
{
	const uint32_t SCREEN_WIDTH = 32 * 20;
	const uint32_t SCREEN_HEIGHT = 32 * 20;

	//The dimensions of the level
	const int LEVEL_WIDTH = 640;
	const int LEVEL_HEIGHT = 480;

	if (0 != SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << "Unable to initialize SDL, error - " << SDL_GetError() << std::endl;
		return;
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		std::cout << "Unable to initialize SDL_image , error - " << IMG_GetError() << std::endl;
		return;
	}

	if (TTF_Init() == -1)
	{
		std::cout << "SDL_ttf could not initialize, error: %s\n" << TTF_GetError();
		return;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		std::cout << "Warning: Linear texture filtering not enabled, error - " << SDL_GetError() << std::endl;
		return;
	}

	Window window;
	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , window.Create("SDLTest", SCREEN_WIDTH, SCREEN_HEIGHT));

	Renderer renderer(window);
	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , renderer.Create());

	//	Image background;
	//	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , background.LoadPng("GRS2ROC.bmp", renderer.GetRenderer()));

	//	Font lazyFont;
	//	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , lazyFont.Create("lazy.ttf", 28));

	//	SDL_Color textColor = { 0, 0, 0 };

	//	Image txtImage;
	//	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , txtImage.LoadText(renderer.GetRenderer(), lazyFont.Get(), "Hello World!", textColor));

	//	Image foo;
	//	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , foo.LoadPng("foo.png", renderer.GetRenderer()));

	//Image tileSet;
	//ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , tileSet.LoadPng("grass-tiles-2-small.png", renderer.GetRenderer()));


	Image tileSet;
	ACTIV_RETURN_IF(STATUS_CODE_SUCCESS, != , tileSet.LoadPng("$Human_HF1_Male_8.png", renderer.GetRenderer()));


	uint32_t map[20][20] =
	{
		{ 1, 1, 1, 1, 1, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 19, 19, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
	};

	//Walking animation
	const int WALKING_ANIMATION_FRAMES = 3;
	SDL_Rect pSpriteClips[WALKING_ANIMATION_FRAMES];;

	//Set sprite clips
	pSpriteClips[0].x = 0;
	pSpriteClips[0].y = 0;
	pSpriteClips[0].w = 64;
	pSpriteClips[0].h = 205;

	pSpriteClips[1].x = 64;
	pSpriteClips[1].y = 0;
	pSpriteClips[1].w = 64;
	pSpriteClips[1].h = 205;

	pSpriteClips[2].x = 128;
	pSpriteClips[2].y = 0;
	pSpriteClips[2].w = 64;
	pSpriteClips[2].h = 205;


	//Current animation frame
	int frame = 0;


	uint32_t pos = 0;
	bool shouldQuit = false;
	while (!shouldQuit)
	{
		//Event handler
		SDL_Event event;

		while (0 != SDL_PollEvent(&event))
		{
			//User requests quit
			if (SDL_QUIT == event.type)
				shouldQuit = true;
			else if (SDL_KEYDOWN == event.type) //User presses a key
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					break;

				case SDLK_DOWN:
					++frame;
					pos += 10;
					break;

				case SDLK_LEFT:
					break;

				case SDLK_RIGHT:
					break;

				default:
					break;
				};
			}
		}

	

		//The camera area
		//SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

		//Clear screen
		renderer.SetColour(0xFF, 0xaa, 0xFF, 0x10);
		renderer.Clear();

		TileSet set;
		set.m_tileHeight = 70;
		set.m_tileWidth = 70;
		set.m_columns = 3;


		//for (int i = 0; i < 20; ++i)
		//{
		//	for (int j = 0; j < 20; ++j)
		//	{
		//		uint32_t gid = map[i][j];
		//		SDL_Rect rect = set.GetTile(gid);
		//		tileSet.Render(renderer, 32 * j, 32 * i, &rect);
		//	}
		//}

		int tile = frame;

		if (1 == tile)
			tile = 2;


			SDL_Rect rect = set.GetTile(tile);
			tileSet.Render(renderer, 0, pos, &rect);

			//Update screen
			renderer.Present();
		

		

		//Cycle animation
		if (frame  >= 2)
		{
			frame = 0;
		}

		
	}
}

int main(int argc, char* args[])
{
	Load();


	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}
