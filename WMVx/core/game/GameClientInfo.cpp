#include "../../stdafx.h"
#include <QFile>
#include <QDir>
#include "GameClientInfo.h"
#include "GameClientAdaptor.h"
#include "BuildInfoFileReader.h"

#ifdef _WINDOWS
#include <shobjidl.h>

    HRESULT GetPropertyStore(PCWSTR pszFilename, GETPROPERTYSTOREFLAGS gpsFlags, IPropertyStore** ppps)
    {
        WCHAR szExpanded[MAX_PATH];
        HRESULT hr = ExpandEnvironmentStrings(pszFilename, szExpanded, ARRAYSIZE(szExpanded)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hr))
        {
            WCHAR szAbsPath[MAX_PATH];
            hr = _wfullpath(szAbsPath, szExpanded, ARRAYSIZE(szAbsPath)) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                hr = SHGetPropertyStoreFromParsingName(szAbsPath, NULL, gpsFlags, IID_PPV_ARGS(ppps));
            }
        }
        return hr;
    }
#endif 

namespace core {
    std::optional<GameClientInfo> GameClientInfo::detect(QString path)
    {
        QString build_info_path = path + QDir::separator() + ".build.info";
        if (QFile::exists(path + QDir::separator() + "Data") && QFile::exists(build_info_path)) {

            BuildInfoFileReader reader(build_info_path);

            if (reader.isValid()) {
                auto game_ver = GameClientVersion::fromString(reader.getVersion());

                if (game_ver.has_value()) {
                    auto info = GameClientInfo();
                    info.directory = path;
                    info.locale = "enUS";
                    info.version = game_ver.value();

                    return info;
                }
            }
            
            return std::nullopt;
        }

#ifdef _WINDOWS

        //handle legacy versions where the version is contained as file attribute.
        if (QFile::exists(path + QDir::separator() + "Data") &&
            QFile::exists(path + QDir::separator() + "Wow.exe")) {

            QString wow_version;
            std::wstring wow_path = path.toStdWString() + L"\\Wow.exe";
            const LPCWCHAR attr_name = L"System.FileVersion";

            {
                PROPERTYKEY key;
                HRESULT hr = PSGetPropertyKeyFromName(attr_name, &key);
                if (SUCCEEDED(hr))
                {
                    IPropertyStore* pps = NULL;
                    hr = GetPropertyStore(wow_path.data(), GPS_DEFAULT, &pps);
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT propvarValue = { 0 };
                        hr = pps->GetValue(key, &propvarValue);

                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszDisplayValue = NULL;
                            hr = PSFormatForDisplayAlloc(key, propvarValue, PDFF_DEFAULT, &pszDisplayValue);
                            if (SUCCEEDED(hr))
                            {
                                wow_version.append(pszDisplayValue);
                                CoTaskMemFree(pszDisplayValue);
                            }
                            PropVariantClear(&propvarValue);
                        }

                        pps->Release();
                    }
                }
            }

            auto game_ver = GameClientVersion::fromString(wow_version);

            if (game_ver.has_value()) {
                auto info = GameClientInfo();
                info.directory = path;
                info.locale = "enUS";
                info.version = game_ver.value();

                return info;
            }
        }
#else
#error unsupported platform
#endif

        return std::nullopt;
    }

    std::unique_ptr<GameClientAdaptor> GameClientInfo::adaptor() const
    {
        if (version.major == 1 && version.minor == 12 && version.patch == 1) {
            return std::make_unique<VanillaGameClientAdaptor>();
        }
        else if (version.major == 3 && version.minor == 3 && version.patch == 5) {
            return std::make_unique<WOTLKGameClientAdaptor>();
        }
        else if (version.major == 8 && version.minor == 3 && version.patch == 7 && version.build == 35435) {
            return std::make_unique<BFAGameClientAdaptor>();
        }

        return nullptr;
    }


}