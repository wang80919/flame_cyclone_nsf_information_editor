#include "CImageSizeUtils.h"

#define MAKEDWORD(_0, _1, _2, _3) (((DWORD)((BYTE)(_3)) << 24) | ((DWORD)((BYTE)(_2)) << 16) | ((DWORD)((BYTE)(_1)) << 8) | ((DWORD)((BYTE)(_0))))

//PNG图片使用
#pragma pack (1)
typedef struct _PNG_IHDR
{
    struct {
        BYTE data3;
        BYTE data2;
        BYTE data1;
        BYTE data0;
    }Width;         //图像宽度(大头字节序)，以像素为单位 
    struct {
        BYTE data3;
        BYTE data2;
        BYTE data1;
        BYTE data0;
    }Height;        //图像高度(大头字节序)，以像素为单位 
    BYTE Depth;     //图像深度：
    //索引彩色图像：1，2，4或8
    //灰度图像：1，2，4，8或16
    //真彩色图像：8或16
    BYTE ColorType; //    颜色类型：
    //0：灰度图像, 1，2，4，8或16
    //2：真彩色图像，8或16
    //3：索引彩色图像，1，2，4或8
    //4：带α通道数据的灰度图像，8或16
    //6：带α通道数据的真彩色图像，8或16
    BYTE CompressionMethod;
    BYTE FilterMethod;
    BYTE InterlaceMethod; //隔行扫描方法: 0：非隔行扫描 1： Adam7(由Adam M.Costello开发的7遍隔行扫描方法)
    _PNG_IHDR()
    {
        ZeroMemory(this, sizeof(_PNG_IHDR));
    }
}PNG_IHDR, * PPNG_IHDR;
#pragma pack()

//GIF图片使用
#pragma pack (1)
typedef struct _IMAGE_GIF
{
    BYTE Magic[3];      //文件头标识
    BYTE Version[3];    //版本号
    WORD Width;         //宽度
    WORD Height;        //高度
    BYTE Background;    //背景色
    BYTE aspect;        //宽高比
    BYTE ColorIndex[0x10][3];
    _IMAGE_GIF()
    {
        ZeroMemory(this, sizeof(_IMAGE_GIF));
    }
}IMAGE_GIF, * PIMAGE_GIF;
#pragma pack()

//JPG图片使用
#pragma pack (1)
typedef struct _IMAGE_JPG
{
    BYTE Type[2];        //start of frame
    struct {
        BYTE data1;
        BYTE data0;
    }Length;            //长度(大头字节序)
    BYTE Precision;     //精度, 每像素多少位
    struct {
        BYTE data1;
        BYTE data0;
    }Height;        //图像高度(大头字节序)，以像素为单位 
    struct {
        BYTE data1;
        BYTE data0;
    }Width;         //图像宽度(大头字节序)，以像素为单位 
    BYTE ComponentsIndex;    //组件数量（应为 1 或 3）
    BYTE Components[3][3];
    _IMAGE_JPG()
    {
        ZeroMemory(this, sizeof(_IMAGE_JPG));
    }
}IMAGE_JPG, * PIMAGE_JPG;
#pragma pack()

//BMP图片使用
#pragma pack (1)
typedef struct _IMAGE_BMP
{
    BYTE bfType[2];
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
    struct BITMAP_INFO
    {
        DWORD biSize;
        DWORD biWidth;
        DWORD biHeight;
        WORD biPlanes;
        WORD biBitCount;
        DWORD biCompression;
        DWORD biSizeImages;
        DWORD biXPelsPerMeter;
        DWORD biYPelsPerMeter;
        DWORD biClrUsed;
        DWORD biClrImportant;
    }bitmapInfo;

    _IMAGE_BMP()
    {
        ZeroMemory(this, sizeof(_IMAGE_BMP));
    }
}IMAGE_BMP, * PIMAGE_BMP;
#pragma pack()

