#ifndef VCL_UTILS_H
#define VCL_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <QByteArray>



namespace vcl_utils {

// Замена AnsiString - используем std::string
using AnsiString = std::string;
using WideString = std::wstring;
using String = std::wstring;

typedef QByteArray ByteArr;

// Вспомогательные функции для работы с wxString (wxWidgets)
#ifdef WXWINDOWS
#include <wx/wx.h>
inline wxString wxStringFromStd(const std::wstring& s) {
    return wxString(s.c_str());
}
inline std::wstring wxStringToStd(const wxString& s) {
    return s.ToStdWstring();
}
#endif

inline std::filesystem::path to_fs_path(const std::wstring& s) {
    return std::filesystem::path(s);
}


// Замена TStringList
class TStringListLike {
public:
    std::vector<std::wstring> lines;
    bool Sorted = false;
    bool CaseSensitive = false;
    
    enum TDuplicates { dupIgnore, dupAccept, dupError };
    TDuplicates Duplicates = dupIgnore;
    
    void Clear() {
        lines.clear();
    }
    
    int Count() const {
        return static_cast<int>(lines.size());
    }
    
    void Add(const std::wstring& line) {
        if (Sorted) {
            auto it = std::lower_bound(lines.begin(), lines.end(), line, 
                [this](const std::wstring& a, const std::wstring& b) {
                    if (CaseSensitive) return a < b;
                    return std::lexicographical_compare(
                        a.begin(), a.end(), b.begin(), b.end(),
                        [](wchar_t c1, wchar_t c2) { return towlower(c1) < towlower(c2); });
                });
            if (Duplicates == dupIgnore && it != lines.end()) {
                bool equal = CaseSensitive ? (*it == line) : 
                    (std::equal(it->begin(), it->end(), line.begin(), line.end(),
                        [](wchar_t c1, wchar_t c2) { return towlower(c1) == towlower(c2); }));
                if (equal) return;
            }
            lines.insert(it, line);
        } else {
            lines.push_back(line);
        }
    }
    
    void Add(const std::string& line) {
        Add(std::wstring(line.begin(), line.end()));
    }
    
    std::wstring& operator[](int index) {
        return lines[index];
    }
    
    const std::wstring& operator[](int index) const {
        return lines[index];
    }
    
    int IndexOf(const std::wstring& value) const {
        for (size_t i = 0; i < lines.size(); ++i) {
            bool match = CaseSensitive ? (lines[i] == value) :
                (std::equal(lines[i].begin(), lines[i].end(), value.begin(), value.end(),
                    [](wchar_t c1, wchar_t c2) { return towlower(c1) == towlower(c2); }));
            if (match) return static_cast<int>(i);
        }
        return -1;
    }
    
    void LoadFromFile(const std::wstring& filename) {
        lines.clear();
        // std::wifstream file(filename);
        std::wifstream file{std::filesystem::path(filename)};

        if (file.is_open()) {
            std::wstring line;
            while (std::getline(file, line)) {
                // Удаляем CR если есть
                if (!line.empty() && line.back() == L'\r')
                    line.pop_back();
                lines.push_back(line);
            }
        }
    }
    
    void SaveToFile(const std::wstring& filename) const {
        //std::wofstream file(filename);
        std::wofstream file{std::filesystem::path(filename)};
        if (file.is_open()) {
            for (const auto& line : lines) {
                file << line << L"\n";
            }
        }
    }
    
    // Добавляем методы для совместимости
    bool GetText(std::wstring& text) const {
        text.clear();
        for (size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) text += L"\r\n";
            text += lines[i];
        }
        return true;
    }
    
    void SetText(const std::wstring& text) {
        lines.clear();
        size_t start = 0;
        while (start < text.length()) {
            size_t end = text.find(L'\n', start);
            if (end == std::wstring::npos) end = text.length();
            std::wstring line = text.substr(start, end - start);
            if (!line.empty() && line.back() == L'\r')
                line.pop_back();
            lines.push_back(line);
            start = end + 1;
        }
    }
};

// Абстрактный базовый класс для потоков (замена TStream)
class TStream {
public:
    virtual ~TStream() = default;
    virtual int64_t Read(void* Buffer, int64_t Count) = 0;
    virtual int64_t Read(ByteArr& Buffer, int64_t Count) = 0;


    virtual int64_t Write(const void* Buffer, int64_t Count) = 0;
    virtual int64_t Seek(int64_t Offset, int Origin) = 0;
    virtual int64_t GetSize() const = 0;
    virtual void SetSize(int64_t NewSize) = 0;
    
