# 文件

1. hello.lua	: wireshark的lua语言API插件,  
	根据sd102规约使用wireshark解析报文.目前正在完善中.
2. sd102.pcap	: wireshark保存的sd102协议数据包.用于测试插件效果.
3. 1.png	: 效果图

# 使用

1. 将hello.lua文件复制到 wireshark 目录下.  
	Linux默认为 /usr/share/wireshark
2. 修改 wireshark 目录下的 init.lua 文件. 在最末尾添加

	dofile(DATA_DIR.."hello.lua")

来调用 hello.lua 脚本



3. Note:  
Windows 下可能需要修改 wireshark 的显示字体以显示中文
