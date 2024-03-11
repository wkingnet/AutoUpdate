<center><font size=7>AutoUpdate</font></center>

# 简介

用于windows系统的，可以轻松集成的在线自动更新模块。



~~GitHub上面有十几个c++自动更新项目，基本都试了，总感觉缺点什么，只好自己写一个了。~~

# 特点

- 基于C++ 20，win32和STL库编写，不使用MFC/ATL/QT/boost等框架和库。
- 即插即用，AutoUpdate更新模块只需要在你的项目中`include auto_update.h`即可，不需要使用cmake、vcpkg等工具安装或编译。
- 对服务器端要求宽松，可使用任意http服务器或CDN作为在线更新源。

# 安装

