#include "../../stdafx.h"
#include <QFile>
#include "CascFileSystem.h"
#include "../utility/Exceptions.h"
#include <future>

namespace core {
    CascFileSystem::CascFileSystem(const QString& root, const QString& list_file) : GameFileSystem(root), listFilePath(list_file) {
        hStorage = nullptr;

        std::map<QString, int> locales;
        locales["frFR"] = CASC_LOCALE_FRFR;
        locales["deDE"] = CASC_LOCALE_DEDE;
        locales["esES"] = CASC_LOCALE_ESES;
        locales["esMX"] = CASC_LOCALE_ESMX;
        locales["ptBR"] = CASC_LOCALE_PTBR;
        locales["itIT"] = CASC_LOCALE_ITIT;
        locales["ptPT"] = CASC_LOCALE_PTPT;
        locales["enGB"] = CASC_LOCALE_ENGB;
        locales["ruRU"] = CASC_LOCALE_RURU;
        locales["enUS"] = CASC_LOCALE_ENUS;
        locales["enCN"] = CASC_LOCALE_ENCN;
        locales["enTW"] = CASC_LOCALE_ENTW;
        locales["koKR"] = CASC_LOCALE_KOKR;
        locales["zhCN"] = CASC_LOCALE_ZHCN;
        locales["zhTW"] = CASC_LOCALE_ZHTW;

        QString casc_params = root + "*wow"; 

        // casc_params = casc_params.replace('/', '\\');

         //TODO not sure why casclib is so picky on \\ vs / , could be problematic?
         //TODO not sure the LPCISTR cast is needed? maybe linking to wrong lib?
        if (!CascOpenStorage((LPCTSTR)casc_params.toStdString().c_str(), CASC_LOCALE_ENUS, &hStorage)) {
            int error = GetLastError();
            throw std::runtime_error(std::string("Unable to initialise casc storage. error - ") + std::to_string(error));
        }

        {
            // attempt to check the existance of known files, only 1 needs to exist.
            bool passed = false;
            int last_error = 0;

            std::array checks = {
                "Interface\\FrameXML\\Localization.lua",
            };

            HANDLE test;

            for (const auto& check : checks) {
                if (!CascOpenFile(hStorage, check, CASC_LOCALE_ENUS, 0, &test)) {
                    last_error = GetLastError();
                }
                else {
                    CascCloseFile(test);
                    passed = true;
                    break;
                }
            }

            if (!passed) {
                throw std::runtime_error(std::string("Unable to open casc storage. error - ") + std::to_string(last_error));
            }
        }

        addExtraEncryptionKeys();
    }

    CascFileSystem::CascFileSystem(CascFileSystem&& instance) : GameFileSystem(std::move(instance))
    {
        hStorage = instance.hStorage;
        instance.hStorage = nullptr;

        fileNameToIdMap = std::move(instance.fileNameToIdMap);
        idToFileNameMap = std::move(instance.idToFileNameMap);
    }

    CascFileSystem::~CascFileSystem() {
        if (hStorage != nullptr) {
            CascCloseStorage(hStorage);
        }
    }

    std::future<void> CascFileSystem::load()
    {
        QFile list(listFilePath);

        std::vector<std::pair<QString, qsizetype>> queued_lines;
        
        if (list.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            queued_lines.reserve(10000);

            QTextStream in(&list);

            while (!in.atEnd())
            {
                QString line = in.readLine();
                const auto first_index = line.indexOf(';');

                if (first_index > 0) {
                    if (line.endsWith(".db2")) {
                        addFileMappingFromLine(line, first_index);
                    }
                    else {
                        queued_lines.emplace_back(line, first_index);
                    }
                }
            }
        }
        else {
            throw FileIOException(listFilePath.toStdString(), "Unable to load list file.");
        }

        const auto queued_size = queued_lines.size();
        idToFileNameMap.reserve(queued_size);
        fileNameToIdMap.reserve(queued_size);

        return std::async(std::launch::async, [&](std::vector<std::pair<QString, qsizetype>>&& lines) {

            for (auto it = lines.begin(); it != lines.end(); ++it) {
                addFileMappingFromLine(it->first, it->second);
            }
        }, std::move(queued_lines));
    }

