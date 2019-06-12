# Python下载工具的安装以及使用 #

## 1.安装 ##
windows：
- 安装python3.0工具 ` https://www.python.org/downloads/ ` 下载对应的系统版本安装，并添加到环境变量。
  
linux(ubuntu)：
```
sudo apt-get intall pyhton3
````
- 进入SDK Tools目录中执行下面的命令安装python依赖工具：
  
windows：
```
python -m pip install --user -r requirements.txt
```
linux：
```
python3 -m pip install --user -r requirements.txt
```
如果下载慢可以在命令后面添加` -i https://pypi.tuna.tsinghua.edu.cn/simple `使用国内的源。
## 2.使用 ##
- 进入Tools目录中执行下面的命令进行image下载：
  
windows：
```
python download.py [COM] [image]
`````
如果不输入参数则使用默认参数COM default：`COM1`; image default：`../Bin/WM_W600_GZ.img`

eg：python download.py COM5 ../Bin/WM_W600_GZ.img

linux：
``` python
python3 download.py [COM] [image]
```
如果不输入参数则使用默认参数COM default：`ttyUSB0`; image default：`../Bin/WM_W600_GZ.img`

eg：python3 download.py ttyUSB1 ../Bin/WM_W600_GZ.img

-输出log如下：
```
serial open success！com: COM9, baudrate: 115200;
please restart device!
serial into high speed mode
start download ./Bin/WM_W600_GZ.img
please wait for download....
0% [############################# ] 100% | ETA: 00:00:00
download ./Bin/WM_W600_GZ.img image success!
```


