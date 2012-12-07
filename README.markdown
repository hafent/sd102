@mainpage 说明文档

## 使用方法

编译:

	make [all|debug]	
之后将会在`/linux`目录下得到一个`libsd102.so`动态链接库文件.

如果选择`make debug`那么得到的文件名为`libsd102-dbg.so`

安装:
1. 将编译得到的动态链接库文件复制到终端库文件目录(默认为/mnt/nor/lib)目录下.
2. 修改终端规约配置文件`pxxx.config`,在最末尾添加:

	1 libsd102[-dbg].so 
3. 重启终端主程序:
	* 直接重启终端,或者
	* 使用telnet登录到终端结束`kill`主程序进程之后在重新运行它.	
	
## 文件说明:

总目录结构图:

	├── doc/
	│   ├── Flow_chart.dia
	│   └── tou_dat_format
	├── include/
	│   ├── sd102_ctDuid.h
	│   ├── sd102_ctStart.h
	│   ├── sd102_ctUdat.h
	│   ├── sd102.h
	│   ├── sd102_stElement.h
	│   ├── sd102_stFrame.h
	│   └── typedefine.h
	├── linux/
	│   └── Makefile
	├── Makefile
	├── README.markdown
	├── sd102.config
	├── sd102.creator
	├── sd102.creator.user
	├── sd102.doxyfile
	├── sd102.files
	├── sd102.includes
	├── src/
	│   ├── sd102.cpp
	│   └── type.c
	└── wireshark-lua-plug-in/
	    ├── 1.png
	    ├── hello.lua
	    ├── README.markdown
	    └── sd102.pcap
文件含义:
* `Makefile` 顶层makefile文件
* `src/` 源代码目录
* `include/` 头文件目录
* `html/` 帮助文档目录
	* `index.html` 帮助文档首页(使用浏览器打开)
* `doc/` 其他帮助信息
	* `tou_dat_format` 终端电量文件结构说明
* `README.markdown`自述文件(本文件)
* `sd102.doxyfile` 帮助文档(doxygen)生成配置文件
* `doxygen.css` 用于生成帮助文档的级联样式表(css)


## 维护事项

## 开发事项

## 参考资料:
* 地区级102(山东102)规约文本: <https://www.dropbox.com/sh/tm8ovs9mu8dtf3b/rZprPjBjjy>
* 国际标准IEC60870-5系列(英文)以及中国等效国标(中文): <https://www.dropbox.com/sh/btnwt3jkbxwkzna/HsB7_mjywa>

