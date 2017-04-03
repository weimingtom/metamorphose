package com.iteye.weimingtom.metamorphose.launcher;

import java.awt.Adjustable;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;

import com.iteye.weimingtom.metamorphose.lua.BaseLib;
import com.iteye.weimingtom.metamorphose.lua.Lua;
import com.iteye.weimingtom.metamorphose.lua.MathLib;
import com.iteye.weimingtom.metamorphose.lua.OSLib;
import com.iteye.weimingtom.metamorphose.lua.PackageLib;
import com.iteye.weimingtom.metamorphose.lua.StringLib;
import com.iteye.weimingtom.metamorphose.lua.TableLib;

/**
 * @see https://github.com/projectServer/projectServer
 * @author Administrator
 * 
 */
public class Launcher {
	private Font font;
	private JFrame frameTop;
	
	private JPanel panelOutput;
	private JMenuTextArea textAreaOutput;
	private JScrollPane scrollPaneOutput;

	private JPanel panelInput;
	private JMenuTextArea textAreaInput;
	private JScrollPane scrollPaneInput;

	public Launcher() {
		initFrame();
	}
	
	public void initFrame() {
		font = new Font("宋体", Font.BOLD, 20);
		
		frameTop = new JFrame();
		frameTop.setSize(800, 600);
		frameTop.setLocationRelativeTo(null);
		frameTop.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
//		frameTop.setLayout(new FlowLayout());
		frameTop.setTitle(Lua.VERSION + " (metamorphose)");
		frameTop.addWindowListener(new WindowAdapter() {
			@Override
			public void windowOpened(WindowEvent arg0) {
				textAreaInput.requestFocus();
				initLua();
			}
		});
		
		initPanel();
		initPanelOutput();
		initPanelInput();
		frameTop.setVisible(true);
	}

	public void initPanel() {
		panelOutput = new JPanel();
		panelOutput.setLayout(new GridLayout(1, 1));
		panelOutput.setBackground(new Color(220, 235, 245));
		frameTop.add(panelOutput, BorderLayout.CENTER);

		panelInput = new JPanel();
		panelInput.setLayout(new BorderLayout());
		panelInput.setPreferredSize(new Dimension(800, 100)); //small height
		panelInput.setBackground(new Color(222, 235, 245));
		frameTop.add(panelInput, BorderLayout.SOUTH);
	}

	public void initPanelOutput() {
		textAreaOutput = new JMenuTextArea();
		textAreaOutput.setBackground(Color.BLACK);
		textAreaOutput.setForeground(Color.GREEN);
		textAreaOutput.setCaretColor(Color.GREEN);
		textAreaOutput.setSelectionColor(Color.WHITE);
		textAreaOutput.setFont(font);
		textAreaOutput.setLineWrap(true);
		textAreaOutput.setWrapStyleWord(true);
		textAreaOutput.setEditable(false);
		textAreaOutput.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
		scrollPaneOutput = new JScrollPane(textAreaOutput);
		scrollPaneOutput.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
		panelOutput.add(scrollPaneOutput);
	}

