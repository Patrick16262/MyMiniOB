#pragma once
#include <string>
#include "../value.h"

/**
 * @brief 描述一个属性
 * @ingroup SQLParser
 * @details 属性，或者说字段(column, field)
 * Rel -> Relation )
 * Attr -> Attribute
 */
struct RelAttrSqlNode
{
  std::string relation_name;   ///< relation name (may be empty) 表名
  std::string attribute_name;  ///< attribute name              属性名
};

enum class RelationType
{
  TABLE,
  SUBQUERY,
  JOIN,
  VIEW,
};

/**
 * @brief Table引用，可以是一个表，也可以是一个查询，也可以是一个join
 *  其子类实现有TableReferenceSqlNode, TableSubquerySqlNode, TableJoinSqlNode
 */
class TableReferenceSqlNode
{
public:
  RelationType type;
  std::string  alias;
};

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

