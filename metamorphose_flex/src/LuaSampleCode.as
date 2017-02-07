package
{
	import flash.utils.ByteArray;
	
	import mx.collections.ArrayCollection;
	
	public class LuaSampleCode
	{
		[Embed(source="../assets/accept-basic/bisect.lua", mimeType="application/octet-stream")]
		private static var _bisectClass:Class;
		[Embed(source="../assets/accept-basic/cf.lua", mimeType="application/octet-stream")]
		private static var _cfClass:Class;
		[Embed(source="../assets/accept-basic/echo.lua", mimeType="application/octet-stream")]
		private static var _echoClass:Class; //miss
		[Embed(source="../assets/accept-basic/env.lua", mimeType="application/octet-stream")]
		private static var _envClass:Class;
		[Embed(source="../assets/accept-basic/factorial.lua", mimeType="application/octet-stream")]
		private static var _factorialClass:Class;
		[Embed(source="../assets/accept-basic/fib.lua", mimeType="application/octet-stream")]
		private static var _fibClass:Class;
		[Embed(source="../assets/accept-basic/fibfor.lua", mimeType="application/octet-stream")]
		private static var _fibforClass:Class;
		[Embed(source="../assets/accept-basic/globals.lua", mimeType="application/octet-stream")]
		private static var _globalsClass:Class; //miss
		[Embed(source="../assets/accept-basic/hello.lua", mimeType="application/octet-stream")]
		private static var _helloClass:Class; //miss
		[Embed(source="../assets/accept-basic/life.lua", mimeType="application/octet-stream")]
		private static var _lifeClass:Class;
		[Embed(source="../assets/accept-basic/luac.lua", mimeType="application/octet-stream")]
		private static var _luacClass:Class; //miss
		[Embed(source="../assets/accept-basic/printf.lua", mimeType="application/octet-stream")]
		private static var _printfClass:Class;
		[Embed(source="../assets/accept-basic/readonly.lua", mimeType="application/octet-stream")]
		private static var _readonlyClass:Class;
		[Embed(source="../assets/accept-basic/sieve.lua", mimeType="application/octet-stream")]
		private static var _sieveClass:Class;
		[Embed(source="../assets/accept-basic/sort.lua", mimeType="application/octet-stream")]
		private static var _sortClass:Class;
		[Embed(source="../assets/accept-basic/table.lua", mimeType="application/octet-stream")]
		private static var _tableClass:Class; //miss
		[Embed(source="../assets/accept-basic/trace-calls.lua", mimeType="application/octet-stream")]
		private static var _traceCallsClass:Class; //miss
		[Embed(source="../assets/accept-basic/trace-globals.lua", mimeType="application/octet-stream")]
		private static var _traceGlobalsClass:Class; //miss
		[Embed(source="../assets/accept-basic/xd.lua", mimeType="application/octet-stream")]
		private static var _xdClass:Class; //miss

		private static var _embeddedLuaFiles:Array = [
			{label:"Bisection method for solving non-linear equations", asset:_bisectClass},
			{label:"Temperature conversion table (celsius to farenheit)", asset:_cfClass},
			{label:"Echo command line arguments", asset:_echoClass}, //miss
			{label:"Environment variables as automatic global variables", asset:_envClass},
			{label:"Factorial without recursion", asset:_factorialClass},
			{label:"Fibonacci function with cache", asset:_fibClass},
			{label:"Fibonacci numbers with coroutines and generators", asset:_fibforClass},
			{label:"Report global variable usage", asset:_globalsClass}, //miss
			{label:"The first program in every language", asset:_helloClass}, //miss
			{label:"Conway's Game of Life", asset:_lifeClass},
			{label:"Bare-bones luac", asset:_luacClass}, //miss
			{label:"An implementation of printf", asset:_printfClass},
			{label:"The sieve of of Eratosthenes programmed with coroutines", asset:_readonlyClass},
			{label:"Make global variables readonly", asset:_sieveClass},
			{label:"Two implementations of a sort function", asset:_sortClass},
			{label:"Make table, grouping all data for the same item", asset:_tableClass}, //miss
			{label:"Trace calls", asset:_traceCallsClass}, //miss
			{label:"Trace assigments to global variables", asset:_traceGlobalsClass}, //miss
			{label:"Hex dump", asset:_xdClass},//miss
		];
		
		[Bindable]
		public static var sampleCode:ArrayCollection = new ArrayCollection();
		
		public static function init():void
		{
			var luaFile:Object;
			sampleCode.removeAll();
			for each (luaFile in _embeddedLuaFiles)
			{
				//import mx.core.ByteArrayAsset;
				//var luaAsset:ByteArrayAsset = ByteArrayAsset(new luaFile.asset());
				//var luaString:String = luaAsset.toString();
				var luaString:String = clsObjToUTF8(new luaFile.asset());
				sampleCode.addItem({label:luaFile.label, code:luaString});
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
