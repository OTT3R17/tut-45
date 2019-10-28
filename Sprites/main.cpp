#include <windows.h>
#include <string>
#include <cassert>
#include <d3d11.h>
#include <iomanip>
#include <vector>

#include "WindowUtils.h"
#include "D3DUtil.h"
#include "D3D.h"
#include "SimpleMath.h"
#include "SpriteFont.h"
#include "DDSTextureLoader.h"
#include "CommonStates.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;





void InitGame(MyD3D& d3d)
{
	
}


//any memory or resources we made need releasing at the end
void ReleaseGame()
{
}

//called over and over, use it to update game logic
void Update(float dTime, MyD3D& d3d)
{
}


//called over and over, use it to render things
void Render(float dTime, MyD3D& d3d)
{
	WinUtil& wu = WinUtil::Get();
	
	d3d.BeginRender(Vector4(0,0,0,0));
	
	d3d.EndRender();
}

//if ALT+ENTER or resize or drag window we might want do
//something like pause the game perhaps, but we definitely
//need to let D3D know what's happened (OnResize_Default).
void OnResize(int screenWidth, int screenHeight, MyD3D& d3d)
{
	d3d.OnResize_Default(screenWidth, screenHeight);
}

//messages come from windows all the time, should we respond to any specific ones?
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//do something game specific here
	switch (msg)
	{
	// Respond to a keyboard event.
	case WM_CHAR:
		switch (wParam)
		{
		case 27:
		case 'q':
		case 'Q':
			PostQuitMessage(0);
			return 0;
		}
	}

	//default message handling (resize window, full screen, etc)
	return WinUtil::Get().DefaultMssgHandler(hwnd, msg, wParam, lParam);
}

//main entry point for the game
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{

	int w(1024), h(768);
	if (!WinUtil::Get().InitMainWindow(w, h, hInstance, "Fezzy", MainWndProc, true))
		assert(false);

	MyD3D d3d;
	if (!d3d.InitDirect3D(OnResize))
		assert(false);
	WinUtil::Get().SetD3D(d3d);
	InitGame(d3d);

	bool canUpdateRender;
	float dTime = 0;
	while (WinUtil::Get().BeginLoop(canUpdateRender))
	{
		if (canUpdateRender)
		{
			Update(dTime, d3d);
			Render(dTime, d3d);
		}
		dTime = WinUtil::Get().EndLoop(canUpdateRender);
	}

	ReleaseGame();
	d3d.ReleaseD3D(true);	
	return 0;
}