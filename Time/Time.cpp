// Time.cpp : 定义应用程序的入口点。
//
/*
   * Copyright Xupeng Studio,2019～2022,All rights reserved.
    *This program starts on Jan. 1st,2022.
    *This file and timacalc.cpp is the source file.
    *The json file is belongs to json.org.
*/
#include "framework.h"
#include "Time.h"
#include <time.h>
#include <fstream>
#include <windows.h>
#include <wingdi.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <math.h>
#include <fstream>
#include <stdio.h>
#include "json.h"
#include "timecalc.h"
#define MAX_LOADSTRING 100
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HFONT BigFont, SmallFont, MiddleFont;           // 将要优化的内容
HANDLE ChildThread;
struct Progresshwnd {                           // 进度条
    HWND Text;
    HWND Tip;
    HWND ProgressBar;
    TCHAR* tips;
    struct tm starttime;
    struct tm endtime;
    struct Progresshwnd* nextStruct;
};
struct Countdownhwnd {                           // 倒计时窗格
    HWND Text;
    HWND Big;
    HWND Small;
    TCHAR* tips;
    struct tm endtime;
    struct Countdownhwnd* nextStruct;
};
struct progressadd
{
    tm starttime;
    tm endtime;
    TCHAR *tips;
};
struct Countdownadd
{
    tm endtime;
    TCHAR* tips;
};
struct Events
{
    TCHAR* EventName;
    TCHAR* EventDetail;
    struct tm StartTime;
    struct tm EndTime;
    int repeatdays;
    Events* nextStruct;
    int remindmin;
};
progressadd     tempPro;
progressadd     tempCount;
Progresshwnd*   ProgressGroup;
Countdownhwnd*  CountdownGroups;
Events*         EventsGroups;
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProgressAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    ProgressManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    CountdownManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    CountdownAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    ProgressEDIT(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    CountdownEDIT(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    EventAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    EventManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    EventEdit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
TCHAR*              pluszero(int number);
void                CalcProgress();
void                CalcCountdown();
void                calcEvent();
void                appendCountdown(HWND hWnd,struct tm endtime, TCHAR* Text);
void                deleteCountdown(struct Countdownhwnd* pointers);
void                appendEvent(TCHAR* EventName, TCHAR* EventDetail, struct tm Starttime, struct tm EndTime, int repeattime, int remindmin);
void                deleteProgress(struct Progresshwnd *pointers);
void                ShowTime();
void                ReadDataFile();
void                WriteFileSE(char* JsonFile);
void                WriteDataFile();
void                SortEvent();
void                arrangeEvents();
void                ReadFileSE(char* FilePaths);
int ChildThreadStop = 0;
DWORD MainThreadId;
HWND hWndMain;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    //ProgressGroups->nextStruct = NULL;
    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TIME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TIME));

    MSG msg;

    // 主消息循环:
    while (GetMessage( & msg, nullptr, 0, 0))
    {
        
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            if ((msg.message == WM_USER + 1) && msg.wParam != NULL)msg.hwnd = hWndMain;
            if ((msg.message == WM_USER + 2) && msg.wParam == NULL)msg.hwnd = hWndMain;
            if ((msg.message == WM_USER + 3) && msg.wParam == NULL)msg.hwnd = hWndMain;
            DispatchMessage(&msg);
        }
        
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TIME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   
   if (!hWnd)
   {
      return FALSE;
   }
  
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//

HWND timelabel, datelabel;
void ShowTime()
{
    
    TCHAR* result = new TCHAR[18];
    struct tm now;
    time_t timenow;
    TCHAR *a, *b, *c;
    time(&timenow);
    localtime_s(&now,&timenow);
    ZeroMemory(result, 18);
    _stprintf_s(result,18, L"%s:%s:%s", a=pluszero(now.tm_hour), b=pluszero(now.tm_min), c=pluszero(now.tm_sec));
    delete[] a; delete[] b; delete[] c;
    SetWindowText(timelabel, result);
    _stprintf_s(result,18, L"%d-%s-%s", now.tm_year + 1900, a=pluszero(now.tm_mon + 1), b=pluszero(now.tm_mday));
    delete[] a;delete[] b;
    SetWindowText(datelabel, result);
    delete[] result;
    //Sleep(100);
}
void UpdateUI()
{
    
    while (1)
    {
        ShowTime();
        //_CrtDumpMemoryLeaks();
        CalcProgress();
        CalcCountdown();
        
        UpdateWindow(hWndMain);
        if (ChildThreadStop==1) {
            ChildThreadStop = 0;ExitThread(0);
        }
        Sleep(150);
        if (ChildThreadStop==1) {
            ChildThreadStop = 0;ExitThread(0);
        }
        
    }
}
void appendprogressBar(HWND hWnd,struct tm Strattime,struct tm endtime,TCHAR* Text)
{
    //HFONT hFont2;
    int n = 1;
    Progresshwnd *temp;
    Progresshwnd *Progress1 = new Progresshwnd;
    Progress1->Text = CreateWindow(_T("STATIC"), Text, WS_CHILD | WS_VISIBLE, 0, 300, 240, 16, hWnd, NULL, hInst, NULL);
    Progress1->Tip = CreateWindow(_T("STATIC"), _T("0%"), WS_CHILD | WS_VISIBLE | SS_RIGHT, 250, 300, 60, 16, hWnd, NULL, hInst, NULL);
    Progress1->ProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 0, 316, 300, 15, hWnd, (HMENU)0, hInst, NULL);
    Progress1->starttime = Strattime;
    Progress1->endtime = endtime;
    Progress1->tips = Text;
    SendMessage(Progress1->Text, WM_SETFONT, (WPARAM)SmallFont, NULL);
    SendMessage(Progress1->Tip, WM_SETFONT, (WPARAM)SmallFont, NULL);
    SendMessage(Progress1->ProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));
    SendMessage(Progress1->ProgressBar, PBM_SETSTEP, (WPARAM)1, 0);
    temp = ProgressGroup;
    if (temp == nullptr) {
        //temp->nextStruct = Progress1;
        ProgressGroup = Progress1;
        ProgressGroup->nextStruct = NULL;
        //HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CalcProgress, NULL, 1, 0); //创建子线程
        return;
    }
    while (temp->nextStruct != NULL)
    {
        temp = temp->nextStruct;
        n++;
        
    }
    Progress1->nextStruct = NULL;
    
    MoveWindow(Progress1->Text, 0, 300 + n * 40, 240, 16, TRUE);
    MoveWindow(Progress1->Tip, 250, 300 + n * 40, 60, 16, TRUE);
    MoveWindow(Progress1->ProgressBar, 0, 316 + n * 40, 300, 15, TRUE);
    temp->nextStruct = Progress1;
    //DeleteObject(hFont2);
    //DestroyWindow(Progress1->ProgressBar);
}
void CalcProgress()
{   
    Progresshwnd* temp = ProgressGroup;
    if (ProgressGroup == nullptr)return;
    double percent;
    TCHAR total[17] = L"";
    while (temp!=nullptr)
    {
        time_t now, start, end;
        time(&now);
        start = mktime(&temp->starttime);
        end = mktime(&temp->endtime);
        percent = difftime(now,start) * 10000.0 / difftime(end,start);
        if (percent > 10000)percent = 10000;
        if (percent < 0)percent = 0;
        _stprintf_s(total, L"%.2f％", percent / 100.0);
        SetWindowText(temp->Tip, total);
        SendMessage(temp->ProgressBar, PBM_SETPOS, (WPARAM)(int)percent, 0);
        temp = temp->nextStruct;
        Sleep(1);
    }
}
void deleteProgress(struct Progresshwnd* pointers)
{
    Progresshwnd* temp, * deleted;
    if (!pointers)return;
    temp = ProgressGroup;
    if (ProgressGroup == pointers)
    {
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Text, NULL);
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Tip, NULL);
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->ProgressBar, NULL);
        delete[]pointers->tips; ProgressGroup = ProgressGroup->nextStruct;
        delete pointers;
        return;
    }
    else
        while (temp->nextStruct != pointers)temp = temp->nextStruct;
    deleted = temp->nextStruct;
    PostThreadMessage(MainThreadId, WM_USER+1, (WPARAM)pointers->Text, NULL);
    PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Tip, NULL);
    PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->ProgressBar, NULL);
    delete[]pointers->tips;
    temp->nextStruct = deleted->nextStruct;
    delete pointers;
}
HWND Eventhwnd;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   
    //TCHAR* result = new TCHAR[14];
    switch (message)
    {
       
    case WM_CREATE:
    {
        hWndMain = hWnd;
    timelabel = CreateWindow(_T("STATIC"), _T("Time"), WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 50, 300, 50, hWnd, NULL, hInst, NULL);
    datelabel = CreateWindow(_T("STATIC"), _T("Date"), WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 100, 300, 50, hWnd, NULL, hInst, NULL);
    BigFont = CreateFont(50, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET,//使用默认字符集
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,//默认输出质量
        FF_DONTCARE, TEXT("宋体")); //字体名
    SmallFont = CreateFont(14, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET,//使用默认字符集
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,//默认输出质量
        FF_DONTCARE, TEXT("宋体")); //字体名
    MiddleFont = CreateFont(20, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET,//使用默认字符集
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,//默认输出质量
        FF_DONTCARE, TEXT("宋体")); //字体名
    MainThreadId=GetCurrentThreadId();
    SendMessage(timelabel, WM_SETFONT, (WPARAM)BigFont, NULL);
    SendMessage(datelabel, WM_SETFONT, (WPARAM)BigFont, NULL);
    ReadDataFile();
    ChildThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateUI, NULL, 1, 0); //创建子线程
    HANDLE EventThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)calcEvent, NULL, 1, 0); //创建子线程
    RECT WindowRect;
    GetClientRect(hWnd, &WindowRect);
    Eventhwnd = CreateWindow(L"STATIC", L"当前没有事件", WS_CHILD | WS_VISIBLE|WS_BORDER, 320,170, WindowRect.right - WindowRect.left-325,20,hWnd,NULL,hInst,NULL);
    SendMessage(Eventhwnd, WM_SETFONT, (WPARAM)SmallFont, NULL);
    
    break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_32772:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDPROGRESS), hWnd, ProgressAdd);
                break;
            case ID_32773:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDCOUNTDOWN), hWnd, CountdownAdd);
                break;
            case ID_32775:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_MANAGECOUNT), hWnd, CountdownManager);
                break;
            case ID_32776:
                
                DialogBox(hInst, MAKEINTRESOURCE(IDD_MANAGEPRO), hWnd, ProgressManager);
                break;
            case ID_32777:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EVENT), hWnd, EventAdd);
                break;
            case ID_32778:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_MANAGEEVENT), hWnd, EventManager);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HBRUSH hbk=NULL;
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, RGB(0, 0, 0));
        if (hbk == NULL)
        {
            hbk = CreateSolidBrush(RGB(255,255,255));
        }
        EndPaint(hWnd, &ps);
        DeleteObject(hbk); DeleteDC(hdc);
        return (INT_PTR)hbk;
    }
    break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            DeleteDC(hdc);
        }
        break;
    case WM_SIZE:
    {
        RECT WindowRect;
        GetClientRect(hWnd, &WindowRect);
        MoveWindow(Eventhwnd, 320, 170, WindowRect.right - WindowRect.left - 325, 16, TRUE);
        UpdateWindow(hWnd);
    }        break;
    case WM_DESTROY:
        WriteDataFile();
        PostQuitMessage(0);
        break;
    case WM_USER+1:
        DestroyWindow((HWND)wParam);
        break;
    case WM_USER+2:
        appendprogressBar(hWnd, tempPro.starttime, tempPro.endtime, tempPro.tips);
        break;
    case WM_USER + 3:
        appendCountdown(hWnd, tempCount.endtime, tempCount.tips);
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *MainFormMinMax=(MINMAXINFO *)lParam;
        //MainFormMinMax.ptMinTrackSize.x = 200; MainFormMinMax.ptMinTrackSize.y = 200;
        //SendMessage(hWnd, WM_GETMINMAXINFO, (WPARAM)MainFormMinMax, NULL);
        MainFormMinMax->ptMinTrackSize.x = 300; MainFormMinMax->ptMinTrackSize.y = 300;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
