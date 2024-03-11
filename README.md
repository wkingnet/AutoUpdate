<center><font size=7>AutoUpdate</font></center>

# 简介

用于windows系统的可以轻松集成的在线自动更新模块。

~~GitHub上面有十几个c++自动更新项目，基本都试了，总感觉缺点什么，只好自己写一个了。~~

# 特点

- 基于C++ 20，win32和STL库编写，不使用MFC/ATL/QT/boost等框架和库。
- 即插即用，只需要在你的项目中`include AutoUpdate.h`即可，无需额外编译。
- 对提供更新的服务器端要求宽松，可使用任意http(s)服务器或CDN作为在线更新源。

# 安装
1. 下载最新Release版本，解压缩。
2. 复制AutoUpdate文件夹内的AutoUpdate.h、tinyxml2.h和tinyxml2.cpp三个文件到你自己的项目。
3. 复制Updater.exe文件到你项目**生成**的主程序文件夹。
4. 在你的项目代码`include "AutoUpdate.h"`。
5. 在需要执行自动更新模块的代码位置加入代码`AutoUpdate::AutoUpdate(nullptr, L"Updater.exe", L"*xml_url*", true);`（*xml_url*请自行替换为在线XML文件的URL）
6. 继续看使用部分

# 使用
## 配置、生成xml
### 首次运行
1. 选择更新目录。也即你程序最新版本生成的目录。
1. 填写更新URL。这个URL是集成到你程序的AutoUpdate模块从什么位置下载更新文件，可与update.xml的URL一致，也可不一致。比如update.xml的URL设定为固定网址http://123.com/update.xml，更新URL可填写为CDN地址，或者填写你自己家的IP地址，从而提高下载速度，或降低带宽费用。

### 添加文件、添加目录
点击对应按钮。

### 删除所选
列表里勾选要删除的行，点击按钮。

### 保存配置、载入配置
点击按钮会在config.exe同目录生成config.cfg文件。且config.exe运行时会自动读取config.cfg文件内容。

### 执行、解压、覆盖
勾选要操作的行，然后点击对应按钮。

执行：更新程序完成更新后，会自动执行该行对应的文件。一般建议你的主程序勾选执行，这样可以自动更新完成后，自动运行主程序。解压缩或者增删改注册表操作，也可以做成批处理文件然后勾选执行。

解压：目前此功能未完成。

覆盖：此动作默认勾选，覆盖操作表示更新程序下载更新文件后会覆盖客户端本地对应文件。有些文件比如配置文件可以不覆盖，这样可以保留客户修改过的设置。

### 生成xml文件
点击按钮会在config.exe同目录生成xml文件（默认文件名为update.xml），并将此xml文件发布。


## 发布xml
将生成的xml文件上传到http(s)网站或CDN，需要让客户端主程序的AutoUpdate模块能访问到即可。

发布后，用实际xml文件的URL替换代码中的\*xml_url*
```
AutoUpdate::AutoUpdate(nullptr, L"Updater.exe", L"*xml_url*", true);
```