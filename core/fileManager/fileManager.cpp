#define __APICALL_EXTERN
#include "fileManager.h"
#undef __APICALL_EXTERN

#include "logging.h"
#include "magic_enum/magic_enum.hpp"
#include "utility/utility.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

LOG_DEFINE_MODULE(FileManager);

struct UniData {
  CLASS_NO_COPY(UniData);

  enum class Type { File, Dir };
  Type const                  m_type;
  std::filesystem::path const m_path;

  UniData(Type type, std::filesystem::path const& path): m_type(type), m_path(path) {}

  virtual ~UniData() = default;
};

struct FileData: public UniData {
  std::unique_ptr<std::fstream> const m_file;

  FileData(std::unique_ptr<std::fstream>&& file, std::filesystem::path const& path): UniData(UniData::Type::File, path), m_file(std::move(file)) {}

  virtual ~FileData() { m_file->sync(); }
};

struct DirData: public UniData {
  std::unique_ptr<std::filesystem::directory_iterator> const m_file;
  uint32_t                                                   count = 0;

  DirData(std::unique_ptr<std::filesystem::directory_iterator>&& dirIt, std::filesystem::path const& path)
      : UniData(UniData::Type::Dir, path), m_file(std::move(dirIt)) {}

  virtual ~DirData() = default;
};

namespace {

int insertItem(std::vector<std::unique_ptr<UniData>>& container, UniData* item) {
  int index = -1;
  for (size_t n = 0; n < container.size(); ++n) {
    if (!container[n]) {
      container[n] = std::unique_ptr<UniData>(item);
      index        = n;
      break;
    }
  }
  if (index < 0) {
    index = container.size();
    container.push_back(std::unique_ptr<UniData>(item));
  }
  return index + FILE_DESCRIPTOR_MIN;
}

} // namespace

class FileManager: public IFileManager {
  std::vector<std::unique_ptr<UniData>> m_openFiles;
  std::mutex                            m_mutext_int;

  /// First: mountpoint Second: RootDir
  std::unordered_map<MountType, std::unordered_map<std::string, std::filesystem::path>> m_mountPointList;
  std::filesystem::path                                                                 m_dirGameFiles;
  std::unordered_map<std::string, std::filesystem::path>                                m_mappedPaths;

  public:
  FileManager()          = default;
  virtual ~FileManager() = default;

  void addMountPoint(std::string_view mountPoint, std::filesystem::path const& root, MountType type) final {
    LOG_USE_MODULE(FileManager);
    std::unique_lock const lock(m_mutext_int);
    m_mountPointList[type][std::string(mountPoint)] = root;

    // Create root dir
    std::filesystem::create_directories(root);
    LOG_INFO(L"+ MountPoint: %S->%s", mountPoint.data(), root.c_str());
  }

  std::filesystem::path getMountPoint(MountType type, std::string mountPoint) final {
    std::unique_lock const lock(m_mutext_int);
    if (auto it = m_mountPointList[type].find(mountPoint); it != m_mountPointList[type].end()) return it->second;
    return {};
  }

  void clearMountPoint(MountType type, std::string mountPoint) final {
    LOG_USE_MODULE(FileManager);
    std::unique_lock const lock(m_mutext_int);
    LOG_INFO(L"- MountPoint for %S", magic_enum::enum_name(type).data());
    m_mountPointList[type].erase(mountPoint);
  }

  std::optional<std::filesystem::path> getMappedPath(std::string_view path) final {
    LOG_USE_MODULE(FileManager);
    std::unique_lock const lock(m_mutext_int);

    // Check Cache
    {
      auto it = m_mappedPaths.find(path.data());
      if (it != m_mappedPaths.end()) {
        return it->second;
      }
    }
    //-

    // Special case: Mounted Gamefiles
    if (path[0] != '/') {
      auto const mapped = m_dirGameFiles / path;
      LOG_INFO(L"Gamefiles mapped:%s", mapped.c_str());
      return mapped;
    }
    // -

    uint8_t offset = (path[0] == '/' && path[1] == '/') ? 1 : 0; // Path can start with //app0 ?

    for (auto const& mountType: m_mountPointList) {
      for (auto const& item: mountType.second) {
        auto const& mountPoint = item.first;
        auto const& rootDir    = item.second;

        if (path.size() >= mountPoint.size() && std::mismatch(mountPoint.begin(), mountPoint.end(), path.begin() + offset).first == mountPoint.end()) {
          if (path[mountPoint.size() + offset] == '/') ++offset; // path.substr() should return relative path
          auto const mapped = rootDir / path.substr(mountPoint.size() + offset);
          LOG_DEBUG(L"mapped: %s root:%s source:%S", mapped.c_str(), rootDir.c_str(), path.data());
          m_mappedPaths[path.data()] = mapped;
          return mapped;
        }
      }
    }

    LOG_WARN(L"Unknown map:%S", path.data());
    return {};
  }

