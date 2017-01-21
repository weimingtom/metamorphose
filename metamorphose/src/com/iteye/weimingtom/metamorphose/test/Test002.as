package com.iteye.weimingtom.metamorphose.test
{
	import com.iteye.weimingtom.metamorphose.lua.BaseLib;
	import com.iteye.weimingtom.metamorphose.lua.Lua;
	import com.iteye.weimingtom.metamorphose.lua.MathLib;
	import com.iteye.weimingtom.metamorphose.lua.OSLib;
	import com.iteye.weimingtom.metamorphose.lua.PackageLib;
	import com.iteye.weimingtom.metamorphose.lua.StringLib;
	import com.iteye.weimingtom.metamorphose.lua.TableLib;
	
	import flash.display.Sprite;
	import flash.utils.ByteArray;

	public class Test002 extends Sprite
	{
		[Embed(source = '../../../../../../assets/accept-basic/fib.lua', mimeType = 'application/octet-stream')]
		private static var fib_lua:Class;
		
		public function Test002()
		{
			trace("Start test...");
			var code:Array = new Array();
			code.push(clsToUTF8(fib_lua));
			
			for (var i:int = 0; i < code.length; ++i) 
			{
				if (i == 0)
				{
					runScript(code[i]);
				}
			}
		}

		public static function runScript(code:String):void 
		{
			const isLoadLib:Boolean = true;
			try
			{
				trace("Start test...");
				var L:Lua = new Lua();
				if(isLoadLib)
				{
					BaseLib.open(L);
					PackageLib.open(L);
					MathLib.open(L);
					OSLib.open(L);
					StringLib.open(L);
					TableLib.open(L);
				}
				var status:int = L.doString(code);
				if (status != 0)
				{
					var errObj:Object = L.value(1);
					var tostring:Object = L.getGlobal("tostring");
					L.pushObject(tostring);
					L.pushObject(errObj);
					L.call(1, 1);
					var errObjStr:String = L.toString(L.value(-1));
					throw new Error("Error compiling : " + L.value(1));
				} else {
					var result:Object = L.value(1);
					var tostring_:Object = L.getGlobal("tostring");
					L.pushObject(tostring_);
					L.pushObject(result);
					L.call(1, 1);
					var resultStr:String = L.toString(L.value(-1));
					trace("Result >>> " + resultStr);
				}
			}
			catch (e:Error)
			{
				trace(e.getStackTrace());
			}
		}
		
		private static function clsToUTF8(cls:Class):String
		{
			var bytes:ByteArray = ByteArray(new cls);
			return bytes.readUTFBytes(bytes.bytesAvailable);
		}
	}
}