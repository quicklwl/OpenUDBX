# OpenUDBX
开源超图的UDBX数据格式

# 目标

开放超图空间数据的存储格式

以Geopackage数据标准为基础，扩展超图的空间对象格式，以扩展插件方式支持。

技术上依赖Sqlite，提供一套C++的接口，用来访问各种空间数据


# 支持标注空间对象类型（ANNOTATION）

标注对象主要存储标注内容以及标注的坐标。


测试此功能需要用到 DB.Browser.for.SQLite 工具，这个工具下载地址为：``https://sqlitebrowser.org/``



运行DB.Browser.for.SQLite，创建一个数据，然后在菜单``工具`` -> ``加载扩展`` 中加载编译好的 libudbx.dll



```
--初始化空间库
select GPKG_InitSpatialMetaData();

--创建表
create table province(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);

--添加空间字段
select AddGeometryColumn('province','geom','annotation',4326);

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


# 编译环境

当前是在windows 7 + vs2015 编译通过