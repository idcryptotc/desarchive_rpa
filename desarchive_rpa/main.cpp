#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>

int main(int i, char **c)
{
    std::ifstream fin;
    std::ofstream fon;

    OPENFILENAME ofn;
    wchar_t szFile[260]{};
    HWND hwnd = nullptr;

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Архив Ren'Py (*.rpa)\0*.rpa\0Любые файлы (без гарантий)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        fin.open(ofn.lpstrFile, std::ios::binary);
    }
    else
    {
        if (CommDlgExtendedError())
        {
            MessageBox(hwnd, L"Не удаётся открыть файл", L"Проблема", 0);
        }
        else
        {
            MessageBox(hwnd, L"Файл не выбран. Программа будет закрыта", L"Проблема", 0);
        }

        return 0;
    }

    std::vector<unsigned char> buffer;
    bool isStart = false;
    bool isFinish = false;
    bool isStop = false;
    int count = 0;

    const std::vector<unsigned char> startSequence
    {
        (unsigned char)'\x89'
        , (unsigned char)'\x50'
        , (unsigned char)'\x4e'
        , (unsigned char)'\x47'
        , (unsigned char)'\x0d'
        , (unsigned char)'\x0a'
        , (unsigned char)'\x1a'
        , (unsigned char)'\x0a'
    };
    const std::vector<unsigned char> finishSequence
    {
        (unsigned char)'\x49'
        , (unsigned char)'\x45'
        , (unsigned char)'\x4e'
        , (unsigned char)'\x44'
        , (unsigned char)'\xae'
        , (unsigned char)'\x42'
        , (unsigned char)'\x60'
        , (unsigned char)'\x82'
    };
    
    if (fin.is_open())
    {
        while (!isStop)
        {
            for (ptrdiff_t i = 0; !isStart && i < 8; ++i)
            {
                buffer.push_back(fin.get());
                if (fin.eof())
                {
                    isStop = true;
                    break;
                }
                if (startSequence[i] != buffer[i])
                {
                    buffer.clear();
                    i = -1;
                    continue;
                }
            }

            if (isStop)
            {
                break;
            }

            if (!isStart)
            {
                isStart = true;
            }

            buffer.push_back(fin.get());
            
            if (fin.eof())
            {
                isStop = true;
                break;
            }

            for (ptrdiff_t i = 0; i < 8; ++i)
            {
                ptrdiff_t indexBuffer = buffer.size() - 8 + i;

                if (finishSequence[i] != buffer[indexBuffer])
                {
                    i = 8;
                    continue;
                }

                if (i == 7)
                {
                    isFinish = true;
                }
            }

            if (isFinish)
            {
                isStart = false;
                isFinish = false;
                std::string path = "default" + std::to_string(count++);
                path += ".png";
                fon.open(path, std::ios::binary);

                if (fon.is_open())
                {
                    for (auto it : buffer)
                    {
                        fon << it;
                    }
                }

                buffer.clear();
                fon.close();
            }
        }
    }
    
    fin.close();
    return 0;
}