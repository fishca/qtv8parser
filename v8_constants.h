// v8_constants.h
// Общие константы формата файлов 1С v8
// Используется в APIcfBase.h и V8File.h (namespace v8unpack)

#ifndef V8_CONSTANTS_H
#define V8_CONSTANTS_H

// Размер страницы по умолчанию
const unsigned int V8_DEFAULT_PAGE_SIZE = 512;

// Сигнатура конца списка (32-бит формат, до 8.3.16)
const unsigned int V8_FF_SIGNATURE = 0x7fffffff;

// Сигнатура конца списка (64-бит формат, 8.3.16+)
const unsigned __int64 V8_FF64_SIGNATURE = 0xffffffffffffffffULL;

// Смещение начала данных в файлах формата 8.3.16+
// (волшебное смещение 0x1359 — происхождение неизвестно)
const int V8_OFFSET_8316 = 0x1359;

// версии базы данных 1CD
enum db_ver
{
    ver8_0_3_0  = 1,
    ver8_0_5_0  = 2,
    ver8_1_0_0  = 3,
    ver8_2_0_0  = 4,
    ver8_2_14_0 = 5,
    ver8_3_8_0  = 6
};

// типы узлов
enum node_type{
    nd_empty      = 0, // пусто
    nd_string     = 1, // строка
    nd_number     = 2, // число
    nd_number_exp = 3, // число с показателем степени
    nd_guid       = 4, // уникальный идентификатор
    nd_list       = 5, // список
    nd_binary     = 6, // двоичные данные (с префиксом #base64:)
    nd_binary2    = 7, // двоичные данные формата 8.2 (без префикса)
    nd_link       = 8, // ссылка
    nd_binary_d   = 9, // двоичные данные (с префиксом #data:)
    nd_unknown         // неизвестный тип
};



#endif // V8_CONSTANTS_H