CImageType CimageSizeUtils::GetImageType(LPCTSTR lpFilePath)
{
    CHAR szFileBuf[1024] = { 0 };
    DWORD dwNumberOfBytesRead = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    const UCHAR HEAD_BMP[2] = { (UCHAR)0x42, (UCHAR)0x4D };
    const UCHAR HEAD_JPG[2] = { (UCHAR)0xFF, (UCHAR)0xD8 };
    const UCHAR HEAD_GIF[3] = { (UCHAR)0x47, (UCHAR)0x49, (UCHAR)0x46 };
    const UCHAR HEAD_PNG[4] = { (UCHAR)0x89, (UCHAR)0x50, (UCHAR)0x4E, (UCHAR)0x47 };

    hFile = CreateFile(
        lpFilePath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return CImageType::CImage_unknown;
    }

    do
    {
        //检查是不是JPG
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_JPG), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_JPG, sizeof(HEAD_JPG)))
        {
            return CImageType::CImage_jpg;
        }

        //检查是不是GIF
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_GIF), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_GIF, sizeof(HEAD_GIF)))
        {
            return CImageType::CImage_gif;
        }

        //检查是不是BMP
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_GIF), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_BMP, sizeof(HEAD_BMP)))
        {
            return CImageType::CImage_bmp;
        }

        //检查是不是PNG
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_PNG), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_PNG, sizeof(HEAD_PNG)))
        {
            return CImageType::CImage_png;
        }
    } while (FALSE);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return CImageType::CImage_unknown;
}

bool CimageSizeUtils::GetImageSize(LPCTSTR lpFilePath, IMAGE_SIZE& size)
{
    CHAR szFileBuf[1024] = { 0 };
    DWORD dwNumberOfBytesRead = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    bool bResult = false;

    const UCHAR HEAD_BMP[2] = { (UCHAR)0x42, (UCHAR)0x4D };
    const UCHAR HEAD_JPG[2] = { (UCHAR)0xFF, (UCHAR)0xD8 };
    const UCHAR HEAD_GIF[3] = { (UCHAR)0x47, (UCHAR)0x49, (UCHAR)0x46 };
    const UCHAR HEAD_PNG[4] = { (UCHAR)0x89, (UCHAR)0x50, (UCHAR)0x4E, (UCHAR)0x47 };

    hFile = CreateFile(
        lpFilePath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    do
    {
        //检查是不是JPG
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_JPG), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_JPG, sizeof(HEAD_JPG)))
        {
            size.imageType = CImageType::CImage_jpg;
            return _GetJpgSize(hFile, size);
        }

        //检查是不是GIF
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_GIF), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_GIF, sizeof(HEAD_GIF)))
        {
            size.imageType = CImageType::CImage_gif;
            return _GetGifSize(hFile, size);
        }

        //检查是不是BMP
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_GIF), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_BMP, sizeof(HEAD_BMP)))
        {
            size.imageType = CImageType::CImage_bmp;
            return _GetBmpSize(hFile, size);
        }

        //检查是不是PNG
        ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
        if (!::ReadFile(hFile, szFileBuf, sizeof(HEAD_PNG), &dwNumberOfBytesRead, NULL))
        {
            break;
        }
        if (0 == memcmp(szFileBuf, HEAD_PNG, sizeof(HEAD_PNG)))
        {
            size.imageType = CImageType::CImage_png;
            return _GetPngSize(hFile, size);
        }
    } while (FALSE);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return bResult;
}

