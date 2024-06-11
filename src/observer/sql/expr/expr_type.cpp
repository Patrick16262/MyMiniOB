#include "expr_type.h"
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

static unordered_map<string, AggregateType> aggregate_name_map{{"count", AggregateType::COUNT},
    {"sum", AggregateType::SUM},
    {"avg", AggregateType::AVG},
    {"max", AggregateType::MAX},
    {"min", AggregateType::MIN}};

static unordered_map<string, FunctionType> function_name_map{
    {"length", FunctionType::LENGTH}, {"round", FunctionType::ROUND}, {"date_format", FunctionType::DATE_FORMAT}};

bool common::is_aggregate_function(const char *function_name)
{
 stringstream ss;
 for (int i = 0; i < strlen(function_name); i++) {
   ss << (char) tolower(function_name[i]);
 }

 return aggregate_name_map.find(ss.str()) != aggregate_name_map.end();
}

FunctionType common::get_function_type(const char *function_name)
{
 stringstream ss;
 for (int i = 0; i < strlen(function_name); i++) {
   ss << (char) tolower(function_name[i]);
 }

 auto it = function_name_map.find(ss.str());
 if (it != function_name_map.end()) {
   return it->second;
 } else {
   return FunctionType::INVALID;
 }
}

AggregateType common::get_aggregate_type(const char *function_name)
{
 stringstream ss;
 for (int i = 0; i < strlen(function_name); i++) {
   ss << (char)tolower(function_name[i]);
 }

 auto it = aggregate_name_map.find(ss.str());
 if (it != aggregate_name_map.end()) {
   return it->second;
 } else {
   return AggregateType::INVALID;
 }
}