    // Добавленный метод WriteBuffer (невиртуальный)
    void WriteBuffer(const void* Buffer, int64_t Count) {
        if (Write(Buffer, Count) != Count) {
            throw std::runtime_error("TStream::WriteBuffer: failed to write all bytes");
        }
    }

    void ReadBuffer(void* Buffer, int64_t Count) {
        if (Read(Buffer, Count) != Count) {
            throw std::runtime_error("TStream::ReadBuffer: failed to read all bytes");
        }
    }

    int64_t Position() const {
        return const_cast<TStream*>(this)->Seek(0, 1); // fmCurrent
    }
    
    void Position(int64_t pos) {
        Seek(pos, 0); // fmFromBeginning
    }
    
    // Helper methods
    void CopyFrom(TStream* Source, int64_t Count) {
        const size_t bufferSize = 8192;
        std::vector<uint8_t> buffer(bufferSize);
        int64_t remaining = Count;
        while (remaining > 0) {
            int64_t toRead = std::min(remaining, static_cast<int64_t>(bufferSize));
            int64_t read = Source->Read(buffer.data(), toRead);
            if (read == 0) break;
            Write(buffer.data(), read);
            remaining -= read;
        }
    }
    
    void LoadFromStream(TStream* Source) {
        Position(0);
        SetSize(Source->GetSize());
        Source->Position(0);
        CopyFrom(Source, Source->GetSize());
    }
};

// Потоковый буфер в памяти (замена TMemoryStream)
class TMemoryStream : public TStream {
private:
    std::vector<uint8_t> data;
    size_t position = 0;
    
public:
    TMemoryStream() = default;
    
    int64_t Read(void* Buffer, int64_t Count) override {
        size_t available = data.size() - position;
        size_t toRead = std::min(static_cast<size_t>(Count), available);
        if (toRead > 0) {
            std::memcpy(Buffer, data.data() + position, toRead);
            position += toRead;
        }
        return static_cast<int64_t>(toRead);
    }
    int64_t Read(ByteArr& Buffer, int64_t Count) override {
        size_t available = data.size() - position;
        size_t toRead = std::min(static_cast<size_t>(Count), available);
        if (toRead > 0) {
            std::memcpy(Buffer.data(), data.data() + position, toRead);
            position += toRead;
        }
        return static_cast<int64_t>(toRead);
    }

    int64_t Write(const void* Buffer, int64_t Count) override {
        size_t newPos = position + Count;
        if (newPos > data.size()) {
            data.resize(newPos);
        }
        std::memcpy(data.data() + position, Buffer, Count);
        position = newPos;
        return Count;
    }
    
    // Добавленный метод WriteBuffer
    void WriteBuffer(const void* Buffer, int64_t Count) {
        if (Write(Buffer, Count) != Count) {
            throw std::runtime_error("WriteBuffer: failed to write all bytes");
        }
    }

    int64_t Seek(int64_t Offset, int Origin) override {
        switch (Origin) {
            case 0: // fmFromBeginning
                position = static_cast<size_t>(Offset);
                break;
            case 1: // fmCurrent
                position = static_cast<size_t>(static_cast<int64_t>(position) + Offset);
                break;
            case 2: // fmFromEnd
                position = static_cast<size_t>(static_cast<int64_t>(data.size()) + Offset);
                break;
        }
        if (position > data.size()) position = data.size();
        return static_cast<int64_t>(position);
    }
    
    int64_t GetSize() const override {
        return static_cast<int64_t>(data.size());
    }
    
    void SetSize(int64_t NewSize) override {
        data.resize(static_cast<size_t>(NewSize));
        if (position > data.size()) position = data.size();
    }
    
    // Дополнительные методы
    uint8_t* GetBuffer() { return data.data(); }
    const uint8_t* GetBuffer() const { return data.data(); }
    size_t Capacity() const { return data.capacity(); }
    void Clear() { data.clear(); position = 0; }
    
    bool SaveToFile(const std::wstring& filename) {
        std::ofstream file{to_fs_path(filename), std::ios::binary};
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
    
    bool LoadFromFile(const std::wstring& filename) {
        std::ifstream file{to_fs_path(filename), std::ios::binary | std::ios::ate};
        if (!file) return false;
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        position = 0;
        return true;
    }
};

// Файловый поток (замена TFileStream)
class TFileStream : public TStream {
private:
    std::fstream file;
    std::wstring filename;
    
public:
    enum FileMode {
        fmCreate = 0,
        fmOpenRead = 1,
        fmOpenWrite = 2,
        fmOpenReadWrite = 3,
        fmShareDenyNone = 0x00,
        fmShareDenyRead = 0x10,
        fmShareDenyWrite = 0x20,
        fmShareExclusive = 0x30
    };
    
