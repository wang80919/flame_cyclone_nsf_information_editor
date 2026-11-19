#pragma once

#include <windows.h>

//图片类型
enum CImageType
{
    CImage_unknown = 0,
    CImage_bmp = 1,
    CImage_gif = 2,
    CImage_jpg = 3,
    CImage_png = 4,
    CImage_max = 5
};

//图片尺寸
typedef struct _IMAGE_SIZE
{
    DWORD dwWidth;
    DWORD dwHeight;
    CImageType imageType;
    _IMAGE_SIZE() :dwWidth(0), dwHeight(0), imageType(CImage_unknown) {};
}IMAGE_SIZE;

class CimageSizeUtils
{
public:
    static CImageType GetImageType(LPCTSTR lpFilePath);
    static bool GetImageSize(LPCTSTR lpFilePath, IMAGE_SIZE& size);

private:
    static bool _GetJpgSize(HANDLE hFile, IMAGE_SIZE& size);
    static bool _GetPngSize(HANDLE hFile, IMAGE_SIZE& size);
    static bool _GetGifSize(HANDLE hFile, IMAGE_SIZE& size);
    static bool _GetBmpSize(HANDLE hFile, IMAGE_SIZE& size);
};