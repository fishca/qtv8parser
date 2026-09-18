#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <QString>
#include <QIODevice>
#include <memory>

#include "v8_constants.h"

//#include "NodeTypes.h"
//#include "MessageRegistration.h"

// Класс узла дерева
class tree
{
private:
    QString value;
    node_type type;
    int num_subnode;   // количество дочерних узлов
    tree* parent;
    tree* next;
    tree* prev;
    tree* first;
    tree* last;
    unsigned int index;

public:
    tree(const QString& _value, node_type _type, tree* _parent);
    ~tree();

    tree* add_child(const QString& _value, node_type _type);
    tree* add_child();
    tree* add_node();

    QString& get_value();
    node_type get_type() const;
    int get_num_subnode() const;
    tree* get_subnode(int _index);
    tree* get_subnode(const QString& node_name);
    tree* get_next() const;
    tree* get_parent() const;
    tree* get_first() const;
    tree* get_last() const;
    tree& operator[](int _index);

    void set_value(const QString& v, node_type t);
    void outtext(QString& text);
    QString path();
};

typedef tree* treeptr;
typedef std::unique_ptr<tree> tree_unique_ptr;
typedef std::shared_ptr<tree> tree_shared_ptr;

// Глобальные функции
tree* parse_1Ctext(const QString& text, const QString& path);
tree* parse_1Cstream(QIODevice* dev, const QString& path);
bool test_parse_1Ctext(QIODevice* dev, const QString& path);
QString outtext(tree* t);
tree* find_node_by_guid(tree* root, const QString& target_guid);
tree* find_metadata_node_by_guid(tree* root, const QString& target_guid);

#endif // PARSE_TREE_H