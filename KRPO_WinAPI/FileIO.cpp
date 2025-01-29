#include "FileIO.h"

int design_width = 484;
int design_height = 341;
float scale = 1.0;
std::vector<DesignLayout> layout;

void OpenFile(HWND hWnd, std::vector<DesignLayout>& layout)
{
    OPENFILENAME ofn;
    wchar_t open_szFile[MAX_PATH] = L"";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = open_szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"txt";

    if (GetOpenFileName(&ofn))
    {
        std::fstream in_file(ofn.lpstrFile);
        if (in_file.is_open())
        {
            design_width = 0;
            design_height = 0;
            layout.clear();

            std::string line;
            DesignLayout line_rect;
            while (!in_file.eof())
            {
                in_file >> line_rect.shape;
                in_file >> line_rect.rect.left;
                in_file >> line_rect.rect.top;
                in_file >> line_rect.rect.right;
                in_file >> line_rect.rect.bottom;
                in_file >> line_rect.layer;
                line_rect.rect.right = line_rect.rect.left + line_rect.rect.right;
                line_rect.rect.bottom = line_rect.rect.top + line_rect.rect.bottom;
                if (line_rect.layer == "METAL") 
                    line_rect.color = METAL_COLOR;
                else 
                    if (line_rect.layer == "POLY") 
                        line_rect.color = POLY_COLOR;
                layout.push_back(line_rect);

                design_width = max(design_width, line_rect.rect.right);
                design_height = max(design_height, line_rect.rect.bottom);
            }
        }
        else
        {
            MessageBox(NULL, ofn.lpstrFile, L"FILE NOT OPEN", MB_OK);
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        }

        in_file.close();
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    float scaleX = float((clientWidth) / design_width);
    float scaleY = float((clientHeight) / design_height);

    scale = min(scaleX, scaleY);

    InvalidateRect(hWnd, nullptr, 1);
}

void SaveFile(HWND hWnd, std::vector<DesignLayout>& layout)
{
    OPENFILENAME save_ofn;
    wchar_t save_as_szFile[MAX_PATH] = L"";
    ZeroMemory(&save_ofn, sizeof(save_ofn));
    save_ofn.lStructSize = sizeof(save_ofn);
    save_ofn.hwndOwner = hWnd;
    save_ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    save_ofn.lpstrFile = save_as_szFile;
    save_ofn.nMaxFile = MAX_PATH;
    save_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
    if (GetSaveFileName(&save_ofn))
    {
        std::ofstream out_file(save_ofn.lpstrFile);
        if (out_file.is_open())
        {
            out_file << std::left;
            for (size_t i = 0; i < layout.size(); i++)
            {
                out_file << std::setw(5) << layout[i].shape;
                out_file << std::setw(5) << layout[i].rect.left;
                out_file << std::setw(5) << layout[i].rect.top;
                out_file << std::setw(5) << layout[i].rect.right - layout[i].rect.left;
                out_file << std::setw(5) << layout[i].rect.bottom - layout[i].rect.top;
                out_file << std::setw(5) << layout[i].layer;
                if (i != layout.size() - 1) 
                    out_file << "\n";
            }
        }
        else
        {
            MessageBox(NULL, save_ofn.lpstrFile, L"OUTPUT FILE NOT OPEN", MB_OK);
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        }

        out_file.close();
    }
}