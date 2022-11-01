/****************************************************************************
**
** Copyright (C) Eageo Information & Exploration Technology Co.,Ltd.,
**
** Use of this file is limited according to the terms specified by
** Eageo Exploration & Information Technology Co.,Ltd.  Details of
** those terms are listed in licence.txt included as part of the distribution
** package ** of this file. This file may not be distributed without including
** the licence.txt file.
**
** Contact aben.lee@foxmail.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GPJSON_H
#define GPJSON_H

#include <QVector>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QVariant>
#include <QHash>
#include <QJsonParseError>
#include <QtGlobal>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>

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

struct DataSet;
struct Feature;
struct FeatureCollect;
using rowData = QJsonArray;

enum FeatureType {Unknow = 0x00, Geometry = 0x01,Dataset = 0x02, GeometryDataset = 0x03};
enum DatasetType {Sheet = 0x11, Matrix = 0x12, Binary = 0x13, URI = 0x14};
enum Geometry    {Point = 0x21, MultiPoint = 0x22,LineString = 0x23, MultiLineString = 0x24, Polygon = 0x25, MultiPolygon = 0x26,GeometryCollection = 0x27 };

//! convet the fectionCollect container to QJsonDocoment
inline QJsonDocument serialize(const FeatureCollect& features); //! Stringify any GPJSON type.

//! serialize QJsonDocumetn which read Gpjson data to FeatrueCollect,
//! if successful will return true, or return false and in return errorInfo
inline bool parse(const QJsonDocument &jdoc,FeatureCollect &featrues, QString &errorInfo );    //! Parse any GPJSON type.

struct FeatureCollect
{
    // Qt中容器类应该如何存储对象（最好使用对象指针类型，如：QList<TestObj*>，而不要使用 QList<TestObj> 这样的定义，建议采用 智能指针QSharedPointer）
    // 参考: https://www.cnblogs.com/findumars/p/11172521.html
    QList<QSharedPointer<Feature>> list;
    QJsonObject common;   //! general properties object of FeatureCollect
    QByteArray id;

    FeatureCollect() : id(""){ common = QJsonObject(); }

    inline void clear();
    inline bool fromDocument(const QJsonDocument &jdoc,QString &errorInfo);
    inline QJsonDocument toDocument() const;
};

struct DataSet
{
    QString type;           //! refer to gpjson specification
    QJsonArray dimensions;
    QVector<rowData> source;

    DataSet() : type("Sheet") {dimensions = QJsonArray(); /*source.append(QJsonArray({""}));*/ }
    DataSet(const QString &type) {this->type = type;
                                  dimensions = QJsonArray(); /*source.append(QJsonArray({""}));*/}
    DataSet(const DataSet &other) : type(other.type)
            , dimensions(other.dimensions), source(other.source) {}
        ~DataSet() {}

    inline DataSet &operator = (const DataSet &other) {
        type = other.type; dimensions = other.dimensions; source = other.source;
        return *this;
    }
    inline bool operator == (const DataSet &other) const {
        return type== other.type && dimensions == other.dimensions && source == other.source;
    }

    inline QJsonValue at ( int row, int column) const {
        if(row < 0 || row  >= source.count() ||  column < 0 || column >= source.at(0).count() )
            return QJsonValue();
        else
            return source.at(row).at(column);
    }

    inline bool replace ( int row, int column, const QJsonValue & value ){
        if(row < 0 || row  >= source.count() ||  column < 0 || column >= source.at(0).count() )
            return false;
        else {
            QJsonArray array = source.at(row);
            array.replace(column, value);
            source.replace(row,array);
            return true;
        }
    }

    inline QJsonArray row(int index) const {
         if( ( index < 0 ) || ( index >= source.count() ) )
             return QJsonArray() << "error row index";
         return  source.at(index);
    }

    inline QJsonArray column(int index) const {
         if( (index < 0 ) || ( index >= source.at(0).count() ) )
             return QJsonArray()<< "error column index";
         QJsonArray result;
         for( int i = 0; i < source.count(); ++i )
             result << source.at(i).at(index);
         return result;
    }

    inline void insertRow(int row, QJsonArray array){
        if( row <=0 ) row = 0;
        if(row>=source.count())  row = source.count();
        source.insert(row,array);
    }

    inline void insertColumn(int column, QJsonArray array){
        if(column <= 0) column = 0;
        if(column >= source.at(0).count() ) column = source.at(0).count();

        QVector<rowData>::iterator it;
        for(it = source.begin(); it != source.end(); ++it)
            it->insert(column,QJsonValue());  // reserve the space

        int rows = qMin( source.count(), array.count() );
        for( int r = 0; r < rows; ++r)
            replace(r,column,array.at(r));
    }
};


