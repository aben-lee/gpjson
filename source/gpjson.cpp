#include "gpjson.h"
#include "json.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <variant>
#include <utility>

namespace GPJson {

    FeatureCollect::FeatureCollect()
    {
        properties = Json::JsonObject();
    }
    DataSet::DataSet():type("sheet"){
        attributes = Json::JsonArray();
    }
    DataSet::DataSet(const std::string &type){
        this->type = type;
        attributes = Json::JsonArray();
    }
    DataSet::DataSet(const DataSet &other):type(other.type),attributes(other.attributes),source(other.source){}
    inline DataSet &DataSet::operator=(const DataSet &other){
        type = other.type;
        attributes = other.attributes;
        source = other.source;
        return *this;
    }
    DataSet::~DataSet(){}
    inline bool DataSet::operator==(const DataSet &other) const {
        return type == other.type && attributes == other.attributes && source == other.source;
    }

    inline int DataSet::rowCount() const
    {
        return source.size();
    }

    int DataSet::columnCount() const
    {
        return source.at(0).size();
    };

    inline Json::JsonValue DataSet::at (int row, int column) const{
        if(row < 0 || row >= int(source.size()) || column < 0 || column >= source.at(0).count())
            return Json::JsonValue();
        else{
            return source.at(row).at(column);
        }
    }
    inline bool DataSet::replace(int row, int column, const Json::JsonValue & value){
        if(row < 0 || row  >= int(source.size()) ||  column < 0 || column >= source.at(0).count() )
            return false;
        else {
            Json::JsonArray array = source.at(row);
            array.replace(column, value);
            source.assign(row,array);
            return true;
        }
    }
    inline Json::JsonArray DataSet::row(int index) const{
        if( ( index < 0 ) || ( index >= int(source.size()) ) )
        {
            Json::JsonArray jsonarray;
            jsonarray.append("error row index");
            return jsonarray;
        }
        return  source.at(index);
    }
    inline Json::JsonArray DataSet::column(int index) const{
        if((index < 0) || (index >= source.at(0).count()))
        {
            Json::JsonArray jsonarray;
            jsonarray.append("error column index");
            return jsonarray;
        }
        Json::JsonArray result;
        for(int i = 0;i < int(source.size()); ++i)
            result.append( source.at(i).at(index) );
        return result;
    }
    inline void DataSet::insertRow(int row,Json::JsonArray array){
        if(row <= 0) row = 0;
        if(row >= int(source.size())) row = int(source.size());
        source.insert(source.begin()+row-1,array);

    }
    inline void DataSet::insertColumn(int column,Json::JsonArray array){
        if(column <= 0) column = 0;
        if(column >= source.at(0).count()) column = source.at(0).count();
        std::vector<rowData>::iterator it;
        for(it = source.begin();it != source.end();++it)
            it->insert(column,Json::JsonValue());
        int rows = std::min(int(source.size()),array.count());
        for(int r = 0;r < rows; ++r)
            replace(r,column,array.at(r));
    }

    void DataSet::prependRow(Json::JsonArray array)
    {
        insertRow(0,array);
    }

    void DataSet::prependColumn(Json::JsonArray array)
    {
        insertColumn(0,array);
    }

    void DataSet::appendRow(Json::JsonArray array)
    {
        insertRow(rowCount(),array);
    }

    void DataSet::appendColumn(Json::JsonArray array)
    {
        insertColumn(columnCount(), array);
    }

    Feature::Feature(){
        properties = Json::JsonObject();
        dataset = DataSet();
    }

    Feature::Feature(const int index){
        id.push_back(index);
        properties = Json::JsonObject();
        dataset = DataSet();
    }

    Feature::Feature(const Feature &other){
        id = other.id;
        dataset = other.dataset;
        properties = other.properties;
    }

    inline Feature &Feature::operator = (const Feature &other)
    {
        id = other.id;
        dataset = other.dataset;
        properties = other.properties;
        return *this;
    }

