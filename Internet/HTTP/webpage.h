#ifndef __WEBPAGE_H
#define __WEBPAGE_H
#define INDEX_HTML  "<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<title>协议转换卡配置</title>"\
"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\
"<meta http-equiv='X_UA-Compatible' content='IE=edge, chrome=1'/>"\
"<style type='text/css'>"\
"#ONE,#heard,#box,#xxx/*中间用，隔开*/"\
"{"\
"width:880px;"\
"}"\
"#ONE,#heard"\
"{"\
"background:'#F0F8FF';"\
"margin:0 auto;"\
"}"\
"#box"\
"{"\
"margin:15px auto;"\
"}"\
"#box #First/*注意两个属性之间要有空格隔开*/"\
"{"\
"/*height:220px;*/"\
"width:200px;"\
"height:375px;"\
"background:'#F0FFFF';"\
"float:left;"\
"margin-right:16px;"\
"display:inline;"\
"}"\
"#box #Second"\
"{"\
"width:200px;"\
"height:375px;"\
"background:'#F0FFFF';"\
"float:left;"\
"margin-right:16px;"\
"display:inline;"\
"}"\
"#box #Third"\
"{"\
"width:200px;"\
"height:375px;"\
"background:'#F0F8FF';"\
"float:left;"\
"margin-right:16px;"\
"display:inline;"\
"}"\
"#box #Fourth"\
"{"\
"/*width:250px;*/"\
"width: 230px;"\
"height:375px;"\
"background:'#F0F8FF';"\
"float:left;"\
"display:inline;"\
"}"\
"#xxx"\
"{"\
"/*width:990px;*/"\
"height:100px;"\
"margin:0 auto;"\
"}"\
"</style>"\
"<script>"\
"function selset(id,val){"\
"var o=id;"\
"for(var i=0;i<o.options.length;i++){"\
"if(i==val){"\
"o.options[i].selected=true"\
"break;"\
"}"\
"}"\
"}"\
"function $(id) { return document.getElementById(id);};"\
"function settingsCallback(o) {"\
"if ($('txtVer')) $('txtVer').value = o.ver;"\
"if ($('txtfuc')) $('txtfuc').value = o.fuc;"\
"if ($('txtMac')) $('txtMac').value = o.mac;"\
"if ($('netprotocol')) netprotocol.options[o.protocol].selected=true;"\
"if ($('textsid')) $('textsid').value = o.sid;"\
"if ($('obligate1')) $('obligate1').value = o.gate1;"\
"if ($('txtmodbusaddr')) $('txtmodbusaddr').value = o.modaddr;"\
"if ($('txtmodbuslen')) $('txtmodbuslen').value = o.modlen;"\
"if ($('txtmodbustim')) $('txtmodbustim').value = o.modtim;"\
"if ($('obligate2')) $('obligate2').value = o.gate2;"\
"if ($('txtIp')) $('txtIp').value = o.ip;"\
"if ($('txtlocport')) $('txtlocport').value = o.locport;"\
"if ($('obligate3')) $('obligate3').value = o.gate3;"\
"if ($('txtIpgoal')) $('txtIpgoal').value = o.ipgoal;"\
"if ($('txtportgoal')) $('txtportgoal').value = o.portgoal;"\
"if ($('obligate4')) $('obligate4').value = o.gate4;"\
"if ($('txtSub')) $('txtSub').value = o.sub;"\
"if ($('txtGw')) $('txtGw').value = o.gw;"\
"if ($('obligate5')) $('obligate5').value = o.gate5;"\
"if ($('seluse4')) seluse4.options[o.use4].selected=true;"\
"if ($('selBaud4')) selBaud4.options[o.baud4].selected=true;"\
"if ($('sellocID4')) $('sellocID4').value = o.CANlocID4;"\
"if ($('selresID4')) $('selresID4').value = o.CANresID4;"\
"if ($('selresIDnum4')) $('selresIDnum4').value = o.CANresIDnum4;"\
"if ($('selresDatatype4')) selresDatatype4.options[o.datatype4].selected=true;"\
"if ($('seluse1')) seluse1.options[o.use1].selected=true;"\
"if ($('selBaud1')) selBaud1.options[o.baud1].selected=true;"\
"if ($('selDb1')) selDb1.options[o.databit1].selected=true;"\
"if ($('selParity1')) selParity1.options[o.parity1].selected=true;"\
"if ($('selStop1')) selStop1.options[o.stopbit1].selected=true;"\
"if ($('selFlow1')) selFlow1.options[o.flow1].selected=true;"\
"if ($('selDatatype1')) selDatatype1.options[o.datatype1].selected=true;"\
"if ($('obligate7')) $('obligate7').value = o.gate7;"\
"if ($('seluse2')) seluse2.options[o.use2].selected=true;"\
"if ($('selBaud2')) selBaud2.options[o.baud2].selected=true;"\
"if ($('selDb2')) selDb2.options[o.databit2].selected=true;"\
"if ($('selParity2')) selParity2.options[o.parity2].selected=true;"\
"if ($('selStop2')) selStop2.options[o.stopbit2].selected=true;"\
"if ($('selFlow2')) selFlow2.options[o.flow2].selected=true;"\
"if ($('selDatatype2')) selDatatype2.options[o.datatype2].selected=true;"\
"if ($('obligate8')) $('obligate8').value = o.gate8;"\
"if ($('seluse3')) seluse3.options[o.use3].selected=true;"\
"if ($('selBaud3')) selBaud3.options[o.baud3].selected=true;"\
"if ($('selDb3')) selDb3.options[o.databit3].selected=true;"\
"if ($('selParity3')) selParity3.options[o.parity3].selected=true;"\
"if ($('selStop3')) selStop3.options[o.stopbit3].selected=true;"\
"if ($('selDatatype3')) selDatatype3.options[o.datatype3].selected=true;"\
"if ($('texttout')) $('texttout').value = o.tout;"\
"};"\
"</script>"\
"</head>"\
"<body>"\
"<form method='POST' action='config.cgi'>"\
"<div id='ONE'>"\
"<fieldset style='border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px; height: 80px; width: 845px;'>"\
"<legend align='center'>协议转换器软件配置说明</legend>"\
"<div style='float:left;width:150px;height:40px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>固件版本号</p>"\
"</div>"\
"<div style='float:left;width:280px;height:40px;border:2px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>"\
"<input type='text' id='txtVer' name='ver' style='width: 158px;' disabled='disabled' >"\
"</p>"\
"</div>"\
"<div style='float:left;width:120px;height:40px;border:2px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>功能描述</p>"\
"</div>"\
"<div style='float:left; width:180px;height:40px; border:2px groove silver; border-left:none; border-bottom: none; border-top: none; border-right: none; margin:0; padding:10px 5px;'>"\
"<p>"\
"<input type='text' id='txtfuc' name='fuc' style='width: 180px;' disabled='disabled'>"\
"</p>"\
"</div>"\
"</fieldset>"\
"</div>"\
"<div id='box'>"\
"<div id='heard'>"\
"<fieldset style='border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px; height: 280px; width: 845px;'>"\
"<legend align='center'>RJ45以太网接口</legend>"\
"<div  style='line-height: 22px;  float:left;width:150px;height:140px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>以太网模式</p>"\
"<p>节点ID/站地址</p>"\
"<p>Modbus起始地址</p>"\
"<p>Modbus数据长度</p>"\
"<p>MAC地址</p>"\
"<p>子网掩码</p>"\
"</div>"\
"<div style='line-height: 20px; float:left;width:280px;height:140px;border:2px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>"\
"<select id='netprotocol' name='protocol' style='width: 158px; height: auto;'>"\
"<option value='0'>MQTT</option>"\
"<option value='1'>TCP/IP CLIENT</option>"\
"<option value='2'>TCP/IP SERVER</option>"\
"<option value='3'>Modbus CLIENT</option>"\
"<option value='4'>Modbus SERVER</option>"\
"</select>"\
"</p>"\
"<p>"\
"<input type='text' id='textsid' name='sid' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtmodbusaddr' name='modaddr' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtmodbuslen'  name='modlen' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtMac' name='mac' style='width: 158px;' disabled='disabled'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtSub' name='sub' style='width: 158px;' />"\
"</p>"\
"</div>"\
"<div style='line-height: 24px;  float:left;width:140px;height:140px;border:2px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 5px;'> "\
"<p>本地主机IP地址</p>"\
"<p>本地主机PORT</p>"\
"<p>远程主机IP</p>"\
"<p>远程主机PORT</p>"\
"<p>Modbus轮询时间</p>"\
"<p>默认网关</p>"\
"</div>"\
"<div style='line-height: 20px; float:left; width:180px;height:140px; border:2px groove silver; border-left:none; border-bottom: none; border-top: none; border-right: none; margin:0; padding:10px 5px;'>"\
"<p>"\
"<input type='text' id='txtIp' name='ip' style='width: 158px;' />"\
"</p>"\
"<p>"\
"<input type='text' id='txtlocport' name='locport' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtIpgoal' name='ipgoal' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtportgoal' name='portgoal' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtmodbustim' name='modtim' style='width: 158px;'>"\
"</p>"\
"<p>"\
"<input type='text' id='txtGw' name='gw' style='width: 158px;' />"\
"</p>"\
"</div> "\
"</fieldset>"\
"</div>"\
"</div>"\
"<div id='box'>"\
"<div id='First'>"\
"<fieldset style='height: 330px; border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px;'>"\
"<legend align='center'>RS232C-1接口（0x00）</legend>"\
"<div style='line-height: 23px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:5px 5px;'> "\
"<p>使能</p>"\
"<p>波特率</p>"\
"<p>数据位</p>"\
"<p>校验位</p>"\
"<p>停止位</p>"\
"<p>流控</p>"\
"<p>数据类型</p>"\
"</div>"\
"<div style='line-height: 20px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 3px;'> "\
"<p>"\
"<select id='seluse1' name='use1' style='width: 95px; height: 20px;'>"\
"<option value='0' onchange='netinfo_block(this.value);'>DISABLE</option>"\
"<option value='1' onchange='netinfo_block(this.value);'>ENABLE</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selBaud1' name='baud1' style='width: 95px; height: 20px;'>"\
"<option value='0'>256000</option>"\
"<option value='1'>128000</option>"\
"<option value='2'>115200</option>"\
"<option value='3'>57600</option>"\
"<option value='4'>56000</option>"\
"<option value='5'>38400</option>"\
"<option value='6'>19200</option>"\
"<option value='7'>14400</option>"\
"<option value='8'>9600</option>"\
"<option value='9'>4800</option>"\
"<option value='10'>2400</option>"\
"<option value='11'>1200</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selDb1' name='databit1' style='width: 95px; height: 20px;'>"\
"<option value='0'>8</option>"\
"<option value='1'>7</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selParity1' name='parity1' style='width: 95px; height: 20px;'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selStop1' name='stopbit1' style='width: 95px; height: 20px;'>"\
"<option value='0'>1</option>"\
"<option value='1'>1.5</option>"\
"<option value='2'>2</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selFlow1' name='flow1' style='width: 95px; height: 23px;'>"\
"<option value='0'>DISABLE</option>"\
"<option value='1'>ENABLE</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selDatatype1' name='datatype1'  style='width: 95px; height: 23px;'>"\
"<option value='0'>16进制数据</option>"\
"<option value='1'>ASCII格式</option>"\
"</select>"\
"</P>"\
"</div>"\
"</fieldset>"\
"</div>"\
"</div>"\
"<div id='box'>"\
"<div id='Second'>"\
"<fieldset style='height: 330px; border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px;'>"\
"<legend align='center'>RS232C-2接口（0x01）</legend>"\
"<div style='line-height: 23px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:5px 5px;'> "\
"<p>使能</p>"\
"<p>波特率</p>"\
"<p>数据位</p>"\
"<p>校验位</p>"\
"<p>停止位</p>"\
"<p>流控</p>"\
"<p>数据类型</p>"\
"</div>"\
"<div style='line-height: 20px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 3px;'> "\
"<p>"\
"<select id='seluse2' name='use2' style='width: 95px; height: 20px;'>"\
"<option value='0' onchange='netinfo_block(this.value);'>DISABLE</option>"\
"<option value='1' onchange='netinfo_block(this.value);'>ENABLE</option>"\
"</select>"\
"</p>"\
"<select id='selBaud2' name='baud2' style='width: 95px; height: 20px;'>"\
"<option value='0'>256000</option>"\
"<option value='1'>128000</option>"\
"<option value='2'>115200</option>"\
"<option value='3'>57600</option>"\
"<option value='4'>56000</option>"\
"<option value='5'>38400</option>"\
"<option value='6'>19200</option>"\
"<option value='7'>14400</option>"\
"<option value='8'>9600</option>"\
"<option value='9'>4800</option>"\
"<option value='10'>2400</option>"\
"<option value='11'>1200</option>"\
"</select>"\
"<p>"\
"<select id='selDb2' name='databit2' style='width: 95px; height: 20px;'>"\
"<option value='0'>8</option>"\
"<option value='1'>7</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selParity2' name='parity2' style='width: 95px; height: 20px;'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selStop2' name='stopbit2' style='width: 95px; height: 20px;'>"\
"<option value='0'>1</option>"\
"<option value='1'>1.5</option>"\
"<option value='2'>2</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selFlow2' name='flow2' style='width: 95px; height: 23px;'>"\
"<option value='0'>DISABLE</option>"\
"<option value='1'>ENABLE</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selDatatype2' name='datatype2'  style='width: 95px;  height: 23px;'>"\
"<option value='0'>16进制数据</option>"\
"<option value='1'>ASCII格式</option>"\
"</select>"\
"</P>"\
"</div>"\
"</fieldset>"\
"</div>"\
"</div>"\
"<div id='box'>"\
"<div id='Third'>"\
"<fieldset style='height: 330px; border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px;'>"\
"<legend align='center'>RS485接口（0x02）</legend>"\
"<div style='line-height: 23px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:5px 5px;'> "\
"<br />"\
"<p>使能</p>"\
"<p>波特率</p>"\
"<p>数据位</p>"\
"<p>校验位</p>"\
"<p>停止位</p>"\
"<p>数据类型</p>"\
"</div>"\
"<div style='line-height: 20px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 3px;'> "\
"<br />"\
"<p>"\
"<select id='seluse3' name='use3' style='width: 95px; height: 20px;'>"\
"<option value='0' onchange='netinfo_block(this.value);'>DISABLE</option>"\
"<option value='1' onchange='netinfo_block(this.value);'>ENABLE</option>"\
"</select>"\
"</p>"\
"<select id='selBaud3' name='baud3' style='width: 95px; height: 20px;'>"\
"<option value='0'>256000</option>"\
"<option value='1'>128000</option>"\
"<option value='2'>115200</option>"\
"<option value='3'>57600</option>"\
"<option value='4'>56000</option>"\
"<option value='5'>38400</option>"\
"<option value='6'>19200</option>"\
"<option value='7'>14400</option>"\
"<option value='8'>9600</option>"\
"<option value='9'>4800</option>"\
"<option value='10'>2400</option>"\
"<option value='11'>1200</option>"\
"</select>"\
"<p>"\
"<select id='selDb3' name='databit3' style='width: 95px; height: 20px;'>"\
"<option value='0'>8</option>"\
"<option value='1'>7</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selParity3' name='parity3' style='width: 95px; height: 20px;'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selStop3' name='stopbit3' style='width: 95px; height: 23px;'>"\
"<option value='0'>1</option>"\
"<option value='1'>1.5</option>"\
"<option value='2'>2</option>"\
"</select>"\
"</p>"\
"<p>"\
"<select id='selDatatype3' name='datatype3'  style='width: 95px; height: 23px;'>"\
"<option value='0'>16进制数据</option>"\
"<option value='1'>ASCII格式</option>"\
"</select>"\
"</P>"\
"</div>"\
"</fieldset>"\
"</div>"\
"</div>"\
"<div id='box'>"\
"<div id='Fourth'>"\
"<fieldset style='height: 330px; border-bottom-width: 3px; border-left-width: 3px; border-right-width: 3px; border-top-width: 3px;'>"\
"<legend align='center'>CAN总线接口（0x03）</legend>"\
"<div style='line-height: 23px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:5px 5px;'> "\
"<br />"\
"<p>使能</p>"\
"<p>波特率</p>"\
"<p>本地ID</p>"\
"<p>响应ID</p>"\
"<p>响应ID数量</p>"\
"<p>数据类型</p>"\
"</div>"\
"<div style='line-height: 21px; float:left;width:49px;height:50px;border:100px groove silver;border-left:none; border-bottom: none; border-top: none; border-right: none;margin:0;padding:10px 3px;'> "\
"<br />"\
"<p>"\
"<select id='seluse4' name='use4' style='width: 95px; height: 20px;'>"\
"<option value='0'>DISABLE</option>"\
"<option value='1'>ENABLE</option>"\
"</select>"\
"</P>"\
"<p>"\
"<select id='selBaud4' name='baud4'  style='width: 95px; height: 20px;'>"\
"<option value='0'>1Mbps</option>"\
"<option value='1'>800Kbps</option>"\
"<option value='2'>666Kbps</option>"\
"<option value='3'>500Kbps</option>"\
"<option value='4'>400Kbps</option>"\
"<option value='5'>250Kbps</option>"\
"<option value='6'>200Kbps</option>"\
"<option value='7'>125Kbps</option>"\
"<option value='8'>100Kbps</option>"\
"<option value='9'>80Kbps</option>"\
"<option value='10'>50Kbps</option>"\
"<option value='11'>40Kbps</option>"\
"<option value='12'>20Kbps</option>"\
"<option value='13'>10Kbps</option>"\
"</select>"\
"</P>"\
"<p>"\
"<input type='text' id='sellocID4' name='CANlocID4' style='width: 95px; height: 18px;'/>"\
"</p>"\
"<p>"\
"<input type='text' id='selresID4' name='CANresID4' style='width: 95px; height: 18px;'/>"\
"</p>"\
"<p>"\
"<input type='text' id='selresIDnum4' name='CANresIDnum4' style='width: 95px; height: 18px;'/>"\
"</p>"\
"<p>"\
"<select id='selresDatatype4' name='datatype4'  style='width: 95px; height: 25px;'>"\
"<option value='0'>16进制数据</option>"\
"<option value='1'>ASCII格式</option>"\
"</select>"\
"</P>"\
"</div>"\
"</fieldset>"\
"</div>"\
"</div>"\
"<div id='xxx'>"\
"<div style='float:left;width:860px;height:140px;border:3px groove silver; margin:0;padding:10px 5px;'> "\
"<p>串口通信完整帧超时时间（ms）"\
"<input type='text' id='texttout' name='tout' style='width: 100px;' />"\
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"\
"连接超时（s）"\
"<input type='text' name='lastname' style='width: 100px;' value='3'>"\
"</p>"\
"<p>"\
"<input type='submit' value='保存并重启' style='width: 100px;'>"\
"</p>"\
"</form>"\
"<p>&copy;Copyright 2017 by MJ 上海明匠智能系统有限公司</p>"\
"</div>"\
"</div>"\
"<script type='text/javascript' src='w5500.js'></script>"\
"</body>"\
"</html>"
#endif
// "if ($('selBaud1')) selBaud1.options[o.baud1].selected=true;"\
// "if ($('selDb1')) selDb1.options[o.databit1].selected=true;"\
// "if ($('selParity1')) selParity1.options[o.parity1].selected=true;"\
// "if ($('selStop1')) selStop1.options[o.stopbit1].selected=true;"\
// "if ($('selFlow1')) selFlow1.options[o.flow1].selected=true;"\
// "if ($('seluse1')) seluse1.options[o.use1].selected=true;"\
// "if ($('selBaud2')) selBaud2.options[o.baud2].selected=true;"\
// "if ($('selDb2')) selDb2.options[o.databit2].selected=true;"\
// "if ($('selParity2')) selParity2.options[o.parity2].selected=true;"\
// "if ($('selStop2')) selStop2.options[o.stopbit2].selected=true;"\
// "if ($('selFlow2')) selFlow2.options[o.flow2].selected=true;"\
// "if ($('seluse2')) seluse2.options[o.use2].selected=true;"\
// "if ($('selBaud3')) selBaud3.options[o.baud3].selected=true;"\
// "if ($('selDb3')) selDb3.options[o.databit3].selected=true;"\
// "if ($('selParity3')) selParity3.options[o.parity3].selected=true;"\
// "if ($('selStop3')) selStop3.options[o.stopbit3].selected=true;"\
// "if ($('selFlow3')) selFlow3.options[o.flow3].selected=true;"\
// "if ($('seluse3')) seluse3.options[o.use3].selected=true;"\
// "if ($('selBaud4')) selBaud4.options[o.baud4].selected=true;"\
// "if ($('seluse4')) seluse4.options[o.use4].selected=true;"\
//"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\

