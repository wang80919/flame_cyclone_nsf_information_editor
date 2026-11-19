#pragma once

#include <wtypesbase.h>
#include <string>

#ifdef _UNICODE
typedef std::wstring _tstring;
#else
typedef std::string _tstring;
#endif

#define DECLARE_FOLDER_API(_fun_name) static _tstring Get##_fun_name();

//已知文件夹
class CFolderUtils
{
public:
        DECLARE_FOLDER_API(NetworkFolder)           //Not applicable—virtual folder
        DECLARE_FOLDER_API(ComputerFolder)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(InternetFolder)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(ControlPanelFolder)      //Not applicable—virtual folder
        DECLARE_FOLDER_API(PrintersFolder)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(SyncManagerFolder)       //Not applicable—virtual folder
        DECLARE_FOLDER_API(SyncSetupFolder)         //Not applicable—virtual folder
        DECLARE_FOLDER_API(ConflictFolder)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(SyncResultsFolder)       //Not applicable—virtual folder
        DECLARE_FOLDER_API(RecycleBinFolder)        //Not applicable—virtual folder
        DECLARE_FOLDER_API(ConnectionsFolder)       //Not applicable—virtual folder
        DECLARE_FOLDER_API(Fonts)                   //%windir%\Fonts
        DECLARE_FOLDER_API(Desktop)                 //%USERPROFILE%\Desktop
        DECLARE_FOLDER_API(Startup)                 //%APPDATA%\Microsoft\Windows\Start Menu\Programs\StartUp
        DECLARE_FOLDER_API(Programs)                //%APPDATA%\Microsoft\Windows\Start Menu\Programs
        DECLARE_FOLDER_API(StartMenu)               //%APPDATA%\Microsoft\Windows\Start Menu
        DECLARE_FOLDER_API(Recent)                  //%APPDATA%\Microsoft\Windows\Recent
        DECLARE_FOLDER_API(SendTo)                  //%APPDATA%\Microsoft\Windows\SendTo
        DECLARE_FOLDER_API(Documents)               //%USERPROFILE%\Documents
        DECLARE_FOLDER_API(Favorites)               //%USERPROFILE%\Favorites
        DECLARE_FOLDER_API(NetHood)                 //%APPDATA%\Microsoft\Windows\Network Shortcuts
        DECLARE_FOLDER_API(PrintHood)               //%APPDATA%\Microsoft\Windows\Printer Shortcuts
        DECLARE_FOLDER_API(Templates)               //%APPDATA%\Microsoft\Windows\Templates
        DECLARE_FOLDER_API(CommonStartup)           //%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu\Programs\StartUp
        DECLARE_FOLDER_API(CommonPrograms)          //%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu\Programs
        DECLARE_FOLDER_API(CommonStartMenu)         //%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu
        DECLARE_FOLDER_API(PublicDesktop)           //%PUBLIC%\Desktop
        DECLARE_FOLDER_API(ProgramData)             //%SystemDrive%\ProgramData)
        DECLARE_FOLDER_API(CommonTemplates)         //%ALLUSERSPROFILE%\Microsoft\Windows\Templates
        DECLARE_FOLDER_API(PublicDocuments)         //%PUBLIC%\Documents
        DECLARE_FOLDER_API(RoamingAppData)          //%APPDATA% (%USERPROFILE%\AppData\Roaming)
        DECLARE_FOLDER_API(LocalAppData)            //%LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
        DECLARE_FOLDER_API(LocalAppDataLow)         //%USERPROFILE%\AppData\LocalLow
        DECLARE_FOLDER_API(InternetCache)           //%LOCALAPPDATA%\Microsoft\Windows\Temporary Internet Files
        DECLARE_FOLDER_API(Cookies)                 //%APPDATA%\Microsoft\Windows\Cookies
        DECLARE_FOLDER_API(History)                 //%LOCALAPPDATA%\Microsoft\Windows\History
        DECLARE_FOLDER_API(System)                  //%windir%\system32
        DECLARE_FOLDER_API(SystemX86)               //%windir%\SysWOW64
        DECLARE_FOLDER_API(Windows)                 //%windir%
        DECLARE_FOLDER_API(Profile)                 //%USERPROFILE% (%SystemDrive%\Users\%USERNAME%)
        DECLARE_FOLDER_API(Pictures)                //%USERPROFILE%\Pictures
        DECLARE_FOLDER_API(ProgramFilesX86)         //%ProgramFiles% (%SystemDrive%\Program Files)
        DECLARE_FOLDER_API(ProgramFilesCommonX86)   //%ProgramFiles%\Common Files
        DECLARE_FOLDER_API(ProgramFilesX64)         //%ProgramFiles% (%SystemDrive%\Program Files)
        DECLARE_FOLDER_API(ProgramFilesCommonX64)   //%ProgramFiles%\Common Files
        DECLARE_FOLDER_API(ProgramFiles)            //%ProgramFiles% (%SystemDrive%\Program Files)
        DECLARE_FOLDER_API(ProgramFilesCommon)      //%ProgramFiles%\Common Files
        DECLARE_FOLDER_API(UserProgramFiles)        //%LOCALAPPDATA%\Programs
        DECLARE_FOLDER_API(UserProgramFilesCommon)  //%LOCALAPPDATA%\Programs\Common
        DECLARE_FOLDER_API(AdminTools)              //%APPDATA%\Microsoft\Windows\Start Menu\Programs\Administrative Tools
        DECLARE_FOLDER_API(CommonAdminTools)        //%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu\Programs\Administrative Tools
        DECLARE_FOLDER_API(Music)                   //%USERPROFILE%\Music
        DECLARE_FOLDER_API(Videos)                  //%USERPROFILE%\Videos
        DECLARE_FOLDER_API(Ringtones)               //%LOCALAPPDATA%\Microsoft\Windows\Ringtones
        DECLARE_FOLDER_API(PublicPictures)          //%PUBLIC%\Pictures
        DECLARE_FOLDER_API(PublicMusic)             //%PUBLIC%\Music
        DECLARE_FOLDER_API(PublicVideos)            //%PUBLIC%\Videos
        DECLARE_FOLDER_API(PublicRingtones)         //%ALLUSERSPROFILE%\Microsoft\Windows\Ringtones
        DECLARE_FOLDER_API(ResourceDir)             //%windir%\Resources
        DECLARE_FOLDER_API(LocalizedResourcesDir)   //%windir%\resources\0409 (code page)
        DECLARE_FOLDER_API(CommonOEMLinks)          //%ALLUSERSPROFILE%\OEM Links
        DECLARE_FOLDER_API(CDBurning)               //%LOCALAPPDATA%\Microsoft\Windows\Burn\Burn
        DECLARE_FOLDER_API(UserProfiles)            //%SystemDrive%\Users
        DECLARE_FOLDER_API(Playlists)               //%USERPROFILE%\Music\Playlists
        DECLARE_FOLDER_API(SamplePlaylists)         //%PUBLIC%\Music\Sample Playlists
        DECLARE_FOLDER_API(SampleMusic)             //%PUBLIC%\Music\Sample Music
        DECLARE_FOLDER_API(SamplePictures)          //%PUBLIC%\Pictures\Sample Pictures
        DECLARE_FOLDER_API(SampleVideos)            //%PUBLIC%\Videos\Sample Videos
        DECLARE_FOLDER_API(PhotoAlbums)             //%USERPROFILE%\Pictures\Slide Shows
        DECLARE_FOLDER_API(Public)                  //%PUBLIC% (%SystemDrive%\Users\Public)
        DECLARE_FOLDER_API(ChangeRemovePrograms)    //Not applicable—virtual folder
        DECLARE_FOLDER_API(AppUpdates)              //Not applicable—virtual folder
        DECLARE_FOLDER_API(AddNewPrograms)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(Downloads)               //%USERPROFILE%\Downloads
        DECLARE_FOLDER_API(PublicDownloads)         //%PUBLIC%\Downloads
        DECLARE_FOLDER_API(SavedSearches)           //%USERPROFILE%\Searches
        DECLARE_FOLDER_API(QuickLaunch)             //%APPDATA%\Microsoft\Internet Explorer\Quick Launch
        DECLARE_FOLDER_API(Contacts)                //%USERPROFILE%\Contacts
        DECLARE_FOLDER_API(SidebarParts)            //%LOCALAPPDATA%\Microsoft\Windows Sidebar\Gadgets
        DECLARE_FOLDER_API(SidebarDefaultParts)     //%ProgramFiles%\Windows Sidebar\Gadgets
        DECLARE_FOLDER_API(PublicGameTasks)         //%ALLUSERSPROFILE%\Microsoft\Windows\GameExplorer
        DECLARE_FOLDER_API(GameTasks)               //%LOCALAPPDATA%\Microsoft\Windows\GameExplorer
        DECLARE_FOLDER_API(SavedGames)              //%USERPROFILE%\Saved Games
        DECLARE_FOLDER_API(Games)                   //Not applicable—virtual folder
        DECLARE_FOLDER_API(SEARCH_MAPI)             //Not applicable—virtual folder
        DECLARE_FOLDER_API(SEARCH_CSC)              //Not applicable—virtual folder
        DECLARE_FOLDER_API(Links)                   //%USERPROFILE%\Links
        DECLARE_FOLDER_API(UsersFiles)              //Not applicable—virtual folder
        DECLARE_FOLDER_API(UsersLibraries)          //Not applicable—virtual folder
        DECLARE_FOLDER_API(SearchHome)              //Not applicable—virtual folder
        DECLARE_FOLDER_API(OriginalImages)          //%LOCALAPPDATA%\Microsoft\Windows Photo Gallery\Original Images
        DECLARE_FOLDER_API(DocumentsLibrary)        //%APPDATA%\Microsoft\Windows\Libraries\Documents.library-ms
        DECLARE_FOLDER_API(MusicLibrary)            //%APPDATA%\Microsoft\Windows\Libraries\Music.library-ms
        DECLARE_FOLDER_API(PicturesLibrary)         //%APPDATA%\Microsoft\Windows\Libraries\Pictures.library-ms
        DECLARE_FOLDER_API(VideosLibrary)           //%APPDATA%\Microsoft\Windows\Libraries\Videos.library-ms
        DECLARE_FOLDER_API(RecordedTVLibrary)       //%PUBLIC%\RecordedTV.library-ms
        DECLARE_FOLDER_API(HomeGroup)               //Not applicable—virtual folder
        DECLARE_FOLDER_API(HomeGroupCurrentUser)    //Not applicable—virtual folder
        DECLARE_FOLDER_API(DeviceMetadataStore)     //%ALLUSERSPROFILE%\Microsoft\Windows\DeviceMetadataStore
        DECLARE_FOLDER_API(Libraries)               //%APPDATA%\Microsoft\Windows\Libraries
        DECLARE_FOLDER_API(PublicLibraries)         //%ALLUSERSPROFILE%\Microsoft\Windows\Libraries
        DECLARE_FOLDER_API(UserPinned)              //%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned
        DECLARE_FOLDER_API(ImplicitAppShortcuts)    //%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\ImplicitAppShortcuts
        DECLARE_FOLDER_API(AccountPictures)         //%APPDATA%\Microsoft\Windows\AccountPictures
        DECLARE_FOLDER_API(PublicUserTiles)         //%PUBLIC%\AccountPictures
        DECLARE_FOLDER_API(AppsFolder)              //Not applicable—virtual folder
        DECLARE_FOLDER_API(StartMenuAllPrograms)    //
        DECLARE_FOLDER_API(CommonStartMenuPlaces)   //
        DECLARE_FOLDER_API(ApplicationShortcuts)    //
        DECLARE_FOLDER_API(RoamingTiles)            //%LOCALAPPDATA%\Microsoft\Windows\RoamingTiles
        DECLARE_FOLDER_API(RoamedTileImages)        //%LOCALAPPDATA%\Microsoft\Windows\RoamedTileImages
        DECLARE_FOLDER_API(Screenshots)             //%USERPROFILE%\Pictures\Screenshots
        DECLARE_FOLDER_API(CameraRoll)              //%USERPROFILE%\Pictures\Camera Roll
        DECLARE_FOLDER_API(SkyDrive)                //%USERPROFILE%\OneDrive
        DECLARE_FOLDER_API(OneDrive)                //%USERPROFILE%\OneDrive
        DECLARE_FOLDER_API(SkyDriveDocuments)       //%USERPROFILE%\OneDrive\Documents
        DECLARE_FOLDER_API(SkyDrivePictures)        //%USERPROFILE%\OneDrive\Pictures
        DECLARE_FOLDER_API(SkyDriveMusic)           //
        DECLARE_FOLDER_API(SkyDriveCameraRoll)      //%USERPROFILE%\OneDrive\Pictures\Camera Roll
        DECLARE_FOLDER_API(SearchHistory)           //%LOCALAPPDATA%\Microsoft\Windows\ConnectedSearch\History
        DECLARE_FOLDER_API(SearchTemplates)         //%LOCALAPPDATA%\Microsoft\Windows\ConnectedSearch\Templates
        DECLARE_FOLDER_API(CameraRollLibrary)       //
        DECLARE_FOLDER_API(SavedPictures)           //%USERPROFILE%\Pictures\Saved Pictures
        DECLARE_FOLDER_API(SavedPicturesLibrary)    //%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Libraries\SavedPictures.library-ms
        DECLARE_FOLDER_API(RetailDemo)              //
        DECLARE_FOLDER_API(Device)                  //
        DECLARE_FOLDER_API(DevelopmentFiles)        //
        DECLARE_FOLDER_API(Objects3D)               //%USERPROFILE%\3D Objects
        DECLARE_FOLDER_API(AppCaptures)             //%USERPROFILE%\Videos\Captures
        DECLARE_FOLDER_API(LocalDocuments)          //%USERPROFILE%\Documents
        DECLARE_FOLDER_API(LocalPictures)           //%USERPROFILE%\Pictures
        DECLARE_FOLDER_API(LocalVideos)             //%USERPROFILE%\Videos
        DECLARE_FOLDER_API(LocalMusic)              //%USERPROFILE%\Music
        DECLARE_FOLDER_API(LocalDownloads)          //%USERPROFILE%\Downloads
        DECLARE_FOLDER_API(RecordedCalls)           //
        DECLARE_FOLDER_API(AllAppMods)              //
        DECLARE_FOLDER_API(CurrentAppMods)          //
        DECLARE_FOLDER_API(AppDataDesktop)          //%LOCALAPPDATA%\Desktop
        DECLARE_FOLDER_API(AppDataDocuments)        //%LOCALAPPDATA%\Documents
        DECLARE_FOLDER_API(AppDataFavorites)        //%LOCALAPPDATA%\Favorites
        DECLARE_FOLDER_API(AppDataProgramData)      //%LOCALAPPDATA%\ProgramData
        DECLARE_FOLDER_API(LocalStorage)            //
};