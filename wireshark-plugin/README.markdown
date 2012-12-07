@page 规约解析插件

本插件是基于 wireshark 网络抓包软件的.

## 使用方法
1. 安装 wireshark 网络抓包软件.支持M\$ Windows(TM)和Linux(R)操作系统
2. 将 `hello.lua` 文件复制到 wireshark 目录下.  
	* Linux默认为 `/usr/share/wireshark` (需要权限)
	* Window默认在... 自己找找 :P
3. 修改 wireshark 目录下的 init.lua 文件.在最末尾(注意文中注释)添加:

	dofile(DATA_DIR.."hello.lua")
来调用 hello.lua 脚本
4. 启动 wireshark
	* 选择监听的网络适配器(网卡)
	* 在`Filter(过滤)`一栏输入 sd102 
	* 点击`Apply(应用)`
	* 即可监视主站和终端之间传输的山东102规约的报文
	
注意:  

Windows 下可能需要修改 wireshark 的显示字体以显示中文,规约中汉字皆以UTF-8编码.

## 文件结构
* hello.lua		: 规约解析插件.使用lua编写,调用wireshark的api解析报文.
* example-dat.pcap	: 保存的sd102协议数据包.用于测试插件效果,使用wireshark打开.
* screenshot1.png	: 效果图
