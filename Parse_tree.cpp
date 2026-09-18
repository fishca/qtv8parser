#include "Parse_tree.h"
//#include "Common.h"   // предположительно содержит tohex()

#include <QTextStream>
#include <QChar>
#include <QStringBuilder>   // для эффективной конкатенации, если нужно

// Если Common.h не предоставляет tohex, можно определить так:
// inline QString tohex(int value) { return QString::number(value, 16); }

// Глобальный указатель на регистратор сообщений (из MessageRegistration.h)
//extern MessageRegistrator* msreg;
//#define error if(msreg) msreg->AddError

// Вспомогательные функции для классификации
namespace {
inline bool is_ascii_digit(QChar ch) {
    return ch >= '0' && ch <= '9';
}

inline bool is_hex_digit(QChar ch) {
    return (ch >= '0' && ch <= '9') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= 'A' && ch <= 'F');
}

inline bool is_base64_digit(QChar ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') ||
           ch == '+' || ch == '=' || ch == '/' ||
           ch == '\r' || ch == '\n';
}

bool is_number_fast(const QString& value) {
    int len = value.length();
    if (len == 0) return false;

    int i = 0;
    if (value[i] == '-') {
        ++i;
        if (i >= len) return false;
    }

    for (; i < len; ++i) {
        if (!is_ascii_digit(value[i])) return false;
    }
    return true;
}

bool is_number_exp_fast(const QString& value) {
    int len = value.length();
    if (len == 0) return false;

    int i = 0;
    if (value[i] == '-') {
        ++i;
        if (i >= len) return false;
    }

    bool has_digits = false;
    while (i < len && is_ascii_digit(value[i])) {
        has_digits = true;
        ++i;
    }
    if (!has_digits) return false;

    if (i < len && value[i] == '.') {
        ++i;
        while (i < len && is_ascii_digit(value[i])) ++i;
    }

    if (i < len && (value[i] == 'e' || value[i] == 'E')) {
        ++i;
        if (i < len && value[i] == '-') ++i;
        if (i >= len || !is_ascii_digit(value[i])) return false;
        while (i < len && is_ascii_digit(value[i])) ++i;
    }

    return i == len;
}

bool is_guid_fast(const QString& value) {
    static const int expected_len = 36;
    if (value.length() != expected_len) return false;

    for (int i = 0; i < expected_len; ++i) {
        QChar ch = value[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (ch != '-') return false;
        } else if (!is_hex_digit(ch)) {
            return false;
        }
    }
    return true;
}

bool is_link_fast(const QString& value) {
    int len = value.length();
    if (len < 34) return false;

    int i = 0;
    if (!is_ascii_digit(value[i])) return false;
    while (i < len && is_ascii_digit(value[i])) ++i;
    if (i >= len || value[i] != ':') return false;
    ++i;

    if (len - i != 32) return false;
    for (; i < len; ++i) {
        if (!is_hex_digit(value[i])) return false;
    }
    return true;
}

bool is_base64_fast(const QString& value, int start_index) {
    for (int i = start_index; i < value.length(); ++i) {
        if (!is_base64_digit(value[i])) return false;
    }
    return true;
}

node_type classification_value(const QString& value) {
    if (value.isEmpty()) return nd_empty;
    if (is_number_fast(value)) return nd_number;
    if (is_number_exp_fast(value)) return nd_number_exp;
    if (is_guid_fast(value)) return nd_guid;
    if (value.length() >= 8 && value.left(8) == "#base64:" && is_base64_fast(value, 8)) return nd_binary;
    if (is_link_fast(value)) return nd_link;
    if (is_base64_fast(value, 0)) return nd_binary2;
    if (value.length() >= 6 && value.left(6) == "#data:" && is_base64_fast(value, 6)) return nd_binary_d;
    return nd_unknown;
}
} // anonymous namespace

// ------------------------------------------------------------
// Реализация класса tree
// ------------------------------------------------------------
tree::tree(const QString& _value, node_type _type, tree* _parent)
    : value(_value), type(_type), parent(_parent)
{
    num_subnode = 0;
    index = 0;
    if (parent) {
        parent->num_subnode++;
        prev = parent->last;
        if (prev) {
            prev->next = this;
            index = prev->index + 1;
        } else {
            parent->first = this;
        }
        parent->last = this;
    } else {
        prev = nullptr;
    }
    next = nullptr;
    first = nullptr;
    last = nullptr;
}

