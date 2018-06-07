# OTCTrade
## 序言

+ 这个项目是在2015年开发的，一直用到现在，原来很乱的结构，做了很多简化，现在整体来看并不是很多。但是每次回头过来看，我还是会看不懂，以前为什么会这么写，这个原因就像我朋友说的，每次都是没有一个总结，所以才导致的问题么？（我也想和你们一起玩 - JinnTao）OTC Trade：用于场外衍生品实时交易、管理、结算。

## 下载
+ [Armadillo](http://arma.sourceforge.net/)：C++下的Matlab替代品,用于C++矩阵运算库，对于适合matlab的人来说，是非常方便的，配置方式详见官方文档。

+ [Oracle Oci](http://www.oracle.com/technetwork/database/database-technologies/instant-client/overview/index.html)：主要用于c++连接数据库的支持包，注意tnsnames.ora的配置，[ODBC配置](https://www.cnblogs.com/shelvenn/p/3799849.html)该教程为了主要是针对excel连接oracle进行测试用的，如下：
```
    ysptestdb = 
       (DESCRIPTION = 
         (ADDRESS = (PROTOCOL = TCP)(HOST = 172.16.8.60)(PORT = 1521)) 
         (CONNECT_DATA = 
           (SERVER = DEDICATED) 
           (SERVICE_NAME = ysptestdb) 
         ) 
       )
```
+ [QT5.2](https://mirror.tuna.tsinghua.edu.cn/qt/development_releases/qt/5.2/5.2.0-rc1/) ：下载qt5.2

+ third：在该文件夹中已经下载好上述文件，不想重新配置可以直接用，只是github的下载会比较麻烦。

## 构建
* 1、创建build文件夹
* 2、CMAKE 
```
cd build && cmake -G"Visual Studio 11 2012" -DCMAKE_PREFIX_PATH="/path/to/Qt5"
```
* 3、打开build下sln文件 进入Vs2012编译

## 运行
+ 1、resources/input 文件放在项目中
+ 2、dll文件 复制到项目中
+ 3、项目中创建流文件夹 MDflow TDflow
+ 4、创建output/product output/settle output/tradeList 文件夹
+ 5、修改input账户信息，运行OTCTrade.exe。

## 结算
+ 每日结算时间点(15：00)后，复制settle中文件到 excel文件夹 settle.xlsx 中，这部分主要用于场外结算，再把场内交易流水按excel中要求的格式导入。最后在settle.xlsx 中OTC.sheet可以查看的到总的对冲后损益。

## 未来
+ 缺少Risk和Scenario analysis的报表导出，后期维护人员在熟悉系统底层的情况下，可以着重开发这些功能，此外按实际情况在未来重构整个系统（我也一直想重构，可一直没有完整的时间去做），在底层上做更好的优化。