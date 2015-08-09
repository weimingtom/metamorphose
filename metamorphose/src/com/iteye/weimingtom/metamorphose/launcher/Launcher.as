package com.iteye.weimingtom.metamorphose.launcher
{
	import flash.display.MovieClip;
	import flash.events.KeyboardEvent;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFieldType;
	import flash.text.TextFormat;
	
	import com.iteye.weimingtom.metamorphose.lua.Lua;
	
	[SWF(width=640, height = 520)]
	public class Launcher extends MovieClip
	{
		private var _format:TextFormat = new TextFormat();
		private var _formatPre:TextFormat = new TextFormat();
		private var _tfOutput:TextField = new TextField();
		private var _tfInput:TextField = new TextField();
		private var _tfInputPre:TextField = new TextField();
		private var _arrHistory:Array = new Array();
		private var _nHistoryIndex:int = 0;
		
		private var _L:Lua = null;
		
		public function Launcher()
		{			
			with(_tfOutput)
			{
				x = 0;
				y = 0;
				width = 640;
				height = 480;
				background = true;
				backgroundColor = 0x000000;//0xFFFFFF;
				textColor = 0x00FF00;
				multiline = true;
				wordWrap = true;
				restrict = "";
				
				selectable = true;
				type = TextFieldType.DYNAMIC;
			}
			this.addChild(_tfOutput);
			
			with(_formatPre)
			{
				font = "宋体";
				color = 0xFF0000;
				size = 20;
				bold = true;
				leftMargin = 0;
			}
			with(_tfInputPre)
			{
				x = 0;
				y = _tfOutput.height;
				restrict = null;
				multiline = false;
				wordWrap = false;
				border = false;
				autoSize = TextFieldAutoSize.LEFT;
				type = TextFieldType.DYNAMIC;
				defaultTextFormat = _formatPre;
				text = ">";
			}
			this.addChild(_tfInputPre);
			
			
			with(_format)
			{
				font = "宋体";
				color = 0xFF0000;
				bold = true;
				size = 20;
				leftMargin = 10;
			}
			with(_tfInput)
			{
				x = 0;
				y = _tfOutput.height;
				width = 640;
				height = 30;
				restrict = null;
				multiline = false;
				wordWrap = false;
				border = true;
				borderColor = 0xCC0000;
				autoSize = TextFieldAutoSize.NONE; //不要调整大小
				type = TextFieldType.INPUT;
				defaultTextFormat = _format;
			}
			this.addChild(_tfInput);
			this.stage.addEventListener(KeyboardEvent.KEY_DOWN, onKeydown);
			
			_tfInput.appendText("input");
			log("Welcome to Lua world!");
			
			_L = new Lua();
		}
		
		private function onKeydown(event:KeyboardEvent):void
		{
			//log("keyCode:" + event.keyCode);
			var strTemp:String;
			switch(event.keyCode)
			{
				case 13: // Enter
					log(">" + _tfInput.text);
					_arrHistory.push(_tfInput.text);
					_nHistoryIndex = _arrHistory.length;
					execute(_tfInput.text);
					_tfInput.text = "";
					break;
				
				case 27: //ESC
					_tfOutput.text = "";
					//_arrHistory.length = 0;
					break;
				
				case 38://UP
					if(_nHistoryIndex > 0)
						_nHistoryIndex--;
					strTemp = _arrHistory[_nHistoryIndex] as String;
					if(strTemp != null)
					{
						//trace(strTemp.length);
						_tfInput.text = "";
						_tfInput.appendText(strTemp);
					}
					break;
					
				case 40://DOWN
					if(_nHistoryIndex < _arrHistory.length - 1)
						_nHistoryIndex++;
					strTemp = _arrHistory[_nHistoryIndex] as String;
					if(strTemp != null)
					{
						//trace(strTemp.length);
						_tfInput.text = "";
						_tfInput.appendText(strTemp);
					}
					break;
			}
		}
		
		private function log(str:String):void
		{
			_tfOutput.appendText(str + "\n");
			_tfOutput.scrollV = _tfOutput.numLines;
		}
		
		private function execute(str:String):void
		{
			_L.setTop(0);
			//trace(str);
			var res:int = _L.doString(str);
			if (res == 0)
			{
				var obj:Object = _L.value(1);
				log(String(obj));
			}
			else
			{
				var result:String = "Error: " + res;
				switch (res)
				{
					case Lua.ERRRUN:    
						result += " Runtime error"; 
						break;
					
					case Lua.ERRSYNTAX: 
						result += " Syntax error" ; 
						break ;
					
					case Lua.ERRERR:    
						result += " Error error" ; 
						break;
					
					case Lua.ERRFILE:   
						result += " File error" ; 
						break ;
				}
				log("[Error] " + result);
				log("[Error Info] " + _L.value(1));
			}
		}
	}
}