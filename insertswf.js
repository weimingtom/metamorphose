function insertswf(myurl, width, height, id, autostart) {
	var flashvars = {};
	var params = {menu: "false", scale: "noScale", allowFullscreen: "true", 
		allowScriptAccess: "always", bgcolor: "", wmode: "direct"};
	var attributes = {id:id};
	swfobject.embedSWF(myurl, 
		id, width, height, "10.0.0", 
		"swfobject/expressInstall.swf", 
		flashvars, params, attributes);
}
function insertflv(myurl, width, height, id, autostart) {
	//var flashvars = {url:myurl};
	//var flashvars = {vcastr_file:myurl, BufferTime:3, IsAutoPlay:(autostart==="true" ? 1 : 0)};
	//var flashvars = {flvToPlay:myurl};
	//var flashvars = {file:myurl, autostart: (autostart==="true" ? "true" : "false"), bufferlength:3};
	
	//http://blog.sina.com.cn/s/blog_72682ca20101663f.html
	var flashvars = {file:myurl, stretching:"uniform", bufferlength:10, 
		"controlbar.position":"bottom",
		autostart: (autostart==="true" ? "true" : "false")};
	
	var params = {menu: "false", scale: "noScale", allowFullscreen: "true", 
		allowScriptAccess: "always", bgcolor: "", wmode: "direct"};
	var attributes = {id:id};
	// /tinymce/plugins/media/
	
	/*
	swfobject.embedSWF("../UploadFiles/moxieplayer.swf", 
		id, width, height, "10.0.0", 
		"../UploadFiles/swfobject/expressInstall.swf", 
		flashvars, params, attributes);
	*/
	
	/*
	swfobject.embedSWF("../UploadFiles/Flvplayer.swf", 
		id, width, height, "10.0.0", 
		"../UploadFiles/swfobject/expressInstall.swf", 
		flashvars, params, attributes);
	*/
	
	/*
	//http://www.comsharp.com/GetKnowledge/zh-CN/FaqComposingFlv.aspx
	//http://oos.moxiecode.com/flvplayer/
	swfobject.embedSWF("../UploadFiles/flvPlayer2.swf", 
		id, width, height, "10.0.0", 
		"../UploadFiles/swfobject/expressInstall.swf", 
		flashvars, params, attributes);	
	*/
	
	/*
	//http://www.bo-blog.com/weblog/203sp1/3/1/
	//http://www.jwplayer.com/
	swfobject.embedSWF("../UploadFiles/mediaplayer.swf", 
		id, width, height, "10.0.0", 
		"../UploadFiles/swfobject/expressInstall.swf", 
		flashvars, params, attributes);
	*/
	
	swfobject.embedSWF("player.swf", 
		id, width, height, "10.0.0", 
		"swfobject/expressInstall.swf", 
		flashvars, params, attributes);
}

