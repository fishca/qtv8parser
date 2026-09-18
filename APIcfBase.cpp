#include "APIcfBase.h"
#include <QFileInfo>
#include <QString>
#include <qdir.h>
#include "UZLib.h"


// массив для преобразования числа в шестнадцатиричную строку
const char _bufhex[] = "0123456789abcdef";

// шаблон заголовка блока
const char _block_header_template[]    = "\r\n00000000 00000000 00000000 \r\n";
const unsigned char _empty_catalog_template[16] = {0xff,0xff,0xff,0x7f,0,2,0,0,0,0,0,0,0,0,0,0};
const unsigned char _empty_catalog_template8316[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

#ifdef __cplusplus
int max(int value1, int value2)
{
    return ( (value1 > value2) ? value1 : value2);
}

int min(int value1, int value2)
{
    return ( (value1 < value2) ? value1 : value2);
}
#endif

//===========================================================================
// преобразует шестнадцатиричную восьмисимвольную строку в число
int hex_to_int(char* hexstr)
{
    int res = 0;
    int sym;
    for(int i = 0; i < 8; i++)
    {
        sym = hexstr[i];
        if(sym >= 'a')
            sym -= 'a' - '9' - 1;
        else if(sym > '9')
            sym -= 'A' - '9' - 1;
        sym -= '0';
        res = (res << 4) | (sym & 0xf);
    }
    return res;
}

//===========================================================================
//
int hex_to_int16(char* hexstr)
{
    int res = 0;
    int sym;
    for(int i = 0; i < 16; i++)
    {
        sym = hexstr[i];
        if(sym >= 'a')
            sym -= 'a' - '9' - 1;
        else if(sym > '9')
            sym -= 'A' - '9' - 1;
        sym -= '0';
        res = (res << 4) | (sym & 0xf);
    }
    return res;
}

//===========================================================================
// преобразует число в шестнадцатиричную восьмисимвольную строку
char* int_to_hex(char* hexstr, int dec)
{
    int _t1 = dec;
    int _t2;
    for(int i = 7; i >= 0; i--)
    {
        _t2 = _t1 & 0xf;
        hexstr[i] = _bufhex[_t2];
        _t1 >>= 4;
    }
    return hexstr;
}

//===========================================================================
// читает блок из потока каталога stream_from, собирая его по страницам
vcl_utils::TStream* read_block(vcl_utils::TStream* stream_from, int start, vcl_utils::TStream* stream_to = NULL)
{
    char temp_buf[32];
    int len, curlen, pos, readlen;

    int soFromBeginning = 0;

    if(!stream_to)
        stream_to = new vcl_utils::TMemoryStream;

    stream_to->Seek(0, soFromBeginning);
    stream_to->SetSize(0);

    if(start < 0 || start == V8_FF_SIGNATURE || start > stream_from->GetSize())
        return stream_to;

    stream_from->Seek(start, soFromBeginning);
    stream_from->Read(temp_buf, 31);

    len = hex_to_int(&temp_buf[2]);
    if(!len)
        return stream_to;
    curlen = hex_to_int(&temp_buf[11]);
    start  = hex_to_int(&temp_buf[20]);

    readlen = min(len, curlen);
    stream_to->CopyFrom(stream_from, readlen);

    pos = readlen;

    while(start != V8_FF_SIGNATURE)
    {
        stream_from->Seek(start, soFromBeginning);
        stream_from->Read(temp_buf, 31);

        curlen = hex_to_int(&temp_buf[11]);
        start = hex_to_int(&temp_buf[20]);

        readlen = min(len - pos, curlen);
        stream_to->CopyFrom(stream_from, readlen);
        pos += readlen;
    }

    return stream_to;
}


//===========================================================================
// читает блок из потока каталога stream_from, собирая его по страницам
vcl_utils::TStream* read_block_16(vcl_utils::TStream* stream_from, __int64 start, vcl_utils::TStream* stream_to = NULL)
{
    char temp_buf[55];
    int len, curlen, pos, readlen;

    int soFromBeginning = 0;

    if(!stream_to)
        stream_to = new vcl_utils::TMemoryStream;

    stream_to->Seek(0, soFromBeginning);
    stream_to->SetSize(0);

    if(start < 0 || start == V8_FF64_SIGNATURE || start > stream_from->GetSize())
        return stream_to;

    stream_from->Seek(start, soFromBeginning);
    stream_from->Read(temp_buf, 55);

    len = hex_to_int16(&temp_buf[2]);
    if(!len)
        return stream_to;
    curlen = hex_to_int16(&temp_buf[19]);
    start = hex_to_int16(&temp_buf[36]);

    readlen = min(len, curlen);
    stream_to->CopyFrom(stream_from, readlen);

    pos = readlen;

    while(start != V8_FF64_SIGNATURE)
    {
        stream_from->Seek(start + V8_OFFSET_8316, soFromBeginning);
        stream_from->Read(temp_buf, 55);

        curlen = hex_to_int16(&temp_buf[19]);
        start  = hex_to_int16(&temp_buf[36]);

        readlen = min(len - pos, curlen);
        stream_to->CopyFrom(stream_from, readlen);
        pos += readlen;
    }

    return stream_to;
}



//===========================================================================
v8file::v8file(v8catalog* _parent, const QString& _name, v8file* _previous, int _start_data, int _start_header, __int64* _time_create, __int64* _time_modify)
{
    is_destructed = false;
    flushed = false;
    parent = _parent;
    name = _name;
    previous = _previous;
    next = NULL;
    //data = NULL;
    start_data = _start_data;
    start_header = _start_header;
    is_datamodified = !start_data;
    is_headermodified = !start_header;
    if(previous)
        previous->next = this;
    else
        parent->first = this;
    iscatalog = iscatalog_unknown;
    self = NULL;
    is_opened = false;
    time_create = *_time_create;
    time_modify = *_time_modify;
    selfzipped = false;
    if(parent)
        parent->files[name.toUpper()] = this;
}

//===========================================================================
QString v8file::GetFileName()
{
    return name;
}

//===========================================================================
QString v8file::GetFullName()
{
    if(parent) if(parent->file)
        {
            QString fulln = parent->file->GetFullName();
            if(!fulln.isEmpty())
            {
                fulln += "\\";
                fulln += name;

                return fulln;
            }
        }
    return name;
}

//===========================================================================
void v8file::SetFileName(const QString& _name)
{
    name = _name;
    is_headermodified = true;
}

//---------------------------------------------------------------------------
v8catalog* v8file::GetCatalog()
{
    v8catalog* ret;

    if(IsCatalog())
    {
        if(!self)
        {
            self = new v8catalog(this);
        }
        ret = self;
    }
    else
        ret = NULL;

    return ret;
}

//===========================================================================
v8catalog* v8file::GetParentCatalog()
{
    return parent;
}

//===========================================================================
v8file* v8file::GetNext()
{
    return next;
}

//===========================================================================
bool v8file::Open()
{
    if(!parent)
        return false;


    if(is_opened)
    {
        return true;
    }

    if (parent->is_8316)
    {
        data = parent->read_datablock(start_data, V8_OFFSET_8316);
    }
    else
    {
        data = parent->read_datablock(start_data);
    }

    is_opened = true;

    return true;
}

//===========================================================================
bool v8file::IsCatalog()
{
    int _filelen;
    int _startempty = -1;
    char _t[32];

    int soFromBeginning = 0;

    if(iscatalog == iscatalog_unknown)
    {
        // эмпирический метод?
        if(!is_opened)
            if(!Open())
            {
                return false;
            }
        //_filelen = data->Size;
        _filelen = 0;
        if(_filelen == 16)
        {
            data->Seek(0, soFromBeginning);
            data->Read(_t, 16);
            if(memcmp(_t, _empty_catalog_template, 16) != 0)
            {
                iscatalog = iscatalog_false;

                return false;
            }
            else
            {
                iscatalog = iscatalog_true;

                return true;
            }
        }

        data->Seek(0, soFromBeginning);
        data->Read(&_startempty, 4);
        if(_startempty != V8_FF_SIGNATURE)
        {
            if(_startempty + 31 >= _filelen)
            {
                iscatalog = iscatalog_false;

                return false;
            }

            data->Seek(_startempty, soFromBeginning);
            data->Read(_t, 31);

            if(_t[0] != 0xd || _t[1] != 0xa || _t[10] != 0x20 || _t[19] != 0x20 || _t[28] != 0x20 || _t[29] != 0xd || _t[30] != 0xa)
            {
                iscatalog = iscatalog_false;

                return false;
            }
        }
        if(_filelen < 31 + 16)
        {
            iscatalog = iscatalog_false;

            return false;
        }
        data->Seek(16, soFromBeginning);
        data->Read(_t, 31);
        if(_t[0] != 0xd || _t[1] != 0xa || _t[10] != 0x20 || _t[19] != 0x20 || _t[28] != 0x20 || _t[29] != 0xd || _t[30] != 0xa)
        {
            iscatalog = iscatalog_false;

            return false;
        }
        iscatalog = iscatalog_true;

        return true;
    }

    return iscatalog == iscatalog_true;
}

bool v8catalog::Is8316()
{
    char _temp_data[8] = "";
    /*
     0: // fmFromBeginning
     1: // fmCurrent
     2: // fmFromEnd
    */
    int soFromBeginning = 0;
    data->Seek(V8_OFFSET_8316, soFromBeginning);
    data->Read(_temp_data, 8);
    if(memcmp(_temp_data, _empty_catalog_template8316, 8) != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}




//===========================================================================
// создать каталог из файла
v8catalog::v8catalog(v8file* f)
{
    is_cfu = false;
    iscatalogdefined = false;
    file = f;
    file->Open();
    data = file->data;
    zipped = false;

    if(IsCatalog())
        initialize();
    else
    {
        first = NULL;
        last = NULL;
        start_empty = 0;
        page_size = 0;
        version = 0;
        zipped = false;

        is_fatmodified = false;
        is_emptymodified = false;
        is_modified = false;
        is_destructed = false;
        flushed = false;
        leave_data = false;
    }
}

//===========================================================================
// создать каталог из физического файла .cf
v8catalog::v8catalog(QString name)
{
    int fmCreate = 0;
    int fmOpenReadWrite = 3;
    int fmShareDenyNone = 0;

    iscatalogdefined = false;

    QFileInfo fileInfo(name);

    QString ext = fileInfo.suffix().toLower(); // ext == "gz"

    //QString ext = ExtractFileExt(name).LowerCase();
    if(ext == ".cfu")
    {
        is_cfu = true;
        zipped = false;
        data = new vcl_utils::TMemoryStream();

        if(!vcl_utils::FileExists(name.toStdWString()))
        {
            data->WriteBuffer(_empty_catalog_template, 16);
            cfu = new vcl_utils::TFileStream(name.toStdWString(), fmCreate);
        }
        else
        {
            cfu = new vcl_utils::TFileStream(name.toStdWString(), fmOpenReadWrite | fmShareDenyNone);
            // TODO: Нужна реализация
            //ZInflateStream(cfu, data);
        }
    }
    else
    {
        zipped = ext == ".cf" || ext == ".epf" || ext == ".erf" || ext == ".cfe";
        is_cfu = false;

        if(!vcl_utils::FileExists(name.toStdWString()))
        {
            data = new vcl_utils::TFileStream(name.toStdWString(), fmCreate);
            data->WriteBuffer(_empty_catalog_template, 16);
            delete data;
        }
        data = new vcl_utils::TFileStream(name.toStdWString(), fmOpenReadWrite | fmShareDenyNone);
    }

    file = NULL;
    if(IsCatalog())
        initialize();
    else
    {
        first = NULL;
        last = NULL;
        start_empty = 0;
        page_size = 0;
        version = 0;
        zipped = false;

        is_fatmodified = false;
        is_emptymodified = false;
        is_modified = false;
        is_destructed = false;
        flushed = false;
        leave_data = false;
    }
}

//===========================================================================
// создать каталог из физического файла
v8catalog::v8catalog(QString name, bool _zipped)
{
    iscatalogdefined = false;
    is_cfu = false;
    zipped = _zipped;
    int fmCreate = 0;
    int fmOpenReadWrite = 3;
    int fmShareDenyNone = 0;

    if(!vcl_utils::FileExists(name.toStdWString()))
    {
        data = new vcl_utils::TFileStream(name.toStdWString(), fmCreate);
        data->WriteBuffer(_empty_catalog_template, 16);
        delete data;
    }

    data = new vcl_utils::TFileStream(name.toStdWString(), fmOpenReadWrite | fmShareDenyNone);

    file = NULL;

    is_8316 = Is8316();

    int curOffset = is_8316 ? V8_OFFSET_8316 : 0;

    if(IsCatalog())
    {
        initialize(curOffset);
    }
    else
    {
        first = NULL;
        last = NULL;
        start_empty = 0;
        page_size   = 0;
        version     = 0;
        zipped = false;

        is_fatmodified   = false;
        is_emptymodified = false;
        is_modified      = false;
        is_destructed    = false;
        flushed          = false;
        leave_data       = false;
    }
}

//===========================================================================
// создать каталог из потока
v8catalog::v8catalog(vcl_utils::TStream* stream, bool _zipped, bool leave_stream)
{
    is_cfu = false;
    iscatalogdefined = false;
    zipped = _zipped;
    // data = new vcl_utils::TMemoryStream;
    // data->CopyFrom(stream, 0);
    data = stream;
    file = NULL;
    if(!data->GetSize())
        data->WriteBuffer(_empty_catalog_template, 16);
    if(IsCatalog())
        initialize();
    else
    {
        first = NULL;
        last = NULL;
        start_empty = 0;
        page_size = 0;
        version = 0;
        zipped = false;

        is_fatmodified = false;
        is_emptymodified = false;
        is_modified = false;
        is_destructed = false;
        flushed = false;
    }
    leave_data = leave_stream;
}


//===========================================================================
void v8catalog::initialize(int Offset)
{
    is_destructed = false;
    catalog_header     _ch;
    catalog_header8316 _ch8316;

    QString _name;

    fat_item     _fi;
    fat_item8316 _fi8316;

    char* _temp_buf;

    vcl_utils::TMemoryStream* _file_header;
    vcl_utils::TStream* _fat;

    v8file* _prev;
    v8file* _file;
    v8file* f;

    int _temp = 0;
    int _countfiles = 0;
    int HeaderSize  = 0;
    int dataStart   = 0;
    int headerStart = 0;

    int soFromBeginning = 0;

    data->Seek(Offset, soFromBeginning);
    if (Offset)
    {
        HeaderSize = 20;
        data->ReadBuffer(&_ch8316, HeaderSize);
        start_empty8316 = _ch8316.start_empty;
        page_size = _ch8316.page_size;
        version = _ch8316.version;
    }
    else
    {
        HeaderSize = 16;
        data->ReadBuffer(&_ch, HeaderSize);
        start_empty = _ch.start_empty;
        page_size = _ch.page_size;
        version = _ch.version;
    }

    first = NULL;
    _prev = NULL;

    _file_header = new vcl_utils::TMemoryStream;

    try
    {
        if(data->GetSize() > HeaderSize)
        {
            if (Offset)
            {
                _fat = read_block_16(data, HeaderSize + Offset);
                _fat->Seek(0, soFromBeginning);
                _countfiles = _fat->GetSize() / 24;
            }
            else
            {
                _fat = read_block(data, HeaderSize);
                _fat->Seek(0, soFromBeginning);
                _countfiles = _fat->GetSize() / 12;
            }

            for(int i = 0; i < _countfiles; i++)
            {
                if (Offset)
                {
                    _fat->Read(&_fi8316, 24);
                    read_block_16(data, _fi8316.header_start + Offset, _file_header);
                }
                else
                {
                    _fat->Read(&_fi, 12);
                    read_block(data, _fi.header_start, _file_header);
                }

                _file_header->Seek(0, soFromBeginning);

                _temp_buf = new char[_file_header->GetSize()];
                _file_header->Read(_temp_buf, _file_header->GetSize());
                //_name = (wchar_t*)(_temp_buf + 20);
                _name = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(_temp_buf + 20));

                dataStart   = Offset ? _fi8316.data_start   + Offset : _fi.data_start;
                headerStart = Offset ? _fi8316.header_start + Offset : _fi.header_start;

                _file = new v8file(this, _name, _prev, dataStart, headerStart, (__int64*)_temp_buf, (__int64*)(_temp_buf + 8));

                delete[] _temp_buf;

                if(!_prev)
                    first = _file;
                _prev = _file;
            }
            delete _file_header;
            delete _fat;
        }
    }
    catch(...)
    {
        f = first;
        while(f)
        {
            //		f->readonly = readonly;
            f->Close();
            f = f->next;
        }

        while(first)
            delete first;

        iscatalog = false;
        iscatalogdefined = true;

        first = NULL;
        last = NULL;
        start_empty = 0;
        page_size = 0;
        version = 0;
        zipped = false;
    }

    last = _prev;

    is_fatmodified   = false;
    is_emptymodified = false;
    is_modified      = false;
    is_destructed    = false;
    flushed          = false;
    leave_data       = false;
}


//===========================================================================
bool __fastcall v8catalog::IsCatalog()
{
    int _filelen;
    int _startempty = -1;
    char _t[32];

    int soFromBeginning = 0;

    if(iscatalogdefined)
    {
        return iscatalog;
    }
    iscatalogdefined = true;
    iscatalog = false;

    // эмпирический метод?
    _filelen = data->GetSize();
    if(_filelen == 16)
    {

        data->Seek(0, soFromBeginning);
        data->Read(_t, 16);
        if(memcmp(_t, _empty_catalog_template, 16) != 0)
        {
            return false;
        }
        else
        {
            iscatalog = true;
            return true;
        }
    }

    data->Seek(0, soFromBeginning);
    data->Read(&_startempty, 4);

    if(_startempty != V8_FF_SIGNATURE)
    {
        if(_startempty + 31 >= _filelen)
        {
            return false;
        }
        data->Seek(_startempty, soFromBeginning);
        data->Read(_t, 31);
        if(_t[0] != 0xd || _t[1] != 0xa || _t[10] != 0x20 || _t[19] != 0x20 || _t[28] != 0x20 || _t[29] != 0xd || _t[30] != 0xa)
        {
            return false;
        }
    }
    if(_filelen < 31 + 16)
    {
        return false;
    }
    data->Seek(16, soFromBeginning);
    data->Read(_t, 31);
    if(_t[0] != 0xd || _t[1] != 0xa || _t[10] != 0x20 || _t[19] != 0x20 || _t[28] != 0x20 || _t[29] != 0xd || _t[30] != 0xa)
    {
        return false;
    }
    iscatalog = true;

    return true;
}

//===========================================================================
void v8catalog::DeleteFile(const QString& FileName)
{
    v8file* f = first;
    while(f)
    {
        if (f->name.compare(FileName, Qt::CaseInsensitive) == 0)
        {
            f->DeleteFile();
            delete f;
        }
        f = f->next;
    }

}

//===========================================================================
v8file* v8catalog::GetFile(const QString& FileName)
{
    v8file* ret;

    std::map<QString, v8file*>::const_iterator it;

    it = files.find(FileName.toUpper());

    if(it == files.end())
        ret = NULL;
    else
        ret = it->second;

    return ret;
}

v8file* v8catalog::GetFirst()
{
    return first;
}

//===========================================================================
v8file* v8catalog::createFile(const QString& FileName, bool _selfzipped)
{
    __int64 v8t;
    v8file* f;

    f = GetFile(FileName);
    if(!f)
    {
        //setCurrentTime(&v8t);
        //f = new v8file(this, FileName, last, 0, 0, &v8t, &v8t);
        f = new v8file(this, FileName, last, 0, 0, 0, 0);
        f->selfzipped = _selfzipped;
        last = f;
        is_fatmodified = true;
    }

    return f;
}

//===========================================================================
v8catalog* v8catalog::GetParentCatalog()
{
    if(!file)
        return NULL;

    return file->parent;
}

//===========================================================================
vcl_utils::TStream* v8catalog::read_datablock(int start, int offset)
{
    vcl_utils::TStream* stream;
    vcl_utils::TStream* stream2;

    int soFromBeginning = 0;

    if(!start)
        return new vcl_utils::TMemoryStream;

    if (offset)
    {
        stream = read_block_16(data, start);
    }
    else
    {
        stream = read_block(data, start);
    }

    if(zipped)
    {
        stream2 = new vcl_utils::TMemoryStream;
        stream->Seek(0, soFromBeginning);
        // TODO: Нужна реализация
        ZInflateStream(stream, stream2);
        delete stream;
    }
    else
        stream2 = stream;

    return stream2;
}

//===========================================================================
void v8catalog::free_block(int start)
{
    char temp_buf[32];
    int nextstart;
    int prevempty;
    int soFromBeginning = 0;

    if(!start)
        return;
    if(start == V8_FF_SIGNATURE)
        return;

    prevempty = start_empty;
    start_empty = start;

    do
    {
        data->Seek(start, soFromBeginning);
        data->ReadBuffer(temp_buf, 31);
        nextstart = hex_to_int(&temp_buf[20]);
        int_to_hex(&temp_buf[2], V8_FF_SIGNATURE);
        if(nextstart == V8_FF_SIGNATURE)
            int_to_hex(&temp_buf[20], prevempty);
        data->Seek(start, soFromBeginning);
        data->WriteBuffer(temp_buf, 31);
        start = nextstart;
    }
    while(start != V8_FF_SIGNATURE);

    is_emptymodified = true;
    is_modified = true;

}

//===========================================================================
int v8catalog::write_datablock(vcl_utils::TStream* block, int start, bool _zipped, int len)
{
    vcl_utils::TMemoryStream* stream2;
    vcl_utils::TMemoryStream* stream;
    int ret;

    int soFromBeginning = 0;

    //if(!file)
    if(zipped || _zipped)
    {
        if(len == -1)
        {
            stream2 = new vcl_utils::TMemoryStream;
            block->Seek(0, soFromBeginning);
            // TODO: Нужна реализация
            ZDeflateStream(block, stream2);

            start = write_block(stream2, start, false);
            ret = start;

            delete stream2;
        }
        else
        {
            stream = new vcl_utils::TMemoryStream;
            stream->CopyFrom(block, len);
            stream2 = new vcl_utils::TMemoryStream;
            stream->Seek(0, soFromBeginning);
            // TODO: Нужна реализация
            ZDeflateStream(stream, stream2);
            delete stream;

            start = write_block(stream2, start, false);
            ret = start;

            delete stream2;
        }
    }
    else
    {

        start = write_block(block, start, false, len);
        ret = start;

    }
    return ret;
}

int v8catalog::get_nextblock(int start)
{
    int ret;

    if(start == 0 || start == V8_FF_SIGNATURE)
    {
        start = start_empty;

        if(start == V8_FF_SIGNATURE)
            start = data->GetSize();
    }
    ret = start;

    return ret;
}

//===========================================================================
int v8catalog::write_block(vcl_utils::TStream* block, int start, bool use_page_size, int len)
{
    char temp_buf[32];
    char* _t;
    int firststart, nextstart, blocklen, curlen;
    bool isfirstblock = true;
    bool addwrite = false; // признак, что надо дозаписать файл при использовании размера страницы по умолчанию

    int soFromBeginning = 0;

    if(data->GetSize() == 16 && start != 16) // если каталог пустой, надо выделить первую страницу!!!
    {
        vcl_utils::TMemoryStream* _ts = new vcl_utils::TMemoryStream;
        write_block(_ts, 16, true);
    }

    if(len == -1)
    {
        len = block->GetSize();
        block->Seek(0, soFromBeginning);
    }
    start = get_nextblock(start);

    do
    {
        if(start == start_empty)
        {
            // пишем в свободный блок
            data->Seek(start, soFromBeginning);
            data->ReadBuffer(temp_buf, 31);
            blocklen = hex_to_int(&temp_buf[11]);
            nextstart = hex_to_int(&temp_buf[20]);
            //start_empty = len <= blocklen ? V8_FF_SIGNATURE : nextstart;
            start_empty = nextstart;
            is_emptymodified = true;
        }
        else if(start == data->GetSize())
        {
            // пишем в новый блок
            memcpy(temp_buf, _block_header_template, 31);
            blocklen = use_page_size ? len > page_size ? len : page_size : len;
            int_to_hex(&temp_buf[11], blocklen);
            nextstart = 0;
            if(blocklen > len)
                addwrite = true;
        }
        else
        {
            // пишем в существующий блок
            data->Seek(start, soFromBeginning);
            data->ReadBuffer(temp_buf, 31);
            blocklen = hex_to_int(&temp_buf[11]);
            nextstart = hex_to_int(&temp_buf[20]);
        }

        int_to_hex(&temp_buf[2], isfirstblock ? len : 0);
        curlen = min(blocklen, len);

        if(!nextstart)
            nextstart = data->GetSize() + 31 + blocklen;
        else
            nextstart = get_nextblock(nextstart);

        int_to_hex(&temp_buf[20], len <= blocklen ? V8_FF_SIGNATURE : nextstart);

        data->Seek(start, soFromBeginning);
        data->WriteBuffer(temp_buf, 31);
        data->CopyFrom(block, curlen);
        if(addwrite)
        {
            _t = new char [blocklen - len];
            memset(_t, 0, blocklen - len);
            data->WriteBuffer(_t, blocklen - len);
            addwrite = false;
        }

        len -= curlen;

        if(isfirstblock)
        {
            firststart = start;
            isfirstblock = false;
        }
        start = nextstart;

    }while(len > 0);

    if(start < data->GetSize() && start != start_empty)
        free_block(start);

    is_modified = true;

    return firststart;
}

v8catalog::~v8catalog()
{
    fat_item fi;
    v8file* f;
    vcl_utils::TMemoryStream* fat = NULL;
    int soFromBeginning = 0;

    is_destructed = true;

    f = first;
    while(f)
    {
        //		f->readonly = readonly;
        f->Close();
        f = f->next;
    }

    if(data)
    {
        if(is_fatmodified)
        {
            try
            {
                fat = new vcl_utils::TMemoryStream;
                fi.ff = V8_FF_SIGNATURE;
                f = first;
                while(f)
                {
                    fi.header_start = f->start_header;
                    fi.data_start = f->start_data;
                    fat->WriteBuffer(&fi, 12);
                    f = f->next;
                }
                write_block(fat, 16, true);
            }
            catch(...)
            {
            }
            delete fat;
        }
    }

    while(first) delete first;

    if(data)
    {
        if(is_emptymodified)
        {
            data->Seek(0, soFromBeginning);
            data->WriteBuffer(&start_empty, 4);
        }
        if(is_modified)
        {
            version++;
            data->Seek(8, soFromBeginning);
            data->WriteBuffer(&version, 4);
        }
    }

    if(file)
    {
        if(is_modified)
        {
            file->is_datamodified = true;
        }
        if(!file->is_destructed) file->Close();
    }
    else
    {
        if(is_cfu)
        {
            if(data && cfu && is_modified)
            {
                data->Seek(0, soFromBeginning);
                cfu->Seek(0, soFromBeginning);
                // TODO: Нужна реализация
                ZDeflateStream(data, cfu);
            }
            delete data;
            data = NULL;
            if(cfu && !leave_data)
            {
                delete cfu;
                cfu = NULL;
            }
        }
        if(data && !leave_data)
        {
            delete data;
            data = NULL;
        }

    }
    //if(!file) delete Lock;
}

//===========================================================================
v8file* v8catalog::GetSelfFile()
{
    return file;
}

//===========================================================================
v8catalog* v8catalog::CreateCatalog(const QString& FileName, bool _selfzipped)
{
    v8catalog* ret;

    v8file* f = createFile(FileName, _selfzipped);
    if(f->GetFileLength())
    {
        if(f->IsCatalog()) ret = f->GetCatalog();
        else ret = NULL;
    }
    else
    {
        f->Write(_empty_catalog_template, 16);
        ret = f->GetCatalog();
    }

    return ret;
}

//===========================================================================
void v8catalog::SaveToDir(QString DirName) {
    // Создаём каталог (создаёт все родительские папки)
    QDir dir;
    if (!dir.mkpath(DirName)) {
        // Обработка ошибки (можно выбросить исключение)
        throw std::runtime_error("Failed to create directory");
    }

    // Добавляем разделитель в конец, если его нет
    if (!DirName.endsWith(QDir::separator())) {
        DirName += QDir::separator();
    }

    v8file* f = first;
    while (f) {
        // Формируем полный путь
        QString fullPath = DirName + f->name;

        if (f->IsCatalog()) {
            f->GetCatalog()->SaveToDir(fullPath);
        } else {
            f->SaveToFile(fullPath);
        }

        f->Close();
        f = f->next;
    }
}

bool v8catalog::isOpen()
{
    return IsCatalog();
}

//===========================================================================
void v8catalog::Flush()
{
    fat_item fi;
    v8file* f;
    int soFromBeginning = 0;


    if(flushed)
    {
        return;
    }
    flushed = true;

    f = first;
    while(f)
    {
        f->Flush();
        f = f->next;
    }

    if(data)
    {
        if(is_fatmodified)
        {
            vcl_utils::TMemoryStream* fat = new vcl_utils::TMemoryStream;
            fi.ff = V8_FF_SIGNATURE;
            f = first;
            while(f)
            {
                fi.header_start = f->start_header;
                fi.data_start = f->start_data;
                fat->WriteBuffer(&fi, 12);
                f = f->next;
            }
            write_block(fat, 16, true);
            is_fatmodified = false;
        }

        if(is_emptymodified)
        {
            data->Seek(0, soFromBeginning);
            data->WriteBuffer(&start_empty, 4);
            is_emptymodified = false;
        }
        if(is_modified)
        {
            version++;
            data->Seek(8, soFromBeginning);
            data->WriteBuffer(&version, 4);
        }
    }

    if(file)
    {
        if(is_modified)
        {
            file->is_datamodified = true;
        }
        //if(!file->is_destructed) file->Close();
        file->Flush();
    }
    else
    {
        if(is_cfu)
        {
            if(data && cfu && is_modified)
            {
                data->Seek(0, soFromBeginning);
                cfu->Seek(0, soFromBeginning);
                // TODO: Нужна реализация
                ZDeflateStream(data, cfu);
            }
        }
    }

    is_modified = false;
    flushed = false;

}

//===========================================================================
void v8catalog::HalfClose()
{

    Flush();
    if(is_cfu)
    {
        delete cfu;
        cfu = NULL;
    }
    else
    {
        delete data;
        data = NULL;
    }

}

//===========================================================================
void v8catalog::HalfOpen(const QString& name)
{
    int fmOpenReadWrite = 3;
    int fmShareDenyNone = 0;

    if(is_cfu)
        cfu = new vcl_utils::TFileStream(name.toStdWString(), fmOpenReadWrite | fmShareDenyNone);
    else
        data = new vcl_utils::TFileStream(name.toStdWString(), fmOpenReadWrite | fmShareDenyNone);
}

void v8catalog::ClearIs8316()
{
    is_8316 = false;
}


//===========================================================================
v8file::~v8file()
{
    std::set<TV8FileStream*>::iterator istreams;


    is_destructed = true;

    for(istreams = streams.begin(); istreams != streams.end(); ++istreams)
        delete *istreams;

    streams.clear();

    Close();

    if(parent)
    {
        if(next)
        {
            next->previous = previous;
        }
        else
        {
            parent->last = previous;
        }
        if(previous)
        {
            previous->next = next;
        }
        else
        {
            parent->first = next;
        }
    }
}


//********************************************************
// Класс TV8FileStream

//===========================================================================
TV8FileStream::TV8FileStream(v8file* f, bool ownfile) : vcl_utils::TStream(), file(f), own(ownfile)
{
    pos = 0l;
    //file->streams.insert(this);
}

//===========================================================================
TV8FileStream::~TV8FileStream()
{
    if(own)
        delete file;
    // else
    //     file->streams.erase(this);
}

//===========================================================================
int TV8FileStream::Read(void *Buffer, int Count)
{
    int r = file->Read(Buffer, pos, Count);
    pos += r;

    return r;
}

//===========================================================================
int TV8FileStream::Write(const void *Buffer, int Count)
{
    int r = file->Write(Buffer, pos, Count);
    pos += r;
    return r;
}

//===========================================================================
int TV8FileStream::Seek(int Offset, int Origin)
{
    const int soFromBeginning = 0;
    const int soFromCurrent = 1;
    const int soFromEnd = 2;

    int l = file->GetFileLength();
    switch(Origin)
    {
    case soFromBeginning:
        if(Offset >= 0)
        {
            if(Offset <= l) pos = Offset;
            else pos = l;
        }
        break;
    case soFromCurrent:
        if(pos + Offset < l) pos += Offset;
        else pos = l;
        break;
    case soFromEnd:
        if(Offset <= 0)
        {
            if(Offset <= l) pos = l - Offset;
            else pos = 0;
        }
        break;
    }
    return pos;
}

//===========================================================================
int v8file::GetFileLength()
{
    int ret;

    if(!is_opened)
        if(!Open())
            return 0;

    ret = data->GetSize();

    return ret;
}

//===========================================================================
int v8file::Read(void* Buffer, int Start, int Length)
{
    int ret;
    int soFromBeginning = 0;

    if(!is_opened)
        if(!Open())
            return 0;

    data->Seek(Start, soFromBeginning);
    ret = data->Read(Buffer, Length);

    return ret;
}

//===========================================================================
int v8file::Read(ByteArr& Buffer, int Start, int Length)
{
    int ret;
    int soFromBeginning = 0;

    if(!is_opened)
        if(!Open())
            return 0;

    data->Seek(Start, soFromBeginning);
    ret = data->Read(Buffer, Length);

    return ret;
}


//===========================================================================
void v8file::Flush()
{
    int _t = 0;

    if(flushed)
    {
        return;
    }
    if(!parent)
    {
        return;
    }
    if(!is_opened)
    {
        return;
    }

    flushed = true;
    if(self)
        self->Flush();

    //	if(parent->data && !readonly)
    if(parent->data)
    {
        if(is_datamodified || is_headermodified)
        {
            if(is_datamodified)
            {
                start_data = parent->write_datablock(data, start_data, selfzipped);
                is_datamodified = false;
            }
            if(is_headermodified)
            {
                vcl_utils::TMemoryStream* hs = new vcl_utils::TMemoryStream();
                hs->Write(&time_create, 8);
                hs->Write(&time_modify, 8);
                hs->Write(&_t, 4);
/*
#ifndef _DELPHI_STRING_UNICODE
                int ws = name.WideCharBufSize();
                char* tb = new char[ws];
                name.WideChar((wchar_t*)tb, ws);
                hs->Write((char*)tb, ws);
                delete[] tb;
#else
                hs->Write(name.c_str(), name.Length() * 2);
#endif
*/
                hs->Write(&_t, 4);

                start_header = parent->write_block(hs, start_header, false);
                delete hs;
                is_headermodified = false;
            }
        }
    }
    flushed = false;
}


//===========================================================================
void v8file::SaveToFile(const QString& FileName)
{
    //FILETIME create, modify;
    int fmCreate = 0;

    if(!is_opened)
        if(!Open())
            return;

    vcl_utils::TFileStream* fs = new vcl_utils::TFileStream(FileName.toStdWString(), fmCreate);

    fs->CopyFrom(data, 0);

    //GetTimeCreate(&create);
    //GetTimeModify(&modify);
    //SetFileTime((HANDLE)fs->Handle, &create, &modify, &modify);

    delete fs;
}


//===========================================================================
// дозапись/перезапись частично
int v8file::Write(const void* Buffer, int Start, int Length)
{
    int ret;
    int soFromBeginning = 0;
    //	if(readonly) return 0;

    if(!is_opened)
        if(!Open())
            return 0;

    //setCurrentTime(&time_modify);

    is_headermodified = true;
    is_datamodified = true;

    data->Seek(Start, soFromBeginning);
    ret = data->Write(Buffer, Length);


    return ret;
}

//===========================================================================
// дозапись/перезапись частично
int v8file::Write(ByteArr Buffer, int Start, int Length)
{
    int ret;
    int soFromBeginning = 0;
    //	if(readonly) return 0;

    if(!is_opened)
        if(!Open())
            return 0;

    //setCurrentTime(&time_modify);

    is_headermodified = true;
    is_datamodified = true;

    data->Seek(Start, soFromBeginning);
    ret = data->Write(Buffer, Length);


    return ret;
}

//===========================================================================
// дозапись/перезапись частично
int v8file::Write(vcl_utils::TStream* Stream, int Start, int Length)
{
    int ret;
    int soFromBeginning = 0;
    //	if(readonly) return 0;

    if(!is_opened)
        if(!Open())
            return 0;

    //setCurrentTime(&time_modify);

    is_headermodified = true;
    is_datamodified = true;

    data->Seek(Start, soFromBeginning);
    ret = Length;
    data->CopyFrom(Stream, Length);


    return ret;
}


//===========================================================================
// перезапись целиком
int v8file::Write(const void* Buffer, int Length)
{
    int ret;
    int soFromBeginning = 0;
    //	if(readonly) return 0;

    if(!is_opened)
        if(!Open())
            return 0;

    //setCurrentTime(&time_modify);
    is_headermodified = true;
    is_datamodified = true;
    if(data->GetSize() > Length)
        data->SetSize(Length);
    data->Seek(0, soFromBeginning);
    ret = data->Write(Buffer, Length);


    return ret;
}

//===========================================================================
void v8file::DeleteFile()
{
    //	if(readonly) return;

    if(parent)
    {

        if(next)
        {
            next->previous = previous;
        }
        else
            parent->last = previous;
        if(previous)
        {
            previous->next = next;
        }
        else
            parent->first = next;

        parent->is_fatmodified = true;
        parent->free_block(start_data);
        parent->free_block(start_header);
        parent->files.erase(name.toUpper());
        parent = NULL;
    }

    delete data;

    data = NULL;

    if(self)
    {
        self->data = NULL;
        delete self;
        self = NULL;
    }

    iscatalog = iscatalog_false;
    next = NULL;
    previous = NULL;
    is_opened = false;
    start_data = 0;
    start_header = 0;
    is_datamodified = false;
    is_headermodified = false;
    //Lock->Release();
    //delete this; // суицид
}

//===========================================================================
void v8file::Close()
{
    int _t = 0;

    if(!parent)
        return;

    if(!is_opened)
        return;

    if(self)
        if(!self->is_destructed)
        {
            // self->readonly = readonly;
            delete self;
        }

    self = NULL;

    //	if(parent->data && !readonly)
    if(parent->data)
    {
        if(is_datamodified || is_headermodified)
        {
            if(is_datamodified)
            {
                start_data = parent->write_datablock(data, start_data, selfzipped);
            }
            if(is_headermodified)
            {
                vcl_utils::TMemoryStream* hs = new vcl_utils::TMemoryStream();
                hs->Write(&time_create, 8);
                hs->Write(&time_modify, 8);
                hs->Write(&_t, 4);
/*
#ifndef _DELPHI_STRING_UNICODE
                int ws = name.WideCharBufSize();
                char* tb = new char[ws];
                name.WideChar((wchar_t*)tb, ws);
                hs->Write((char*)tb, ws);
                delete[] tb;
#else
                hs->Write(name.c_str(), name.Length() * 2);
#endif
*/
                hs->Write(&_t, 4);

                start_header = parent->write_block(hs, start_header, false);
                delete hs;
            }
        }
    }
    delete data;
    data = NULL;
    iscatalog = iscatalog_unknown;
    is_opened = false;
    is_datamodified = false;
    is_headermodified = false;
}






