# OTCTrade
## ����

+ �����Ŀ����2015�꿪���ģ�һֱ�õ����ڣ�ԭ�����ҵĽṹ�����˺ܶ�򻯣������������������Ǻܶࡣ����ÿ�λ�ͷ���������һ��ǻῴ��������ǰΪʲô����ôд�����ԭ�����������˵�ģ�ÿ�ζ���û��һ���ܽᣬ���Բŵ��µ�����ô������Ҳ�������һ���� - JinnTao��OTC Trade�����ڳ�������Ʒʵʱ���ס��������㡣

## ����
+ [Armadillo](http://arma.sourceforge.net/)��C++�µ�Matlab���Ʒ,����C++��������⣬�����ʺ�matlab������˵���Ƿǳ�����ģ����÷�ʽ����ٷ��ĵ���

+ [Oracle Oci](http://www.oracle.com/technetwork/database/database-technologies/instant-client/overview/index.html)����Ҫ����c++�������ݿ��֧�ְ���ע��tnsnames.ora�����ã�[ODBC����](https://www.cnblogs.com/shelvenn/p/3799849.html)�ý̳�Ϊ����Ҫ�����excel����oracle���в����õģ����£�
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
+ [QT5.2](https://mirror.tuna.tsinghua.edu.cn/qt/development_releases/qt/5.2/5.2.0-rc1/) ������qt5.2

+ third���ڸ��ļ������Ѿ����غ������ļ��������������ÿ���ֱ���ã�ֻ��github�����ػ�Ƚ��鷳��

## ����
* 1������build�ļ���
* 2��CMAKE 
```
cd build && cmake -G"Visual Studio 11 2012" -DCMAKE_PREFIX_PATH="/path/to/Qt5"
```
* 3����build��sln�ļ� ����Vs2012����

## ����
+ 1��resources/input �ļ�������Ŀ��
+ 2��dll�ļ� ���Ƶ���Ŀ��
+ 3����Ŀ�д������ļ��� MDflow TDflow
+ 4������output/product output/settle output/tradeList �ļ���
+ 5���޸�input�˻���Ϣ����easylog.conf�ļ����Ƶ���ĿĿ¼�У�����OTCTrade.exe��
+ 6��ע��Ӧ��Debug�����µ��ԣ�release�Ļ���ע�⣨tdcpi\mdcpi������Ҫ��OT bin·�����뵽����������

## ����
+ ÿ�ս���ʱ���(15��00)�󣬸���settle���ļ��� excel�ļ��� settle.xlsx �У��ⲿ����Ҫ���ڳ�����㣬�ٰѳ��ڽ�����ˮ��excel��Ҫ��ĸ�ʽ���롣�����settle.xlsx ��OTC.sheet���Բ鿴�ĵ��ܵĶԳ�����档

## δ��
+ ȱ��Risk��Scenario analysis�ı�����������ά����Ա����Ϥϵͳ�ײ������£��������ؿ�����Щ���ܣ����ⰴʵ�������δ���ع�����ϵͳ����Ҳһֱ���ع�����һֱû��������ʱ��ȥ�������ڵײ��������õ��Ż���