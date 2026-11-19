#include "CPCMPlayer.h"
#include <fstream>
#include <mfapi.h>

CPCMPlayer::CPCMPlayer()
    :
    m_hPlayEvent(NULL)
{
    m_hPlayEvent = ::CreateEvent(NULL, 0, 0, NULL);
}

CPCMPlayer::~CPCMPlayer()
{
    ::CloseHandle(m_hPlayEvent);
}

bool CPCMPlayer::SyncPlay(_tstring strFile)
{
    HWAVEOUT hwo = NULL;

    std::ifstream inputFile(strFile.c_str(), std::ios::binary | std::ios::in);
    if (!inputFile.is_open())
    {
        return false;
    }

    //获取文件大小
    inputFile.seekg(0, std::ios::end);
    std::streamoff nSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    m_pcmData.resize(nSize);
    inputFile.read((char*)&m_pcmData[0], nSize);
    inputFile.close();

    // 初始化波形格式信息
    WAVEFORMATEXTENSIBLE wfx = { 0 };
    wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wfx.Format.wBitsPerSample = 32;
    wfx.Format.nChannels = 2;
    wfx.Format.nSamplesPerSec = 44100;
    wfx.Format.nBlockAlign = wfx.Format.nChannels * wfx.Format.wBitsPerSample / 8;
    wfx.Format.nAvgBytesPerSec = wfx.Format.nBlockAlign * wfx.Format.nSamplesPerSec;
    wfx.Format.cbSize = sizeof(wfx) - sizeof(wfx.Format);
    wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
    //wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;           // 32位整数
    wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;    // 32位浮点数
    wfx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;

    // 打开给定的波形音频输出设备进行播放
    MMRESULT mmr =  ::waveOutOpen(&hwo, WAVE_MAPPER, (PWAVEFORMATEX)& wfx, (DWORD_PTR)m_hPlayEvent, 0L, CALLBACK_EVENT);
    if (NULL == hwo)
    {
        return false;
    }

    WAVEHDR wh = { 0 };
    uint32_t remainingSize = m_pcmData.size();
    uint32_t playedSize = 0;
    uint32_t bufSize = (wfx.Format.nSamplesPerSec * wfx.Format.nChannels);

    // 播放采样数据
    while (remainingSize > 0)
    {
        // 缓冲块大小限定
        if (remainingSize < bufSize)
        {
            bufSize = remainingSize;
        }

        // 填充数据块相关信息
        wh.lpData = (LPSTR)&m_pcmData[playedSize];
        wh.dwFlags = WHDR_DONE;
        wh.dwLoops = 1L;
        wh.dwBufferLength = bufSize;

        remainingSize -= bufSize;
        playedSize += bufSize;

        // 准备用于播放的波形音频数据块
        ::waveOutPrepareHeader(hwo, &wh, sizeof(WAVEHDR));

        // 将数据块发送到给定的波形音频输出设备
        ::waveOutWrite(hwo, &wh, sizeof(WAVEHDR));

        // 等待播放设备处理完毕
        DWORD dwResult = ::WaitForSingleObject(m_hPlayEvent, INFINITE);
        if (WAIT_OBJECT_0 != dwResult)
        {
            break;
        }
    }

    // 音频输出设备上停止播放
    ::waveOutReset(hwo);
    ::waveOutClose(hwo);

    return true;
}