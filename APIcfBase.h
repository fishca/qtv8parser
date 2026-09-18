#ifndef APICFBASE_H
#define APICFBASE_H

#include <map>
#include <qobject.h>
#include <set>

#include "v8_constants.h"
#include "vcl_utils.h"

typedef QByteArray ByteArr;

/**
 * @brief The v8header_struct class
 */
struct v8header_struct
{
    __int64 time_create;
    __int64 time_modify;
    int zero;
    //wchar_t* name;
};

/**
 * @brief The fat_item class
 */
struct fat_item
{
    int header_start;
    int data_start;
    int ff; // всегда 7fffffff
};

/**
 * @brief The fat_item8316 class
 */
struct fat_item8316
{
    __int64 header_start;
    __int64 data_start;
    __int64 ff; // всегда 7fffffff
};

/**
 * @brief The catalog_header class
 */
struct catalog_header
{
    int start_empty; // начало первого пустого блока
    int page_size;   // размер страницы по умолчанию
    int version;     // версия
    int zero;        // всегда ноль?
};

/**
 * @brief The catalog_header8316 class
 */
struct catalog_header8316
{
    __int64 start_empty; // начало первого пустого блока
    int page_size;       // размер страницы по умолчанию
    int version;         // версия
    int zero;            // всегда ноль?
};

//===========================================================================
class v8catalog;
class TV8FileStream;

/**
 * @brief The FileIsCatalog enum
 */
enum FileIsCatalog
{
    iscatalog_unknown,
    iscatalog_true,
    iscatalog_false
};

/**
 * @brief The v8file class
 */
class v8file
{
private:
    friend v8catalog;
    friend TV8FileStream;

    QString name;

    __int64 time_create;
    __int64 time_modify;

    vcl_utils::TStream* data;
    v8catalog* parent;

    FileIsCatalog iscatalog;

    v8catalog* self; // указатель на каталог, если файл является каталогом

    v8file* next;      // следующий файл в каталоге
    v8file* previous;  // предыдущий файл в каталоге

    bool is_opened;    // признак открытого файла (инициализирован поток data)

    int start_data;   // начало блока данных файла в каталоге (0 означает, что файл в каталоге не записан)
    int start_header; // начало блока заголовка файла в каталоге

    bool is_datamodified;   // признак модифицированности данных файла (требуется запись в каталог при закрытии)
    bool is_headermodified; // признак модифицированности заголовка файла (требуется запись в каталог при закрытии)

    bool is_destructed; // признак, что работает деструктор
    bool flushed;       // признак, что происходит сброс
    //	bool readonly;
    bool selfzipped; // Признак, что файл является запакованным независимо от признака zipped каталога

    std::set<TV8FileStream*> streams;

public:
    v8file(v8catalog* _parent, const QString& _name, v8file* _previous, int _start_data, int _start_header, __int64* _time_create, __int64* _time_modify);

    ~v8file();

    bool IsCatalog();

    v8catalog* GetCatalog();

    int GetFileLength();
    __int64 GetFileLength64();

    int Read(void* Buffer, int Start, int Length);
    int Read(ByteArr& Buffer, int Start, int Length);


    // дозапись/перезапись частично
    int Write(const void* Buffer, int Start, int Length);
    // дозапись/перезапись частично
    int Write(ByteArr Buffer, int Start, int Length); //надо разобраться что такое ByteArr Buffer

    // перезапись целиком
    int Write(const void* Buffer, int Length);

    // дозапись/перезапись частично
    int Write(vcl_utils::TStream* Stream, int Start, int Length);

    // перезапись целиком
    //int Write(TStream* Stream);

    QString GetFileName();
    QString GetFullName();

    void SetFileName(const QString& _name);

    v8catalog* GetParentCatalog();

    void DeleteFile();

    v8file* GetNext();

    bool Open();
    void Close();

    // перезапись целиком и закрытие файла (для экономии памяти не используется data файла)
    //int WriteAndClose(TStream* Stream, int Length = -1);