TCHAR* pluszero(int number)
{
    TCHAR* result = new TCHAR[3];
    ZeroMemory(result, 3);
    if (number < 10)
        _stprintf_s(result,3, L"0%d", number);
    else
        _stprintf_s(result,3, L"%d", number);
    return result;
}
void appendCountdown(HWND hWnd, struct tm endtime, TCHAR* Text)
{
    int n = 1;
    Countdownhwnd* temp;
    Countdownhwnd* Countdown1 = new Countdownhwnd;
    Countdown1->Text = CreateWindow(_T("STATIC"), Text, WS_CHILD | WS_VISIBLE|SS_CENTER, 300, 15, 200, 20, hWnd, NULL, hInst, NULL);
    Countdown1->Big = CreateWindow(_T("STATIC"), _T("0"), WS_CHILD | WS_VISIBLE | SS_CENTER, 300, 50, 200, 60, hWnd, NULL, hInst, NULL);
    Countdown1->Small = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_CENTER, 300, 130, 200, 20, hWnd, NULL, hInst, NULL);

    SendMessage(Countdown1->Text, WM_SETFONT, (WPARAM)MiddleFont, NULL);
    //DeleteObject(hFont2);
    Countdown1->endtime = endtime;
    Countdown1->tips = Text;
    SendMessage(Countdown1->Big, WM_SETFONT, (WPARAM)BigFont, NULL);
    //DeleteObject(hFont2);
    SendMessage(Countdown1->Small, WM_SETFONT, (WPARAM)MiddleFont, NULL);
    //DeleteObject(hFont2);
    temp = CountdownGroups;
    if (temp == nullptr) {
        //temp->nextStruct = Progress1;
        CountdownGroups = Countdown1;
        CountdownGroups->nextStruct = NULL;
        //HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CalcCountdown, NULL, 1, 0); //创建子线程
        return;
    }
    while (temp->nextStruct != NULL)
    {
        temp = temp->nextStruct;
        n++;

    }
    Countdown1->nextStruct = NULL;
    MoveWindow(Countdown1->Text, 300+n*200, 15, 200, 20, TRUE);
    MoveWindow(Countdown1->Big, 300+n*200, 50, 200, 60, TRUE);
    MoveWindow(Countdown1->Small, 300+n*200, 130, 200, 20, TRUE);
    temp->nextStruct = Countdown1;
    //DeleteObject(hFont2);
    //DestroyWindow(Progress1->ProgressBar);
}
void CalcCountdown()
{
    Countdownhwnd* temp = CountdownGroups;
    if (CountdownGroups == nullptr)return;
    while (1)
    {
        double timeleft;
        TCHAR *del1, *del2, *del3;
        time_t now,  end;
        time(&now);
        end = mktime(&temp->endtime);
        timeleft = difftime(end,now);
        if (timeleft >= 86400)
        {
            TCHAR *total= new TCHAR[static_cast<size_t>((int)log10(timeleft/86400))+(int)11];
            _stprintf_s(total, static_cast<size_t>((int)log10(timeleft / 86400)) + 11, L"%ld天", (long)timeleft / 86400+1);
            SetWindowText(temp->Big, total);
            _stprintf_s(total, static_cast<size_t>((int)log10(timeleft / 86400)) + 11, L"%ld天%s:%s:%s", (long)timeleft / 86400, del1= pluszero((int)timeleft / 3600 % 24), del2=pluszero((int)timeleft / 60 % 60), del3=pluszero((int)timeleft % 60));
            SetWindowText(temp->Small, total);
            delete[] total; delete[] del1; delete[] del2;delete[] del3;
        }
        else if (timeleft >= 3600)
        {
            TCHAR* total = new TCHAR[9];
            _stprintf_s(total, 9, L"%ld小时", (long)timeleft / 3600 + 1);
            SetWindowText(temp->Big, total);
            _stprintf_s(total, 9, L"%d:%s:%s",  (int)timeleft / 3600 % 24, del1 = pluszero((int)timeleft / 60 % 60), del2 = pluszero((int)timeleft % 60));
            SetWindowText(temp->Small, total);
            delete[] total; delete[] del1; delete[] del2;
        }
        else if (timeleft >= 60)
        {
            TCHAR* total = new TCHAR[6];
            _stprintf_s(total, 6, L"%ld分", (long)timeleft / 60 + 1);
            SetWindowText(temp->Big, total);
            _stprintf_s(total, 6, L"%d:%s", (int)timeleft / 60 % 60, del1 = pluszero((int)timeleft % 60));
            SetWindowText(temp->Small, total);
            delete[] total; delete[] del1;
        }
        else if(timeleft>=0)
        {
            TCHAR* total = new TCHAR[4];
            _stprintf_s(total, 4, L"%ld秒", (int)timeleft + 1);
            SetWindowText(temp->Big, total);
            _stprintf_s(total, 4, L"");
            SetWindowText(temp->Small, total);
            delete[] total;
        }
        else
        {
            SetWindowText(temp->Big, L"0秒");
            SetWindowText(temp->Small, L"");
        }
        //SendMessage(temp->ProgressBar, PBM_SETPOS, (WPARAM)(int)percent, 0);
        if (temp->nextStruct)temp = temp->nextStruct; else break;
        Sleep(1);
    }
}
void deleteCountdown(struct Countdownhwnd* pointers)
{
    Countdownhwnd* temp, * deleted;
    if (!pointers)return;
    temp = CountdownGroups;
    if (CountdownGroups == pointers)
    {
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Text, NULL);
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Big, NULL);
        PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Small, NULL);
        delete[] pointers->tips;
        CountdownGroups = CountdownGroups->nextStruct;
        delete pointers;
        return;
    }
    else
        while (temp->nextStruct != pointers)temp = temp->nextStruct;
    deleted = temp->nextStruct;
    PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Text, NULL);
    PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Big, NULL);
    PostThreadMessage(MainThreadId, WM_USER + 1, (WPARAM)pointers->Small, NULL);
    temp->nextStruct = deleted->nextStruct; delete[] pointers->tips;
    delete pointers;
}
INT_PTR CALLBACK ProgressAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), 20);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            struct tm start, end;
            SYSTEMTIME time;
            HWND StartTime = GetDlgItem(hDlg,IDC_DATETIMEPICKER2);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            start.tm_year = time.wYear - 1900;
            start.tm_mon = time.wMonth - 1;
            start.tm_mday = time.wDay;
            StartTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            start.tm_hour = time.wHour;
            start.tm_min = time.wMinute;
            start.tm_sec = time.wSecond;
            HWND EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER3);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_year = time.wYear - 1900;
            end.tm_mon = time.wMonth - 1;
            end.tm_mday = time.wDay;
            EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER4);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_hour = time.wHour;
            end.tm_min = time.wMinute;
            end.tm_sec = time.wSecond;
            HWND Title = GetDlgItem(hDlg,IDC_EDIT1);
            int TextLength = Edit_GetTextLength(Title) + 1;
            if (TextLength <= 1)
            {
                MessageBox(hDlg, L"请输入标题！", L"未输入标题指示", MB_OK|MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            TCHAR *x=new TCHAR[TextLength];
            Edit_GetText(Title, x, TextLength);
            for (int i = 0; i <= TextLength; i++)
            {
                if(*(x+i)==0x20){
                    MessageBox(hDlg, L"标题中不能带空格！", L"错误指示", MB_OK | MB_ICONSTOP);
                    return (INT_PTR)FALSE;
                }
            }
            if (difftime(mktime(&start), mktime(&end)) > 0)
                MessageBox(hDlg, L"开始时间不能超过结束时间！", L"错误指示", MB_OK | MB_ICONSTOP);
            else if(difftime(mktime(&start), mktime(&end)) == 0)
                MessageBox(hDlg, L"开始时间不能等于结束时间！", L"错误指示", MB_OK | MB_ICONSTOP);
            else
            {
                tempPro.starttime = start;
                tempPro.endtime = end;
                tempPro.tips=x;
                PostThreadMessage(MainThreadId, WM_USER + 2, NULL, NULL);
                EndDialog(hDlg, LOWORD(wParam));
            }
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK CountdownAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            struct tm end;
            SYSTEMTIME time;
            HWND StartTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_year = time.wYear - 1900;
            end.tm_mon = time.wMonth - 1;
            end.tm_mday = time.wDay;
            StartTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER2);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_hour = time.wHour;
            end.tm_min = time.wMinute;
            end.tm_sec = time.wSecond;
            HWND Title = GetDlgItem(hDlg, IDC_EDIT1);
            int TextLength = Edit_GetTextLength(Title) + 1;
            TCHAR* x = new TCHAR[TextLength];
            if (TextLength <= 1)
            {
                MessageBox(hWndMain, L"请输入标题！", L"未输入标题指示", MB_OK | MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            Edit_GetText(Title, x, TextLength);
            for (int i = 0; i <= TextLength; i++)
            {
                if (*(x + i) == 0x20) {
                    MessageBox(hWndMain, L"标题中不能带空格！", L"错误指示", MB_OK | MB_ICONSTOP);
                    return (INT_PTR)FALSE;
                }
            }
            tempCount.endtime = end;
            tempCount.tips = x;
            PostThreadMessage(MainThreadId, WM_USER + 3, NULL, NULL);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
void addDirectory()
{
    DWORD dwFilter = FALSE;
    TCHAR lpPath[1024];
    GetEnvironmentVariable(L"APPDATA", lpPath, 1024);
    wcscat_s(lpPath, 1024, L"\\Xupeng Studio");
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(lpPath, &fd);
    BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
    BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
    if (!RetValue) { CreateDirectory(lpPath, NULL); }
    FindClose(hFind);
    hFind = FindFirstFile(lpPath, &fd);
    wcscat_s(lpPath, 1024, L"\\Time");
    dwFilter = FALSE;
    hFind = FindFirstFile(lpPath, &fd);
    bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
    RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
    if (!RetValue) { CreateDirectory(lpPath, NULL); }
    FindClose(hFind);
}
void ReadDataFile()
{
    TCHAR filepath[1024];
    char filepaths[2048];
    GetEnvironmentVariable(L"APPDATA", filepath, 1024);
    wcscat_s(filepath, 1024, L"\\Xupeng Studio\\Time\\UserData.json");
    DWORD dwFilter = FALSE;
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(filepath, &fd);
    BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
    BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
    FindClose(hFind);
    //CloseHandle(hFind);
    if (!RetValue) { addDirectory(); return; }
    WideCharToMultiByte(CP_ACP, NULL, filepath, 2048, filepaths, 1024, NULL, NULL);
    ReadFileSE(filepaths);
    //FILE* fp;
    //if (fopen_s(&fp, filepaths, "a+")) {
    //    MessageBox(hWndMain, L"There is an error because your file is not read properly. please check your file.", L"ERROR", MB_OK | MB_ICONSTOP);
    //    CreateDirectory(filepath, NULL);
    //    PostQuitMessage(-1);
    //    return; 
    //}
    //while (!feof(fp))
    //{
    //    
    //    int type=0;
    //    char *tempa = new char[100];
    //    char starttt[21] = "";
    //    char endtt[21] = "";
    //    
    //    struct tm starttm, endtm;
    //    fscanf_s(fp, "%d,",&type);
    //    if (type == 1)
    //    {
    //        TCHAR* c = new TCHAR[50];
    //        fscanf_s(fp, "%s", tempa,100);
    //        fscanf_s(fp, "%s", starttt, 20);
    //        sscanf_s(starttt, "%d-%d-%d-%d:%d:%d,", &starttm.tm_year, &starttm.tm_mon, &starttm.tm_mday, &starttm.tm_hour, &starttm.tm_min, &starttm.tm_sec);
    //        fscanf_s(fp, "%s", endtt, 20);
    //        sscanf_s(endtt, "%d-%d-%d-%d:%d:%d", &endtm.tm_year, &endtm.tm_mon, &endtm.tm_mday, &endtm.tm_hour, &endtm.tm_min, &endtm.tm_sec);
    //        MultiByteToWideChar(CP_ACP, NULL, tempa, 100, c, 50);
    //        appendprogressBar(hWndMain, starttm, endtm, c);
    //        //delete[] c;
    //    }
    //    else if (type == 2)
    //    {
    //        TCHAR* c = new TCHAR[50];
    //        fscanf_s(fp, "%s", tempa, 100);
    //        fscanf_s(fp, "%s", endtt, 20);
    //        sscanf_s(endtt, "%d-%d-%d-%d:%d:%d", &endtm.tm_year, &endtm.tm_mon, &endtm.tm_mday, &endtm.tm_hour, &endtm.tm_min, &endtm.tm_sec);
    //        MultiByteToWideChar(CP_ACP, NULL, tempa, 100, c, 50);
    //        appendCountdown(hWndMain, endtm, c);
    //        //delete[] c;
    //      }
    //    delete[] tempa;
    //    //
    //}
    //fclose(fp);
}
void WriteDataFile()
{
    TCHAR filepath[1024];
    char filepaths[2048];
    GetEnvironmentVariable(L"APPDATA", filepath, 1024);
    wcscat_s(filepath, 1024, L"\\Xupeng Studio\\Time\\UserData.json");
    WideCharToMultiByte(CP_ACP, NULL, filepath, 1024, filepaths, 2048, NULL, NULL);
    WriteFileSE(filepaths);
    //FILE* fp;
    //fopen_s(&fp, filepaths, "w+");
    //Progresshwnd* temp=ProgressGroup;
    //if (fp == NULL) { MessageBox(hWndMain, L"ERROR", L"ERROR", MB_OK | MB_ICONSTOP); PostQuitMessage(-1); return; }
    //while (temp != nullptr)
    //{
    //    char a[100], start[20], end[20];
    //    
    //    sprintf_s(start, "%d-%d-%d-%d:%d:%d", temp->starttime.tm_year, temp->starttime.tm_mon, temp->starttime.tm_mday, temp->starttime.tm_hour, temp->starttime.tm_min, temp->starttime.tm_sec);
    //    sprintf_s(end, "%d-%d-%d-%d:%d:%d", temp->endtime.tm_year, temp->endtime.tm_mon, temp->endtime.tm_mday, temp->endtime.tm_hour, temp->endtime.tm_min, temp->endtime.tm_sec);
    //    WideCharToMultiByte(CP_ACP, NULL, temp->tips, 50, a, 100, NULL, NULL);
    //    fprintf(fp,"1 %100s %20s %20s\n",a,start,end);
    //    temp = temp->nextStruct;
    //}
    ////delete temp;
    //Countdownhwnd* temple = CountdownGroups;
    //while (temple != nullptr)
    //{
    //    char a[100]="", end[20] = "";
    //    sprintf_s(end, "%d-%d-%d-%d:%d:%d", temple->endtime.tm_year, temple->endtime.tm_mon, temple->endtime.tm_mday, temple->endtime.tm_hour, temple->endtime.tm_min, temple->endtime.tm_sec);
    //    WideCharToMultiByte(CP_ACP, NULL, temple->tips, 50, a, 100, NULL, NULL);
    //    fprintf(fp, "2 %100s %20s\n", a, end);
    //    temple = temple->nextStruct;
    //}
}
HWND MAINLIST;
int itempos = 0;
INT_PTR CALLBACK ProgressManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    { 
        MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
        Progresshwnd *temps = ProgressGroup;
        HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);//The ID which is called IDC_LIST1 is refered to the Time.rc->IDD_MANAGEPRO,which will be used to show the Progress which is showed information.
        LV_COLUMN lvc;
        LV_ITEM lvi;
        TCHAR COL1[] = L"标题";
        lvc.mask = LVCF_TEXT|LVCF_WIDTH;
        lvc.cx = 120;
        lvc.pszText = COL1;
        ListView_InsertColumn(MAINLIST,0,&lvc);
        TCHAR COL2[] = L"开始时间";
        lvc.cx = 120;
        lvc.pszText = COL2;
        ListView_InsertColumn(MAINLIST, 1, &lvc);
        TCHAR COL3[] = L"结束时间";
        lvc.cx = 120;
        lvc.pszText = COL3;
        ListView_InsertColumn(MAINLIST, 2, &lvc);
        int itempos = 0;
        TCHAR x[25] = L"";
        while (temps != nullptr)
        {
            lvi.mask = LVIF_TEXT;
            lvi.iItem = itempos;
            lvi.iSubItem = 0;
            lvi.pszText = temps->tips;
            ListView_InsertItem(MAINLIST, &lvi);
            lvi.iSubItem = 1;
            _stprintf_s(x, L"%d-%d-%d %d:%s:%s", temps->starttime.tm_year + 1900, temps->starttime.tm_mon + 1, temps->starttime.tm_mday, temps->starttime.tm_hour, pluszero(temps->starttime.tm_min), pluszero(temps->starttime.tm_sec));
            lvi.pszText = x;
            ListView_SetItem(MAINLIST, &lvi);
            _stprintf_s(x, L"%d-%d-%d %d:%s:%s", temps->endtime.tm_year + 1900, temps->endtime.tm_mon + 1, temps->endtime.tm_mday, temps->endtime.tm_hour, pluszero(temps->endtime.tm_min), pluszero(temps->endtime.tm_sec));
            lvi.iSubItem = 2;
            lvi.pszText = x;
            ListView_SetItem(MAINLIST, &lvi);
            itempos++;
            temps = temps->nextStruct;
        }
    }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            ChildThreadStop = 1;
            while (ChildThreadStop)Sleep(10);
            Progresshwnd* temp = ProgressGroup;
            while (temp != nullptr)
            {
                deleteProgress(temp);
                temp = ProgressGroup;
            }
            int itemCount = ListView_GetItemCount(MAINLIST);
            for (int i = 0; i < itemCount; i++)
            {
                TCHAR* str1 = new TCHAR[100];
                TCHAR str2[100] = L"";
                struct tm starttime, endtime;
                ListView_GetItemText(MAINLIST, i, 0, str1, 100);
                ListView_GetItemText(MAINLIST, i, 1, str2, 100);
                int numbers[6];
                _stscanf_s(str2, L"%d-%d-%d %d:%d:%d", numbers, numbers + 1, numbers + 2, numbers + 3, numbers + 4, numbers + 5);
                starttime.tm_year = *numbers - 1900; starttime.tm_mon = *(numbers + 1) - 1; starttime.tm_mday = *(numbers + 2);
                starttime.tm_hour = *(numbers + 3); starttime.tm_min = *(numbers + 4); starttime.tm_sec = *(numbers + 5);
                ListView_GetItemText(MAINLIST, i, 2, str2, 100);
                _stscanf_s(str2, L"%d-%d-%d %d:%d:%d", numbers, numbers + 1, numbers + 2, numbers + 3, numbers + 4, numbers + 5);
                endtime.tm_year = *numbers - 1900; endtime.tm_mon = *(numbers + 1) - 1; endtime.tm_mday = *(numbers + 2);
                endtime.tm_hour = *(numbers + 3); endtime.tm_min = *(numbers + 4); endtime.tm_sec = *(numbers + 5);
                appendprogressBar(hWndMain, starttime, endtime, str1);
            }
        }
        ChildThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateUI, NULL, 1, 0); //创建子线程
        DestroyWindow(MAINLIST);
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
        case IDCANCEL:
            DestroyWindow(MAINLIST);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        case IDC_BUTTON2:
        {
            HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
            int itemsCount = ListView_GetItemCount(MAINLIST);
            
            for (itempos = 0; itempos < itemsCount; itempos++)
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
            if (itempos >= itemsCount)break;
            //LV_ITEM lvi;
            //ListView_GetItem(MAINLIST, &lvi);
            ListView_DeleteItem(MAINLIST, itempos);

        }
        break;
        case IDC_BUTTON1:
        {
            int itemsCount = ListView_GetItemCount(MAINLIST);

            for (itempos = 0; itempos < itemsCount; itempos++)
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
            if (itempos >= itemsCount)break;
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDPROGRESS), hDlg, ProgressEDIT);
            break;
        }
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ProgressEDIT(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), 20);
        SYSTEMTIME gettime;
        SetWindowText(hDlg, L"编辑进度条");
        //HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
        TCHAR* Text = new TCHAR[21];
        int itemsCount = ListView_GetItemCount(MAINLIST);
        ListView_GetItemText(MAINLIST, itempos, 0, Text, 21);
        Edit_SetText(GetDlgItem(hDlg,IDC_EDIT1), Text);
        ListView_GetItemText(MAINLIST, itempos, 1, Text, 21);
        int timeS[6];
        _stscanf_s(Text, L"%d-%d-%d %d:%d:%d", timeS,timeS+1,timeS+2,timeS+3,timeS+4,timeS+5);
        gettime.wYear = timeS[0];gettime.wMonth = timeS[1];gettime.wDay = timeS[2];gettime.wHour = timeS[3];gettime.wMinute = timeS[4];
        gettime.wSecond = timeS[5]; gettime.wDayOfWeek = 0; gettime.wMilliseconds = 0;
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER1), GDT_VALID, &gettime);
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER2), GDT_VALID, &gettime);
        ListView_GetItemText(MAINLIST, itempos, 2, Text, 21);
        _stscanf_s(Text, L"%d-%d-%d %d:%d:%d", timeS, timeS + 1, timeS + 2, timeS + 3, timeS + 4, timeS + 5);
        gettime.wYear = timeS[0];gettime.wMonth = timeS[1];gettime.wDay = timeS[2];gettime.wHour = timeS[3];gettime.wMinute = timeS[4];
        gettime.wSecond = timeS[5]; gettime.wDayOfWeek = 0; gettime.wMilliseconds = 0;
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER3), GDT_VALID, &gettime);
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER4), GDT_VALID, &gettime);
        delete[] Text;
    return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            struct tm start, end;
            SYSTEMTIME time;
            HWND StartTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER2);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            start.tm_year = time.wYear - 1900;
            start.tm_mon = time.wMonth - 1;
            start.tm_mday = time.wDay;
            StartTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
            SendMessage(StartTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            start.tm_hour = time.wHour;
            start.tm_min = time.wMinute;
            start.tm_sec = time.wSecond;
            HWND EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER3);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_year = time.wYear - 1900;
            end.tm_mon = time.wMonth - 1;
            end.tm_mday = time.wDay;
            EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER4);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_hour = time.wHour;
            end.tm_min = time.wMinute;
            end.tm_sec = time.wSecond;
            HWND Title = GetDlgItem(hDlg, IDC_EDIT1);
            int TextLength = Edit_GetTextLength(Title) + 1;
            if (TextLength <= 1)
            {
                MessageBox(hDlg, L"请输入标题！", L"未输入标题指示", MB_OK | MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            TCHAR* x = new TCHAR[TextLength];
            Edit_GetText(Title, x, TextLength);
            for (int i = 0; i <= TextLength; i++)
            {
                if (*(x + i) == 0x20) {
                    MessageBox(hDlg, L"标题中不能带空格！", L"错误指示", MB_OK | MB_ICONSTOP);
                    return (INT_PTR)FALSE;
                }
            }
            if (difftime(mktime(&start), mktime(&end)) > 0)
                MessageBox(hDlg, L"开始时间不能超过结束时间！", L"错误指示", MB_OK | MB_ICONSTOP);
            else if (difftime(mktime(&start), mktime(&end)) == 0)
                MessageBox(hDlg, L"开始时间不能等于结束时间！", L"错误指示", MB_OK | MB_ICONSTOP);
            else
            {
                ListView_SetItemText(MAINLIST, itempos, 0, x);
                TCHAR str[21] = L"";
                _stprintf_s(str, L"%d-%d-%d %d:%s:%s", start.tm_year + 1900, start.tm_mon + 1, start.tm_mday, start.tm_hour, pluszero(start.tm_min), pluszero(start.tm_sec));
                ListView_SetItemText(MAINLIST, itempos, 1, str);
                _stprintf_s(str, L"%d-%d-%d %d:%s:%s", end.tm_year + 1900, end.tm_mon + 1, end.tm_mday, end.tm_hour, pluszero(end.tm_min), pluszero(end.tm_sec));
                ListView_SetItemText(MAINLIST, itempos, 2, str);

                EndDialog(hDlg, LOWORD(wParam));
            }
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK CountdownManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
        Countdownhwnd* temps = CountdownGroups;
        HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);//The ID which is called IDC_LIST1 is refered to the Time.rc->IDD_MANAGEPRO,which will be used to show the Progress which is showed information.
        LV_COLUMN lvc;
        LV_ITEM lvi;
        TCHAR COL1[] = L"标题";
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.cx = 120;
        lvc.pszText = COL1;
        ListView_InsertColumn(MAINLIST, 0, &lvc);
        TCHAR COL2[] = L"开始时间";
        lvc.cx = 120;
        lvc.pszText = COL2;
        ListView_InsertColumn(MAINLIST, 1, &lvc);
        int itempos = 0;
        TCHAR x[25] = L"";
        while (temps != nullptr)
        {
            lvi.mask = LVIF_TEXT;
            lvi.iItem = itempos;
            lvi.iSubItem = 0;
            lvi.pszText = temps->tips;
            ListView_InsertItem(MAINLIST, &lvi);
            lvi.iSubItem = 1;
            _stprintf_s(x, L"%d-%d-%d %d:%s:%s", temps->endtime.tm_year + 1900, temps->endtime.tm_mon + 1, temps->endtime.tm_mday, temps->endtime.tm_hour, pluszero(temps->endtime.tm_min), pluszero(temps->endtime.tm_sec));
            lvi.pszText = x;
            ListView_SetItem(MAINLIST, &lvi);
            itempos++;
            temps = temps->nextStruct;
        }
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            ChildThreadStop = 1;
            while (ChildThreadStop)Sleep(10);
            Sleep(10);
            Countdownhwnd* temp = CountdownGroups;

            while (temp != nullptr)
            {
                deleteCountdown(temp);
                temp = CountdownGroups;
            }
            int itemCount = ListView_GetItemCount(MAINLIST);
            for (int i = 0; i < itemCount; i++)
            {
                TCHAR* str1 = new TCHAR[100];
                TCHAR str2[100] = L"";
                struct tm endtime;
                ListView_GetItemText(MAINLIST, i, 0, str1, 100);
                ListView_GetItemText(MAINLIST, i, 1, str2, 100);
                int numbers[6];
                _stscanf_s(str2, L"%d-%d-%d %d:%d:%d", numbers, numbers + 1, numbers + 2, numbers + 3, numbers + 4, numbers + 5);
                endtime.tm_year = *numbers - 1900; endtime.tm_mon = *(numbers + 1) - 1; endtime.tm_mday = *(numbers + 2);
                endtime.tm_hour = *(numbers + 3); endtime.tm_min = *(numbers + 4); endtime.tm_sec = *(numbers + 5);
                ListView_GetItemText(MAINLIST, i, 2, str2, 100);
                appendCountdown(hWndMain, endtime, str1);
            }
        }
        ChildThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateUI, NULL, 1, 0); //创建子线程
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        case IDC_BUTTON2:
        {
            HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
            int itemsCount = ListView_GetItemCount(MAINLIST);
            for (itempos = 0; itempos < itemsCount; itempos++)
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
            if (itempos >= itemsCount)break;
            //LV_ITEM lvi;
            //ListView_GetItem(MAINLIST, &lvi);
            ListView_DeleteItem(MAINLIST, itempos);

        }
        break;
        case IDC_BUTTON1:
        {
            int itemsCount = ListView_GetItemCount(MAINLIST);
            for (itempos = 0; itempos < itemsCount; itempos++)
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
            if (itempos >= itemsCount)break;
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDCOUNTDOWN), hDlg, CountdownEDIT);
            break;
        }
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK CountdownEDIT(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), 20);
        SYSTEMTIME gettime;
        SetWindowText(hDlg, L"编辑倒计时");
        TCHAR* Text = new TCHAR[21];
        ListView_GetItemText(MAINLIST, itempos, 0, Text, 21);
        Edit_SetText(GetDlgItem(hDlg, IDC_EDIT1), Text);
        ListView_GetItemText(MAINLIST, itempos, 1, Text, 21);
        int timeS[6];
        _stscanf_s(Text, L"%d-%d-%d %d:%d:%d", timeS, timeS + 1, timeS + 2, timeS + 3, timeS + 4, timeS + 5);
        gettime.wYear = timeS[0]; gettime.wMonth = timeS[1]; gettime.wDay = timeS[2]; gettime.wHour = timeS[3]; gettime.wMinute = timeS[4];
        gettime.wSecond = timeS[5]; gettime.wDayOfWeek = 0; gettime.wMilliseconds = 0;
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER1), GDT_VALID, &gettime);
        DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATETIMEPICKER2), GDT_VALID, &gettime);
        delete[] Text;
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            struct tm end;
            SYSTEMTIME time;
            HWND EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_year = time.wYear - 1900;
            end.tm_mon = time.wMonth - 1;
            end.tm_mday = time.wDay;
            EndTime = GetDlgItem(hDlg, IDC_DATETIMEPICKER2);
            SendMessage(EndTime, DTM_GETSYSTEMTIME, NULL, (LPARAM)&time);
            end.tm_hour = time.wHour;
            end.tm_min = time.wMinute;
            end.tm_sec = time.wSecond;
            HWND Title = GetDlgItem(hDlg, IDC_EDIT1);
            int TextLength = Edit_GetTextLength(Title) + 1;
            if (TextLength <= 1)
            {
                MessageBox(hDlg, L"请输入标题！", L"未输入标题指示", MB_OK | MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            TCHAR* x = new TCHAR[TextLength];
            Edit_GetText(Title, x, TextLength);
            for (int i = 0; i <= TextLength; i++)
            {
                if (*(x + i) == 0x20) {
                    MessageBox(hDlg, L"标题中不能带空格！", L"错误指示", MB_OK | MB_ICONSTOP);
                    return (INT_PTR)FALSE;
                }
            }
            int itemsCount = ListView_GetItemCount(MAINLIST);
            int i = 0;
            for (i = 0; i < itemsCount; i++)
                if (ListView_GetItemState(MAINLIST, i, LVIS_SELECTED))
                    break;
            ListView_SetItemText(MAINLIST, i, 0, x);
            TCHAR str[21] = L"";
            _stprintf_s(str, L"%d-%d-%d %d:%s:%s", end.tm_year + 1900, end.tm_mon + 1, end.tm_mday, end.tm_hour, pluszero(end.tm_min), pluszero(end.tm_sec));
            ListView_SetItemText(MAINLIST, i, 1, str);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
void ThreadEventCalc()
{
    Events* a = EventsGroups;
    if (a == nullptr) return;
    double remaintime;
    struct tm calctime;
    time_t now, temptime;
    TCHAR status[4] = L"";
    TCHAR EventTip[100] = L"";
    time(&now);
    localtime_s(&calctime, &now);
    temptime = mktime(&a->StartTime);
    remaintime = difftime(temptime, now);
    wcscpy_s(status, L"后开始");
    if (remaintime < 0)
    {
        temptime = mktime(&a->EndTime);
        remaintime = difftime(temptime, now);
        wcscpy_s(status, L"后结束");
        if (remaintime / 60 > 99)
            wcscpy_s(status, L"进行中");
    }
    else if (remaintime / 60 > a->remindmin)
        return;
    
    if (remaintime == 0&&wcscmp(status,L"后结束")==0)
        arrangeEvents();//需要重新计算下一次的时间，并进行排序，需要调用函数sortevents
    if (remaintime < 0)return;
    if (wcscmp(status, L"后结束") == 0 || wcscmp(status, L"后开始") == 0)
        _stprintf_s(EventTip, L"%d:%s%s %s", (int)(remaintime / 60), pluszero((int)remaintime % 60), status, a->EventName);
    else
        _stprintf_s(EventTip, L"%s %s", status, a->EventName);
    SetWindowText(Eventhwnd, EventTip);
}
void calcEvent() //在线程中计算有关事件变化的事情
{
    while (1)
    {
        ThreadEventCalc();
        Sleep(100);
        if (ChildThreadStop==2)
        {
            ChildThreadStop = 0;
            ExitThread(0);
            return;
        }
    }
}
int judgeEventisEnd(Events* EventPointer)
{
    time_t now,EventEndTime;
    time(&now);
    EventEndTime = mktime(&EventPointer->EndTime);
    if (difftime(EventEndTime, now) <= 0)return 1; else return 0;
}
void MoveEventtoEnd(Events* PointerEventPosition)//has Problem to move this Event Structure to the end.This problem need to Solve.
{
    Events* temp = EventsGroups;
    if (PointerEventPosition == nullptr)return;
    if (EventsGroups == PointerEventPosition && EventsGroups->nextStruct == nullptr)return;
    //if (EventsGroups == PointerEventPosition)EventsGroups = EventsGroups->nextStruct;
    while (temp->nextStruct != nullptr)
    {
        if (temp== PointerEventPosition)
        {
            temp= temp->nextStruct;
            continue;
        }
        temp = temp->nextStruct;
    }
    EventsGroups = EventsGroups->nextStruct;
    temp->nextStruct = PointerEventPosition;
    PointerEventPosition->nextStruct = nullptr;
}
void arrangeEvents()//根据事件结束，需要重新调整一下序列
{
    SetWindowText(Eventhwnd, L"");
    Events* a = EventsGroups;
    Events* temp = EventsGroups;
    struct tm newStarttime, newEndtime;
    time_t now;
    time(&now);
    //if (difftime(mktime(&a->StartTime), now) <= 0 && difftime(mktime(&a->EndTime), now) >= 0)return;
    newStarttime = a->StartTime;
    newEndtime = a->EndTime;
    newStarttime = timeadddays(newStarttime,a->repeatdays);
    a->StartTime = newStarttime;
    newEndtime = timeadddays(newEndtime, a->repeatdays);
    a->EndTime = newEndtime;
    if (judgeEventisEnd(EventsGroups))
    {
        MoveEventtoEnd(EventsGroups);
        return;
    }
    //EventsGroups = EventsGroups->nextStruct;
    while (temp->nextStruct != nullptr)
    {
        //Events* findposition;
        time_t compareStarttime1,compareStarttime2;
        time_t compareEndtime1,compareEndtime2;
        compareStarttime1=mktime(&temp->nextStruct->StartTime);
        compareStarttime2 = mktime(&newStarttime);
        compareEndtime1 = mktime(&temp->nextStruct->EndTime);
        compareEndtime2 = mktime(&newEndtime);
        if (difftime(compareStarttime1, compareStarttime2) < 0)
        {
            temp = temp->nextStruct;
        }
        else break;
    }
    if (temp == EventsGroups)return;
    EventsGroups = EventsGroups->nextStruct;
    a->nextStruct = temp->nextStruct;
    temp->nextStruct = a;

}
void appendEvent(TCHAR* EventName, TCHAR* EventDetail, struct tm Starttime, struct tm EndTime,int repeattime,int remindmin)
{
    Events* eventtemp = new Events;
    eventtemp->EventName = EventName;
    eventtemp->EventDetail = EventDetail;
    eventtemp->StartTime = Starttime;
    eventtemp->EndTime = EndTime;
    eventtemp->repeatdays = repeattime;
    eventtemp->remindmin = remindmin;
    eventtemp->nextStruct = EventsGroups;
    EventsGroups = eventtemp;
    if (!judgeEventisEnd(eventtemp))SortEvent(); else arrangeEvents();    //this function is to sort the events you need.
}
void SortEvent()
{
    Events* temp = EventsGroups;
    Events* a = EventsGroups;
    time_t compareStarttime2, compareEndtime2;
    //this sort code have an issue while running and need to solve.
    while (temp->nextStruct != nullptr)
    {
        //Events* findposition;
        time_t compareStarttime1;
        time_t compareEndtime1;
        compareStarttime1 = mktime(&temp->nextStruct->StartTime);
        compareStarttime2 = mktime(&a->StartTime);
        compareEndtime1 = mktime(&temp->nextStruct->EndTime);
        compareEndtime2 = mktime(&a->EndTime);
        if (difftime(compareStarttime1, compareStarttime2) < 0 && !judgeEventisEnd(temp))
        {
            temp = temp->nextStruct;
        }
        else break;
    }
    if (temp == EventsGroups)return;
    EventsGroups = EventsGroups->nextStruct;
    a->nextStruct = temp->nextStruct;
    temp->nextStruct= a;
    
}
INT_PTR CALLBACK EventAdd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    HWND CheckBox1 = GetDlgItem(hDlg, IDC_CHECK1);
    HWND EditEnabled = GetDlgItem(hDlg, IDC_EDIT2);
    HWND EventNameHWND = GetDlgItem(hDlg, IDC_EDIT1);
    HWND EventDetialHWND = GetDlgItem(hDlg, IDC_EDIT3);
    HWND EventStartDateHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
    HWND EventStartTimeHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER2);
    HWND EventEndDateHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER4);
    HWND EventEndTimeHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER3);
    HWND EventRemindTime = GetDlgItem(hDlg, IDC_EDIT4);
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        Edit_LimitText(EventNameHWND, 20);
        SetWindowText(hDlg, L"创建事件");
        return (INT_PTR)TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_CHECK1:
        {
            int status = Button_GetCheck(CheckBox1);
            Edit_Enable(EditEnabled, status);
        }
        break;
        case IDOK:
        {
            SYSTEMTIME comparetime1, comparetime2;
            struct tm time1, time2;
            time_t temptm1, temptm2;
            DateTime_GetSystemtime(EventStartDateHWND, &comparetime1);
            time1.tm_year = comparetime1.wYear - 1900; time1.tm_mon = comparetime1.wMonth - 1; time1.tm_mday = comparetime1.wDay;
            DateTime_GetSystemtime(EventStartTimeHWND, &comparetime1);
            time1.tm_hour = comparetime1.wHour; time1.tm_min = comparetime1.wMinute; time1.tm_sec = comparetime1.wSecond;
            DateTime_GetSystemtime(EventEndDateHWND, &comparetime2);
            time2.tm_year = comparetime2.wYear - 1900; time2.tm_mon = comparetime2.wMonth - 1; time2.tm_mday = comparetime2.wDay;
            DateTime_GetSystemtime(EventEndTimeHWND, &comparetime2);
            time2.tm_hour = comparetime2.wHour; time2.tm_min = comparetime2.wMinute; time2.tm_sec = comparetime2.wSecond;
            temptm1 = mktime(&time1); temptm2 = mktime(&time2);
            if (difftime(temptm1, temptm2) > 0)
            {
                MessageBox(hDlg, L"事件的结束时间不能早于事件的开始时间！", L"添加事件", MB_OK | MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            int status = Button_GetCheck(CheckBox1);
            int TextLength = Edit_GetTextLength(EventNameHWND);
            TCHAR* newEventNameTemp = new TCHAR[TextLength + 1];
            Edit_GetText(EventNameHWND, newEventNameTemp, TextLength + 1);
            TextLength = Edit_GetTextLength(EventDetialHWND);
            TCHAR* newEventDetailTemp = new TCHAR[TextLength + 1];
            Edit_GetText(EventDetialHWND, newEventDetailTemp, TextLength + 1);
            TextLength = Edit_GetTextLength(EditEnabled);
            TCHAR* Tempnumbers = new TCHAR[TextLength+1];
            Edit_GetText(EditEnabled, Tempnumbers, TextLength + 1);
            int remindtimes=0, repeattimes=0;
            _stscanf_s(Tempnumbers, L"%d", &repeattimes);
            repeattimes *= status;
            delete[] Tempnumbers;
            TextLength = Edit_GetTextLength(EventRemindTime);
            Tempnumbers = new TCHAR[TextLength+1];
            Edit_GetText(EventRemindTime, Tempnumbers, TextLength + 1);
            _stscanf_s(Tempnumbers, L"%d", &remindtimes);
            delete[]Tempnumbers;
            appendEvent(newEventNameTemp, newEventDetailTemp, time1, time2,repeattimes, remindtimes);
            EndDialog(hDlg, (INT_PTR)TRUE);
        }
        break;
        case IDCANCEL:
            EndDialog(hDlg, (INT_PTR)TRUE);
            break;
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}
void deleteEvent(Events* PointEvent)
{
    if (PointEvent == EventsGroups)
    {
        EventsGroups = EventsGroups->nextStruct;
        delete PointEvent;
        return;
    }
    else
    {
        Events* temp = EventsGroups;
        while (temp != nullptr && temp->nextStruct!=nullptr)
        {
            if (temp->nextStruct == PointEvent)
            {
                temp->nextStruct = temp->nextStruct->nextStruct;
                delete PointEvent;
                return;
            }
            temp = temp->nextStruct;
        }
    }
}
Events* NeedChangeEvent;
INT_PTR CALLBACK EventManager(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
        Events* temps = EventsGroups;
        HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);//The ID which is called IDC_LIST1 is refered to the Time.rc->IDD_MANAGEPRO,which will be used to show the Progress which is showed information.
        LV_COLUMN lvc;
        LV_ITEM lvi;
        TCHAR COL1[] = L"标题";
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.cx = 120;
        lvc.pszText = COL1;
        ListView_InsertColumn(MAINLIST, 0, &lvc);
        TCHAR COL2[] = L"开始时间";
        lvc.cx = 120;
        lvc.pszText = COL2;
        ListView_InsertColumn(MAINLIST, 1, &lvc);
        TCHAR COL3[] = L"结束时间";
        lvc.cx = 120;
        lvc.pszText = COL3;
        ListView_InsertColumn(MAINLIST, 1, &lvc);
        int itempos = 0;
        TCHAR x[25] = L"";
        while (temps != nullptr)
        {
            lvi.mask = LVIF_TEXT;
            lvi.iItem = itempos;
            lvi.iSubItem = 0;
            lvi.pszText = temps->EventName;
            ListView_InsertItem(MAINLIST, &lvi);
            lvi.iSubItem = 1;
            _stprintf_s(x, L"%d-%d-%d %d:%s:%s", temps->StartTime.tm_year + 1900, temps->StartTime.tm_mon + 1, temps->StartTime.tm_mday, temps->StartTime.tm_hour, pluszero(temps->StartTime.tm_min), pluszero(temps->StartTime.tm_sec));
            lvi.pszText = x;
            ListView_SetItem(MAINLIST, &lvi);
            lvi.iSubItem = 2;
            _stprintf_s(x, L"%d-%d-%d %d:%s:%s", temps->EndTime.tm_year + 1900, temps->EndTime.tm_mon + 1, temps->EndTime.tm_mday, temps->EndTime.tm_hour, pluszero(temps->EndTime.tm_min), pluszero(temps->EndTime.tm_sec));
            lvi.pszText = x;
            ListView_SetItem(MAINLIST, &lvi);
            itempos++;
            temps = temps->nextStruct;
        }
        ChildThreadStop = 2;
        while (!ChildThreadStop)Sleep(10);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            HANDLE EventThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)calcEvent, NULL, 1, 0); //创建子线程
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        case IDC_BUTTON2:
        {
            HWND MAINLIST = GetDlgItem(hDlg, IDC_LIST1);
            int itemsCount = ListView_GetItemCount(MAINLIST);
            Events* temp = EventsGroups;
            for (itempos = 0; itempos < itemsCount; itempos++)
            {
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
                temp = temp->nextStruct;
            }
            if (itempos >= itemsCount)break;
            deleteEvent(temp);
            //if(wcscmp(temp->EventName))
            ListView_DeleteItem(MAINLIST, itempos);
        }
        break;
        case IDC_BUTTON1:
        {
            int itemsCount = ListView_GetItemCount(MAINLIST);
            Events *temp = EventsGroups;
            for (itempos = 0; itempos < itemsCount; itempos++)
            {
                if (ListView_GetItemState(MAINLIST, itempos, LVIS_SELECTED))
                    break;
                temp = temp->nextStruct;
            }
            if (itempos >= itemsCount)break;
            NeedChangeEvent = temp;
            DialogBox(hInst, MAKEINTRESOURCE(IDD_EVENT), hDlg, EventEdit);
            break;
        }
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK EventEdit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND CheckBox1 = GetDlgItem(hDlg, IDC_CHECK1);
    HWND EditEnabled = GetDlgItem(hDlg, IDC_EDIT2);
    HWND EventNameHWND = GetDlgItem(hDlg, IDC_EDIT1);
    HWND EventDetialHWND = GetDlgItem(hDlg, IDC_EDIT3);
    HWND EventStartDateHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER1);
    HWND EventStartTimeHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER2);
    HWND EventEndDateHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER4);
    HWND EventEndTimeHWND = GetDlgItem(hDlg, IDC_DATETIMEPICKER3);
    HWND EventRemindTime = GetDlgItem(hDlg, IDC_EDIT4);
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        Edit_LimitText(EventNameHWND, 3);
        SetWindowText(EventNameHWND, NeedChangeEvent->EventName);
        SetWindowText(EventDetialHWND, NeedChangeEvent->EventDetail);
        TCHAR* RemindText = new TCHAR[4];
        _stprintf_s(RemindText,4, L"%d", NeedChangeEvent->repeatdays);
        SetWindowText(EditEnabled, RemindText);
        SetWindowText(hDlg, L"编辑事件");
        _stprintf_s(RemindText, 4, L"%d", NeedChangeEvent->remindmin);
        SetWindowText(EventRemindTime, RemindText);
        SetWindowText(EventDetialHWND, NeedChangeEvent->EventDetail);
        Button_SetCheck(CheckBox1, NeedChangeEvent->repeatdays > 0);
        Edit_Enable(EditEnabled, NeedChangeEvent->repeatdays > 0);
        SYSTEMTIME systm;
        systm.wYear = NeedChangeEvent->StartTime.tm_year + 1900;
        systm.wMonth = NeedChangeEvent->StartTime.tm_mon + 1;
        systm.wDay = NeedChangeEvent->StartTime.tm_mday;
        systm.wHour = NeedChangeEvent->StartTime.tm_hour;
        systm.wMinute = NeedChangeEvent->StartTime.tm_min;
        systm.wSecond = NeedChangeEvent->StartTime.tm_sec;
        SYSTEMTIME endtm;
        endtm.wMonth = NeedChangeEvent->EndTime.tm_mon + 1;
        endtm.wYear = NeedChangeEvent->EndTime.tm_year + 1900;
        endtm.wDay = NeedChangeEvent->EndTime.tm_mday;
        endtm.wHour = NeedChangeEvent->EndTime.tm_hour;
        endtm.wMinute = NeedChangeEvent->EndTime.tm_min;
        endtm.wSecond = NeedChangeEvent->EndTime.tm_sec;
        DateTime_SetSystemtime(EventStartDateHWND, GDT_VALID, &systm);
        DateTime_SetSystemtime(EventStartTimeHWND, GDT_VALID, &systm);
        DateTime_SetSystemtime(EventEndDateHWND, GDT_VALID, &endtm);
        DateTime_SetSystemtime(EventEndTimeHWND, GDT_VALID, &endtm);
    }
        return (INT_PTR)TRUE;
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK1:
        {
            int status = Button_GetCheck(CheckBox1);
            Edit_Enable(EditEnabled, status);
        }
        break;
        case IDOK:
        {
            SYSTEMTIME comparetime1, comparetime2;
            struct tm time1, time2;
            time_t temptm1, temptm2;
            DateTime_GetSystemtime(EventStartDateHWND, &comparetime1);
            time1.tm_year = comparetime1.wYear - 1900; time1.tm_mon = comparetime1.wMonth - 1; time1.tm_mday = comparetime1.wDay;
            DateTime_GetSystemtime(EventStartTimeHWND, &comparetime1);
            time1.tm_hour = comparetime1.wHour; time1.tm_min = comparetime1.wMinute; time1.tm_sec = comparetime1.wSecond;
            DateTime_GetSystemtime(EventEndDateHWND, &comparetime2);
            time2.tm_year = comparetime2.wYear - 1900; time2.tm_mon = comparetime2.wMonth - 1; time2.tm_mday = comparetime2.wDay;
            DateTime_GetSystemtime(EventEndTimeHWND, &comparetime2);
            time2.tm_hour = comparetime2.wHour; time2.tm_min = comparetime2.wMinute; time2.tm_sec = comparetime2.wSecond;
            temptm1 = mktime(&time1); temptm2 = mktime(&time2);
            if (difftime(temptm1, temptm2) > 0)
            {
                MessageBox(hDlg, L"事件的结束时间不能早于事件的开始时间！", L"编辑事件", MB_OK | MB_ICONSTOP);
                return (INT_PTR)FALSE;
            }
            int status = Button_GetCheck(CheckBox1);
            int TextLength = Edit_GetTextLength(EventNameHWND);
            TCHAR* newEventNameTemp = new TCHAR[TextLength + 1];
            Edit_GetText(EventNameHWND, newEventNameTemp, TextLength + 1);
            TextLength = Edit_GetTextLength(EventDetialHWND);
            TCHAR* newEventDetailTemp = new TCHAR[TextLength + 1];
            Edit_GetText(EventDetialHWND, newEventDetailTemp, TextLength + 1);
            TextLength = Edit_GetTextLength(EditEnabled);
            TCHAR* Tempnumbers = new TCHAR[TextLength + 1];
            Edit_GetText(EditEnabled, Tempnumbers, TextLength + 1);
            int remindtimes = 0, repeattimes = 0;
            _stscanf_s(Tempnumbers, L"%d", &repeattimes);
            repeattimes *= status;
            delete[] Tempnumbers;
            TextLength = Edit_GetTextLength(EventRemindTime);
            Tempnumbers = new TCHAR[TextLength + 1];
            Edit_GetText(EventRemindTime, Tempnumbers, TextLength + 1);
            _stscanf_s(Tempnumbers, L"%d", &remindtimes);
            delete[]Tempnumbers;
            NeedChangeEvent->EventName = newEventNameTemp;
            NeedChangeEvent->EventDetail = newEventDetailTemp;
            NeedChangeEvent->StartTime = time1;
            NeedChangeEvent->EndTime = time2;
            NeedChangeEvent->repeatdays = repeattimes;
            NeedChangeEvent->remindmin = remindtimes;
            NeedChangeEvent = nullptr;
            //appendEvent(newEventNameTemp, newEventDetailTemp, time1, time2, repeattimes, remindtimes);
            EndDialog(hDlg, (INT_PTR)TRUE);
        }
        break;
        case IDCANCEL:
            EndDialog(hDlg, (INT_PTR)TRUE);
            break;
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}
void WriteFileSE(char *JsonFile)
{
    Json::Value root;
    while (CountdownGroups != nullptr)
    {
        Json::Value Countdown;
        char endtime[20] = "";
        sprintf_s(endtime, "%d-%d-%2d %d:%d:%d", CountdownGroups->endtime.tm_year, CountdownGroups->endtime.tm_mon, CountdownGroups->endtime.tm_mday, CountdownGroups->endtime.tm_hour, CountdownGroups->endtime.tm_min, CountdownGroups->endtime.tm_sec);
        int Textlength = WideCharToMultiByte(CP_UTF8, NULL, CountdownGroups->tips, wcslen(CountdownGroups->tips) + 1, NULL, NULL, NULL, NULL);
        char* x = new char[Textlength + 1];
        WideCharToMultiByte(CP_UTF8, NULL, CountdownGroups->tips, wcslen(CountdownGroups->tips) + 1, x, (Textlength + 1), NULL, NULL);
        Countdown["name"] = x;
        Countdown["endTime"] = endtime;
        root["Countdown"].append(Countdown);
        CountdownGroups = CountdownGroups->nextStruct;
        delete[] x;
    }
    while (ProgressGroup != nullptr)
    {
        Json::Value Progress;
        char endtime[20] = "";
        sprintf_s(endtime, "%d-%d-%2d %d:%d:%d", ProgressGroup->endtime.tm_year, ProgressGroup->endtime.tm_mon, ProgressGroup->endtime.tm_mday, ProgressGroup->endtime.tm_hour, ProgressGroup->endtime.tm_min, ProgressGroup->endtime.tm_sec);
        char starttime[20] = "";
        sprintf_s(starttime, "%d-%d-%2d %d:%d:%d", ProgressGroup->starttime.tm_year, ProgressGroup->starttime.tm_mon, ProgressGroup->starttime.tm_mday, ProgressGroup->starttime.tm_hour, ProgressGroup->starttime.tm_min, ProgressGroup->starttime.tm_sec);
        int Textlength = WideCharToMultiByte(CP_UTF8, NULL, ProgressGroup->tips, wcslen(ProgressGroup->tips) + 1, NULL, NULL, NULL, NULL);
        char* x = new char[Textlength + 1];
        WideCharToMultiByte(CP_UTF8, NULL, ProgressGroup->tips, wcslen(ProgressGroup->tips) + 1, x, (Textlength + 1), NULL, NULL);
        Progress["name"] = x;
        Progress["endTime"] = endtime;
        Progress["startTime"] = starttime;
        root["Progress"].append(Progress);
        ProgressGroup = ProgressGroup->nextStruct;
        delete[] x;

    }
    while (EventsGroups != nullptr)
    {
        Json::Value Event;
        char endtime[20] = "";
        sprintf_s(endtime, "%d-%d-%2d %d:%d:%d", EventsGroups->EndTime.tm_year, EventsGroups->EndTime.tm_mon, EventsGroups->EndTime.tm_mday, EventsGroups->EndTime.tm_hour, EventsGroups->EndTime.tm_min, EventsGroups->EndTime.tm_sec);
        char starttime[20] = "";
        sprintf_s(starttime, "%d-%d-%2d %d:%d:%d", EventsGroups->StartTime.tm_year, EventsGroups->StartTime.tm_mon, EventsGroups->StartTime.tm_mday, EventsGroups->StartTime.tm_hour, EventsGroups->StartTime.tm_min, EventsGroups->StartTime.tm_sec);
        int Textlength = WideCharToMultiByte(CP_UTF8, NULL, EventsGroups->EventName, wcslen(EventsGroups->EventName) + 1, NULL, NULL, NULL, NULL);
        char* x = new char[Textlength + 1];
        WideCharToMultiByte(CP_UTF8, NULL, EventsGroups->EventName, wcslen(EventsGroups->EventName) + 1, x, (Textlength + 1), NULL, NULL);
        int Detaillength = WideCharToMultiByte(CP_UTF8, NULL, EventsGroups->EventDetail, wcslen(EventsGroups->EventDetail) + 1, NULL, NULL, NULL, NULL);
        char* Detailname = new char[Detaillength + 1];
        WideCharToMultiByte(CP_UTF8, NULL, EventsGroups->EventDetail, wcslen(EventsGroups->EventDetail) + 1, Detailname, (Detaillength + 1), NULL, NULL);
        Event["EventName"] = x;
        Event["EventDetail"] = Detailname;
        Event["endTime"] = endtime;
        Event["startTime"] = starttime;
        Event["remindTime"] = Json::Value(EventsGroups->remindmin);
        Event["repeatTime"] = Json::Value(EventsGroups->repeatdays);
        root["Event"].append(Event);
        EventsGroups = EventsGroups->nextStruct;
        delete[] x; delete[] Detailname;

    }
    Json::StyledWriter sw;
    sw.write(root);
    std::ofstream os;
    os.open(JsonFile, std::ios::out | std::ios::trunc);
    if (!os.is_open())
        MessageBox(hWndMain,L"An error ocured and need to be shutdown.",L"Error",0);
    os << sw.write(root);
    os.close();
}
void ReadFileSE(char *FilePaths) 
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream is(FilePaths,std::ios::binary);
    
    //int error=GetLastError();
    //MessageBox(NULL, L"ERROR!", L"ERROR!", 0);
    
    if (!reader.parse(is, root)) {
        MessageBox(hWndMain, L"ERROR!", L"ERROR!", 0);
        PostQuitMessage(0);
        return;
    }
    int i = root["Countdown"].size();
    for (int count = 0; count < i; count++)
    {
        tm endtime;
        Json::String temp = root["Countdown"][count]["endTime"].asString();
        char *endtimestruct = new char[temp.length()+1];
        sprintf_s(endtimestruct, temp.length()+1, "%s", temp.data());
        sscanf_s(endtimestruct, "%d-%d-%d %d:%d:%d", &endtime.tm_year, &endtime.tm_mon, &endtime.tm_mday, &endtime.tm_hour, &endtime.tm_min, &endtime.tm_sec);
        temp = root["Countdown"][count]["name"].asString();
        char* CountdownName = new char[temp.length()+1];
        sprintf_s(CountdownName, temp.length()+1, "%s", temp.data());
        int length = MultiByteToWideChar(CP_UTF8, NULL, CountdownName, temp.length() + 1, NULL, NULL);
        TCHAR *change = new TCHAR[length + 1];
        MultiByteToWideChar(CP_UTF8, NULL, CountdownName, temp.length() + 1, change, length);
        appendCountdown(hWndMain, endtime, change);
        delete[] endtimestruct; delete[] CountdownName;
        //for(int j = 0;j<)
        //appendCountdown()
    }
    i = root["Progress"].size();
    for (int count = 0; count < i; count++)
    {
        tm endtime;
        Json::String temp = root["Progress"][count]["endTime"].asString();
        char* endtimestruct = new char[temp.length() + 1];
        sprintf_s(endtimestruct, temp.length()+1, "%s", temp.data());
        sscanf_s(endtimestruct, "%d-%d-%d %d:%d:%d", &endtime.tm_year, &endtime.tm_mon, &endtime.tm_mday, &endtime.tm_hour, &endtime.tm_min, &endtime.tm_sec);
        tm starttime;
        temp = root["Progress"][count]["startTime"].asString();
        char* starttimestruct = new char[temp.length() + 1];
        sprintf_s(starttimestruct, temp.length()+1, "%s", temp.data());
        sscanf_s(starttimestruct, "%d-%d-%d %d:%d:%d", &starttime.tm_year, &starttime.tm_mon, &starttime.tm_mday, &starttime.tm_hour, &starttime.tm_min, &starttime.tm_sec);
        temp = root["Progress"][count]["name"].asString();
        char* ProgressName = new char[temp.length() + 1];
        sprintf_s(ProgressName, temp.length()+1, "%s", temp.data());
        int length = MultiByteToWideChar(CP_UTF8, NULL, ProgressName, temp.length() + 1, NULL, NULL);
        TCHAR* change = new TCHAR[length + 1];
        MultiByteToWideChar(CP_UTF8, NULL, ProgressName, temp.length() + 1, change, length);
        appendprogressBar(hWndMain, starttime,endtime, change);
        delete[] endtimestruct; delete[] ProgressName; delete[]starttimestruct;
    }
    i = root["Event"].size();
    for (int count = 0; count < i; count++)
    {
        tm endtime;
        Json::String temp = root["Event"][count]["endTime"].asString();
        char* endtimestruct = new char[temp.length() + 1];
        sprintf_s(endtimestruct, temp.length()+1, "%s", temp.data());
        sscanf_s(endtimestruct, "%d-%d-%d %d:%d:%d", &endtime.tm_year, &endtime.tm_mon, &endtime.tm_mday, &endtime.tm_hour, &endtime.tm_min, &endtime.tm_sec);
        tm starttime;
        temp = root["Event"][count]["startTime"].asString();
        char* starttimestruct = new char[temp.length() + 1];
        sprintf_s(starttimestruct, temp.length()+1, "%s", temp.data());
        sscanf_s(starttimestruct, "%d-%d-%d %d:%d:%d", &starttime.tm_year, &starttime.tm_mon, &starttime.tm_mday, &starttime.tm_hour, &starttime.tm_min, &starttime.tm_sec);
        temp = root["Event"][count]["EventName"].asString();
        char* EventName = new char[temp.length() + 1];
        sprintf_s(EventName, temp.length()+1, "%s", temp.data());
        int length = MultiByteToWideChar(CP_UTF8, NULL, EventName, temp.length() + 1, NULL, NULL);
        TCHAR* changename = new TCHAR[length + 1];
        MultiByteToWideChar(CP_UTF8, NULL, EventName, temp.length() + 1, changename, length);
        temp = root["Event"][count]["EventDetail"].asString();
        char* EventDetail = new char[temp.length() + 1];
        sprintf_s(EventDetail , temp.length()+1, "%s", temp.data());
        length = MultiByteToWideChar(CP_UTF8, NULL, EventDetail, temp.length() + 1, NULL, NULL);
        TCHAR* changedetail = new TCHAR[length + 1];
        MultiByteToWideChar(CP_UTF8, NULL, EventDetail , temp.length() + 1, changedetail, length);
        int repeatdays = root["Event"][count]["repeatTime"].asInt();
        int remindtimes = root["Event"][count]["remindTime"].asInt();
        appendEvent(changename, changedetail,starttime,endtime,repeatdays,remindtimes );
        delete[] endtimestruct; delete[] EventName; delete[]starttimestruct; delete[]EventDetail;
    }
    is.close();
}