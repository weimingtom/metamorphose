package com.iteye.weimingtom.metamorphose.launcher
{
	import com.iteye.weimingtom.metamorphose.lua.BaseLib;
	import com.iteye.weimingtom.metamorphose.lua.IOLib;
	import com.iteye.weimingtom.metamorphose.lua.Lua;
	import com.iteye.weimingtom.metamorphose.lua.MathLib;
	import com.iteye.weimingtom.metamorphose.lua.OSLib;
	import com.iteye.weimingtom.metamorphose.lua.PackageLib;
	import com.iteye.weimingtom.metamorphose.lua.StringLib;
	import com.iteye.weimingtom.metamorphose.lua.TableLib;
	
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFieldType;
	import flash.text.TextFormat;
	
	[SWF(width=640, height = 520)]
	public class Launcher extends MovieClip
	{
		private var _format:TextFormat = new TextFormat();
		private var _formatPre:TextFormat = new TextFormat();
		private var _formatOutput:TextFormat = new TextFormat();
		private var _tfOutput:TextField = new TextField();
		private var _tfInput:TextField = new TextField();
		private var _tfInputPre:TextField = new TextField();
		private var _arrHistory:Array = new Array();
		private var _nHistoryIndex:int = 0;
		
		private var _L:Lua = null;
		private const _isLoadLib:Boolean = true;
		
		public function Launcher()
		{			
			if (stage) init();
			else addEventListener(Event.ADDED_TO_STAGE, init);
		}
		
		private function init(e:Event = null):void
		{
			removeEventListener(Event.ADDED_TO_STAGE, init);
			// entry point
			
			with(_formatOutput)
			{
				font = "宋体";
				size = 20;
				bold = true;
				leftMargin = 0;
			}
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
				defaultTextFormat = _formatOutput;
				
				selectable = true;
				type = TextFieldType.DYNAMIC;
			}
			this.addChild(_tfOutput);
			
			
			with(_format)
			{
				font = "宋体";
				color = 0x00FF00;
				bold = true;
				size = 20;
				leftMargin = 20;
			}
			
			with(_tfInput)
			{
				x = 0;
				y = _tfOutput.height;
				width = 640;
				height = 50;
				restrict = null;
				multiline = false;
				wordWrap = false;
				border = true;
				background = true;
				backgroundColor = 0x000000;//0xFFFFFF;
				//borderColor = 0xCC0000;
				autoSize = TextFieldAutoSize.NONE; //不要调整大小
				type = TextFieldType.INPUT;
				defaultTextFormat = _format;
			}
			this.addChild(_tfInput);
			this.stage.addEventListener(KeyboardEvent.KEY_DOWN, onKeydown);
			
			with(_formatPre)
			{
				font = "宋体";
				color = 0x00FF00;
				bold = true;
				size = 20;
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
				background = true;
				backgroundColor = 0x000000;//0xFFFFFF;
				autoSize = TextFieldAutoSize.LEFT;
				type = TextFieldType.DYNAMIC;
				defaultTextFormat = _formatPre;
				text = ">";
			}
			this.addChild(_tfInputPre);
			
			_tfInput.appendText("return 1 + 1");
			stage.focus = _tfInput;
			
			log(Lua.RELEASE + "  " + Lua.COPYRIGHT);
			
			_L = new Lua();
			if (_isLoadLib) 
			{
				BaseLib.open(_L);
				PackageLib.open(_L);
				MathLib.open(_L);
				OSLib.open(_L);
				StringLib.open(_L);
				TableLib.open(_L);
				IOLib.open(_L);
			}
		}
		
		private function onKeydown(event:KeyboardEvent):void
		{
			//log("keyCode:" + event.keyCode);
			var strTemp:String;
			switch(event.keyCode)
			{
				case 13: // Enter
					log("> " + _tfInput.text);
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
		
		private function log(str:String, lineReturn:Boolean = true):void
		{
			_tfOutput.appendText(str + (lineReturn ? "\n" : ""));
			_tfOutput.scrollV = _tfOutput.numLines;
		}
		
		private function execute(str:String):void
		{
			//PrintStream.init();
			BaseLib.OutputArr = [];
			BaseLib.OutputArr.push("");
			_L.setTop(0);
			//trace(str);
			var res:int = _L.doString(str);
			log(BaseLib.OutputArr.join("\n"), false);
			if (res == 0)
			{
				var obj:Object = _L.value(1);
//				log(String(obj));
				var tostring:Object = _L.getGlobal("tostring");
				_L.pushObject(tostring);
				_L.pushObject(obj);
				_L.call(1, 1);
				var resultStr:String = _L.toString(_L.value(-1));
				log(resultStr);
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