    //void __fastcall GetTimeCreate(FILETIME* ft);
    //void __fastcall GetTimeModify(FILETIME* ft);
    //void __fastcall SetTimeCreate(FILETIME* ft);
    //void __fastcall SetTimeModify(FILETIME* ft);

    void SaveToFile(const QString& FileName);
    //void __fastcall SaveToStream(TStream* stream);
    //TStream* __fastcall get_data();
    void Flush();
};

/**
 * @brief The v8catalog class
 */
class v8catalog
{
private:
    friend v8file;

    v8file* file; // файл, которым является каталог. Для корневого каталога NULL

    vcl_utils::TStream* data; // поток каталога. Если file не NULL (каталог не корневой), совпадает с file->data
    vcl_utils::TStream* cfu;  // поток файла cfu. Существует только при is_cfu == true

    void initialize(int Offset = 0);


    v8file* first; // первый файл в каталоге
    v8file* last;  // последний файл в каталоге

    std::map<QString, v8file*> files; // Соответствие имен и файлов

    int     start_empty; // начало первого пустого блока
    __int64 start_empty8316;

    int page_size; // размер страницы по умолчанию
    int version;   // версия

    bool zipped; // признак зазипованности файлов каталога
    bool is_cfu; // признак файла cfu (файл запакован deflate'ом)
    bool iscatalog;
    bool iscatalogdefined;

    bool is_fatmodified;
    bool is_emptymodified;
    bool is_modified;

    bool is_8316;

    void free_block(int start);

    // возвращает адрес начала блока
    int write_block(vcl_utils::TStream* block, int start, bool use_page_size, int len = -1);

    // возвращает адрес начала блока
    int write_datablock(vcl_utils::TStream* block, int start, bool _zipped = false, int len = -1);

    vcl_utils::TStream* __fastcall read_datablock(int start, int offset = 0);

    int get_nextblock(int start);

    bool is_destructed; // признак, что работает деструктор
    bool flushed;       // признак, что происходит сброс
    bool leave_data;    // признак, что не нужно удалять основной поток (data) при уничтожении объекта

public:
    //	bool readonly;
    v8catalog(v8file* f);   // создать каталог из файла
    v8catalog(QString name); // создать каталог из физического файла (cf, epf, erf, hbk, cfu)
    v8catalog(QString name, bool _zipped); // создать каталог из физического файла (cf, epf, erf, hbk, cfu)
    v8catalog(vcl_utils::TStream* stream, bool _zipped, bool leave_stream = false); // создать каталог из потока

    ~v8catalog();

    bool IsCatalog();
    bool Is8316();
    v8file* GetFile(const QString& FileName);
    v8file* GetFirst();

    // CreateFile в win64 определяется как CreateFileW, пришлось заменить на маленькую букву
    v8file* createFile(const QString& FileName, bool _selfzipped = false);
    v8catalog* CreateCatalog(const QString& FileName, bool _selfzipped = false);
    void DeleteFile(const QString& FileName);
    v8catalog* GetParentCatalog();
    //void __fastcall Defrag(bool Recursively);
    v8file* GetSelfFile();
    void SaveToDir(QString DirName);
    bool isOpen();
    void Flush();
    void HalfClose();
    void HalfOpen(const QString& name);
    //void __fastcall set_leave_data(bool ld);
    void ClearIs8316();
};

/**
 * @brief The TV8FileStream class
 */
class TV8FileStream : public vcl_utils::TStream
{
protected:
    v8file* file;
    bool own;
    __int64 pos;
public:
    TV8FileStream(v8file* f, bool ownfile = false);
    virtual ~TV8FileStream();

    virtual int Read(void *Buffer, int Count);
    //virtual int Read(ByteArr Buffer, int Offset, int Count);

    virtual int Write(const void *Buffer, int Count);
    //virtual int Write(const ByteArr Buffer, int Offset, int Count);

    virtual int     Seek(int Offset, int Origin);
    //virtual __int64 Seek(const __int64 Offset, TSeekOrigin Origin);
};


#endif // APICFBASE_H