tree::~tree() {
    while (last) delete last;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    if (parent) {
        if (parent->first == this) parent->first = next;
        if (parent->last == this) parent->last = prev;
        parent->num_subnode--;
    }
}

tree* tree::add_child(const QString& _value, node_type _type) {
    return new tree(_value, _type, this);
}

tree* tree::add_child() {
    return new tree(QString(), nd_empty, this);
}

tree* tree::add_node() {
    return new tree(QString(), nd_empty, this->parent);
}

QString& tree::get_value() {
    return value;
}

node_type tree::get_type() const {
    return type;
}

void tree::set_value(const QString& v, node_type t) {
    value = v;
    type = t;
}

int tree::get_num_subnode() const {
    return num_subnode;
}

tree* tree::get_subnode(int _index) {
    if (_index >= num_subnode) return nullptr;
    tree* t = first;
    while (_index) {
        t = t->next;
        --_index;
    }
    return t;
}

tree* tree::get_subnode(const QString& node_name) {
    tree* t = first;
    while (t) {
        if (t->value == node_name) return t;
        t = t->next;
    }
    return nullptr;
}

tree* tree::get_next() const {
    return next;
}

tree* tree::get_parent() const {
    return parent;
}

tree* tree::get_first() const {
    return first;
}

tree* tree::get_last() const {
    return last;
}

tree& tree::operator[](int _index) {
    // Внимание: исходный код содержал проверку if(!this) - она бессмысленна.
    // Мы просто проходим по списку.
    tree* ret = first;
    while (_index && ret) {
        ret = ret->next;
        --_index;
    }
    // Если индекс вышел за пределы, возвращаем ссылку на последний узел?
    // В оригинале возвращали *this (себя) при нулевом this,
    // что тоже небезопасно. Лучше вернуть ссылку на последний найденный.
    // Для совместимости оставим как в оригинале, но добавим проверку.
    if (!ret) {
        // fallback: возвращаем последний существующий или себя
        ret = last ? last : this;
    }
    return *ret;
}

void tree::outtext(QString& text) {
    if (num_subnode) {
        if (!text.isEmpty()) text += "\r\n";
        text += '{';
        tree* t = first;
        while (t) {
            t->outtext(text);
            t = t->next;
            if (t) text += ',';
        }
        if (last && last->type == nd_list) text += "\r\n";
        text += '}';
    } else {
        switch (type) {
        case nd_string:
            text += '"';
            text += value;
            text.replace('"', "\"\"");   // экранирование кавычек
            text += '"';
            break;
        case nd_number:
        case nd_number_exp:
        case nd_guid:
        case nd_list:
        case nd_binary:
        case nd_binary2:
        case nd_link:
        case nd_binary_d:
            text += value;
            break;
        default:
            // можно выдать ошибку через msreg
            //if (msreg) msreg->AddError("Ошибка вывода потока. Пустой или неизвестный узел.");
            break;
        }
    }
}

QString tree::path() {
    QString p;
    tree* t = this;
    while (t->parent) {
        p = QString(":%1").arg(t->index) + p;
        t = t->parent;
    }
    return p.isEmpty() ? ":??" : p;
}

// ------------------------------------------------------------
// Парсеры
// ------------------------------------------------------------

