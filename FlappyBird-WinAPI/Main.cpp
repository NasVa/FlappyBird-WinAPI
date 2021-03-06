#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#include <atlimage.h>
#include <string>
#include <sstream>
#include<iostream>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

// string const
static const TCHAR szWindowClass[] = _T("DesktopAppClass");
static const TCHAR szTitle[] = _T("Flappy Bird");
static WCHAR birdFileName[] = L"bird1.png";
static WCHAR backFileName[] = L"back.png";
static WCHAR wallFileName[] = L"wall.png";
static WCHAR buttonFileName[] = L"playButton.png";
static WCHAR scoreboardFileName[] = L"scoreBoard.png";
static WCHAR birdFileNames[8][12] = { L"1/bird1.png",
 L"1/bird2.png", 
 L"1/bird3.png", 
 L"1/bird4.png", 
 L"1/bird5.png", 
 L"1/bird6.png", 
 L"1/bird7.png", 
 L"1/bird8.png" };


HFONT hFont;
char szNev_ch[] = "1.fnt";
LOGFONT lf;

// brushes
const HBRUSH ELLIPSE_BRUSH = CreateSolidBrush(RGB(255, 255, 0));
const HBRUSH BACKGROUND_BRUSH = CreateSolidBrush(Color::MakeARGB(100, 0, 0, 0));

static const int IDT_BIRD_ANIMATION_TIMER = 1;
static const int IDT_SPEED_TIMER = 2;
static const int IDT_BACK_ANIMATION_TIMER = 3;
static const int IDT_SECOND_BACK_ANIMATION_TIMER = 4;
static const int IDT_WALLS_TIMER = 5;
static const int IDT_WALLS_ANIMATION_TIMER = 6;
static const int IDT_BUTTON_ANIMATION_TIMER = 7;
static const int IDT_REPAINT = 7;
static int score = 0;

static const int WALLS_NUM = 8;
static float SPEED = 5;
static const float BACK_SPEED = 10;
int const WALL_HEIGHT = 650;
int const WALL_WIDTH = 50;
int const BIRD_HEIGHT = 60;
int const BIRD_WIDTH = 80;
int const BUTTON_HEIGHT = 100;
int const BUTTON_WIDTH = 300;
static bool isGame;
static int curBirdAnimation = 0;
static POINT wallsCentres[8] = { {1600, 1600},
                                    {1600, 1600},
                                {1600, 1600 },
                                    {1600, 1600},
                                {1600, 1600 },
                                    {1600, 1600},
                                {1600, 1600 },
                                    {1600, 1600} };
WNDCLASSEX wcex; // window class

