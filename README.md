# OpenUDBX
开源超图的UDBX数据格式

# 目标

开放超图空间数据的存储格式

以Geopackage数据标准为基础，扩展超图的空间对象格式，以扩展插件方式支持。

技术上依赖Sqlite，提供一套C++的接口，用来访问各种空间数据


<<<<<<< HEAD
# 支持标注空间对象类型（ANNOTATION）

标注对象主要存储标注内容以及标注的坐标。

```
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
=======
支持标注空间对象类型
ANNOTATION


create table testa(id int);
select AddGeometryColumn('testa','geom','annotation',4326);
insert into testa values(1, ST_GeomFromText('ANNOTATION(-121.34900 46.65100 "北京")', 4326));
insert into testa values(2, ST_GeomFromText('ANNOTATION(121.34900 37.65100 "beijing")', 4326));
insert into testa values(3, ST_GeomFromText('ANNOTATION(149.34900 6.65100 "上海")', 4326));
insert into testa values(4, ST_GeomFromText('ANNOTATION(90.34900 6.65100 "广州")', 4326));
insert into testa values(5, ST_GeomFromText('ANNOTATION(121.34900 40.65100 "天津")', 4326));
insert into testa values(6, ST_GeomFromText('ANNOTATION(110.34900 70.65100 "武汉")', 4326));
commit;

select st_astext(geom) from testa ;

select GPKG_CreateSpatialIndex('testa','geom','id');
	
select st_astext(geom) from testa where testa.id in (select rtree_testa_geom.id  as smid from rtree_testa_geom where minx <= 122 and maxx >= 50 and miny <= 80 and maxy >= 40);
>>>>>>> 8a6fe423238557b461695ce4741280a96f5210c0
