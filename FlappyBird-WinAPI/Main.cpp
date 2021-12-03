#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#include <atlimage.h>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

// string const
static const TCHAR szWindowClass[] = _T("DesktopAppClass");
static const TCHAR szTitle[] = _T("Flappy Bird");
static WCHAR birdFileName[] = L"bird1.png";
static WCHAR backFileName[] = L"back.png";
static WCHAR wallFileName[] = L"wall.png";

// brushes
const HBRUSH ELLIPSE_BRUSH = CreateSolidBrush(RGB(255, 255, 0));
const HBRUSH BACKGROUND_BRUSH = CreateSolidBrush(Color::MakeARGB(100,0,0,0));

static const int IDT_BIRD_ANIMATION_TIMER = 1;
static const int IDT_SPEED_TIMER = 2;
static const int IDT_BACK_ANIMATION_TIMER = 3;
static const int IDT_SECOND_BACK_ANIMATION_TIMER = 4;
static const int IDT_WALLS_TIMER = 5;
static const int IDT_WALLS_ANIMATION_TIMER = 6;
static const int IDT_REPAINT = 7;
static const int wallsNum = 8;
static float speed = 5;
static float backSpeed = 10;
static POINT wallsCentres[8] = { {300, -50},
                                    {300, 800},
                                {500, -50 },
                                    {500, 700},
                                {700, -100 },
                                    {700, 600},
                                {900, -50 },
                                    {900, 800} };
WNDCLASSEX wcex; // window class

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void drawBmp(HDC hdc, POINT ptCenter, HBITMAP hBitmap);
HBITMAP PngToBitmap(WCHAR* pngFilePath);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow)
{
    HWND hWnd; MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc; // function foe msg handling
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = BACKGROUND_BRUSH;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex); // register WNDCLASSEX

    hWnd = CreateWindow(szWindowClass, szTitle,  // window descriptor
        WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    //loop to listen for the messages that Windows sends
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static const int ImageRadius = 70;
    static const int lengthWalls = 8;
    static int cxClient, cyClient;
    static HBRUSH hbrush = ELLIPSE_BRUSH;

    static HBITMAP birdBmp = PngToBitmap(birdFileName);
    static HBITMAP backBmp = PngToBitmap(backFileName);
    static HBITMAP back2Bmp = PngToBitmap(backFileName);
    static HBITMAP wallsBmp[8] = { PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName),
        PngToBitmap(wallFileName)
    };
    static POINT ptCenter = { 100, 300 };
    static POINT backCenter = { 800, 300 };
    static POINT back2Center = { 2400, 300 };
    static int upperWallYPos = 0;
    static int wallsDistance = 0;
    
    static float g = 0.1;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, IDT_SPEED_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_BACK_ANIMATION_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_SECOND_BACK_ANIMATION_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_WALLS_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_WALLS_ANIMATION_TIMER, 5, (TIMERPROC)NULL);
        break;
    case WM_SIZE:
    {
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case WM_TIMER:
    {
        switch (wParam) {
        case IDT_BACK_ANIMATION_TIMER:
            backCenter.x -= backSpeed;
            if (backCenter.x == -800) {
                backCenter.x = 2400;
            }
            back2Center.x -= backSpeed;
            if (back2Center.x == -800) {
                back2Center.x = 2400;
            }

        case IDT_BIRD_ANIMATION_TIMER: 
            ptCenter.y += speed;
        case IDT_SPEED_TIMER: 
            speed += g;
        case IDT_WALLS_TIMER:
            for (int i = 0; i < wallsNum; i += 2)
            {
                if (wallsCentres[i].x < 0) {
                    wallsCentres[i].x = 800;
                    wallsCentres[i+1].x = 800;
                    upperWallYPos = -200 + rand() % 150;
                    wallsCentres[i].y = upperWallYPos;
                    wallsDistance = 100 + rand() % 250;
                    wallsCentres[i + 1].y = wallsCentres[i].y + wallsDistance + 650;
                }
            }
        }
        case IDT_WALLS_ANIMATION_TIMER:
        for (int i = 0; i < wallsNum; i++)
        {
            wallsCentres[i].x -= 2;
        }
    InvalidateRect(hWnd, NULL, FALSE);
    }

    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        SelectObject(hdc, hbrush);
        drawBmp(hdc, backCenter, backBmp);
        drawBmp(hdc, ptCenter, birdBmp);
        drawBmp(hdc, back2Center, back2Bmp);
        for (int i = 0; i < wallsNum; i++)
        {
            drawBmp(hdc, wallsCentres[i], wallsBmp[i]);
        }
        drawBmp(hdc, ptCenter, birdBmp);
        EndPaint(hWnd, &ps);
        
    }
    break;

    case WM_KEYDOWN:
    {
        int objectRadius = 20;
        switch (wParam){
        case VK_SPACE:
            speed = -7;
            break;
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case WM_DESTROY:
        KillTimer(hWnd, IDT_BIRD_ANIMATION_TIMER);
        KillTimer(hWnd, IDT_SPEED_TIMER); 
        KillTimer(hWnd, IDT_BACK_ANIMATION_TIMER);
        KillTimer(hWnd, IDT_SECOND_BACK_ANIMATION_TIMER);
        KillTimer(hWnd, IDT_WALLS_TIMER);
        KillTimer(hWnd, IDT_WALLS_ANIMATION_TIMER);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void drawBmp(HDC hdc, POINT ptCenter, HBITMAP hBitmap) {
    HBITMAP hNewBmp;
    HDC compDc;
    BITMAP bmp;
    int bmpWidth, bmpHeight;

    compDc = CreateCompatibleDC(hdc);
    hNewBmp = (HBITMAP)SelectObject(compDc, hBitmap);

    if (hNewBmp) {
        SetMapMode(compDc, GetMapMode(hdc));
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmp);

        bmpWidth = bmp.bmWidth;
        bmpHeight = bmp.bmHeight;

        POINT bmpSize;
        bmpSize.x = bmpWidth;
        bmpSize.y = bmpHeight;

        DPtoLP(hdc, &bmpSize, 1);

        POINT point;
        point.x = 0;
        point.y = 0;

        DPtoLP(compDc, &point, 1);

        BLENDFUNCTION blend;
        blend.BlendFlags = 0;
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        AlphaBlend(hdc, ptCenter.x - bmpWidth / 2, ptCenter.y - bmpHeight / 2, bmpWidth, bmpHeight, compDc, point.x, point.y, bmpWidth, bmpHeight, blend);
        SelectObject(compDc, hNewBmp);
    }

    DeleteDC(compDc);
}

HBITMAP PngToBitmap(WCHAR* pngFilePath) {
    GdiplusStartupInput gdi;
    ULONG_PTR token;
    GdiplusStartup(&token, &gdi, NULL);
    Color Back = Color(Color::MakeARGB(100, 0, 0, 0));
    HBITMAP convertedBitmap = NULL;
    Bitmap* Bitmap = Bitmap::FromFile(pngFilePath, false);
    if (Bitmap) {
        Bitmap->GetHBITMAP(Back, &convertedBitmap);
        delete Bitmap;
    }
    GdiplusShutdown(token);
    return convertedBitmap;
}