  void setGameFilesDir(std::filesystem::path const& path) final {
    std::unique_lock const lock(m_mutext_int);
    m_dirGameFiles = path;
  }

  std::filesystem::path const& getGameFilesDir() const final { return m_dirGameFiles; }

  int addFileStream(std::unique_ptr<std::fstream>&& file, std::filesystem::path const& path) final {
    std::unique_lock const lock(m_mutext_int);

    return insertItem(m_openFiles, std::make_unique<FileData>(std::move(file), path).release());
  }

  int addDirIterator(std::unique_ptr<std::filesystem::directory_iterator>&& it, std::filesystem::path const& path) final {
    std::unique_lock const lock(m_mutext_int);

    return insertItem(m_openFiles, std::make_unique<DirData>(std::move(it), path).release());
  }

  void remove(int handle) final {
    if (handle < FILE_DESCRIPTOR_MIN) return;
    handle -= FILE_DESCRIPTOR_MIN;

    std::unique_lock const lock(m_mutext_int);
    m_openFiles[handle].reset();
  }

  std::fstream* getFile(int handle) final {
    if (handle < FILE_DESCRIPTOR_MIN) return nullptr;
    handle -= FILE_DESCRIPTOR_MIN;

    std::unique_lock const lock(m_mutext_int);
    if (handle < m_openFiles.size() && m_openFiles[handle] && m_openFiles[handle]->m_type == UniData::Type::File) {
      return static_cast<FileData*>(m_openFiles[handle].get())->m_file.get();
    }
    return nullptr;
  }

  std::filesystem::path getPath(int handle) final {
    if (handle < FILE_DESCRIPTOR_MIN) return {};
    handle -= FILE_DESCRIPTOR_MIN;
    std::unique_lock const lock(m_mutext_int);
    if (handle < m_openFiles.size() && m_openFiles[handle]) {
      return m_openFiles[handle]->m_path;
    }
    return {};
  }

  int getDents(int handle, char* buf, int nbytes, int64_t* basep) final {
    LOG_USE_MODULE(FileManager);

    if (handle < FILE_DESCRIPTOR_MIN) return -1;
    handle -= FILE_DESCRIPTOR_MIN;

    std::unique_lock const lock(m_mutext_int);

    auto& oFile = m_openFiles[handle];
    if (oFile->m_type != UniData::Type::Dir) return -1;

    auto dir    = static_cast<DirData*>(oFile.get());
    auto endDir = std::filesystem::directory_iterator();

    if ((*dir->m_file) == endDir) return 0;

    struct DataStruct {
      uint32_t fileno;
      uint16_t reclen;
      uint8_t  type;
      uint8_t  namlen;
      char     name[256];
    };

    auto count = dir->count;
    int  n     = 0;

    while (n < nbytes - sizeof(DataStruct)) {
      auto item = (DataStruct*)(buf + n);

      auto const filename = (*dir->m_file)->path().filename().string();
      if (sizeof(DataStruct) + std::min(filename.size(), 255llu) >= nbytes) break;

      item->fileno             = 0;
      item->type               = ((*dir->m_file)->is_regular_file() ? 8 : 4);
      item->namlen             = filename.copy(item->name, 255);
      item->name[item->namlen] = '\0';

      n += sizeof(DataStruct);
      item->reclen = sizeof(DataStruct);

      LOG_DEBUG(L"KernelGetdirentries[%d]: %S %u offset:%u count:%u", handle, item->name, item->type, item->reclen, count);

      std::error_code err;
      (*dir->m_file).increment(err);
      count = ++dir->count;

      if ((*dir->m_file) == endDir || err) break;
    };

    if (basep != nullptr) {
      *basep = count;
    }
    return n;
  }
};

IFileManager& accessFileManager() {
  static FileManager inst;
  return inst;
}