    TFileStream(const std::wstring& fname, int mode) : filename(fname) {
        std::ios::openmode ios_mode = std::ios::binary;
        
        if (mode & fmCreate) {
            ios_mode |= std::ios::out | std::ios::trunc;
        } else if ((mode & 0x0F) == fmOpenRead) {
            ios_mode |= std::ios::in;
        } else if ((mode & 0x0F) == fmOpenWrite) {
            ios_mode |= std::ios::out;
        } else if ((mode & 0x0F) == fmOpenReadWrite) {
            ios_mode |= std::ios::in | std::ios::out;
        }
        
        // std::ifstream file{to_fs_path(filename), std::ios::binary | std::ios::ate};
        file.open(to_fs_path(fname), ios_mode);
        if (!file.is_open() && (mode & fmCreate)) {
            // Попытка создать файл
            file.open(to_fs_path(fname), std::ios::binary | std::ios::out | std::ios::trunc);
        }
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + std::string(fname.begin(), fname.end()));
        }
    }
    
    ~TFileStream() override {
        if (file.is_open()) file.close();
    }
    
    int64_t Read(void* Buffer, int64_t Count) override {
        if (!file.is_open()) return 0;
        file.read(reinterpret_cast<char*>(Buffer), Count);
        return file.gcount();
    }

    /*
    int64_t Read(ByteArr Buffer, int64_t Count) override {
        if (!file.is_open()) return 0;
        file.read(reinterpret_cast<char*>(Buffer), Count);
        return file.gcount();
    }
    */


    int64_t Read(ByteArr& Buffer, int64_t Count) override {
        if (!file.is_open())
            return 0;

        // На случай, если буфер меньше, чем надо прочитать
        if (Buffer.size() < Count)
            Buffer.resize(Count);

        file.read(Buffer.data(), Count);
        return file.gcount();
    }

    
    int64_t Write(const void* Buffer, int64_t Count) override {
        if (!file.is_open()) return 0;
        file.write(reinterpret_cast<const char*>(Buffer), Count);
        return Count;
    }
    
    int64_t Seek(int64_t Offset, int Origin) override {
        if (!file.is_open()) return 0;
        std::ios::seekdir dir;
        switch (Origin) {
            case 0: dir = std::ios::beg; break;
            case 1: dir = std::ios::cur; break;
            case 2: dir = std::ios::end; break;
            default: dir = std::ios::beg;
        }
        file.seekg(Offset, dir);
        file.seekp(Offset, dir);
        return file.tellg();
    }
    
    /*
    int64_t GetSize() const override {
        if (!file.is_open())
            return 0;
         auto pos = file.tellg();
         auto f = const_cast<std::fstream*>(&file);
         f->seekg(0, std::ios::end);
         auto size = f->tellg();
         f->seekg(pos);
         return size;
        //return 0;
    }
    */

    int64_t GetSize() const override {
        if (!file.is_open())
            return 0;

        std::error_code ec;
        auto size = std::filesystem::file_size(filename, ec);
        if (ec)
            return 0;

        return static_cast<int64_t>(size);
    }
    
    void SetSize(int64_t NewSize) override {
        if (!file.is_open()) return;
        auto pos = file.tellp();
        file.seekp(0, std::ios::end);
        if (GetSize() > NewSize) {
            // Усечение не поддерживается напрямую в standard C++,
            // оставляем как есть или можно использовать platform-specific
        } else {
            // Расширение
            for (int64_t i = GetSize(); i < NewSize; ++i) {
                file.put(0);
            }
        }
        file.seekp(pos);
    }
};

// Функции для работы с путями (замена SysUtils)
inline std::wstring ExtractFileDir(const std::wstring& path) {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            return p.parent_path().wstring();
        }
        return L"";
    } catch (...) {
        return L"";
    }
}

inline std::string ExtractFileDir(const std::string& path) {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            return p.parent_path().string();
        }
        return "";
    } catch (...) {
        return "";
    }
}

inline std::wstring ExtractFileName(const std::wstring& path) {
    try {
        std::filesystem::path p(path);
        return p.filename().wstring();
    } catch (...) {
        return L"";
    }
}

inline std::string ExtractFileName(const std::string& path) {
    try {
        std::filesystem::path p(path);
        return p.filename().string();
    } catch (...) {
        return "";
    }
}