    inline bool Feature::operator == (const Feature &other) const{
        return id == other.id && dataset == other.dataset && properties == other.properties;
    }
    template <>
    bool parse(const Json::JsonDocument &jdoc, DataSet &dataset, std::string &errorInfo)
    {
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
            auto type_1 = (jdoc.object().value("type").toString());
            std::transform(type_1.begin(),type_1.end(),type_1.begin(),::tolower); // 转换成小写
            const auto &type = type_1;
            if (type == "sheet" || type == "binary" )
            {
                if (!jdoc.object().contains("attributes")){
                    errorInfo = "DataSet must have a attributes object because of the data type is" + type ;
                    return false;
                }
            }

            if ( type == "matrix" ||  type == "uri" || type == "sheet" || type == "binary" )
            {

                if (jdoc.object().contains("attributes") && jdoc.object().value("attributes").isArray()){
                    dataset.attributes = jdoc.object().value("attributes").toArray();
                }
                else {
                    errorInfo = "the dimension value of dataset  MUST be a json array" ;
                    return false;
                }

                dataset.source.clear();
                if(jdoc.object().value("source").isArray())
                {
                     for (auto v : jdoc.object().value("source").toArray())
                     {
                         if(v.isArray())  // array of arrays
                            dataset.source.push_back(v.toArray());
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
    inline bool parse<GPJson::Feature>(const Json::JsonDocument &jdoc,GPJson::Feature &fea,std::string &errorInfo){
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

            if (!parse<GPJson::DataSet>(Json::JsonDocument(jdoc.object().value("DataSet").toObject()),fea.dataset,errorInfo) )
            {
                return false;
            }

            if (jdoc.object().contains("id"))
                fea.id = jdoc.object().value("id").toString();

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
    inline bool parse<FeatureCollect>(const Json::JsonDocument &jdoc,FeatureCollect &features,std::string &errorInfo)
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

        auto type = jdoc.object().value("type").toString();
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        features.clear();
        if (type == "featurecollection") {
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

            Json::JsonArray array = json_features.toArray();

            for (Json::JsonValue v : array) {
                std::shared_ptr<GPJson::Feature> fea = std::shared_ptr<GPJson::Feature>(new GPJson::Feature);
                if(!parse<GPJson::Feature>(Json::JsonDocument(v.toObject()),*fea,errorInfo))
                    return false;
                features.list.push_back(fea);
            }
        }
        else if (type == "feature")
        {
            std::shared_ptr<GPJson::Feature> fea = std::shared_ptr<GPJson::Feature>(new GPJson::Feature);
            if(!parse<GPJson::Feature>(jdoc,*fea,errorInfo))
                return false;
            features.list.push_back(fea);
            return true;
        }
        else {
            errorInfo = "Unkown type value to FeatureCollection";
            return false;
        }

        if (jdoc.object().contains("id")) {
            features.id = jdoc.object().value("id").toString();
        }

        if (jdoc.object().contains("properties")) {
            features.properties = jdoc.object().value("properties").toObject();
        }
        return true;
    }

    inline bool parse(const Json::JsonDocument &jdoc, FeatureCollect &featrues, std::string &errorInfo ){
        return parse<FeatureCollect>(jdoc,featrues,errorInfo);
    }

    template<>
    inline Json::JsonDocument serialize<GPJson::DataSet>(const GPJson::DataSet& element){
        Json::JsonObject obj;
        obj.insert("type",element.type);
        std::string str = element.type;
        std::transform(str.begin(),str.end(),str.begin(),::tolower);
        if(str == "sheet" || str == "binary" || str == "uri" || str == "matrix" )
        {
            obj.insert("attributes",element.attributes);
            Json::JsonArray array;
            for(GPJson::rowData row : element.source)
            {
                array.push_back(row);
            }
            obj.insert("source",array);
        }
        else if( str == "sequence"  )
        {
            obj.insert("attributes",element.attributes);
            obj.insert("source",Json::JsonValue::Null);
        }
        return Json::JsonDocument(obj);
    }

    template<>
    inline Json::JsonDocument serialize<GPJson::Feature>(const GPJson::Feature &element)
    {
        Json::JsonObject obj;
        obj.insert("type","Feature");

        Json::JsonObject obj2;
        obj2.insert("type",element.dataset.type);
        std::string str = element.dataset.type;
        std::transform(str.begin(),str.end(),str.begin(),::tolower);
        if(str == "sheet" || str == "binary" || str == "uri" || str == "matrix" )
        {
            obj2.insert("attributes",element.dataset.attributes);
            Json::JsonArray array;
            for(GPJson::rowData row : element.dataset.source)
            {
                array.push_back(row);
            }
            obj2.insert("source",array);
        }
        else if( str == "sequence"  )
        {
            obj.insert("attributes",element.dataset.attributes);
            obj.insert("source",Json::JsonValue::Null);
        }
        return Json::JsonDocument(obj);

        obj.insert("DataSet",obj2);
        obj.insert("properties",element.properties);
        if(element.id != "")
        {
            obj.insert("id",element.id);
        }
        return Json::JsonDocument(obj);
    }

    template<>
    inline Json::JsonDocument serialize<GPJson::FeatureCollect>(const FeatureCollect& element){
        Json::JsonObject obj;
        obj.insert("type","FeatureCOllection");
        obj.insert("properties",element.properties);
        Json::JsonArray array;
        for(auto fea : element.list){
            array.push_back(serialize<GPJson::Feature>(*fea).object());
        }
        obj.insert("features",array);
        if(element.id != ""){
            obj.insert("id",element.id);
        }
        return Json::JsonDocument(obj);
    }

    Json::JsonDocument serialize(const FeatureCollect &features)
    {
        return Json::JsonDocument(serialize<FeatureCollect>(features));
    }

    inline void GPJson::FeatureCollect::clear()
    {
        id = "";
        properties = Json::JsonObject();
        list.clear();
    }

    inline bool GPJson::FeatureCollect::fromDocument(const Json::JsonDocument &jdoc,std::string &errorInfo){
        return parse<GPJson::FeatureCollect>(jdoc,*this,errorInfo);
    }

    inline Json::JsonDocument GPJson::FeatureCollect::toDocument() const{
        return Json::JsonDocument(serialize<GPJson::FeatureCollect>(*this));
    }

    inline bool GPJson::Feature::fromDocument(const Json::JsonDocument &jdoc,std::string &errorInfo)
    {
        return parse<GPJson::Feature>(jdoc,*this,errorInfo);
    }

    inline Json::JsonDocument GPJson::Feature::toDocument() const
    {
        return Json::JsonDocument(serialize<GPJson::Feature>(*this));
    }
}





