struct Feature
{
    QByteArray id;          //! option in feture object of gpjson
    DataSet dataset;  //! refer to DataSet object in feture object of gpjson
    QJsonObject properties; //! refer to properties object in feture object of gpjson

    Feature() : id(""){ properties = QJsonObject();  dataset = DataSet(); }
    Feature(const QByteArray &index) : id(index){ properties = QJsonObject();  dataset = DataSet(); }
    Feature(const Feature &other) : id(other.id),dataset(other.dataset),properties(other.properties) {}
        ~ Feature() {}

    inline Feature &operator = (const Feature &other) {
        id = other.id; dataset = other.dataset; properties = other.properties;
        return *this;
    }
    inline bool operator == (const Feature &other) const {
        return id == other.id && dataset == other.dataset && properties == other.properties;
    }

    inline bool fromDocument(const QJsonDocument &jdoc, QString &errorInfo);
    inline QJsonDocument toDocument() const;
};

///////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool parse(const QJsonDocument &, T &, QString &errorInfo);

template <typename T>
inline QJsonDocument serialize(const T &);

template <>
inline bool parse<GPJson::DataSet>(const QJsonDocument &jdoc,GPJson::DataSet &datset,QString &errorInfo)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);
    if ( jdoc.isNull() || jdoc.isEmpty()) {
        errorInfo = "document is Null of emppty";
        return false;;
    }

    if (!jdoc.isObject()) {
        errorInfo = "DataSet must be an object";
        return false;
    }

    if (!jdoc.object().contains("type")) {
        errorInfo = "DataSet must have a type property";
        return false;
    }

    if (!jdoc.object().contains("source")) {
        errorInfo = "DataSet must have a source key";
        return false;
    }

    const auto &type = jdoc.object().value("type").toString().toLower();
    if (type == "sheet" || type == "binary" )
    {
        if (!jdoc.object().contains("dimensions")){
            errorInfo = "DataSet must have a dimensions object because of the data type is" + type ;
            return false;
        }
    }

    if ( type == "matrix" ||  type == "uri" || type == "sheet" || type == "binary" )
    {

        if (jdoc.object().contains("dimensions") && jdoc.object().value("dimensions").isArray()){
            datset.dimensions = jdoc.object().value("dimensions").toArray();
        }
        else {
            errorInfo = "the dimension value of dataset  MUST be a json array" ;
            return false;
        }

        datset.source.clear();
        if(jdoc.object().value("source").isArray())
        {
             for (auto v : jdoc.object().value("source").toArray())
             {
                 if(v.isArray())  // array of arrays
                    datset.source.push_back(v.toArray());
//                 if(v.isObject() && jdoc.object().contains("dimensions")) // array of objects
//                 {
//                     QJsonArray arr;
//                    for( int i = 0 ; i < datset.dimensions.count(); i++ )
//                    {
//                        arr << v.toObject().value(datset.dimensions[i].toString());
//                    }
//                    datset.source.push_back(arr);

//                 }
//                 else{
//                   errorInfo = "Unknow the dataset source json objcet" ;
//                   return false;
//                 }
             }
             return true;
        }
        else
        {
           errorInfo = "the source value of dataset MUST be a json array" ;
           return false;
        }

    }

    errorInfo = "Unknow the dataset json objcet" ;
    return false;
}

