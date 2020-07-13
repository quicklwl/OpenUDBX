# OpenUDBX
开源超图的UDBX数据格式

# 目标
开放超图空间数据的存储格式
以Geopackage数据标准为基础，扩展超图的空间对象格式，以扩展插件方式支持。
技术上依赖Sqlite，提供一套C++的接口，用来访问各种空间数据

# 支持标注空间对象类型（ANNOTATION）
标注对象主要存储标注内容以及标注的坐标。
测试此功能需要用到 DB.Browser.for.SQLite 工具，这个工具下载地址为：``https://sqlitebrowser.org/``
运行DB.Browser.for.SQLite，创建一个数据库，然后在菜单``工具`` -> ``加载扩展`` 中加载编译好的 libudbx.dll

```
--初始化空间库
select GPKG_InitSpatialMetaData();

--创建表
create table province(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province','geom','annotation',4326,0,0);

--写入数据
insert into province(geom) values(ST_GeomFromText('ANNOTATION(116.400 39.900 "北京")', 4326));
insert into province(geom) values(ST_GeomFromText('ANNOTATION(121.47 31.2300 "上海")', 4326));
insert into province(geom) values(ST_GeomFromText('ANNOTATION(113.27 23.1300 "广州")', 4326));
insert into province(geom) values(ST_GeomFromText('ANNOTATION(114.30 30.600 "武汉")', 4326));
insert into province(geom) values(ST_GeomFromText('ANNOTATION(117.2 39.130 "天津")', 4326));
commit;

--查询确认
select st_astext(geom) from province;

--创建空间索引
select GPKG_CreateSpatialIndex('province','geom','id');
	
--范围查询
select st_astext(geom) from province where province.id in (select rtree_province_geom.id  as smid from rtree_province_geom where minx <= 122 and maxx >= 50 and miny <= 40 and maxy >= 30);

```

# 支持参数化点空间对象类型（PARAMETRICPOINT）
参数化点对象主要存储参数化点的XYZ坐标以及点的类型和名称。
测试此功能与标注空间对象类型（ANNOTATION）的步骤相同，只有添加空间字段和写入数据有所不同。

```
--创建表
create table province1(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province1','geom','parametricpoint',4326,1,0);

--写入数据
insert into province1(geom) values(ST_GeomFromText('PARAMETRICPOINT Z (116.4 39.9 1.0 "16" "北京")', 4326));
insert into province1(geom) values(ST_GeomFromText('PARAMETRICPOINT Z (121.47 31.23 2.0 "16" "上海")', 4326));

```

# 支持参数化线空间对象类型（PARAMETRICLINESTRING）
参数化线对象主要存储参数化线串的XYZ坐标以及类型和名称。
测试此功能与标注空间对象类型（ANNOTATION）的步骤相同，只有添加空间字段和写入数据有所不同。

```
--创建表
create table province2(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province2','geom','parametriclinestring',4326,1,0);

--写入数据
insert into province2(geom) values(ST_GeomFromText('PARAMETRICLINESTRING Z (116.4 39.9 1.0 "16" "北京", 117.2 39.13 1.0 "16" "天津")', 4326));
insert into province2(geom) values(ST_GeomFromText('PARAMETRICLINESTRING Z (121.47 31.23 2.0 "16" "上海", 113.27 23.13 2.0 "192" "广州", 114.30 30.6 2.0 "16" "武汉")', 4326));

```

# 支持参数化面空间对象类型（PARAMETRICPOLYGON）
参数化面对象主要存储参数化面环串的XYZ坐标以及类型和名称。
测试此功能与标注空间对象类型（ANNOTATION）的步骤相同，只有添加空间字段和写入数据有所不同。

```
--创建表
create table province3(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province3','geom','parametricpolygon',4326,1,0);

--写入数据
insert into province3(geom) values(ST_GeomFromText('PARAMETRICPOLYGON Z ((116.4 39.9 1.0 "16" "北京", 118.4 39.9 1.0 "16" "", 117.4 40.9 1.0 "16" "", 116.4 39.9 1.0 "16" "北京"))', 4326));
insert into province3(geom) values(ST_GeomFromText('PARAMETRICPOLYGON Z ((115.4 38.9 1.0 "16" "北京", 119.4 38.9 1.0 "16" "北京", 119.4 41.9 1.0 "16" "北京", 115.4 41.9 1.0 "16" "北京", 115.4 38.9 1.0 "16" "北京"),(116.4 39.9 1.0 "16" "", 118.4 39.9 1.0 "16" "", 117.4 40.9 1.0 "16" "", 116.4 39.9 1.0 "16" ""))', 4326));

```

# 支持参数化标注空间对象类型（PARAMETRICANNOTATION）
参数化标注对象主要存储参数化标注的XYZ坐标、类型、名称以及标注内容。
测试此功能与标注空间对象类型（ANNOTATION）的步骤相同，只有添加空间字段和写入数据有所不同。

```
--创建表
create table province4(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province4','geom','parametricannotation',4326,1,0);

--写入数据
insert into province4(geom) values(ST_GeomFromText('PARAMETRICANNOTATION Z (116.4 39.9 1.0 "16" "北京" "北京")', 4326));
insert into province4(geom) values(ST_GeomFromText('PARAMETRICANNOTATION Z (121.47 31.23 2.0 "16" "" "上海")', 4326));

```

# 编译环境

当前是在windows 7 + vs2015 编译通过