tree* parse_1Ctext(const QString& text, const QString& path) {
    enum _state {
        s_value,
        s_delimitier,
        s_string,
        s_quote_or_endstring,
        s_nonstring
    } state = s_value;

    QString curvalue;
    tree* ret = new tree(QString(), nd_list, nullptr);
    tree* t = ret;
    int len = text.length();

    for (int i = 0; i < len; ++i) {
        QChar sym = text[i];
        if (sym.isNull()) break;

        switch (state) {
        case s_value:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // пробел игнорируем
            } else if (sym == '"') {
                curvalue.clear();
                state = s_string;
            } else if (sym == '{') {
                t = new tree(QString(), nd_list, t);
            } else if (sym == '}') {
                if (t->get_first()) t->add_child(QString(), nd_empty);
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", i,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
                state = s_delimitier;
            } else if (sym == ',') {
                t->add_child(QString(), nd_empty);
            } else {
                curvalue.clear();
                curvalue += sym;
                state = s_nonstring;
            }
            break;

        case s_delimitier:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // пропускаем
            } else if (sym == ',') {
                state = s_value;
            } else if (sym == '}') {
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", i,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
                // state остаётся s_delimitier (как в оригинале)
            } else {
//                if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                    "Символ", sym,
//                                    "Код символа", tohex(sym.unicode()),
//                                    "Путь", path);
                delete ret;
                return nullptr;
            }
            break;

        case s_string:
            if (sym == '"') {
                state = s_quote_or_endstring;
            } else {
                curvalue += sym;
            }
            break;

        case s_quote_or_endstring:
            if (sym == '"') {
                curvalue += sym;
                state = s_string;
            } else {
                t->add_child(curvalue, nd_string);
                if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                    state = s_delimitier;
                } else if (sym == ',') {
                    state = s_value;
                } else if (sym == '}') {
                    t = t->get_parent();
                    if (!t) {
//                        if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                            "Позиция", i,
//                                            "Путь", path);
                        delete ret;
                        return nullptr;
                    }
                    state = s_delimitier;
                } else {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                        "Символ", sym,
//                                        "Код символа", tohex(sym.unicode()),
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
            }
            break;

        case s_nonstring:
            if (sym == ',') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg)
//                    msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
                t->add_child(curvalue, nt);
                state = s_value;
            } else if (sym == '}') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg)
//                    msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
                t->add_child(curvalue, nt);
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", i,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
                state = s_delimitier;
            } else {
                curvalue += sym;
            }
            break;

        default:
//            if (msreg) msreg->AddError("Ошибка формата потока. Неизвестный режим разбора.",
//                                "Режим разбора", tohex(state),
//                                "Путь", path);
            delete ret;
            return nullptr;
        }
    }

    // Обработка конца строки
    if (state == s_nonstring) {
        node_type nt = classification_value(curvalue);
//        if (nt == nd_unknown && msreg)
//            msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                            "Значение", curvalue,
//                            "Путь", path);
        t->add_child(curvalue, nt);
    } else if (state == s_quote_or_endstring) {
        t->add_child(curvalue, nd_string);
    } else if (state != s_delimitier) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Незавершенное значение",
//                            "Режим разбора", tohex(state),
//                            "Путь", path);
        delete ret;
        return nullptr;
    }

    if (t != ret) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Не хватает закрывающих скобок } в конце текста разбора.",
//                            "Путь", path);
        delete ret;
        return nullptr;
    }

    return ret;
}

tree* parse_1Cstream(QIODevice* dev, const QString& path) {
    if (!dev || !dev->isOpen()) {
//        if (msreg) msreg->AddError("Поток не открыт или неверный указатель", "Путь", path);
        return nullptr;
    }

    QTextStream reader(dev);
    // При необходимости установите кодировку:
    // reader.setCodec("UTF-8"); // или "Windows-1251"

    enum _state {
        s_value,
        s_delimitier,
        s_string,
        s_quote_or_endstring,
        s_nonstring
    } state = s_value;

    QString curvalue;
    tree* ret = new tree(QString(), nd_list, nullptr);
    tree* t = ret;
    int pos = 0;  // позиция в потоке (примерно)

    while (!reader.atEnd()) {
        QChar sym;
        reader >> sym;
        ++pos;

        // Далее код идентичен parse_1Ctext, только вместо text[i] используем sym
        // (копируем логику из parse_1Ctext, заменив text[i] на sym)
        // Для краткости здесь приведём только отличающуюся часть — все case-блоки
        // точно такие же, как в parse_1Ctext, только обращение к text заменено на sym.
        // Во избежание дублирования, я вставлю полный цикл, но его можно вынести в отдельную функцию.

        // -------- повторяем switch(state) из parse_1Ctext, используя символ sym --------
        switch (state) {
        case s_value:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // пробел игнорируем
            } else if (sym == '"') {
                curvalue.clear();
                state = s_string;
            } else if (sym == '{') {
                t = new tree(QString(), nd_list, t);
            } else if (sym == '}') {
                if (t->get_first()) t->add_child(QString(), nd_empty);
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
                state = s_delimitier;
            } else if (sym == ',') {
                t->add_child(QString(), nd_empty);
            } else {
                curvalue.clear();
                curvalue += sym;
                state = s_nonstring;
            }
            break;

        case s_delimitier:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // пропускаем
            } else if (sym == ',') {
                state = s_value;
            } else if (sym == '}') {
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
            } else {
//                if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                    "Символ", sym,
//                                    "Код символа", tohex(sym.unicode()),
//                                    "Путь", path);
                delete ret;
                return nullptr;
            }
            break;

        case s_string:
            if (sym == '"') {
                state = s_quote_or_endstring;
            } else {
                curvalue += sym;
            }
            break;

        case s_quote_or_endstring:
            if (sym == '"') {
                curvalue += sym;
                state = s_string;
            } else {
                t->add_child(curvalue, nd_string);
                if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                    state = s_delimitier;
                } else if (sym == ',') {
                    state = s_value;
                } else if (sym == '}') {
                    t = t->get_parent();
                    if (!t) {
//                        if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                            "Позиция", pos,
//                                            "Путь", path);
                        delete ret;
                        return nullptr;
                    }
                    state = s_delimitier;
                } else {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                        "Символ", sym,
//                                        "Код символа", tohex(sym.unicode()),
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
            }
            break;

        case s_nonstring:
            if (sym == ',') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg)