    ArchiveFile* CascFileSystem::openFile(const GameFileUri& uri)
    {
        auto file = new CascFile();
        file->uri = uri;

        auto id = 0;

        try {
            id = std::visit([&](const auto& var) -> GameFileUri::id_t {
                if constexpr (std::is_same_v<const GameFileUri::id_t&, decltype(var)>) {
                    return var;
                }
                else if constexpr (std::is_same_v<const GameFileUri::path_t&, decltype(var)>) {
                    return fileNameToIdMap[var];
                }
                return 0;
                }, uri);

        }
        catch (std::exception e) {
            id = 0;
        }

        if (id == 0) {
            return nullptr;
        }

        if (!CascOpenFile(hStorage, CASC_FILE_DATA_ID(id), CASC_LOCALE_ENUS, CASC_OPEN_BY_FILEID, &file->casc_file)) {
            int error = GetLastError();
            delete file;

            return nullptr;
        }

        return file;
    }

    void CascFileSystem::closeFile(ArchiveFile* file)
    {
        CascFile* casc_file = (CascFile*)file;
        if (casc_file->casc_file != nullptr) {
            CascCloseFile(casc_file->casc_file);
        }

        delete file;
    }

    std::unique_ptr<std::list<GameFileUri::path_t>> CascFileSystem::fileList()
    {
        std::unique_ptr<std::list<QString>> list_items = std::make_unique<std::list<QString>>();

        for (auto it = fileNameToIdMap.begin(); it != fileNameToIdMap.end(); ++it) {
            list_items->push_back(it->first);
        }

        return std::move(list_items);
    }

    GameFileUri CascFileSystem::asFileId(const GameFileUri& uri)
    {
        if (uri.isPath()) {
            return fileNameToIdMap[uri.getPath()];
        }

        return uri;
    }

    GameFileUri CascFileSystem::asFilePath(const GameFileUri& uri)
    {
        if (uri.isId()) {
            return idToFileNameMap[uri.getId()];
        }

        return uri;
    }

    GameFileUri CascFileSystem::asInternal(const GameFileUri& uri)
    {
        return asFileId(uri);
    }

    GameFileUri CascFileSystem::asInternal(const GameFileInfo& info)
    {
        return info.id;
    }

    GameFileInfo CascFileSystem::asInfo(const GameFileUri& uri)
    {
        auto info = GameFileInfo();

        if (uri.isId()) {
            info.id = uri.getId();
            info.path = idToFileNameMap[info.id];
        }
        else {
            info.path = uri.getPath();
            info.id = fileNameToIdMap[info.path];
        }

        return info;
    }


    void CascFileSystem::addExtraEncryptionKeys() {
        QFile tactKeys("Support Files\\extra-encryption-keys.csv");

        if (tactKeys.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&tactKeys);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.startsWith("##") || line.startsWith("\"##"))  // ignore lines beginning with ##, useful for adding comments.
                    continue;

                QStringList lineData = line.split(';');
                if (lineData.size() != 2)
                    continue;
                QString keyName = lineData.at(0);
                QString keyValue = lineData.at(1);
                if (keyName.isEmpty() || keyValue.isEmpty())
                    continue;

                bool ok, ok2;
                ok2 = CascAddStringEncryptionKey(hStorage, keyName.toULongLong(&ok, 16), keyValue.toStdString().c_str());

                if (!ok2) {
                    assert(false);
                    //TODO error
                }
            }
        }
    }

    /*
        specialist atoi implementation - must faster than regular conversion - see https://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi
        input must be only characters, this is fine as the listfile input is known.
    */
    uint32_t fast_atoi(const char* str)
    {
        uint32_t val = 0;
        while (*str) {
            val = val * 10 + (*str++ - '0');
        }
        return val;
    }

    inline void CascFileSystem::addFileMappingFromLine(const QString& line, qsizetype seperator_pos)
    {
        const GameFileUri::id_t id = fast_atoi(line.first(seperator_pos).toStdString().c_str());
        const GameFileUri::path_t fileName = line.last(line.length() - (seperator_pos + 1)).toLower();

        fileNameToIdMap[fileName] = id;
        idToFileNameMap[id] = fileName;
    }

    uint64_t CascFile::getFileSize()
    {
        return CascGetFileSize(casc_file, 0);
    }

    void CascFile::read(void* dest, uint64_t bytes, uint64_t offset)
    {
        DWORD res = 0;
        CascSetFilePointer(casc_file, offset, NULL, FILE_BEGIN);
        auto result = CascReadFile(casc_file, dest, bytes, &res);
        if (!result) {
            auto error = GetLastError();
            //TODO handle error
            assert(false);
        }
        
    }

}