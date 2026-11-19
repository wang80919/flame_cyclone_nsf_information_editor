#include "CCommandLine.h"

_tstring CCommandLine::GetCommand()
{
    _tstring strResult;

    LPCTSTR pCommandLine = ::GetCommandLine();
    if (nullptr != pCommandLine)
    {
        strResult = pCommandLine;
    }

    return strResult;
}

std::map<_tstring, _tstring> CCommandLine::GetCommandKeyValue(TCHAR chDelim)
{
    std::map<_tstring, _tstring> vResult;

    _tstring strCommandLine = ::GetCommandLine();

    do
    {
        if (strCommandLine.empty())
        {
            break;
        }

        int nArgs = 0;
        LPWSTR* pArglist = nullptr;

        std::wstring strCommand = _TStrToWStr(strCommandLine);
        pArglist = ::CommandLineToArgvW(strCommand.c_str(), &nArgs);
        if (nullptr == pArglist)
        {
            break;
        }

        for (int i = 1; i < nArgs; i++)
        {
            _tstring strName = _WStrToTStr(pArglist[i]);
            _tstring strValue;

            // 单个字符则认为只有值名, 没有值数据
            if (strName.size() > 1)
            {
                size_t nPos = strName.find_first_of(chDelim, 1);
                if (_tstring::npos != nPos && nPos > 0)
                {
                    strValue = strName.substr(nPos + 1);
                    strName.resize(nPos);
                }
            }

            // 检查插入结果, 插入失败则更新值数据
            auto pairInsert = vResult.insert(std::make_pair(strName, strValue));
            if (!pairInsert.second)
            {
                pairInsert.first->second = strValue;
            }
        }

        ::LocalFree(pArglist);

    } while (false);

    return vResult;
}

std::wstring CCommandLine::_TStrToWStr(const _tstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _MultiStrToWStr(CP_ACP, str);
#endif
}

_tstring CCommandLine::_WStrToTStr(const std::wstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _WStrToMultiStr(CP_ACP, str);
#endif
}

std::string CCommandLine::_WStrToMultiStr(UINT CodePage, const std::wstring& str)
{
    //计算缓冲区所需的字节长度
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strResult(cbMultiByte, 0);

    //成功则返回写入到指示的缓冲区的字节数
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

std::wstring CCommandLine::_MultiStrToWStr(UINT CodePage, const std::string& str)
{
    //计算缓冲区所需的字符长度
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}
