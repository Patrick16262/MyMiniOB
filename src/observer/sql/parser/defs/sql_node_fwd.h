#pragma once
#include <string>
#include "../value.h"

/**
 * @brief 描述一个属性
 * @ingroup SQLParser
 * @details 属性，或者说字段(column, field)
 * Rel -> Relation
 * Attr -> Attribute
 */
struct RelAttrSqlNode
{
  std::string relation_name;   ///< relation name (may be NULL) 表名
  std::string attribute_name;  ///< attribute name              属性名
  std::string alias;           ///< alias name                  别名
};

enum class RelationType
{
  TABLE,
  SELECT,
  JOIN,
};

/**
 * @brief Table引用，可以是一个表，也可以是一个查询，也可以是一个join 
 */
class TableReferenceSqlNode
{
public:
  RelationType type;
  std::string  alias;
};

/**
 * @brief 描述一个primary table或者subquery, 用于join从句
 * 
 */
class TableFactorSqlNode : public TableReferenceSqlNode
{};




/**
 * @brief 描述一个属性
 * @ingroup SQLParser
 * @details 属性，或者说字段(column, field)
 */
struct AttrInfoSqlNode
{
  AttrType    type;    ///< Type of attribute
  std::string name;    ///< Attribute name
  size_t      length;  ///< Length of attribute
};