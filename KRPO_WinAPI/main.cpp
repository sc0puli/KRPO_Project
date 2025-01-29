
#include "DesignLayout.h"
#include "FileIO.h"
#include "ColorLib.h"

const wchar_t windowClass[] = L"win32app";
const wchar_t windowTitle[] = L"Really bad Innovus";

enum POPUP_MENU_ID
{
    OPEN_ID,
    SAVE_AS_ID,
    EXIT_ID,
    METAL_ID,
    POLY_ID
};

bool isDrawing = false;
COLORREF currentColor = METAL_COLOR;
POINT newRectStartPoint, newRectEndPoint;

void DrawLayout(HDC hdc, HWND hWnd, std::vector<DesignLayout>& layout)
{
    for (size_t i = 0; i < layout.size(); i++)
    {
        int rect_left = layout[i].rect.left * scale;
        int rect_top = layout[i].rect.top * scale;
        int rect_right = layout[i].rect.right * scale;
        int rect_bottom = layout[i].rect.bottom * scale;

        RECT scaledRect = { rect_left, rect_top, rect_right, rect_bottom };

        HBRUSH hBrush = CreateSolidBrush(layout[i].color);

        FillRect(hdc, &scaledRect, hBrush);
        DeleteObject(hBrush);

        for (size_t j = 0; j < layout.size(); j++)
        {
            if (layout[i].color != layout[j].color)
            {
                if (&layout[i] != &layout[j])
                {
                    int rect_left = layout[j].rect.left * scale;
                    int rect_top = layout[j].rect.top * scale;
                    int rect_right = layout[j].rect.right * scale;
                    int rect_bottom = layout[j].rect.bottom * scale;

                    RECT scaledOtherRect = { rect_left, rect_top, rect_right, rect_bottom };
                    RECT intersectRect;

                    if (IntersectRect(&intersectRect, &scaledRect, &scaledOtherRect))
                    {
                        HBRUSH hIntersectBrush = CreateSolidBrush(INTERSECT_COLOR);
                        HDC memDC = CreateCompatibleDC(hdc);
                        SelectObject(memDC, hIntersectBrush);

                        int dcPrevMode = SetROP2(hdc, R2_XORPEN);
                        FillRect(hdc, &intersectRect, hIntersectBrush);
                        SetROP2(hdc, dcPrevMode);

                        DeleteObject(hIntersectBrush);
                        DeleteDC(memDC);
                    }
                }
            }
        }
    }
}


long __stdcall WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    PAINTSTRUCT ps;
    HDC hdc;

    RECT r;
    GetClientRect(hWnd, &r);

    switch (message) {
    case WM_CREATE:
    {
        HMENU hMenubar = CreateMenu();

        HMENU hPopupFileMenu = CreatePopupMenu();
        AppendMenu(hPopupFileMenu, MF_STRING, OPEN_ID, L"Open");
        AppendMenu(hPopupFileMenu, MF_STRING, SAVE_AS_ID, L"Save as");
        AppendMenu(hPopupFileMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hPopupFileMenu, MF_STRING, EXIT_ID, L"Exit");

        HMENU hPopupDrawMenu = CreatePopupMenu();
        AppendMenu(hPopupDrawMenu, MF_STRING, METAL_ID, L"Metal");
        AppendMenu(hPopupDrawMenu, MF_STRING, POLY_ID, L"Poly");

        AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hPopupFileMenu, L"&File");
        AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hPopupDrawMenu, L"&Draw");

        SetMenu(hWnd, hMenubar);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        isDrawing = true;
        newRectStartPoint.x = LOWORD(lParam);
        newRectStartPoint.y = HIWORD(lParam);
        break;
    }

    case WM_LBUTTONUP:
    {
        if (isDrawing)
        {
            isDrawing = false;
            RECT newRect = { newRectStartPoint.x, newRectStartPoint.y, LOWORD(lParam), HIWORD(lParam) };
            if (!(newRect.right == 0 || newRect.bottom == 0))
            {
                DesignLayout newDesignLayout;

                int hui = LOWORD(lParam);
                int vagina = HIWORD(lParam);

                int end_x = int((LOWORD(lParam) / scale));
                int end_y = int((HIWORD(lParam) / scale));
                int start_x = int((newRectStartPoint.x / scale));
                int start_y = int((newRectStartPoint.y / scale));

                RECT rect = { min(start_x, end_x), min(start_y, end_y), max(start_x, end_x), max(start_y, end_y) };

                design_width = max(design_width, newDesignLayout.rect.right);
                design_height = max(design_height, newDesignLayout.rect.bottom);

                newDesignLayout.rect = rect;
                newDesignLayout.shape = "REC";
                if (currentColor == METAL_COLOR)
                {
                    newDesignLayout.color = currentColor;
                    newDesignLayout.layer = "METAL";
                }
                else if (currentColor == POLY_COLOR)
                {
                    newDesignLayout.color = currentColor;
                    newDesignLayout.layer = "POLY";
                }
                layout.push_back(newDesignLayout);
            }
            InvalidateRect(hWnd, nullptr, 1);
        }
        break;
    }

    case WM_COMMAND:
    {
        switch ((LOWORD(wParam)))
        {
        case OPEN_ID:
            OpenFile(hWnd, layout);
            InvalidateRect(hWnd, nullptr, 1);
            break;

        case SAVE_AS_ID:
            SaveFile(hWnd, layout);
            break;

        case EXIT_ID:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
            break;

        case METAL_ID:
            currentColor = METAL_COLOR;
            break;

        case POLY_ID:
            currentColor = POLY_COLOR;
            break;
        }
        break;
    }

    case WM_SIZE:
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int currentWidth = clientRect.right - clientRect.left;
        int currentHeight = clientRect.bottom - clientRect.top;
        float scaleX = float((currentWidth) / design_width);
        float scaleY = float((currentHeight) / design_height);
        scale = min(scaleX, scaleY);
        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);
        DrawLayout(hdc, hWnd, layout);
        EndPaint(hWnd, &ps);
        break;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = windowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"Can’t register window class!", L"Win32 API Test", NULL);
        return 1;
    }

    HWND hWnd = CreateWindow(windowClass, windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, L"Can’t create window!", L"Win32 API Test", NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
