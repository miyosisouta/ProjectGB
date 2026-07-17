#include "stdafx.h"
#include "system.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/RenderingEngine.h"
#include "sound/SoundEngine.h"
#include "resource.h"

HWND            g_hWnd = NULL;              // ウィンドウハンドル

///////////////////////////////////////////////////////////////////
// メッセージプロシージャ
// hWnd : メッセージを送ってきたウィンドウのハンドル
// msg : メッセージの種類
// wParam, lParam : 引数
///////////////////////////////////////////////////////////////////
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 送られてきたメッセージで処理を分岐させる
    switch (msg)
    {
    case WM_DESTROY:
        // ウィンドウの破棄
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////
// ウィンドウの初期化
///////////////////////////////////////////////////////////////////
void InitWindow(HINSTANCE, HINSTANCE, LPWSTR, int, const TCHAR*);

void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName)
{
    //ウィンドウクラスのパラメータを設定(単なる構造体の変数の初期化です。)
    WNDCLASSEX wc =
    {
        sizeof(WNDCLASSEX),		//構造体のサイズ。
        CS_CLASSDC,				//ウィンドウのスタイル。
        //ここの指定でスクロールバーをつけたりできるが、ゲームでは不要なのでCS_CLASSDCでよい。
        MsgProc,				//メッセージプロシージャ(後述)
        0,						//0でいい。
        0,						//0でいい。
        GetModuleHandle(NULL),	//このクラスのためのウインドウプロシージャがあるインスタンスハンドル。
        //何も気にしなくてよい。
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),//アイコンのハンドル。アイコンを変えたい場合ここを変更する。とりあえずこれでいい。
        NULL,					//マウスカーソルのハンドル。NULLの場合はデフォルト。
        NULL,					//ウィンドウの背景色。NULLの場合はデフォルト。
        NULL,					//メニュー名。NULLでいい。
        appName,				//ウィンドウクラスに付ける名前。
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)) //NULLでいい。
    };

    RegisterClassEx(&wc);

    g_hWnd = CreateWindow(
        appName,
        appName,
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        FRAME_BUFFER_W,
        FRAME_BUFFER_H,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(g_hWnd, nCmdShow);
}


// ゲームの初期化
void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName)
{
    // ウィンドウ初期化
    InitWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow, appName);
    // K2エンジンの初期化
    K2Engine::InitData initData;
    initData.isSoftShadow = true;
    initData.frameBufferWidth = FRAME_BUFFER_W;
    initData.frameBufferHeight = FRAME_BUFFER_H;
    initData.hwnd = g_hWnd;
    K2Engine::CreateInstance(initData);
}

// ウィンドウメッセージをディスパッチ。falseが返ってきたらゲーム終了
bool DispatchWindowMessage()
{
    MSG msg = { 0 };
    while (WM_QUIT != msg.message) {
        // ウィンドウからのメッセージを受け取る
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // ウィンドウメッセージが空になった
            break;
        }
    }
    return msg.message != WM_QUIT;
}