bool CimageSizeUtils::_GetJpgSize(HANDLE hFile, IMAGE_SIZE& size)
{
    IMAGE_JPG jpgTag;
    DWORD dwNumberOfBytesRead = 0;
    DWORD dwFilePointer = 0;
    DWORD dwFileSize = ::SetFilePointer(hFile, 0x00, NULL, FILE_END);

    if (2 != ::SetFilePointer(hFile, 0x02, NULL, FILE_BEGIN))
    {
        return false;
    }

    while (dwFilePointer < dwFileSize)
    {
        //读书数据标签头
        if (!::ReadFile(hFile, &jpgTag, 4, &dwNumberOfBytesRead, NULL))
        {
            break;
        }

        //得到数据标签长度
        WORD wLength = MAKEWORD(jpgTag.Length.data0, jpgTag.Length.data1);
        DWORD wReadSize = wLength - 2;

        //遇到SOF0 帧开始, 读取数据信息解析
        if (0xFF == jpgTag.Type[0] && 0xC0 == jpgTag.Type[1])
        {
            //防止缓冲区读入溢出(万一有人篡改数据)
            DWORD njpgTagSize = sizeof(jpgTag);
            if (wReadSize > njpgTagSize - 4)
            {
                wReadSize = njpgTagSize - 4;
            }

            if (!::ReadFile(hFile, ((LPBYTE)&jpgTag) + 4, wReadSize, &dwNumberOfBytesRead, NULL))
            {
                break;
            }

            size.dwWidth = MAKEWORD(jpgTag.Width.data0, jpgTag.Width.data1);
            size.dwHeight = MAKEWORD(jpgTag.Height.data0, jpgTag.Height.data1);
            return true;
        }

        //遇到SOS开始扫描则结束
        if (0xFF == jpgTag.Type[0] && 0xDA == jpgTag.Type[1])
        {
            break;
        }
        else
        {
            dwFilePointer = ::SetFilePointer(hFile, wReadSize, NULL, FILE_CURRENT);
        }
    }

    return true;
}

bool CimageSizeUtils::_GetPngSize(HANDLE hFile, IMAGE_SIZE& size)
{
    CHAR szFileBuf[1024] = { 0 };
    DWORD dwNumberOfBytesRead = 0;
    if (0x10 != ::SetFilePointer(hFile, 0x10, NULL, FILE_BEGIN))
    {
        return false;
    }

    if (::ReadFile(hFile, &szFileBuf, sizeof(PNG_IHDR), &dwNumberOfBytesRead, NULL))
    {
        PPNG_IHDR pData = reinterpret_cast<PPNG_IHDR>(szFileBuf);
        size.dwWidth = MAKEDWORD(pData->Width.data0, pData->Width.data1, pData->Width.data2, pData->Width.data3);
        size.dwHeight = MAKEDWORD(pData->Height.data0, pData->Height.data1, pData->Height.data2, pData->Height.data3);
        return true;
    }

    return false;
}

bool CimageSizeUtils::_GetGifSize(HANDLE hFile, IMAGE_SIZE& size)
{
    CHAR szFileBuf[1024] = { 0 };
    DWORD dwNumberOfBytesRead = 0;

    if (0 != ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN))
    {
        return false;
    }

    if (::ReadFile(hFile, &szFileBuf, sizeof(IMAGE_GIF), &dwNumberOfBytesRead, NULL))
    {
        PIMAGE_GIF pData = reinterpret_cast<PIMAGE_GIF>(szFileBuf);
        size.dwWidth = pData->Width;
        size.dwHeight = pData->Height;
        return true;
    }
    return false;
}

bool CimageSizeUtils::_GetBmpSize(HANDLE hFile, IMAGE_SIZE& size)
{
    CHAR szFileBuf[1024] = { 0 };
    DWORD dwNumberOfBytesRead = 0;

    if (0 != ::SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN))
    {
        return false;
    }

    if (::ReadFile(hFile, &szFileBuf, sizeof(IMAGE_BMP), &dwNumberOfBytesRead, NULL))
    {
        PIMAGE_BMP pData = reinterpret_cast<PIMAGE_BMP>(szFileBuf);
        size.dwWidth = pData->bitmapInfo.biWidth;
        size.dwHeight = pData->bitmapInfo.biHeight;
        return true;
    }

    return false;
}