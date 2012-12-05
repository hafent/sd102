-- lua wireshark 山东102协议插件. file encode :utf-8
--简单参考中文 http://yoursunny.com/study/IS409/ScoreBoard.htm
--位操作 参考 http://blog.chinaunix.net/uid-24931444-id-3372735.html
--api定义参考 http://www.wireshark.org/docs/wsug_html_chunked/lua_module_Proto.html  
do
	-------------- 函数前向声明 ---------------
	local FT_farme --解析单字节帧
	local FT_static_farme --解析固定长度帧
	local FT_change_farme --解析变长度帧
	-- 显示函数
	-- 	上行报文解析函数
	local show_time
	local showti
	local show_syn
	local showsp
	--	下行报文解析函数

	-------------- 变量定义  ---------------
	--协议名称为shandong102，在Packet Details（中间可以展开的那部分）
	--    窗格显示为山东102主站通讯规约
	local p_ShanDong102 = Proto("sd102","山东102",base.DEC)
	--协议的各个字段 标识符/第一字段
	local f_identifier = ProtoField.bytes("sd102.id","标识符/规约号之类") 
	local f_len = ProtoField.uint16("sd102.len","长度(字节)",base.DEC) 
	--所有可能
	local f_start = ProtoField.uint8("sd102.start","起始字节",base.HEX,
	{[0x68]="长帧/变长帧",[0x10]="短帧/固定帧",[0xE5]="应答/单字节帧"})
	local f_ctrl = ProtoField.uint8("sd102.ctrl","控制字节",base.HEX)
	local f_onebyte = ProtoField.uint8("sd102.onebyte","单字节",base.HEX)
	-- 控制端到采集终端	
	local f_funcode = ProtoField.uint8("sd102.funcode","功能码(FC)",base.DEC,
	{[0]="S2:复位通信单元",[3]="S2:传送数据",[9]="S3:召唤链路状态",
	[10]="S3:召唤1级用户数据",[11]="S3:召唤2级用户数据"},0x0F) 
	local f_fcv = ProtoField.uint8("sd102.fcv","帧计数有效位(FCV)",base.HEX,
	{[0]="帧计数位(FCB)的变化无效",[1]="帧计数位(FCB)的变化有效"},0x10) 
	local f_fcb = ProtoField.uint8("sd102.fcb","帧计数位(FCB)",base.HEX,
	{[0]="",[1]=""},0x20)
	local f_prm= ProtoField.uint8("sd102.prm","启动报文位(PRM)",base.HEX,
	{[0]="采集端向控制站传输",[1]="控制站向采集端传输"},0x40) 
	--非平衡传输,根据GBT18657.2-2002 5.1.2,传输方向位保留
	local f_dir= ProtoField.uint8("sd102.dir","备用"--[[ 传输方向位(DIR)--]],base.HEX,
	{[0]="备用为0",[1]="备用,应为0"},0x80) 
	-- 采集终端到控制端
	local f_funcode_rsp = ProtoField.uint8("sd102.funcode_rsp","功能码(FC)",base.DEC,
	{[0]="S2:确认",[1]="S2:链路忙,没有收到报文",[2]="备用",[3]="备用",[4]="备用",[5]="备用",
	[6]="制造厂和用户协商定义",
	[7]="制造厂和用户协商定义",
	[8]="S3:以数据回答请求帧",
	[9]="S3:没有所召唤的数据",
	[10]="备用",
	[11]="S3:以链路状态或访问请求回答请求帧",
	[12]="备用",
	[13]="制造厂和用户协商定义",
	[14]="链路服务未工作",
	[15]="链路服务未完成"},0x0F) 
	local f_dfc = ProtoField.uint8("sd102.dfc","数据流控制位(DFC)",base.HEX,
	{[0]="终端可以接收数据",[1]="终端的缓冲区已满"},0x10) 
	local f_acd = ProtoField.uint8("sd102.acd","要求访问位(ACD)",base.HEX,
	{[0]="从站没有1级用户数据要求访问",[1]="从站有1级用户数据要求访问"},0x20) 
	local f_linkaddr= ProtoField.uint16("sd102.linkaddr","链路地址(Link Address)",base.DEC) 
	local f_addr1= ProtoField.uint8("sd102.addr1","低字节",base.HEX) 
	local f_addr2= ProtoField.uint8("sd102.addr2","高字节",base.HEX)
	local f_p= ProtoField.uint8("sd102.p","校验和",base.HEX)
	local f_end= ProtoField.uint8("sd102.end","结束符",base.HEX,{[0x16]="结束"})
	--变长帧 
	local f_farmehead=ProtoField.uint8("sd102.farmehead","帧头",base.HEX)
	local f_ASDU=ProtoField.uint8("sd102.ASDU","应用服务数据单元",base.HEX)
	local f_len1=ProtoField.uint8("sd102.len1","长度",base.DEC)
	local f_len2=ProtoField.uint8("sd102.len2","长度(复本)",base.DEC)
	local f_typeID_up=ProtoField.uint8("sd102.typeID2_up","类型标识(TYP)",base.DEC,
	{[0]="M_UNUSED:未用",[1]="M_SP_TA_2:带时标的单点信息",
	[2]="M_IT_TA_2:读电量返回帧",[5]="M_IT_TD_2:周期复位记账(计费)电能累计量",
	[70]="M_EI_NA_2:初始化结束",
	[71]="P_MP_NA_2:电能累计量数据终端设备的制造厂和产品规范",
	[72]="M_TI_TA_2:电能累计量数据终端设备的当前系统时间",
	[128]="M_SYN_TA_2:电能累计量数据终端系统时间同步确认",
	[162]="M_YC_TA_2:读遥测量返回帧",[163]="M_XL_TA_2:读需量返回帧",
	[164]="M_IT_TA_C_2:月结算复费率电能累计量",[165]="M_IT_TA_D_2:读表计谐波数据返回帧"})
	local f_typeID_down=ProtoField.uint8("sd102.typeID2_down","类型标识(TYP)",base.DEC,
	{[100]="C_RD_NA_2:读制造厂和产品规范",
	[101]="C_SP_NA_2:读带时标的单点信息的记录",
	[102]="C_SP_NB_2:读一个所选定时间范围的带时标的单点信息的记录",
	[103]="C_TI_NA_2:读电能累计量数据终端设备的当前系统时间",
	[120]="C_CI_NR_2:读一个选定的时间范围和一个选定的地址范围的记帐（计费）电能累计量",
	[121]="C_CI_NS_2:读周期地复位的一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量",
	[128]="C_SYN_TA_2:电能累计量数据终端系统时间同步命令",
	[170]="C_CI_NA_B_2:读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量",
	[172]="C_YC_TA_2:读一个选定的时间范围和一个选定的地址范围的遥测量",
	[174]="读一个选定的时间范围和一个选定的地址范围的最大需量"})
	local f_vsq=ProtoField.uint8("sd102.vsq","可变结构限定词(VSQ)",base.HEX)
	local f_sq=ProtoField.uint8("sd102.sq","息体寻址方法(SQ)",base.HEX,
	{[0]="每一个单个元素或综合元素由信息体地址寻址",
	[1]="一个顺序的类似的信息元素(见IEC60870-5-3 5.1.5)"},0x80)
	local f_vsq_num=ProtoField.uint8("sd102.vsq_num","可变结构数目",base.HEX,nil,0x7F)
	local f_cot=ProtoField.uint8("sd102.cot","传送原因(Cause Of Transmission)",base.HEX)
	local f_cot_t=ProtoField.uint8("sd102.cot_t","试验(Test)",base.HEX,
	{[0]="不试验,执行,一般为0",[1]="仅试验传输,不执行实际动作"},0x80)
	local f_cot_pn=ProtoField.uint8("sd102.cot_pn","激活确认",base.HEX,
	{[0]="肯定确认",[1]="否定确认"},0x40)
	local f_cot_cot=ProtoField.uint8("sd102.cot_cot","传送原因",base.DEC,
	{[0]="未用",[1]="试验(专用范围定义)",[2]="周期、循环(专用范围定义)",
	[3]="自发（突发）",[4]="初始化",[5]="请求或被请求",
	[6]="激活(Act)",[7]="激活确认(actcon)",[8]="停止激活(deact)",
	[9]="停止激活确认(deactcon)",[10]="激活终止(actterm)",
	[11]="未用",[12]="未用",[13]="无所请求的数据记录",
	[14]="无所请求的应用服务数据单元――类型",
	[15]="由主站（控制站）发送的应用服务数据单元中的记录序号不可知",
	[16]="由主站（控制站）发送的应用服务数据单元中的地址说明不可知",
	[17]="无所请求的信息体",[18]="无所请求的累计时段",
	[19]="为将来兼容定义保留",--[[[20-41]="未用",
	[42-47]="为将来兼容定义保留",[48-63]="为特殊应用(专用范围)",--]]
	[48]="时间同步(专用范围定义)"},0x3F)
	local f_ASDU_addr=ProtoField.uint16("sd102.ASDU_addr","应用服务单元公共地址(ASDU address)",base.DEC)
	local f_ASDU_addr_lo=ProtoField.uint8("sd102.ASDU_addr_lo","低字节(lo)",base.HEX)
	local f_ASDU_addr_hi=ProtoField.uint8("sd102.ASDU_addr_hi","高字节(hi_",base.HEX)
	local f_recordAddr=ProtoField.uint8("sd102.recordAddr","记录地址(RAD)",base.DEC,{[0]="默认"})
	--时间 ,其中有写位没解析 RSE
	local f_Tb = ProtoField.bytes("sd102.Tb","时间(Tb)",base.HEX)
	local f_Tb_ms=ProtoField.uint16("sd102.Tb_ms","毫秒(ms)",base.DEC,nil,0x03FF)
	local f_Tb_sec=ProtoField.uint16("sd102.Tb_sec","秒(sec)",base.DEC,nil,0xFC00)
	local f_Tb_min=ProtoField.uint8("sd102.Tb_min","分钟(min)",base.DEC,nil,0x3F)
	local f_Tb_tis=ProtoField.uint8("sd102.Tb_tis","费率陈述(tis)",base.HEX,
	{[0]="断开OFF",[1]="合上ON"},0x40)
	local f_Tb_iv=ProtoField.uint8("sd102.Tb_iv","时间陈述无效标志(iv)",base.HEX,{[0]="有效",[1]="无效"},0x80)
	local f_Tb_hour=ProtoField.uint8("sd102.Tb_hour","小时(hour)",base.DEC,nil,0x1F)
	local f_Tb_res1=ProtoField.uint8("sd102.Tb_res1","备用1",base.HEX,nil,0x60)
	local f_Tb_su=ProtoField.uint8("sd102.Tb_su","标准时间(SU)",base.HEX,{[0]="标准时间",[1]="夏时制"},0x80)
	local f_Tb_day=ProtoField.uint8("sd102.Tb_day","日(day)",base.DEC,nil,0x1F)
	local f_Tb_week=ProtoField.uint8("sd102.Tb_week","周次(week)",base.DEC,nil,0xE0)
	local f_Tb_mon=ProtoField.uint8("sd102.Tb_mon","月(month)",base.DEC,nil,0x0F)
	local f_Tb_eti=ProtoField.uint8("sd102.Tb_eti","能量费率(eti)",	base.HEX,{[0]="能量费率"},0x30)
	local f_Tb_pti=ProtoField.uint8("sd102.Tb_pti","功率费率(pti)",base.HEX,{[0]="功率费率"},0xC0)
	local f_Tb_year=ProtoField.uint8("sd102.Tb_year","年(year)",base.DEC,nil,0x7F)
	local f_Tb_res2=ProtoField.uint8("sd102.Tb_res2","备用2",base.HEX,nil,0x80)
	--读电量
	local f_msgaddr_start = ProtoField.uint8("sd102.msgaddr","起始消息体地址(IOA)",base.DEC)
	local f_msgaddr_end = ProtoField.uint8("sd102.msgaddr","终止消息体地址(IOA)",base.DEC)
	local f_ioa = ProtoField.uint8("sd102.ioa","信息体地址(IOA)",base.DEC)
	--Ta
	local f_Ta = ProtoField.bytes("sd102.Ta","时间(Ta)",base.HEX)
	local f_Ta_min = ProtoField.uint8("sd102.Ta_min","分钟(min)",base.DEC,nil,0x3F)
	local f_Ta_tis = ProtoField.uint8("sd102.Ta_tis","费率信息开关",base.DEC,{[0]="费率陈述OFF",[1]="费率陈述ON"},0x40)
	local f_Ta_iv=ProtoField.uint8("sd102.Ta_iv","时间陈述无效标志(iv)",base.HEX,{[0]="有效",[1]="无效"},0x80)
	local f_Ta_hour = ProtoField.uint8("sd102.Ta_hour","小时(hour)",base.DEC,nil,0x1F)
	local f_Ta_res1=ProtoField.uint8("sd102.Ta_res1","备用1",base.HEX,nil,0x60)
	local f_Ta_su=ProtoField.uint8("sd102.Ta_su","标准时间(SU)",base.HEX,{[0]="标准时间",[1]="夏时制"},0x80)
	local f_Ta_day = ProtoField.uint8("sd102.Ta_day","日(day)",base.DEC,nil,0x3F)
	local f_Ta_week = ProtoField.uint8("sd102.Ta_week","周几(week)",base.DEC,nil,0xE0)
	local f_Ta_mon = ProtoField.uint8("sd102.Ta_mon","月(month)",base.DEC,nil,0x0F)
	local f_Ta_year = ProtoField.uint8("sd102.Ta_year","年(year)",base.DEC,nil,0x7F)
	--单点信息
	local f_sp = ProtoField.bytes("sd102.Sp","单点信息(SP)",base.HEX)
	local f_sp_spa = ProtoField.uint8("sd102.Sp_spa","单点信息地址(SP)",base.DEC)
	local f_sp_spi = ProtoField.uint8("sd102.Sp_spi","单点信息状态(SPI)",base.HEX,
	{[0]="分开",[1]="闭合"},0x01)
	local f_sp_spq = ProtoField.uint8("sd102.Sp_spq","单点信息质量(SPQ)",base.HEX)
	--电量信息(共7字节)
	local f_ti =ProtoField.bytes("sd102.TI","电量信息体(TI)",base.HEX)
	--信息体地址 1字节
	local f_ti_val = ProtoField.uint32("sd102.TIval","电量值",base.DEC) --注意小端模式 4 字节
	local f_ti_iv = ProtoField.uint8("sd102.TIiv","电量有效",base.HEX,{[0]="无效",[1]="有效"},0x80)--掩码位,最高位表示电量数值是否有效 1字节
	local f_ti_cs = ProtoField.uint8("sd102.TIcs","电量校验",base.HEX) --1字节
	-- 通用字节留
	local f_bs =ProtoField.bytes("sd102.com_cs","集合",base.HEX)
	local f_frame_tail =ProtoField.bytes("sd102.frame_tail","帧尾",base.HEX)

	------添加到域
	p_ShanDong102.fields = { f_onebyte,f_len,f_start, f_frame_tail, --开始的字节
	f_funcode,f_funcode_rsp,f_ctrl,
	f_fcv,f_dfc,f_fcb,f_acd,f_prm,f_dir,f_linkaddr,f_addr1,f_addr2,f_p,f_end,
	f_farmehead,f_len1,f_len2,f_ASDU,f_typeID_up,f_typeID_down,f_vsq,f_sq,f_vsq_num,
	f_cot,f_cot_t,f_cot_pn,f_cot_cot,f_ASDU_addr,f_ASDU_addr_lo,f_ASDU_addr_hi,f_recordAddr,
	f_Tb,f_Tb_ms,f_Tb_sec,f_Tb_min,f_Tb_tis,f_Tb_iv,f_Tb_hour,f_Tb_res1,f_Tb_day,f_Tb_week, --Tb
	f_Tb_mon,f_Tb_year,f_Tb_res2,f_Tb_su,f_Tb_pti,f_Tb_eti,
	f_ioa,--信息体地址
	f_msgaddr_start,f_msgaddr_end, --开始可结束信息体地址
	f_Ta,f_Ta_tis,f_Ta_iv,f_Ta_min,f_Ta_hour,f_Ta_res1,f_Ta_su,f_Ta_day,f_Ta_week,f_Ta_mon,f_Ta_year,--Ta
	f_sp,f_sp_spa,f_sp_spi,f_sp_spq, 	--单点信息
	f_ti,f_ti_val,f_ti_iv,f_ti_cs ,f_bs}--电量信息 
	local data_dis = Dissector.get("data")

	-------------------- 主体函数 ----------------------------------
	local function shandong102_dissector(buf,pkt,root)
		local buf_len = buf:len();
		if buf_len < 1 then  --长度 < 1 绝对错误
			return false
		end

		if buf(0,1):uint() == 0xe5 then --1.单字节帧解析
			FT_farme(buf,pkt,root)
			return true
		end 

		if buf(0,1):uint() == 0x10 then --2.固定帧长帧解析
			FT_static_farme(buf,pkt,root)
			return true
		end 

		if buf(0,1):uint() == 0x68 then --3.变帧长帧解析
			FT_change_farme(buf,pkt,root)
			return true
		else
			return false
		end 
		return true
	end

	----------------解析单字节数据帧函数-----------------------
	function FT_farme(buf,pkt,root)
		--local p_ShanDong102 = Proto("sd102","山东102")
		--p_ShanDong102.fields = {f_onebyte}
		--添加协议
		local t = root:add(p_ShanDong102,buf,nil,"长度: ",f_len,buf:len(),"字节")
		t:append_text(" 单字节帧(CS)")
		pkt.cols.protocol = "sd102-单字符" --显示在第一栏的协议名称
		--t:add(f_start,buf(0,1)) 
		--t:add("单字节数据")
		return true
	end

	----------------短帧/固定长帧----------------
	function FT_static_farme(buf,pkt,root)
		local len = buf:len();
		--添加协议
		local t = root:add(p_ShanDong102,buf(0,len),nil,"长度: ",f_len,buf:len(),"字节") 
		t:append_text(" 固定帧长帧(短帧)")
		pkt.cols.protocol = "sd102-定长帧" --显示在第一栏的协议名称
		--开始判断:
		if len ~= 6 then --短帧的长度固定,长度不对,错误
			t:add("len != 6 ,len=",len)
			return false
		end
		if buf(5,1):uint() ~= 0x16 then --结束符错误
			t:add("结束符!=0x16,结束符=",buf(5,1):uint())
			return false
		end
		--校验和
		--全部判断完成:

		--t:add(f_start,buf(0,1)) 
		local ctrlbyte = t:add(f_ctrl,buf(1,1))
		--	分上下行解析
		if bit.rshift(bit.band(buf(1,1):uint(), 0x40), 6) == 1 then --控制站向采集端
			ctrlbyte:add(f_funcode,buf(1,1))
			ctrlbyte:add(f_fcv,buf(1,1))
			ctrlbyte:add(f_fcb,buf(1,1))
			ctrlbyte:add(f_prm,buf(1,1))
			--ctrlbyte:add(f_dir,buf(1,1))
		else --采集端 向控制端
			ctrlbyte:add(f_funcode_rsp,buf(1,1))
			ctrlbyte:add(f_dfc,buf(1,1))
			ctrlbyte:add(f_acd,buf(1,1))
			ctrlbyte:add(f_prm,buf(1,1))
			--ctrlbyte:add(f_dir,buf(1,1))
		end

		local linkaddr = t:add_le(f_linkaddr,buf(2,2))
		linkaddr:add(f_addr1,buf(2,1)) 
		linkaddr:add(f_addr2,buf(3,1))
		t:add(f_p,buf(4,1)) 
		t:add(f_end,buf(5,1)) 
		return true
	end

	----------------长帧/变长帧---------------- 函数长度有限制
	function FT_change_farme(buf,pkt,root)
		local len = buf:len();
		--添加协议
		local t = root:add(p_ShanDong102,buf(0,len),nil,"长度:",f_len,buf:len(),"字节")
		t:append_text(" 变帧长帧(长帧)")
		pkt.cols.protocol = "sd102-变长帧" --显示在第一栏的协议名称
		--开始判断:
		if len < 6 then --长度太小错误
			t:add("长度太小:",len)
			return false
		end
		if buf(3,1):uint() ~= 0x68 then --帧边界 两个0x68
			t:add("边界不等于0x68",buf(3,1):uint())
			return false
		end
		if buf(1,1):uint() ~= buf(2,1):uint() then --两个长度必须相同
			t:add("长度不等",buf(1,1):uint(),buf(2,1):uint())
			return false
		end
		if buf(len-1,1):uint() ~= 0x16 then --结束符错误
			t:add("结束符不等于0x16: ",buf(len-1,1):uint())
			return false
		end
		--全部判断完成:
		--t:add(f_start,buf(0,1)) 
		local farmehead = t:add(f_farmehead,buf(0,4))
		farmehead:add(f_start,buf(0,1))
		farmehead:add(f_len1,buf(1,1))
		farmehead:add(f_len2,buf(2,1))
		farmehead:add(f_start,buf(3,1))
		local ctrlbyte = t:add(f_ctrl,buf(4,1))
		if bit.rshift(bit.band(buf(4,1):uint(), 0x40), 6) == 1 then --控制站向采集端
			ctrlbyte:add(f_funcode,buf(4,1))
			ctrlbyte:add(f_fcv,buf(4,1))
			ctrlbyte:add(f_fcb,buf(4,1))
			ctrlbyte:add(f_prm,buf(4,1))
			--ctrlbyte:add(f_dir,buf(4,1))
		else --采集端 向控制端
			ctrlbyte:add(f_funcode_rsp,buf(4,1))
			ctrlbyte:add(f_dfc,buf(4,1))
			ctrlbyte:add(f_acd,buf(4,1))
			ctrlbyte:add(f_prm,buf(4,1))
			--ctrlbyte:add(f_dir,buf(4,1))
		end
		local linkaddr = t:add_le(f_linkaddr,buf(5,2))
		linkaddr:add(f_addr1,buf(5,1)) 
		linkaddr:add(f_addr2,buf(6,1))
		if bit.rshift(bit.band(buf(4,1):uint(), 0x40), 6) == 1 then --控制站向采集端
			t:add(f_typeID_down,buf(7,1))
		else --采集端 向控制端
			t:add(f_typeID_up,buf(7,1))
		end
		local vsq = t:add(f_vsq,buf(8,1))
		vsq:append_text(" 数量: ")
		local vsq_n =bit.rshift(bit.band(buf(8,1):uint(), 0x7F), 0) 
		vsq:append_text(vsq_n)
		vsq:add(f_sq,buf(8,1))
		vsq:add(f_vsq_num,buf(8,1))
		local cot = t:add(f_cot,buf(9,1))
		cot:add(f_cot_t,buf(9,1))
		cot:add(f_cot_pn,buf(9,1))
		cot:add(f_cot_cot,buf(9,1))
		local asdu_addr = t:add_le(f_ASDU_addr,buf(10,2))
		asdu_addr:add(f_ASDU_addr_lo,buf(10,1))
		asdu_addr:add(f_ASDU_addr_hi,buf(11,1))
		t:add(f_recordAddr,buf(12,1))
		-- 是下行报文吗?
		local isDownMsg =bit.rshift(bit.band(buf(4,1):uint(), 0x40), 6) 
		------ 按上下行报文分类: 
		if isDownMsg==1 then ------- 下行 ---------
			--	按TYP分类(因为lua没有switch,所以只能用if-elseif语句
			--什么都不做,信息体为空
			if buf(7,1):uint() == 103 then --读终端时间,
				--设置终端时间
			elseif buf(7,1):uint() == 128 then
				addTb(t,13,buf)
			--读一个选定的时间范围和一个选定的地址范围的记帐（计费）电能累计量
			elseif buf(7,1):uint() == 120 then 
				t:add(f_msgaddr_start,buf(13,1))
				t:add(f_msgaddr_end,buf(14,1))
				addTa(t,15,buf)
				addTa(t,20,buf)
			end
		else -----------上行 -------
			--按TYP分类
			if buf(7,1):uint() == 1 then --单点信息
				t:add("sp")
				showsp(t,buf)
			elseif buf(7,1):uint() == 2 then --读电量
				--t:add("ti")
				showti(t,buf)
			elseif buf(7,1):uint() == 72 then --返回当前系统时间
				--t:add("time")
				show_time(t,buf)
			elseif buf(7,1):uint() == 128 then --时间同步确认
				t:add("syn")
				show_syn(t,buf)
			end
		end
		-- 结束
		local sizeft=2--帧尾的大小
		local frame_tail =t:add(f_frame_tail,buf(len-sizeft,sizeft),"帧尾")
		frame_tail:add(f_p,buf(len-sizeft,1))
		frame_tail:add(f_end,buf(len-sizeft+1,1))
		return true
	end
	--------------------      其他被调用的函数      ----------------------
	function addTa(t,base,buf)
		local sizeTa=5
		local Ta = t:add(f_Ta,buf(base,sizeTa))
		Ta:add(f_Ta_min,buf(base+0,1))
		Ta:add(f_Ta_tis,buf(base+0,1))
		Ta:add(f_Ta_iv,buf(base+0,1))
		Ta:add(f_Ta_hour,buf(base+1,1))
		Ta:add(f_Ta_res1,buf(base+1,1))
		Ta:add(f_Ta_su,buf(base+1,1))
		Ta:add(f_Ta_day,buf(base+2,1))
		Ta:add(f_Ta_week,buf(base+3,1))
		Ta:add(f_Ta_mon,buf(base+3,1))
		Ta:add(f_Ta_year,buf(base+4,1))
		local min=bit.band(buf(base+0,1):uint(),0x3f)
		local hour=bit.band(buf(base+1,1):uint(),0x1f)
		local day=bit.band(buf(base+2,1):uint(),0x1f)
		local month=bit.band(buf(base+3,1):uint(),0x0f)
		local year=bit.band(buf(base+4,1):uint(),0x7f)
		Ta:append_text(" 时刻: ")
		Ta:append_text(year)
		Ta:append_text("-")
		Ta:append_text(month)
		Ta:append_text("-")
		Ta:append_text(day)
		Ta:append_text(" ")
		Ta:append_text(hour)
		Ta:append_text(":")
		Ta:append_text(min)
	end
	function addTb(t,base,buf)
		local sizeTb=7
		local Tb = t:add(f_Tb,buf(base,sizeTb))		
		Tb:add_le(f_Tb_ms,buf(base+0,2))
		Tb:add_le(f_Tb_sec,buf(base+0,2))
		Tb:add(f_Tb_min,buf(base+2,1))
		Tb:add(f_Tb_tis,buf(base+2,1))
		Tb:add(f_Tb_iv,buf(base+2,1))
		Tb:add(f_Tb_hour,buf(base+3,1))
		Tb:add(f_Tb_res1,buf(base+3,1))
		Tb:add(f_Tb_su,buf(base+3,1))
		Tb:add(f_Tb_day,buf(base+4,1))
		Tb:add(f_Tb_week,buf(base+4,1))
		Tb:add(f_Tb_mon,buf(base+5,1))
		Tb:add(f_Tb_eti,buf(base+5,1))
		Tb:add(f_Tb_pti,buf(base+5,1))
		Tb:add(f_Tb_year,buf(base+6,1))
		Tb:add(f_Tb_res2,buf(base+6,1))
		local ms=bit.band(buf(base+0,2):le_uint(),0x03ff)
		local sec=bit.rshift(bit.band(buf(base+0,2):le_uint(),0xfc00),10)
		local min=bit.band(buf(base+2,1):uint(),0x3f)
		local hour=bit.band(buf(base+3,1):uint(),0x1f)
		local day=bit.band(buf(base+4,1):uint(),0x1f)
		local month=bit.band(buf(base+5,1):uint(),0x0f)
		local year=bit.band(buf(base+6,1):uint(),0x7f)
		Tb:append_text(" 时间: ")
		Tb:append_text(year)
		Tb:append_text("-")
		Tb:append_text(month)
		Tb:append_text("-")
		Tb:append_text(day)
		Tb:append_text(" ")
		Tb:append_text(hour)
		Tb:append_text(":")
		Tb:append_text(min)
		Tb:append_text(":")
		Tb:append_text(sec)
		Tb:append_text(" ")
		Tb:append_text(ms)
	end
	---------- 显示系统当前时间
	function show_time(t,buf)
		addTb(t,13,buf)
	end
	---------- 显示 终端系统同步世界确认帧
	function show_syn(t,buf)
		addTb(t,13,buf)
	end
	---------- 显示单点信息 信息体 函数
	function showsp(t,buf)
		--待完善
		t:add("TODO: 显示单点信息返回信息体值")
	end
	---------- 添加显示 ti电量 n个信息体(TI_Obj)和 一个信息体单元(Ta)函数
	function showti(t,buf)
		local n=buf(8,1):uint()--信息体个数
		--t:add(n)
		local sizeIT=7
		local sizeTa=5
		local base=13
		local i=0 --循环变量
		-- 一组信息体集合
		local tis=t:add(f_bs,buf(base,n*sizeIT),"","电量信息体集合: ")
		tis:append_text(" 开始地址: ")
		tis:append_text(buf(base+0,1):uint())
		tis:append_text(" 结束地址: ")
		tis:append_text(buf(base+(n-1)*sizeIT+0,1):uint())
		tis:append_text(" 总数量: ")
		tis:append_text(n)
		--添加信息体
		for i=0,(n-1) do --遍历所有信息体
			--t:add("内部偏移base=",base,"帧内序号i=",i) --for debug
			local ti = tis:add(f_ti,buf(base,sizeIT))
			ti:add(f_ioa,buf(base,1))
			ti:add_le(f_ti_val,buf(base+1,4))
			ti:add(f_ti_iv,buf(base+1+4,1))
			ti:add(f_ti_cs,buf(base+1+4+1,1))
			--添加一些信息
			ti:append_text(" 帧内序号: ")
			ti:append_text(i)
			ti:append_text(" 帧内偏移: ")
			ti:append_text(base)
			ti:append_text(" 字节")
			--向后偏移 一个信息体长度
			base=base+(sizeIT)
		end
		--t:add("i=",i)
		addTa(t,base,buf)
		--添加Ta公共信息单元
	end
	---------------- 全局函数?api? ----------------
	function p_ShanDong102.dissector(buf,pkt,root) 
		if shandong102_dissector(buf,pkt,root) then
			--valid shandong102 diagram
		else
			--data这个dissector几乎是必不可少的；当发现不是我的协议时，就应该调用data
			data_dis:call(buf,pkt,root)
		end
	end

	local tcp_encap_table = DissectorTable.get("tcp.port")
	--只需要处理UDP1127端口就可以了
	tcp_encap_table:add(10001,p_ShanDong102)
	tcp_encap_table:add(10002,p_ShanDong102)
	tcp_encap_table:add(10003,p_ShanDong102)
	tcp_encap_table:add(10004,p_ShanDong102)
	tcp_encap_table:add(10005,p_ShanDong102)

	--tcp_encap_table:add(35243,p_ShanDong102)
	--tcp_encap_table:add(50187,p_ShanDong102)
	--udp_encap_table:add(10003,p_ShanDong102)
	--udp_encap_table:add(10001,p_ShanDong102)
end