	public void initPanelInput() {
		JLabel textAreaPre = new JLabel();
		textAreaPre.setBackground(Color.BLACK);
		textAreaPre.setForeground(Color.GREEN);
		textAreaPre.setFont(font);
		textAreaPre.setText("> ");
		textAreaPre.setOpaque(true);
		textAreaPre.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 0));
		textAreaPre.setHorizontalAlignment(SwingConstants.LEADING);
		textAreaPre.setVerticalAlignment(SwingConstants.TOP);
		panelInput.add(textAreaPre, BorderLayout.WEST);
		
		textAreaInput = new JMenuTextArea();
		textAreaInput.setBackground(Color.BLACK);
		textAreaInput.setForeground(Color.GREEN);
		textAreaInput.setCaretColor(Color.GREEN);
		textAreaInput.setSelectionColor(Color.WHITE);
		textAreaInput.setFont(font);
		textAreaInput.setLineWrap(true);
		textAreaInput.setWrapStyleWord(true);
		textAreaInput.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent event) {
				if (event.getKeyCode() == KeyEvent.VK_ENTER) {
					String input = textAreaInput.getText();
					log("> " + input);
					_arrHistory.add(input);
					_nHistoryIndex = _arrHistory.size();
					execute(input);
					textAreaInput.setText(null);
					event.consume();
				} else if (event.getKeyCode() == KeyEvent.VK_ESCAPE) {
					textAreaOutput.setText(null);
				} else if (event.getKeyCode() == KeyEvent.VK_UP) {
					if (_nHistoryIndex > 0) {
						_nHistoryIndex--;
					}
					if (_nHistoryIndex >= 0 && _nHistoryIndex < _arrHistory.size()) {
						String strTemp = _arrHistory.get(_nHistoryIndex);
						if (strTemp != null) {
							textAreaInput.setText(null);
							textAreaInput.append(strTemp);
						}
					}
				} else if (event.getKeyCode() == KeyEvent.VK_DOWN) {
					if (_nHistoryIndex < _arrHistory.size() - 1) {
						_nHistoryIndex++;
					}
					if (_nHistoryIndex >= 0 && _nHistoryIndex < _arrHistory.size()) {
						String strTemp = _arrHistory.get(_nHistoryIndex);
						if (strTemp != null) {
							textAreaInput.setText(null);
							textAreaInput.append(strTemp);
						}
					}
				}
			}
		});
		textAreaInput.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 5));
		scrollPaneInput = new JScrollPane(textAreaInput);
		scrollPaneInput.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
		panelInput.add(scrollPaneInput, BorderLayout.CENTER);
	}
	
	/*
	 * @see http://bbs.csdn.net/topics/330223621
	 */
	private void scrollBottom() {
//		textAreaInput.setCaretPosition(textAreaInput.getDocument().getLength());
//		JScrollBar sbar = scrollPaneInput.getVerticalScrollBar(); 
//		int max = sbar.getMaximum();
//		sbar.setValue(max);
//		Document doc = textAreaInput.getDocument();
//		textAreaInput.select(doc.getLength(), doc.getLength()); 
	}
	
	/*
	 * @see http://stackoverflow.com/questions/5147768/scroll-jscrollpane-to-bottom
	 */
	private void scrollToBottom(JScrollPane scrollPane) {
	    final JScrollBar verticalBar = scrollPane.getVerticalScrollBar();
	    AdjustmentListener downScroller = new AdjustmentListener() {
	        @Override
	        public void adjustmentValueChanged(AdjustmentEvent e) {
	            Adjustable adjustable = e.getAdjustable();
	            adjustable.setValue(adjustable.getMaximum());
	            verticalBar.removeAdjustmentListener(this);
	        }
	    };
	    verticalBar.addAdjustmentListener(downScroller);
	}

	/**
	 * @see http://www.oschina.net/code/snippet_54100_1242
	 * @author Administrator
	 *
	 */
	private final static class JMenuTextArea extends JTextArea implements MouseListener {
		private static final long serialVersionUID = -2308615404205560110L;

		private JPopupMenu pop;
		private JMenuItem copy;
		private JMenuItem paste;
		private JMenuItem cut;
		
		public JMenuTextArea() {
			super();
			init();
		}

		private void init() {
			this.addMouseListener(this);
			pop = new JPopupMenu();
			pop.add(copy = new JMenuItem("Copy"));
			pop.add(paste = new JMenuItem("Paste"));
			pop.add(cut = new JMenuItem("Cut"));
			copy.setAccelerator(KeyStroke.getKeyStroke('C', InputEvent.CTRL_MASK));
			paste.setAccelerator(KeyStroke.getKeyStroke('V', InputEvent.CTRL_MASK));
			cut.setAccelerator(KeyStroke.getKeyStroke('X', InputEvent.CTRL_MASK));
			copy.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					action(e);
				}
			});
			paste.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					action(e);
				}
			});
			cut.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					action(e);
				}
			});
			this.add(pop);
		}
		
		public void action(ActionEvent e) {
			String str = e.getActionCommand();
			if (str.equals(copy.getText())) {
				this.copy();
			} else if (str.equals(paste.getText())) { 
				this.paste();
			} else if (str.equals(cut.getText())) {
				this.cut();
			}
		}

		public JPopupMenu getPop() {
			return pop;
		}

		public void setPop(JPopupMenu pop) {
			this.pop = pop;
		}

		public boolean isClipboardString() {
			boolean b = false;
			Clipboard clipboard = this.getToolkit().getSystemClipboard();
			Transferable content = clipboard.getContents(this);
			try {
				if (content.getTransferData(DataFlavor.stringFlavor) instanceof String) {
					b = true;
				}
			} catch (Exception e) {
				
			}
			return b;
		}

		public boolean isCanCopy() {
			boolean b = false;
			int start = this.getSelectionStart();
			int end = this.getSelectionEnd();
			if (start != end) {
				b = true;
			}
			return b;
		}

		public void mouseClicked(MouseEvent e) {
			
		}

		public void mouseEntered(MouseEvent e) {
			
		}

		public void mouseExited(MouseEvent e) {
			
		}

		public void mousePressed(MouseEvent e) {
			if (e.getButton() == MouseEvent.BUTTON3) {
				copy.setEnabled(isCanCopy());
				paste.setEnabled(isClipboardString());
				cut.setEnabled(isCanCopy());
				pop.show(this, e.getX(), e.getY());
			}
		}

		public void mouseReleased(MouseEvent e) {
			
		}
	}
	
	private Lua _L = null;
	private final static boolean _isLoadLib = true;
	private List<String> _arrHistory = new ArrayList<String>();
	private int _nHistoryIndex = 0;
	
	private void execute(String str) {
		_L.setTop(0);
		BaseLib.OutputArr = new ArrayList<String>();
		BaseLib.OutputArr.add("");
		int res = _L.doString(str);
		StringBuffer OutputArrBuffer = new StringBuffer();
		for (int i = 0; i < BaseLib.OutputArr.size(); ++i) {
			String output = BaseLib.OutputArr.get(i);
			if (i == BaseLib.OutputArr.size() - 1) {
				OutputArrBuffer.append(output);
			} else {
				OutputArrBuffer.append(output).append("\n");
			}
		}
		log(OutputArrBuffer.toString(), false);
		if (res == 0) {
			Object obj = _L.value(1);
//			log(String(obj));
			Object tostring = _L.getGlobal("tostring");
			_L.push(tostring);
			_L.push(obj);
			_L.call(1, 1);
			String resultStr = _L.toString(_L.value(-1));
			log(resultStr);
		} else {
			String result = "Error: " + res;
			switch (res) {
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
	
	private void initLua() {
		textAreaInput.setText("return 1 + 1");
		
		log(Lua.RELEASE + "  " + Lua.COPYRIGHT);
		
		_L = new Lua();
		if (_isLoadLib) {
			BaseLib.open(_L);
			PackageLib.open(_L);
			MathLib.open(_L);
			OSLib.open(_L);
			StringLib.open(_L);
			TableLib.open(_L);
		}
	}
	
	private void log(String str) {
		log(str, true);
	}
	private void log(String str, boolean lineReturn) {
		textAreaOutput.append(str + (lineReturn ? "\n" : ""));
		textAreaInput.setText(null);
		scrollToBottom(scrollPaneInput);
	}
	
	public static void main(String[] args) {
		new Launcher();
	}
}
