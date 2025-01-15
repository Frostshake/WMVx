#include "../../stdafx.h"
#include <QFile>
#include "CascFileSystem.h"
#include "../utility/Exceptions.h"
#include <future>
#include <fstream>

namespace core {
    CascFileSystem::CascFileSystem(const QString& root, const QString& locale, const QString& product, const QString& list_file) : GameFileSystem(root, locale), listFilePath(list_file) {

        cascLocale = WDBReader::Filesystem::CASCLocaleConvert(locale.toStdString());

        if (cascLocale == CASC_LOCALE_NONE) {
            throw std::runtime_error("Unsupported client locale.");
        }

        const auto product_str = product.toStdString();

        _impl = std::make_unique<WDBReader::Filesystem::CASCFilesystem>(root.toStdString(), cascLocale, product_str.c_str());

        {
            // attempt to check the existance of known files, only 1 needs to exist.
            bool passed = false;
            int last_error = 0;

            std::array checks = {
                "interface\\cursor\\point.blp",
                "Interface\\FrameXML\\Localization.lua",
            };

            HANDLE test;

            for (const auto& check : checks) {
                if (!CascOpenFile(_impl->getHandle(), check, cascLocale, 0, &test)) {
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

    std::unique_ptr<ArchiveFile> CascFileSystem::openFile(const GameFileUri& uri)
    {
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

        auto raw = _impl->open(id);
        if (raw != nullptr) {
            return std::make_unique<CascFile>(uri, std::move(raw));
        }
        
        return nullptr;

    }

    std::unique_ptr<std::vector<GameFileUri::path_t>> CascFileSystem::fileList(std::function<bool(const GameFileUri::path_t&)> pred)
    {
        auto list_items = std::make_unique<std::vector<QString>>(); 

        for (auto it = fileNameToIdMap.begin(); it != fileNameToIdMap.end(); ++it) {
            if (pred(it->first)) {
                HANDLE temp;
                if (CascOpenFile(_impl->getHandle(), CASC_FILE_DATA_ID(it->second), CASC_LOCALE_ALL, CASC_OPEN_BY_FILEID, &temp)) { 
                    list_items->push_back(it->first);
                    CascCloseFile(temp);
                }
            }
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
        std::ifstream stream("Support Files/tactkeys.txt");
        std::string line;

        // format is '{16 char key}{space}{32 char key}'

        while (std::getline(stream, line)) {
            if (line.size() < (16 + 1 + 32)) {
                continue;
            }

            std::string_view key_str(line.data(), 16);
            std::string_view val_str(line.data() + 16 + 1, 32);
            uint64_t key;
            const auto key_end = key_str.data() + key_str.size();
            const auto res = std::from_chars(key_str.data(), key_end, key, 16);

            if (res.ec == std::errc{} && res.ptr == key_end)
            {
                const bool ok = CascAddStringEncryptionKey(_impl->getHandle(), key, std::string(val_str).c_str());
                assert(ok);
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
        return _impl->size();
    }

    void CascFile::read(void* dest, uint64_t bytes, uint64_t offset)
    {
        _impl->setPos(offset);
        _impl->read(dest, bytes);
    }

}