static POINT ptCenter = { 100, 300 };
static POINT scoreboardCenter = { 750, 50 };

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void drawBmp(HDC hdc, POINT ptCenter, HBITMAP hBitmap);
void StartGame();
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
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex); // register WNDCLASSEX

    hWnd = CreateWindow(szWindowClass, szTitle,  // window descriptor
        WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    isGame = false;
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
    static HBITMAP buttonBmp = PngToBitmap(buttonFileName); 
    static HBITMAP birdBmp[8] = { PngToBitmap(birdFileNames[0]),
     PngToBitmap(birdFileNames[1]),
     PngToBitmap(birdFileNames[2]),
     PngToBitmap(birdFileNames[3]),
     PngToBitmap(birdFileNames[4]),
     PngToBitmap(birdFileNames[5]),
     PngToBitmap(birdFileNames[6]),
     PngToBitmap(birdFileNames[7]) 
    };
    static HBITMAP scoreboardBmp = PngToBitmap(scoreboardFileName);
    static POINT backCenter = { 800, 300 };
    static POINT back2Center = { 2400, 300 };
    static POINT buttonCenter = { 400, 300 };
    
    static int upperWallYPos = 0;
    static int wallsDistance = 0;

    static float g = 0.7;

    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, IDT_SPEED_TIMER, 3, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_BACK_ANIMATION_TIMER, 3, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_BIRD_ANIMATION_TIMER, 3, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_SECOND_BACK_ANIMATION_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_WALLS_TIMER, 5, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_WALLS_ANIMATION_TIMER, 10, (TIMERPROC)NULL);
        SetTimer(hWnd, IDT_BUTTON_ANIMATION_TIMER, 5, (TIMERPROC)NULL);
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
            backCenter.x -= BACK_SPEED;
            if (backCenter.x == -800) {
                backCenter.x = 2400;
            }
            back2Center.x -= BACK_SPEED;
            if (back2Center.x == -800) {
                back2Center.x = 2400;
            }
            break;
        case IDT_BIRD_ANIMATION_TIMER:
            if (curBirdAnimation == 7) {
                curBirdAnimation = 0;
            }
            else {
                curBirdAnimation++;
            }
            if (isGame) {
                ptCenter.y += SPEED;
            }
            break;
        case IDT_SPEED_TIMER:
            if (isGame){
                SPEED += g;
            }
            break;
        case IDT_WALLS_TIMER:
            if (isGame) {
                for (int i = 0; i < WALLS_NUM; i++)
                {
                    if (wallsCentres[i].x < -20.0) {
                        wallsCentres[i].x = 800;
                        wallsCentres[i + 1].x = 800;
                        upperWallYPos = -200 + rand() % 150;
                        wallsCentres[i].y = upperWallYPos;
                        wallsDistance = 100 + rand() % 250;
                        wallsCentres[i + 1].y = wallsCentres[i].y + wallsDistance + 650;
                    }
                    if ((ptCenter.x + BIRD_WIDTH/2 > wallsCentres[i].x - WALL_WIDTH / 2) && (ptCenter.x + BIRD_WIDTH/2 < wallsCentres[i].x + WALL_WIDTH / 2)) {                      
                        if (((i % 2 == 0 && ptCenter.y - BIRD_HEIGHT/2 < wallsCentres[i].y + WALL_HEIGHT / 2) ||
                            (i % 2 == 1 && ptCenter.y + BIRD_HEIGHT/2 > wallsCentres[i].y - WALL_HEIGHT / 2)) ||
                            ptCenter.y < 0 || ptCenter.y > 600 ) {
                            isGame = false;
                        }
                        else{
                        //if (ptCenter.x == wallsCentres[i].x + 50) {
                            score += 1;
                        }
                    }
                }
            }
        }
    case IDT_BUTTON_ANIMATION_TIMER:
        if (isGame) {
            buttonCenter = { -400, -300 };
        }
        else {
            buttonCenter = { 400, 300 };
        }
    case IDT_WALLS_ANIMATION_TIMER:
        for (int i = 0; i < WALLS_NUM; i++)
        {
            if (isGame) {
                wallsCentres[i].x -= 1;
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
 
    break;
    case WM_PAINT:
    {
        AddFontResourceA("1.fnt");
        lf.lfHeight = 30; 
        strcpy(lf.lfFaceName, szNev_ch);
        

        std::string s = std::to_string(score);
        char const* strScore = s.c_str();
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HDC buffDC = CreateCompatibleDC(hdc);
        HBITMAP buffBtmp = CreateCompatibleBitmap(hdc, cxClient, cyClient);
        SelectObject(buffDC, buffBtmp);
        drawBmp(buffDC, backCenter, backBmp);
        drawBmp(buffDC, back2Center, back2Bmp);
        for (int i = 0; i < WALLS_NUM; i++)
        {
            drawBmp(buffDC, wallsCentres[i], wallsBmp[i]);
        }
        drawBmp(buffDC, ptCenter, birdBmp[curBirdAnimation]); 
        drawBmp(buffDC, buttonCenter, buttonBmp);
        drawBmp(buffDC, scoreboardCenter, scoreboardBmp);
        hFont = CreateFontIndirect(&lf);
        SelectObject(buffDC, GetStockObject(NULL_BRUSH));
        SetBkMode(buffDC, TRANSPARENT);
        SelectObject(buffDC, hFont);
        RECT rect = { 700, 35, 800, 100 };
        DrawText(buffDC, strScore,
            sizeof(strScore), &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
        SetBkMode(buffDC, TRANSPARENT);
        BitBlt(hdc, 0, 0, cxClient, cyClient, buffDC, 0, 0, SRCCOPY);
        DeleteDC(buffDC);
        DeleteObject(buffBtmp);
        DeleteObject(hFont);
        EndPaint(hWnd, &ps);

    }
    break;
    case WM_LBUTTONDOWN:{
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        if (xPos > buttonCenter.x - BUTTON_WIDTH / 2 && xPos < buttonCenter.x + BUTTON_WIDTH / 2 &&
            yPos > buttonCenter.y - BUTTON_HEIGHT / 2 && yPos < buttonCenter.y + BUTTON_HEIGHT / 2) {
            StartGame();
        }
    }
    break;
    case WM_KEYDOWN:
    {
        int objectRadius = 20;
        switch (wParam) {
        case VK_SPACE:
            if (isGame) {
                SPEED = -6;
            }
            else {
                StartGame();
            }
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

static void StartGame() {
    int centres[16] = { 500, -50,
                            500, 800,
                        700, -50,
                            700, 700,
                        900, -100,
                            900, 700,
                        1100, -50,
                            1100, 800 };
    int k = 0;
    for (int i = 0; i < WALLS_NUM; i++)
    {
        wallsCentres[i].x = centres[k];
        k++;
        wallsCentres[i].y = centres[k];
        k++;
    }
    ptCenter = { 100, 300 };
    SPEED = 5;
    isGame = true;
    score = 0;
}