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
	private static String fib_lua = "assets/accept-basic/fib.lua";
	
	public static void main(String[] args) {
		System.out.println("Start test...");
		List<String> code = new ArrayList<String>();
		code.add(clsToUTF8(fib_lua));
		
		for (int i = 0; i < code.size(); ++i) {
			if (i == 0) {
				runScript(code.get(i));
			}
		}
	}

	public static void runScript(String code) {
		final boolean isLoadLib = true;
		final boolean useArg = true;
		final String[] argv = new String[]{"hello", "world"};
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
