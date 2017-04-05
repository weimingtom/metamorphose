package com.iteye.weimingtom.metamorphose.test;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import com.iteye.weimingtom.metamorphose.lua.BaseLib;
import com.iteye.weimingtom.metamorphose.lua.IOLib;
import com.iteye.weimingtom.metamorphose.lua.Lua;
import com.iteye.weimingtom.metamorphose.lua.LuaTable;
import com.iteye.weimingtom.metamorphose.lua.MathLib;
import com.iteye.weimingtom.metamorphose.lua.OSLib;
import com.iteye.weimingtom.metamorphose.lua.PackageLib;
import com.iteye.weimingtom.metamorphose.lua.StringLib;
import com.iteye.weimingtom.metamorphose.lua.TableLib;

public class Test002 {
	private static String _bisectClass = "assets/accept-basic/bisect.lua";
	private static String _cfClass = "assets/accept-basic/cf.lua";
	private static String _echoClass = "assets/accept-basic/echo.lua"; //miss
	private static String _envClass = "assets/accept-basic/env.lua";
	private static String _factorialClass = "assets/accept-basic/factorial.lua";
	private static String _fibClass = "assets/accept-basic/fib.lua";
	private static String _fibforClass = "assets/accept-basic/fibfor.lua";
	private static String _globalsClass = "assets/accept-basic/globals.lua"; //miss
	private static String _helloClass = "assets/accept-basic/hello.lua"; //miss
	private static String _lifeClass = "assets/accept-basic/life.lua";
	private static String _luacClass = "assets/accept-basic/luac.lua"; //miss
	private static String _printfClass = "assets/accept-basic/printf.lua";
	private static String _readonlyClass = "assets/accept-basic/readonly.lua";
	private static String _sieveClass = "assets/accept-basic/sieve.lua";
	private static String _sortClass = "assets/accept-basic/sort.lua";
	private static String _tableClass = "assets/accept-basic/table.lua"; //miss
	private static String _traceCallsClass = "assets/accept-basic/trace-calls.lua"; //miss
	private static String _traceGlobalsClass = "assets/accept-basic/trace-globals.lua"; //miss
	private static String _xdClass = "assets/accept-basic/xd.lua"; //miss
	
	private final static class LuaFile {
		public boolean test;
		public String label;
		public String asset;
		public String code;
		public String filename;
		
		public LuaFile() {
			
		}
		
		public LuaFile(boolean test, String label, String asset) {
			this.test = test;
			this.label = label;
			this.asset = asset;
		}
		
		public LuaFile(String label, String asset) {
			this(false, label, asset);
		}
	}
	
	private static LuaFile[] _embeddedLuaFiles = {
		new LuaFile(false, "Bisection method for solving non-linear equations", _bisectClass),
		new LuaFile(false, "Temperature conversion table (celsius to farenheit)", _cfClass),
		new LuaFile(false, "Echo command line arguments", _echoClass), //miss
		new LuaFile(false, "Environment variables as automatic global variables", _envClass),
		new LuaFile(false, "Factorial without recursion", _factorialClass),
		new LuaFile(false, "Fibonacci function with cache", _fibClass),
		new LuaFile(false, "Fibonacci numbers with coroutines and generators", _fibforClass),
		new LuaFile("Report global variable usage", _globalsClass), //miss
		new LuaFile(true, "The first program in every language", _helloClass), //miss
		new LuaFile("Conway's Game of Life", _lifeClass),
		new LuaFile("Bare-bones luac", _luacClass), //miss
		new LuaFile("An implementation of printf", _printfClass),
		new LuaFile("The sieve of of Eratosthenes programmed with coroutines", _readonlyClass),
		new LuaFile("Make global variables readonly", _sieveClass),
		new LuaFile("Two implementations of a sort function", _sortClass),
		new LuaFile("Make table, grouping all data for the same item", _tableClass), //miss
		new LuaFile("Trace calls", _traceCallsClass), //miss
		new LuaFile("Trace assigments to global variables", _traceGlobalsClass), //miss
		new LuaFile("Hex dump", _xdClass),//miss
	};
	
	public static void main(String[] args) {
		System.out.println("Start test...");
		
		List<LuaFile> code = new ArrayList<LuaFile>();
		for (LuaFile luaFile : _embeddedLuaFiles) {
			String luaString = clsToUTF8(luaFile.asset);
			LuaFile item = new LuaFile();
			item.test = luaFile.test; 
			item.label = luaFile.label; 
			item.code = luaString;
			item.filename = luaFile.asset;
			code.add(item);
		}
		for (int i = 0; i < code.size(); ++i) {
			if (code.get(i).test) {
				System.out.println(code.get(i).label);
				runScript(code.get(i).code, code.get(i).filename);
			}
		}
	}

	public static void runScript(String code, String filename) {
		final boolean isLoadLib = true;
		final boolean useArg = true;
		final String[] argv = new String[]{};
		try {
			Lua L = new Lua();
			if (isLoadLib) {
				BaseLib.open(L);
				PackageLib.open(L);
				MathLib.open(L);
				OSLib.open(L);
				StringLib.open(L);
				TableLib.open(L);
				IOLib.open(L);
			}
			if (useArg) {
				//FIXME: index may be minus (for example, arg[-1], before script file name)
				//@see http://www.ttlsa.com/lua/lua-install-and-lua-variable-ttlsa/
				int narg = argv.length;
				LuaTable tbl = L.createTable(narg, narg);
				L.rawSetI(tbl, 0, filename);
				for (int i = 0; i < narg; i++) {
					L.rawSetI(tbl, i, argv[i]);
				}
				L.setGlobal("arg", tbl);
			}
			
			int status = L.doString(code);
			if (status != 0) {
				Object errObj = L.value(1);
			    Object tostring = L.getGlobal("tostring");
			    L.push(tostring);
			    L.push(errObj);
			    L.call(1, 1);
			    String errObjStr = L.toString(L.value(-1));
				throw new Exception("Error compiling : " + errObjStr);
			} else {
				Object result = L.value(1);
			    Object tostring_ = L.getGlobal("tostring");
			    L.push(tostring_);
			    L.push(result);
			    L.call(1, 1);
			    String resultStr = L.toString(L.value(-1));
				System.err.println("Result >>> " + resultStr);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private static String clsToUTF8(String filename)
	{
		StringBuffer sb = new StringBuffer();
		InputStream instr = null;
		InputStreamReader reader = null;
		BufferedReader buffer = null;
		try {
			instr = new FileInputStream(filename);
			reader = new InputStreamReader(instr, "UTF-8");
			buffer = new BufferedReader(reader);
			String line = null;
			while (null != (line = buffer.readLine())) {
				sb.append(line);
				sb.append("\n");
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (buffer != null) {
				try {
					buffer.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (instr != null) {
				try {
					instr.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return sb.toString();
	}
}