//                    msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
                t->add_child(curvalue, nt);
                state = s_value;
            } else if (sym == '}') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg)
//                    msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
                t->add_child(curvalue, nt);
                t = t->get_parent();
                if (!t) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    delete ret;
                    return nullptr;
                }
                state = s_delimitier;
            } else {
                curvalue += sym;
            }
            break;

        default:
//            if (msreg) msreg->AddError("Ошибка формата потока. Неизвестный режим разбора.",
//                                "Режим разбора", tohex(state),
//                                "Путь", path);
            delete ret;
            return nullptr;
        }
    }

    // Обработка конца потока (аналогично parse_1Ctext)
    if (state == s_nonstring) {
        node_type nt = classification_value(curvalue);
//        if (nt == nd_unknown && msreg)
//            msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                            "Значение", curvalue,
//                            "Путь", path);
        t->add_child(curvalue, nt);
    } else if (state == s_quote_or_endstring) {
        t->add_child(curvalue, nd_string);
    } else if (state != s_delimitier) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Незавершенное значение",
//                            "Режим разбора", tohex(state),
//                            "Путь", path);
        delete ret;
        return nullptr;
    }

    if (t != ret) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Не хватает закрывающих скобок } в конце текста разбора.",
//                            "Путь", path);
        delete ret;
        return nullptr;
    }

    return ret;
}

