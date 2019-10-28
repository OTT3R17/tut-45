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




DirectX::SpriteFont *gpFont = nullptr, *gpFont2 = nullptr;
DirectX::SpriteBatch *gpSpriteBatch = nullptr;
ID3D11ShaderResourceView *gpCat = nullptr, *gpOpaque=nullptr, *gpTrack = nullptr, *gpCar = nullptr;
struct Car
{
	vector<Vector2> waypoints{ { 38.f,37.f }, { 219,37 }, { 222.f,221.f }, { 38.f,219.f } };
	const float trackScale = 2.f;
	const Vector2 trackPixels{ 256,256 };
	const float carRightRot = PI;
	Vector2 ppos, pdir, psteer;
	int wpId = 0;

	void Init()
	{
		//convert waypoints into world space
		Vector2 off = Vector2((float)WinUtil::Get().GetData().clientWidth / 2.f, (float)WinUtil::Get().GetData().clientHeight / 2.f);
		off.x -= (trackPixels.x / 2) * trackScale;
		off.y -= (trackPixels.y / 2) * trackScale;
		for (int i = 0; i < 4; ++i)
			waypoints[i] = waypoints[i] * trackScale + off;

		ppos = waypoints[0];
		pdir = waypoints[1] - waypoints[0];
		pdir.Normalize();
		psteer = pdir;
		wpId = 1; 
	}
	void Update(float dTime)
	{
		Vector2& tgt = waypoints[wpId];
		pdir = tgt - ppos;
		float d = pdir.Length();
		if (d < 0.2f)
		{
			wpId++;
			if (wpId >= (int)waypoints.size())
				wpId = 0;
		}
		pdir /= d;

		psteer += (pdir - psteer) * 2.f * dTime; //really this should be timed properly (look up linear interpolation and cosine interpolation)
		psteer.Normalize();
		ppos += psteer * dTime * 75;
	}
	void Render(float dTime, MyD3D& d3d)
	{
		float alpha = atan2f(psteer.y, psteer.x);

		gpSpriteBatch->Draw(gpCar, ppos, nullptr, Vector4(1, 1, 1, 1), carRightRot + alpha, Vector2(128, 64), Vector2(0.25f, 0.25f));
	}
};
Car gCar;

ID3D11ShaderResourceView *LoadTexture(MyD3D& d3d, const wstring& file)
{
	DDS_ALPHA_MODE alpha;
	ID3D11ShaderResourceView *pT = nullptr;
	if (CreateDDSTextureFromFile(&d3d.GetDevice(), file.c_str(), nullptr, &pT, 0, &alpha) != S_OK)
	{
		WDBOUT(L"Cannot load " << file << L"\n");
		assert(false);
		return false;
	}
	assert(pT);
	return pT;
}

void InitGame(MyD3D& d3d)
{
	
	gpSpriteBatch = new SpriteBatch(&d3d.GetDeviceCtx());
	assert(gpSpriteBatch);

	gpFont = new SpriteFont(&d3d.GetDevice(), L"../bin/data/comicSansMS.spritefont");
	assert(gpFont);

	gpFont2 = new SpriteFont(&d3d.GetDevice(), L"../bin/data/algerian.spritefont");
	assert(gpFont2);

	gpOpaque = LoadTexture(d3d, L"../bin/data/2dsprite.dds");
	gpCat = LoadTexture(d3d, L"../bin/data/cat.dds");
	gpTrack = LoadTexture(d3d, L"../bin/data/race_track.dds");
	gpCar = LoadTexture(d3d, L"../bin/data/police-car-top-view-hi.dds");

	gCar.Init();
}


//any memory or resources we made need releasing at the end
void ReleaseGame()
{
	delete gpFont;
	gpFont = nullptr;

	delete gpFont2;
	gpFont2 = nullptr;

	delete gpSpriteBatch;
	gpSpriteBatch = nullptr;

	ReleaseCOM(gpCat);
	ReleaseCOM(gpOpaque);
	ReleaseCOM(gpTrack);
	ReleaseCOM(gpCar);
}

//called over and over, use it to update game logic
void Update(float dTime, MyD3D& d3d)
{
	gCar.Update(dTime);
}


//called over and over, use it to render things
void Render(float dTime, MyD3D& d3d)
{
	WinUtil& wu = WinUtil::Get();
	
	d3d.BeginRender(Vector4(0,0,0,0));
	
	CommonStates dxstate(&d3d.GetDevice());
	gpSpriteBatch->Begin(SpriteSortMode_Deferred,dxstate.NonPremultiplied());


	gpSpriteBatch->Draw(gpOpaque, Vector2(10, 100), nullptr, Vector4(1, 1, 1, 1), 0, Vector2(0, 0), Vector2(0.5f, 0.5f));

	gpSpriteBatch->Draw(gpCat, Vector2(10, 300), nullptr, Vector4(1, 1, 1, 1), 0, Vector2(0, 0), Vector2(0.3f, 0.3f));

	Vector2 pos{ (float)wu.GetData().clientWidth / 2.f, (float)wu.GetData().clientHeight / 2.f };



	gpSpriteBatch->Draw(gpTrack, pos, nullptr, Vector4(1, 1, 1, 1), 0, Vector2(128,128), Vector2(2,2));

	gCar.Render(dTime, d3d);

	gpFont->DrawString(gpSpriteBatch, "Car sprite bonanza", Vector2(0,0));

	gpSpriteBatch->End();
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