template <>
inline bool parse<GPJson::Feature>(const QJsonDocument &jdoc,GPJson::Feature &fea,QString &errorInfo)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);
    if ( jdoc.isNull() || jdoc.isEmpty()) {
        errorInfo = "document is Null of emppty";
        return false;;
    }

    if (!jdoc.isObject()) {
        errorInfo = "Feature must be an object";
        return false;
    }

    if (!jdoc.object().contains("type")) {
        errorInfo = "Feature must have a type object";
        return false;
    }

    if (jdoc.object().value("type").toString() != "Feature") {
        errorInfo = "The type value of Feature must = Feature";
        return false;
    }

    if (!jdoc.object().contains("DataSet")) {
        errorInfo = "Feature must have a DataSet property";
        return false;
    }

    if (!parse<GPJson::DataSet>(QJsonDocument(jdoc.object().value("DataSet").toObject()),fea.dataset,errorInfo) )
    {
        return false;
    }

    if (jdoc.object().contains("id"))
        fea.id = jdoc.object().value("id").toVariant().toByteArray();

    if (!jdoc.object().contains("properties")) {
        errorInfo = "Feature must have a properties property";
        return false;
    }

    if (!jdoc.object().value("properties").isObject() ){
        errorInfo = "Feature's properties property value of Feature shoulb be object ";
        return false;
    }
    else
        fea.properties = jdoc.object().value("properties").toObject();

    return true;
}
template <>
inline bool parse<FeatureCollect>(const QJsonDocument &jdoc,FeatureCollect &features,QString &errorInfo)
{
//    auto const& jdoc = QJsonDocument::fromJson(json);

    if ( jdoc.isNull() || jdoc.isEmpty()) {
        errorInfo = "document is Null of emppty";
        return false;;
    }

    if (!jdoc.isObject()) {
        errorInfo = "GPJson must be an object";
        return false;
    }

    if (!jdoc.object().contains("type")) {
        errorInfo = "GPJson must have a type object";
        return false;
    }

    const auto &type = jdoc.object().value("type").toString();
    features.clear();
    if (type == "FeatureCollection") {
        if (!jdoc.object().contains("features"))
        {
            errorInfo = "FeatureCollection must have a features property";
            return false;
        }

        const auto &json_features = jdoc.object().value("features");

        if (!json_features.isArray()){
            errorInfo = "FeatureCollection features property must be an array";
            return false;
        }

        QJsonArray array = json_features.toArray();

        for (QJsonValue v : array) {
            QSharedPointer<GPJson::Feature> fea = QSharedPointer<GPJson::Feature>(new GPJson::Feature);
            if(!parse<GPJson::Feature>(QJsonDocument(v.toObject()),*fea,errorInfo))
                return false;
            features.list.append(fea);
        }
    }
    else if (type == "Feature")
    {
        QSharedPointer<GPJson::Feature> fea = QSharedPointer<GPJson::Feature>(new GPJson::Feature);
        if(!parse<GPJson::Feature>(jdoc,*fea,errorInfo))
            return false;
        features.list.append(fea);
        return true;
    }
    else {
        errorInfo = "Unkown type value to FeatureCollection";
        return false;
    }

    if (jdoc.object().contains("id")) {
        features.id = jdoc.object().value("id").toString().toUtf8();
    }

    if (jdoc.object().contains("common")) {
        features.common = jdoc.object().value("common").toObject();
    }
    return true;

}

inline bool parse(const QJsonDocument &jdoc, FeatureCollect &featrues, QString &errorInfo )
{
    return parse<FeatureCollect>(jdoc,featrues,errorInfo);
}

template <>
inline QJsonDocument serialize<GPJson::DataSet>(const GPJson::DataSet& element)
{
    QHash<QString, QVariant> result;
    result["type"]=element.type;

    QString str = element.type.toLower();
    if(str == "sheet" || str == "binary" || str == "uri" || str == "matrix" )
    {
        result["dimensions"] = element.dimensions;
        QJsonArray array;
        for (GPJson::rowData row : element.source )
        {
           array << row;
        }
        result["source"] = array;
    }

    return QJsonDocument(QJsonObject::fromVariantHash(result));
}

template <>
inline QJsonDocument serialize<GPJson::Feature>(const GPJson::Feature& element)
{
    QHash<QString, QVariant> result;
    result["type"] = "Feature";
    result["DataSet"] = serialize<GPJson::DataSet>(element.dataset);
    result["properties"] = element.properties;
    if (!element.id.isNull()) {
        result["id"] = element.id;
    }
   return QJsonDocument(QJsonObject::fromVariantHash(result));
}

template <>
inline QJsonDocument serialize<FeatureCollect>(const FeatureCollect& element)
{
    QHash<QString, QVariant> result;
    result["type"] = "FeatureCollection";
    result["common"] = element.common;
    QJsonArray array;
    for(auto fea : element.list) {
        array << serialize<GPJson::Feature>(*fea).object();
    }
    result["features"] = array;
    if (!element.id.isNull()) {
        result["id"] = element.id;
    }
    return QJsonDocument(QJsonObject::fromVariantHash(result));
}

inline QJsonDocument serialize(const FeatureCollect &features)
{
   return QJsonDocument(serialize<FeatureCollect>(features));
}

///////////////////////////////////////////////////////////////////////////////
inline void FeatureCollect::clear()
{
    id = QByteArray();
    common = QJsonObject();

    for(auto it : list)
        it.clear();
    list.clear();
}

inline bool FeatureCollect::fromDocument(const QJsonDocument &jdoc, QString &errorInfo)
{
    return parse<GPJson::FeatureCollect>(jdoc,*this,errorInfo);
}

inline QJsonDocument FeatureCollect::toDocument() const
{
    return QJsonDocument(serialize<GPJson::FeatureCollect>(*this));;
}

inline bool Feature::fromDocument(const QJsonDocument &jdoc, QString &errorInfo)
{
    return parse<GPJson::Feature>(jdoc,*this,errorInfo);
}

inline QJsonDocument Feature::toDocument() const
{
   return QJsonDocument(serialize<GPJson::Feature>(*this));
}
}

Q_DECLARE_TYPEINFO( GPJson::Feature, Q_MOVABLE_TYPE );

#endif // GPJSON_H
