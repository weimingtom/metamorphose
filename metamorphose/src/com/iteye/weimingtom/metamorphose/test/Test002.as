package com.iteye.weimingtom.metamorphose.test
{
	import com.iteye.weimingtom.metamorphose.lua.BaseLib;
	import com.iteye.weimingtom.metamorphose.lua.Lua;
	import com.iteye.weimingtom.metamorphose.lua.LuaTable;
	import com.iteye.weimingtom.metamorphose.lua.MathLib;
	import com.iteye.weimingtom.metamorphose.lua.OSLib;
	import com.iteye.weimingtom.metamorphose.lua.PackageLib;
	import com.iteye.weimingtom.metamorphose.lua.StringLib;
	import com.iteye.weimingtom.metamorphose.lua.TableLib;
	
	import flash.display.Sprite;
	import flash.utils.ByteArray;

	public class Test002 extends Sprite
	{
		[Embed(source="../../../../../../assets/accept-basic/bisect.lua", mimeType="application/octet-stream")]
		private static var _bisectClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/cf.lua", mimeType="application/octet-stream")]
		private static var _cfClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/echo.lua", mimeType="application/octet-stream")]
		private static var _echoClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/env.lua", mimeType="application/octet-stream")]
		private static var _envClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/factorial.lua", mimeType="application/octet-stream")]
		private static var _factorialClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/fib.lua", mimeType="application/octet-stream")]
		private static var _fibClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/fibfor.lua", mimeType="application/octet-stream")]
		private static var _fibforClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/globals.lua", mimeType="application/octet-stream")]
		private static var _globalsClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/hello.lua", mimeType="application/octet-stream")]
		private static var _helloClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/life.lua", mimeType="application/octet-stream")]
		private static var _lifeClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/luac.lua", mimeType="application/octet-stream")]
		private static var _luacClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/printf.lua", mimeType="application/octet-stream")]
		private static var _printfClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/readonly.lua", mimeType="application/octet-stream")]
		private static var _readonlyClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/sieve.lua", mimeType="application/octet-stream")]
		private static var _sieveClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/sort.lua", mimeType="application/octet-stream")]
		private static var _sortClass:Class;
		[Embed(source="../../../../../../assets/accept-basic/table.lua", mimeType="application/octet-stream")]
		private static var _tableClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/trace-calls.lua", mimeType="application/octet-stream")]
		private static var _traceCallsClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/trace-globals.lua", mimeType="application/octet-stream")]
		private static var _traceGlobalsClass:Class; //miss
		[Embed(source="../../../../../../assets/accept-basic/xd.lua", mimeType="application/octet-stream")]
		private static var _xdClass:Class; //miss
		
		private static var _embeddedLuaFiles:Array = [
			{test:true, label:"Bisection method for solving non-linear equations", asset:_bisectClass, filename:"assets/accept-basic/bisect.lua"},
			{test:false, label:"Temperature conversion table (celsius to farenheit)", asset:_cfClass, filename:"assets/accept-basic/cf.lua"},
			{test:false, label:"Echo command line arguments", asset:_echoClass, filename:"assets/accept-basic/echo.lua"}, //miss
			{test:false, label:"Environment variables as automatic global variables", asset:_envClass, filename:"assets/accept-basic/env.lua"},
			{test:false, label:"Factorial without recursion", asset:_factorialClass, filename:"assets/accept-basic/factorial.lua"},
			{test:false, label:"Fibonacci function with cache", asset:_fibClass, filename:"assets/accept-basic/fib.lua"},
			{test:false, label:"Fibonacci numbers with coroutines and generators", asset:_fibforClass, filename:"assets/accept-basic/fibfor.lua"},
			{label:"Report global variable usage", asset:_globalsClass, filename:"assets/accept-basic/globals.lua"}, //miss
			{label:"The first program in every language", asset:_helloClass, filename:"assets/accept-basic/hello.lua"}, //miss
			{test:false, label:"Conway's Game of Life", asset:_lifeClass, filename:"assets/accept-basic/life.lua"},
			{label:"Bare-bones luac", asset:_luacClass, filename:"assets/accept-basic/luac.lua"}, //miss
			{label:"An implementation of printf", asset:_printfClass, filename:"assets/accept-basic/printf.lua"},
			{label:"The sieve of of Eratosthenes programmed with coroutines", asset:_readonlyClass, filename:"assets/accept-basic/readonly.lua"},
			{label:"Make global variables readonly", asset:_sieveClass, filename:"assets/accept-basic/sieve.lua"},
			{test:false, label:"Two implementations of a sort function", asset:_sortClass, filename:"assets/accept-basic/sort.lua"},
			{label:"Make table, grouping all data for the same item", asset:_tableClass, filename:"assets/accept-basic/table.lua"}, //miss
			{label:"Trace calls", asset:_traceCallsClass, filename:"assets/accept-basic/trace-calls.lua"}, //miss
			{label:"Trace assigments to global variables", asset:_traceGlobalsClass, filename:"assets/accept-basic/trace-globals.lua"}, //miss
			{label:"Hex dump", asset:_xdClass, filename:"assets/accept-basic/xd.lua"},//miss
		];
		
		public function Test002()
		{
			trace("Start test...");
			var code:Array = new Array();
			for each (var luaFile:Object in _embeddedLuaFiles)
			{
				var luaString:String = clsObjToUTF8(new luaFile.asset());
				code.push({
					test:luaFile.test, 
					label:luaFile.label, 
					code:luaString,
					filename:luaFile.filename
				});
			}
			for (var i:int = 0; i < code.length; ++i) 
			{
				if (code[i].test)
				{
					trace(code[i].label);
					runScript(code[i].code, code[i].filename);
				}
			}
		}

		public static function runScript(code:String, filename:String):void 
		{
			const isLoadLib:Boolean = true;
			const useArg:Boolean = true;
			var argv:Array = [];
			try
			{
				var L:Lua = new Lua();
				if (isLoadLib)
				{
					BaseLib.open(L);
					PackageLib.open(L);
					MathLib.open(L);
					OSLib.open(L);
					StringLib.open(L);
					TableLib.open(L);
				}
				if (useArg) 
				{
					//FIXME: index may be minus (for example, arg[-1], before script file name)
					//@see http://www.ttlsa.com/lua/lua-install-and-lua-variable-ttlsa/
					var narg:int = argv.length;
					var tbl:LuaTable = L.createTable(narg, narg);
					L.rawSetI(tbl, 0, filename);
					for (var i:int = 0; i < narg; i++) 
					{
						L.rawSetI(tbl, i + 1, argv[i]);
					}
					L.setGlobal("arg", tbl);
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
				} 
				else 
				{
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
				
		private static function clsObjToUTF8(clsObj:Object):String
		{
			var bytes:ByteArray = ByteArray(clsObj);
			var str:String = bytes.readUTFBytes(bytes.bytesAvailable);
			var result:String = str.replace(/\r\n/g, "\n");
			return result;
		}
	}
}