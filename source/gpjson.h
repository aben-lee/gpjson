#ifndef GPJSON_H
#define GPJSON_H

#include "json.h"
#include <array>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <utility>

/************************************************************
 * Featur, FeatureCollect are container of GPJson data
 * there are 3 ways to parse or serialize jdoc (GPJson file)
 * to Features
 * for exmaple:
 *     FeatureCollect features;
 *     QString error;
 *     if (!parse(jdoc,features,error) ){
 *      cout << error;  // 输出错误信息
 *     }
 * or
 *     FeatureCollect features;
 *     QString error;
 *     if(!parse<GPJson::FeatureCollect>(jdoc,features,error))
 *     {
 *       cout << error;  // 输出错误信息
 *     }
 * or
 *     FeatureCollect features;
 *     QString error;
 *     if(!feature.fromDeocument(jdoc,error) )
 *     {
 *       cout << error;  // 输出错误信息
 *     }
 *
 *
 * *********************************************************/

namespace GPJson {
class DataSet;
class Feature;
class FeatureCollect;
using rowData = Json::JsonArray;

enum FeatureType {Unknow = 0x00, Geometry = 0x01,Dataset = 0x02, GeometryDataset = 0x03};
enum DatasetType {Sheet = 0x11, Matrix = 0x12, Binary = 0x13, URI = 0x14};
enum Geometry    {Point = 0x21, MultiPoint = 0x22,LineString = 0x23, MultiLineString = 0x24, Polygon = 0x25, MultiPolygon = 0x26,GeometryCollection = 0x27 };

inline Json::JsonDocument serialize(const FeatureCollect& features); // JsonDocument是一个用于序列化和反序列化的对象

inline bool parse(const Json::JsonDocument &jdoc,FeatureCollect &features,std::string &errorInfo);

class FeatureCollect
{
public:
    std::list<std::shared_ptr <Feature>> list; // 设置一个list里面存放Feature的地址
    Json::JsonObject properties;
    std::string id;
public:
    FeatureCollect();
    inline void clear();
    inline bool fromDocument(const Json::JsonDocument &jdoc,std::string &errorInfo);
    inline Json::JsonDocument toDocument() const;
};

class DataSet
{
public:
    std::string type;
	Json::JsonArray attributes;
    std::vector<rowData> source;
	
public:
    DataSet();
    DataSet(const std::string &type);
    DataSet(const DataSet &other);
    ~DataSet();
    inline DataSet &operator = (const DataSet &other);
    inline bool operator == (const DataSet &other) const;
    inline int rowCount() const;
    inline int columnCount() const;
    inline Json::JsonValue at (int row, int column) const;
    inline bool replace(int row, int column, const Json::JsonValue & value);
    inline Json::JsonArray row(int index) const;
    inline Json::JsonArray column(int index)const;
    inline void insertRow(int row,Json::JsonArray array);
    inline void insertColumn(int column,Json::JsonArray array);
    inline void prependRow(Json::JsonArray array);
    inline void prependColumn(Json::JsonArray array);
    inline void appendRow(Json::JsonArray array);
    inline void appendColumn(Json::JsonArray array);
};

class Feature
{
public:
    std::string id;
	std::vector<double> bbox;   //! option
	Json::JsonObject properties;
    DataSet dataset;

    Feature();
    Feature(const int index);
    Feature(const Feature &other);
    ~Feature();

    inline Feature &operator = (const Feature &other);
    inline bool operator == (const Feature &other) const;
    inline bool fromDocument(const Json::JsonDocument &jdoc, std::string &errorInfo);
    inline Json::JsonDocument toDocument() const;
};

template <typename T>
inline bool parse(const Json::JsonDocument &, T &, std::string &errorInfo);

template <typename T>
inline Json::JsonDocument serialize(const T &);

template <>
inline bool parse<GPJson::DataSet>(const Json::JsonDocument &jdoc,GPJson::DataSet &dataset,std::string &errorInfo);

template <>
inline bool parse<GPJson::Feature>(const Json::JsonDocument &jdoc,GPJson::Feature &fea,std::string &errorInfo);

template <>
inline bool parse<FeatureCollect>(const Json::JsonDocument &jdoc,FeatureCollect &features,std::string &errorInfo);

inline bool parse(const Json::JsonDocument &jdoc, FeatureCollect &featrues, std::string &errorInfo );

template <>
inline Json::JsonDocument serialize<GPJson::DataSet>(const GPJson::DataSet& element);

template<>
inline Json::JsonDocument serialize<GPJson::Feature>(const GPJson::Feature& element);

template<>
inline Json::JsonDocument serialize<GPJson::FeatureCollect>(const FeatureCollect& element);

inline Json::JsonDocument serialize(const FeatureCollect &features);

}
























#endif // GPJSON_H
