#ifndef __WEBPAGE_H
#define __WEBPAGE_H
#define INDEX_HTML  "<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<title>协议转换卡配置</title>"\
"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\
"<style type='text/css'>"\
"body {text-align:left; background-color:#c0deed;font-family:Verdana;}"\
"#main {margin-right:auto;margin-left:auto;margin-top:30px;}"\
"label{display:inline-block;width:100px;}"\
"label2{padding-top:3px; text-align:center; vertical-align:middle; width:70px;}"\
"#main h3{color:#66b3ff; text-decoration:underline;}"\
"#main h2{color:#6603ff;}"\
"</style>"\
"<SCRIPT>"\
"var d = new Date();"\
"document.write(d.getFullYear() + '年' +(d.getMonth() + 1) + '月' + d.getDate() + '日');"\
"document.write(' 星期'+'日一二三四五六'.charAt(new Date().getDay()));"\
"</SCRIPT>"\
"<script>"\
"function $(id) { return document.getElementById(id);};"\
"function settingsCallback(o) {"\
"if ($('txtVer')) $('txtVer').value = o.ver;"\
"if ($('txtMac')) $('txtMac').value = o.mac;"\
"if ($('txtfuc')) $('txtfuc').value = o.fuc;"\
"if ($('txtIp')) $('txtIp').value = o.ip;"\
"if ($('txtIpgoal')) $('txtIpgoal').value = o.ipgoal;"\
"if ($('txtSub')) $('txtSub').value = o.sub;"\
"if ($('txtportgoal')) $('txtportgoal').value = o.portgoal;"\
"if ($('txtGw')) $('txtGw').value = o.gw;"\
"if ($('selBaud1')) {selBaud1.options[o.baud1].selected=true;}"\
"if ($('selDb1')) selDb1.options[o.databit1].selected=true;"\
"if ($('selParity1')) selParity1.options[o.parity1].selected=true;"\
"if ($('selStop1')) selStop1.options[o.stopbit1].selected=true;"\
"if ($('selFlow1')) selFlow1.options[o.flow1].selected=true;"\
"if ($('seluse1')) seluse1.options[o.use1].selected=true;"\
"if ($('selBaud2')) selBaud2.options[o.baud2].selected=true;"\
"if ($('selDb2')) selDb2.options[o.databit2].selected=true;"\
"if ($('selParity2')) selParity2.options[o.parity2].selected=true;"\
"if ($('selStop2')) selStop2.options[o.stopbit2].selected=true;"\
"if ($('selFlow2')) selFlow2.options[o.flow2].selected=true;"\
"if ($('seluse2')) seluse2.options[o.use2].selected=true;"\
"if ($('selBaud3')) selBaud3.options[o.baud3].selected=true;"\
"if ($('selDb3')) selDb3.options[o.databit3].selected=true;"\
"if ($('selParity3')) selParity3.options[o.parity3].selected=true;"\
"if ($('selStop3')) selStop3.options[o.stopbit3].selected=true;"\
"if ($('selFlow3')) selFlow3.options[o.flow3].selected=true;"\
"if ($('seluse3')) seluse3.options[o.use3].selected=true;"\
"if ($('selBaud4')) selBaud4.options[o.baud4].selected=true;"\
"if ($('textcan')) $('textcan').value = o.canid;"\
"if ($('seluse4')) seluse4.options[o.use4].selected=true;"\
"if ($('textaddr')) $('textaddr').value = o.addr;"\
"if ($('textleth')) $('textleth').value = o.leth;"\
"if ($('textsid')) $('textsid').value = o.sid;"\
"if ($('texttout')) $('texttout').value = o.tout;"\
"};"\
"</script>"\
"</head>"\
"<body>"\
"<div id='main'>"\
"<div style='background:snow; display:block;padding:10px 20px;'>"\
"<h3>板卡配置网络参数&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp目标配置网络参数</h3>"\
"<form id='frmSetting' method='POST' action='config.cgi'>"\
"<p><label for='txtIp'>固件版本号:</label><input type='text' id='txtVer' name='ver' size='16' disabled='disabled' /></p>"\
"<p><label for='txtIp'>MAC地址:</label><input type='text' id='txtMac' name='mac' size='16' disabled='disabled' /></p>"\
"<p><label for='txtIp'>功能描述:</label><input type='text' id='txtfuc' name='fuc' size='16' disabled='disabled' /></p>"\
"<p><label for='txtIp'>板卡IP地址:</label><input type='text' id='txtIp' name='ip' size='16' />"\
"&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp"\
"<label for='txtIpgoal'>目标IP地址:</label><input type='text' id='txtIpgoal' name='ipgoal' size='16' /></p>"\
"<p><label for='txtSub'>子网掩码:</label><input type='text' id='txtSub' name='sub' size='16' />"\
"&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp"\
"<label for='txtportgoal'>目标端口:</label><input type='text' id='txtportgoal' name='portgoal' size='16' /></p>"\
"<p><label for='txtGw'>默认网关:</label><input type='text' id='txtGw' name='gw' size='16' /></p>"\
"<div>"\
"<label2 for='selBaud1'>USART1&nbsp----&nbsp 波特率:</label2>"\
"<select id='selBaud1' name='baud1'>"\
"<option value='0'>600</option>"\
"<option value='1'>254</option>"\
"<option value='2'>2400</option>"\
"<option value='3'>4800</option>"\
"<option value='4'>9600</option>"\
"<option value='5'>19200</option>"\
"<option value='6'>38400</option>"\
"<option value='7'>57600</option>"\
"<option value='8'>115200</option>"\
"<option value='9'>230400</option>"\
"</select>"\
"<label2 for='selDb1'>数据位:</label2>"\
"<select id='selDb1' name='databit1'>"\
"<option value='0'>7</option>"\
"<option value='1'>8</option>"\
"<option value='2'>9</option>"\
"</select>"\
"<label2 for='selParity1'>校验位:</label2>"\
"<select id='selParity1' name='parity1'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"<label2 for='selStop1'>停止位:</label2>"\
"<select id='selStop1' name='stopbit1'>"\
"<option value='0'>1</option>"\
"<option value='1'>2</option>"\
"</select>"\
"<label2 for='selFlow1'>流控:</label2>"\
"<select id='selFlow1' name='flow1'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>CTS/RTS</option>"\
"<option value='2'>RS-422</option>"\
"<option value='3'>RS-485</option>"\
"</select>"\
"<label2 for='seluse1'>使能:</label2>"\
"<select id='seluse1' name='use1'>"\
"<option value='0' onchange='netinfo_block(this.value);'>NONE</option>"\
"<option value='1' onchange='netinfo_block(this.value);'>USER</option>"\
"</select>"\
"</div>"\
"<div>"\
"<label2 for='selBaud2'>USART2&nbsp----&nbsp 波特率:</label2>"\
"<select id='selBaud2' name='baud2'>"\
"<option value='0'>600</option>"\
"<option value='1'>1200</option>"\
"<option value='2'>2400</option>"\
"<option value='3'>4800</option>"\
"<option value='4'>9600</option>"\
"<option value='5'>19200</option>"\
"<option value='6'>38400</option>"\
"<option value='7'>57600</option>"\
"<option value='8'>115200</option>"\
"<option value='9'>230400</option>"\
"</select>"\
"<label2 for='selDb2'>数据位:</label2>"\
"<select id='selDb2' name='databit2'>"\
"<option value='0'>7</option>"\
"<option value='1'>8</option>"\
"<option value='2'>9</option>"\
"</select>"\
"<label2 for='selParity2'>校验位:</label2>"\
"<select id='selParity2' name='parity2'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"<label2 for='selStop2'>停止位:</label2>"\
"<select id='selStop2' name='stopbit2'>"\
"<option value='0'>1</option>"\
"<option value='1'>2</option>"\
"</select>"\
"<label2 for='selFlow2'>流控:</label2>"\
"<select id='selFlow2' name='flow2'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>CTS/RTS</option>"\
"<option value='2'>RS-422</option>"\
"<option value='3'>RS-485</option>"\
"</select>"\
"<label2 for='selFlow2'>使能:</label2>"\
"<select id='seluse2' name='use2'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>USER</option>"\
"</select>"\
"</div>"\
"<div>"\
"<label2 for='selBaud3'>RS485&nbsp&nbsp&nbsp----&nbsp 波特率:</label2>"\
"<select id='selBaud3' name='baud3'>"\
"<option value='0'>600</option>"\
"<option value='1'>1200</option>"\
"<option value='2'>2400</option>"\
"<option value='3'>4800</option>"\
"<option value='4'>9600</option>"\
"<option value='5'>19200</option>"\
"<option value='6'>38400</option>"\
"<option value='7'>57600</option>"\
"<option value='8'>115200</option>"\
"<option value='9'>230400</option>"\
"</select>"\
"<label2 for='selDb3'>数据位:</label2>"\
"<select id='selDb3' name='databit3'>"\
"<option value='0'>7</option>"\
"<option value='1'>8</option>"\
"<option value='2'>9</option>"\
"</select>"\
"<label2 for='selParity3'>校验位:</label2>"\
"<select id='selParity3' name='parity3'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>ODD</option>"\
"<option value='2'>EVEN</option>"\
"</select>"\
"<label2 for='selStop3'>停止位:</label2>"\
"<select id='selStop3' name='stopbit3'>"\
"<option value='0'>1</option>"\
"<option value='1'>2</option>"\
"</select>"\
"<label2 for='selFlow3'>流控:</label2>"\
"<select id='selFlow3' name='flow3'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>CTS/RTS</option>"\
"<option value='2'>RS-422</option>"\
"<option value='3'>RS-485</option>"\
"</select>"\
"<label2 for='selFlow3'>使能:</label2>"\
"<select id='seluse3' name='use3'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>USER</option>"\
"</select>"\
"</div>"\
"<div>"\
"<label2 for='selBaud4'>CAN&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp----&nbsp 波特率:</label2>"\
"<select id='selBaud4' name='baud4'>"\
"<option value='0'>125000</option>"\
"<option value='1'>500000</option>"\
"</select>"\
"<label2 for='textcan'>ID:</label2><input type='text' id='textcan' name='canid' size='10' />"\
"<label2 for='seluse4'>使能:</label2>"\
"<select id='seluse4' name='use4'>"\
"<option value='0'>NONE</option>"\
"<option value='1'>USER</option>"\
"</select>"\
"</div>"\
"<div>"\
"<p>"\
"<h2>软件配置</h2>"\
"<label2 for='textaddr'>起始地址:</label2><input type='text' id='textaddr' name='addr' size='10' />"\
"<label2 for='textleth'>上传阈值:</label2><input type='text' id='textleth' name='leth' size='10' />"\
"<label2 for='textsid'>服务器ID:</label2><input type='text' id='textsid' name='sid' size='10' />"\
"<label2 for='texttout'>超时时间(1~1000ms):</label2><input type='text' id='texttout' name='tout' size='10' />"\
"</div>"\
"<p><input type='submit' value='保存并重启' /></p>"\
"<p><form action='Upload.ashx' method='post' enctype='multipart/form-data'> 选择程序 <input type='file' name='fileUp' size='56'/> <input type='submit' value='上传' /> <p>"\
"</form>"\
"</form>"\
"</div>"\
"</div>"\
"<div style='margin:5px 5px;'>"\
"&copy;Copyright 2015 by MJ 上海明匠智能系统有限公司"\
"</div>"\
"<script type='text/javascript' src='w5500.js'></script>"\
"</body>"\
"</html>"

#endif