inline std::wstring ChangeFileExt(const std::wstring& path, const std::wstring& ext) {
    try {
        std::filesystem::path p(path);
        return (p.replace_extension(ext)).wstring();
    } catch (...) {
        return path;
    }
}

inline std::string ChangeFileExt(const std::string& path, const std::string& ext) {
    try {
        std::filesystem::path p(path);
        return (p.replace_extension(ext)).string();
    } catch (...) {
        return path;
    }
}

inline std::wstring IncludeTrailingPathDelimiter(const std::wstring& path) {
    if (path.empty()) return L"\\";
    wchar_t lastChar = path.back();
    if (lastChar == L'\\' || lastChar == L'/') {
        return path;
    }
    return path + L"\\";
}

inline std::string IncludeTrailingPathDelimiter(const std::string& path) {
    if (path.empty()) return "\\";
    char lastChar = path.back();
    if (lastChar == '\\' || lastChar == '/') {
        return path;
    }
    return path + "\\";
}

inline std::wstring ExcludeTrailingPathDelimiter(const std::wstring& path) {
    if (path.empty()) return path;
    wchar_t lastChar = path.back();
    if (lastChar == L'\\' || lastChar == L'/') {
        return path.substr(0, path.length() - 1);
    }
    return path;
}

inline std::wstring ExtractFilePath(const std::wstring& path) {
    return ExtractFileDir(path);
}

inline std::wstring ExtractFileExt(const std::wstring& path) {
    try {
        std::filesystem::path p(path);
        return p.extension().wstring();
    } catch (...) {
        return L"";
    }
}

inline bool FileExists(const std::wstring& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

inline bool DirectoryExists(const std::wstring& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (...) {
        return false;
    }
}

inline bool ForceDirectories(const std::wstring& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (...) {
        return false;
    }
}

inline std::wstring GetCurrentDir() {
    try {
        return std::filesystem::current_path().wstring();
    } catch (...) {
        return L".";
    }
}

inline void SetCurrentDir(const std::wstring& path) {
    try {
        std::filesystem::current_path(path);
    } catch (...) {
        // ignore
    }
}

inline std::wstring ExpandFileName(const std::wstring& path) {
    try {
        return std::filesystem::absolute(path).wstring();
    } catch (...) {
        return path;
    }
}

inline std::wstring IncludeLeadingPathDelimiter(const std::wstring& path) {
    if (path.empty()) return L"\\";
    if (path[0] == L'\\' || path[0] == L'/') return path;
    return L"\\" + path;
}

// Преобразования типов (замена IntToStr, StrToInt)
inline std::wstring IntToStr(int value) {
    return std::to_wstring(value);
}

inline std::string IntToStrA(int value) {
    return std::to_string(value);
}

inline int StrToInt(const std::wstring& str) {
    return std::stoi(str);
}

inline int StrToInt(const std::string& str) {
    return std::stoi(str);
}

inline std::wstring FloatToStr(double value) {
    std::wostringstream oss;
    oss.precision(15);
    oss << value;
    return oss.str();
}

inline double StrToFloat(const std::wstring& str) {
    return std::stod(str);
}

// Trim функции
inline std::wstring Trim(const std::wstring& str) {
    size_t first = str.find_first_not_of(L" \t\n\r");
    if (first == std::wstring::npos) return L"";
    size_t last = str.find_last_not_of(L" \t\n\r");
    return str.substr(first, last - first + 1);
}

inline std::wstring TrimLeft(const std::wstring& str) {
    size_t first = str.find_first_not_of(L" \t\n\r");
    if (first == std::wstring::npos) return L"";
    return str.substr(first);
}

inline std::wstring TrimRight(const std::wstring& str) {
    size_t last = str.find_last_not_of(L" \t\n\r");
    if (last == std::wstring::npos) return L"";
    return str.substr(0, last + 1);
}

inline std::wstring LowerCase(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), towlower);
    return result;
}

inline std::wstring UpperCase(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), towupper);
    return result;
}

// Вспомогательные функции для работы с потоками
template<typename T>
std::vector<uint8_t> stream_to_vector(T& stream) {
    std::vector<uint8_t> buffer;
    stream.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, std::ios::beg);
    buffer.resize(size);
    if (size > 0) {
        stream.read(reinterpret_cast<char*>(buffer.data()), size);
    }
    return buffer;
}

template<typename T>
void vector_to_stream(const std::vector<uint8_t>& data, T& stream) {
    stream.write(reinterpret_cast<const char*>(data.data()), data.size());
}

} // namespace vcl_utils

#endif // VCL_UTILS_H