bool test_parse_1Ctext(QIODevice* dev, const QString& path) {
    if (!dev || !dev->isOpen()) {
//        if (msreg) msreg->AddError("Поток не открыт или неверный указатель", "Путь", path);
        return false;
    }

    QTextStream reader(dev);
    // reader.setCodec("UTF-8");

    enum _state {
        s_value,
        s_delimitier,
        s_string,
        s_quote_or_endstring,
        s_nonstring
    } state = s_value;

    QString curvalue;
    int pos = 0;
    int level = 0;
    bool result = true;

    while (!reader.atEnd()) {
        QChar sym;
        reader >> sym;
        ++pos;

        switch (state) {
        case s_value:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // ignore
            } else if (sym == '"') {
                curvalue.clear();
                state = s_string;
            } else if (sym == '{') {
                ++level;
            } else if (sym == '}') {
                if (level <= 0) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    result = false;
                }
                state = s_delimitier;
                --level;
            } else {
                curvalue.clear();
                curvalue += sym;
                state = s_nonstring;
            }
            break;

        case s_delimitier:
            if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                // ignore
            } else if (sym == ',') {
                state = s_value;
            } else if (sym == '}') {
                if (level <= 0) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    result = false;
                }
                --level;
            } else {
//                if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                    "Символ", sym,
//                                    "Код символа", tohex(sym.unicode()),
//                                    "Путь", path);
                return false;
            }
            break;

        case s_string:
            if (sym == '"') {
                state = s_quote_or_endstring;
            } else {
                curvalue += sym;
            }
            break;

        case s_quote_or_endstring:
            if (sym == '"') {
                curvalue += sym;
                state = s_string;
            } else {
                if (sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n') {
                    state = s_delimitier;
                } else if (sym == ',') {
                    state = s_value;
                } else if (sym == '}') {
                    if (level <= 0) {
//                        if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                            "Позиция", pos,
//                                            "Путь", path);
                        result = false;
                    }
                    --level;
                    state = s_delimitier;
                } else {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Ошибочный символ в режиме ожидания разделителя.",
//                                        "Символ", sym,
//                                        "Код символа", tohex(sym.unicode()),
//                                        "Путь", path);
                    return false;
                }
            }
            break;

        case s_nonstring:
            if (sym == ',') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg) {
//                    msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
//                    result = false;
//                }
                state = s_value;
            } else if (sym == '}') {
                node_type nt = classification_value(curvalue);
//                if (nt == nd_unknown && msreg) {
//                   msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                                    "Значение", curvalue,
//                                    "Путь", path);
//                    result = false;
//                }
                if (level <= 0) {
//                    if (msreg) msreg->AddError("Ошибка формата потока. Лишняя закрывающая скобка }.",
//                                        "Позиция", pos,
//                                        "Путь", path);
                    result = false;
                }
                --level;
                state = s_delimitier;
            } else {
                curvalue += sym;
            }
            break;

        default:
//            if (msreg) msreg->AddError("Ошибка формата потока. Неизвестный режим разбора.",
//                                "Режим разбора", tohex(state),
//                                "Путь", path);
            return false;
        }
    }

    // Проверка завершения
    if (state == s_nonstring) {
        node_type nt = classification_value(curvalue);
//        if (nt == nd_unknown && msreg) {
//            msreg->AddError("Ошибка формата потока. Неизвестный тип значения.",
//                            "Значение", curvalue,
//                            "Путь", path);
//            result = false;
//        }
    } else if (state == s_quote_or_endstring) {
        // строка завершена? Здесь ничего не делаем
    } else if (state != s_delimitier) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Незавершенное значение",
//                            "Режим разбора", tohex(state),
//                            "Путь", path);
        result = false;
    }

    if (level > 0) {
//        if (msreg) msreg->AddError("Ошибка формата потока. Не хватает закрывающих скобок } в конце текста разбора.",
//                            "Путь", path);
        result = false;
    }

    return result;
}

// ------------------------------------------------------------
// Вспомогательные функции
// ------------------------------------------------------------

QString outtext(tree* t) {
    QString text;
    if (t && t->get_first()) {
        t->get_first()->outtext(text);
    }
    return text;
}

tree* find_node_by_guid(tree* root, const QString& target_guid) {
    if (!root) return nullptr;
    if (root->get_value() == target_guid) return root;

    tree* result = nullptr;
    for (int i = 0; i < root->get_num_subnode(); ++i) {
        result = find_node_by_guid(root->get_subnode(i), target_guid);
        if (result) return result;
    }
    return nullptr;
}

namespace {
tree* find_metadata_node_by_guid_impl(tree* root, const QString& target_guid,
                                      tree*& fallback, int& bestCount) {
    if (!root) return nullptr;

    if (root->get_value() == target_guid) {
        if (!fallback) fallback = root;

        tree* next = root->get_next();
        if (next && next->get_type() == nd_number) {
            int currentCount = next->get_value().toInt(); // или toIntDef
            if (currentCount > bestCount) {
                bestCount = currentCount;
                fallback = root;
            }
        }
    }

    for (int i = 0; i < root->get_num_subnode(); ++i) {
        tree* result = find_metadata_node_by_guid_impl(root->get_subnode(i), target_guid,
                                                       fallback, bestCount);
        if (result) return result;
    }
    return nullptr;
}
}

tree* find_metadata_node_by_guid(tree* root, const QString& target_guid) {
    tree* fallback = nullptr;
    int bestCount = -1;
    tree* result = find_metadata_node_by_guid_impl(root, target_guid, fallback, bestCount);
    if (result) return result;
    return fallback;
}