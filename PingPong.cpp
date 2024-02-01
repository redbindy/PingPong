#include <Windows.h>

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnCreate(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
void DrawFrame(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
void OnKeyDown(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
void OnPaint(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
void MoveBall();
inline DWORD GetRandomBrightness();

HWND gHWndMain;
HINSTANCE gHInstance;
const TCHAR* mainClassName = TEXT("PingPong");

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	gHInstance = hInstance;

	WNDCLASS wndClass;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = gHInstance;
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = mainClassName;
	RegisterClass(&wndClass);

	gHWndMain = CreateWindow(
		mainClassName,
		mainClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(gHWndMain, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

enum ID
{
	ID_FRAME_TIMER
};

typedef struct ball
{
	int x;
	int y;
	int radius;
	float dx;
	float dy;
	COLORREF color;
} ball_t;

typedef struct playerBar
{
	int x;
	int width;
	int height;
} playerBar_t;

static playerBar_t sPlayerBar;
static ball_t sBall;

static HBITMAP sHBackBuffer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		OnCreate(hWnd, message, wParam, lParam);
		return 0;

	case WM_TIMER:
		MoveBall();
		DrawFrame(hWnd, message, wParam, lParam);
		return 0;

	case WM_KEYDOWN:
		OnKeyDown(hWnd, message, wParam, lParam);
		return 0;

	case WM_PAINT:
		OnPaint(hWnd, message, wParam, lParam);
		return 0;

	case WM_SIZE:
	{
		HDC hDC = GetDC(hWnd);
		{
			DeleteObject(sHBackBuffer);

			RECT rt;
			GetClientRect(hWnd, &rt);

			sHBackBuffer = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		}
		ReleaseDC(hWnd, hDC);

		DrawFrame(hWnd, message, wParam, lParam);
	}
	return 0;

	case WM_DESTROY:
		KillTimer(hWnd, ID_FRAME_TIMER);
		DeleteObject(sHBackBuffer);
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void OnCreate(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
#define FRAME_PER_SEC_60 (1000 / 60)

	RECT rt;
	GetClientRect(hWnd, &rt);

	SetTimer(hWnd, ID_FRAME_TIMER, FRAME_PER_SEC_60, nullptr);

	srand(GetTickCount64());

	sBall.x = 20;
	sBall.y = 20;
	sBall.dx = 5.f;
	sBall.dy = 5.f;
	sBall.radius = 10;
	sBall.color = RGB(GetRandomBrightness(), GetRandomBrightness(), GetRandomBrightness());

	sPlayerBar.x = rt.right / 2;
	sPlayerBar.width = 50;
	sPlayerBar.height = 10;

	HDC hDC = GetDC(hWnd);
	{
		sHBackBuffer = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
	}
	ReleaseDC(hWnd, hDC);
}

#define TO_VERTEX_DIST (sPlayerBar.width / 2)
#define PLAYER_Y_POS (rt.bottom - 40)

void DrawFrame(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	HDC hDC = GetDC(hWnd);
	{
		HDC hMemDC = CreateCompatibleDC(hDC);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, sHBackBuffer);
		{
			FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_WINDOW));

			HBRUSH hBrush = CreateSolidBrush(sBall.color);
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
			{
				Ellipse(hMemDC, sBall.x - sBall.radius, sBall.y - sBall.radius, sBall.x + sBall.radius, sBall.y + sBall.radius);
			}
			SelectObject(hMemDC, hOldBrush);
			DeleteObject(hBrush);

			hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
			{
				Rectangle(hMemDC, sPlayerBar.x - TO_VERTEX_DIST, PLAYER_Y_POS, sPlayerBar.x + TO_VERTEX_DIST, PLAYER_Y_POS + sPlayerBar.height);
			}
			SelectObject(hMemDC, hOldBrush);
		}
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
	}
	ReleaseDC(hWnd, hDC);

	InvalidateRect(hWnd, nullptr, FALSE);
}

void OnKeyDown(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	const int;

	switch (wParam)
	{
	case VK_LEFT:

		if (sPlayerBar.x - TO_VERTEX_DIST > rt.left)
		{
			sPlayerBar.x -= sPlayerBar.width / 2;
		}
		else
		{
			sPlayerBar.x = rt.left + TO_VERTEX_DIST;
		}

		break;

	case VK_RIGHT:

		if (sPlayerBar.x + TO_VERTEX_DIST < rt.right)
		{
			sPlayerBar.x += sPlayerBar.width / 2;
		}
		else
		{
			sPlayerBar.x = rt.right - TO_VERTEX_DIST;
		}

	default:
		return;
	}
}

void OnPaint(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	{
		if (sHBackBuffer != nullptr)
		{
			HDC hMemDC = CreateCompatibleDC(hDC);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, sHBackBuffer);
			{
				BITMAP bitmap;
				GetObject(sHBackBuffer, sizeof(BITMAP), &bitmap);
				BitBlt(hDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hMemDC, 0, 0, SRCCOPY);
			}
			SelectObject(hMemDC, hOldBitmap);
			DeleteDC(hMemDC);
		}
	}
	EndPaint(hWnd, &ps);
}

void MoveBall()
{
	RECT rt;
	GetClientRect(gHWndMain, &rt);

	sBall.x += sBall.dx;
	if (sBall.x - sBall.radius < rt.left)
	{
		sBall.x = rt.left - (sBall.x - sBall.radius) + sBall.radius;
		sBall.dx *= -1;
	}
	else if (sBall.x + sBall.radius > rt.right)
	{
		sBall.x = rt.right - (sBall.x + sBall.radius - rt.right) - sBall.radius;
		sBall.dx *= -1;
	}

	sBall.y += sBall.dy;
	if (sBall.y - sBall.radius < rt.top)
	{
		sBall.y = rt.top - (sBall.y - sBall.radius) + sBall.radius;
		sBall.dy *= -1;
	}
	else if (sBall.y + sBall.radius > PLAYER_Y_POS && sBall.y - sBall.radius <= PLAYER_Y_POS + sPlayerBar.height)
	{
		const int BALL_LEFT = sBall.x - sBall.radius;
		const int BALL_RIGHT = sBall.x + sBall.radius;

		const int BAR_LEFT_VERTEX = sPlayerBar.x - TO_VERTEX_DIST;
		const int BAR_RIGHT_VERTEX = sPlayerBar.x + TO_VERTEX_DIST;

		if (BALL_LEFT >= BAR_LEFT_VERTEX && BALL_LEFT <= BAR_RIGHT_VERTEX
			|| BALL_RIGHT <= BAR_RIGHT_VERTEX && BALL_RIGHT >= BAR_LEFT_VERTEX)
		{
			sBall.y = PLAYER_Y_POS - (sBall.y + sBall.radius - PLAYER_Y_POS) - sBall.radius;
			sBall.dy *= -1;
		}
	}
}

DWORD GetRandomBrightness()
{
	return rand() % (UCHAR_MAX + 1);
}
