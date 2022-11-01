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

#include <QApplication>
#include <QDebug>
#include "GPJson.hpp"
#include <QFileDialog>
#include <QSaveFile>
#include <QString>

#include <iostream>
#include "source/gpjson.h"
#include <string>
#include <utility>
#include <vector>

using namespace GPJson;

void test_gpjson(){
    { ///测试1：测试解析from and to Dataset
        QString str1 = "{\"type\": \"Sheet\", \"fields\" : [\"Time/second\", \"X/meter\", \"Y/meter\" ],\n"
                      "   \"data\" : [[\"10.0\", 236, 12.4], [\"30.0\", 234,23.4],[\"50.0\",457,34.7 ]] \n "
                      "}";

        Dataset element = parse<Dataset>(str1.toUtf8());

        qDebug() << "type :" << element.type;
        qDebug() << "fields :" << element.fields;
        qDebug() << "data :" << element.data;

        QJsonObject obj = convert<Dataset>(element);
        Dataset element2 = parse<Dataset>(QJsonDocument(obj).toJson());
        if(element.type == element2.type && element.fields == element2.fields && element.data == element2.data)
            qDebug() << "convert to json" << QJsonDocument(obj).toJson();

    }
    { ///测试2：测试Dataset对象的json字符串

        QString str2 = "{\"type\": \"Binary\", \"fields\" : [\"Timesecond.nc\"], \n"
                      "   \"data\" : [[\"10.0\"]] }";


        Dataset element = parse<Dataset>(str2.toUtf8());

        qDebug() << "type :" << element.type;
        qDebug() << "fields :" << element.fields;
        qDebug() << "data :" << element.data;

        QJsonObject obj = convert<Dataset>(element);
        Dataset element2 = parse<Dataset>(QJsonDocument(obj).toJson());
        if(element.type == element2.type && element.fields == element2.fields && element.data == element2.data)
            qDebug() << "convert to json" << QJsonDocument(obj).toJson();
    }
    { ///测试3：测试Feature对象的json字符串

        QString str3 = "{ \"type\": \"Feature\", \"properties\": { \"prop0\": \"value0\" }, \n"
                       " \"dataset\" :{\"type\": \"Sheet\", \"fields\" : [\"Time/second\", \"X/meter\", \"Y/meter\" ],\n"
                       "   \"data\" : [[\"10.0\", 236, 12.4], [\"30.0\", 234,23.4],[\"50.0\",457,34.7 ]] \n "
                       "} }";


        Feature element = parse<Feature>(str3.toUtf8());

        qDebug() << "ID :" << element.id;
        qDebug() << "properties :" << element.properties;
        QJsonObject obj = convert<Feature>(element);
        Feature element2 = parse<Feature>(QJsonDocument(obj).toJson());
        if(element.id == element2.id && element.properties == element2.properties )
           qDebug() << "convert to json" << QJsonDocument(obj).toJson();

    }
    { ///测试4：测试FeatureCollection对象转为json字符串

       QString str4 = 	"{ \"type\":\"FeatureCollection\",\n"
                        " \"id\" : 123645789,\n"
                        " \"features\":[{\"type\": \"Feature\","
                        " \"properties\": { \"prop0\": \"value0\"},"
                        "  \"dataset\" :{\"type\": \"Sheet\","
                        " \"fields\" : [\"Time/second\", \"X/meter\", \"Y/meter\" ],"
                        " \"data\" : [[\"10.0\", 236, 12.4], [\"30.0\", 234,23.4],[\"50.0\",457,34.7 ]]}},"
                        " {\"type\": \"Feature\","
                        " \"properties\": { \"prop0\": \"value0\"},"
                        " \"dataset\" :{\"type\": \"Binary\","
                        " \"fields\" : [\"abc.nc\"],"
                        " \"data\" : [[\"1234567893369\"]]}}"
                        "]}";

                featureCollect element = parse(str4.toUtf8());
                QJsonObject obj = convert<featureCollect>(element);
                featureCollect element2 = parse(serialize(element));
                if(element.at(0).dataset.type == element2.at(0).dataset.type)
                      qDebug() << "the object is == " << obj;

                // 测试将obj存文件

           QString  fileName = QFileDialog::getSaveFileName(0,
                    ("Save "), "", ("JSON File (*.json)"));

           QFile file(fileName);
           if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
               return;
           file.write(QJsonDocument(obj).toJson());
    }
}

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;  
	test_gpjson();
    return 0;
}
