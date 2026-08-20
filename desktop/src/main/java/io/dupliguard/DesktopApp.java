package io.dupliguard;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.List;

public final class DesktopApp extends JFrame {
    private final JTextField folder = new JTextField();
    private final JTextArea results = new JTextArea();
    private final JButton scan = new JButton("Scan for duplicates");
    private final JLabel status = new JLabel("Choose a folder to begin.");
    private final String executable;

    private DesktopApp(String executable) {
        super("DupliGuard"); this.executable = executable;
        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE); setMinimumSize(new Dimension(780, 520));
        var root = new JPanel(new BorderLayout(12,12)); root.setBorder(new EmptyBorder(18,18,18,18));
        var title = new JLabel("DupliGuard — Private duplicate file auditor"); title.setFont(title.getFont().deriveFont(Font.BOLD,20f)); root.add(title,BorderLayout.NORTH);
        var body = new JPanel(new BorderLayout(8,8));
        var chooser = new JPanel(new BorderLayout(8,8)); chooser.add(folder,BorderLayout.CENTER);
        var browse = new JButton("Browse…"); browse.addActionListener(e -> chooseFolder()); chooser.add(browse,BorderLayout.EAST); body.add(chooser,BorderLayout.NORTH);
        results.setEditable(false); results.setFont(new Font(Font.MONOSPACED,Font.PLAIN,13)); results.setLineWrap(false); body.add(new JScrollPane(results),BorderLayout.CENTER);
        var footer = new JPanel(new BorderLayout(8,8)); footer.add(status,BorderLayout.CENTER); scan.addActionListener(e -> startScan()); footer.add(scan,BorderLayout.EAST); body.add(footer,BorderLayout.SOUTH);
        root.add(body,BorderLayout.CENTER); setContentPane(root); pack(); setLocationRelativeTo(null);
    }

    private void chooseFolder() {
        var chooser = new JFileChooser(); chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        if (chooser.showOpenDialog(this)==JFileChooser.APPROVE_OPTION) folder.setText(chooser.getSelectedFile().getAbsolutePath());
    }

    private void startScan() {
        var selected=folder.getText().trim(); if(selected.isEmpty()){JOptionPane.showMessageDialog(this,"Choose a folder first.","Folder required",JOptionPane.WARNING_MESSAGE);return;}
        scan.setEnabled(false); results.setText(""); status.setText("Scanning locally… no data leaves this computer.");
        new SwingWorker<String,Void>() {
            protected String doInBackground() throws Exception {
                var process=new ProcessBuilder(List.of(executable,"scan", Path.of(selected).toString(),"--format","text")).redirectErrorStream(true).start();
                String text; try(var in=process.getInputStream()){text=new String(in.readAllBytes(), StandardCharsets.UTF_8);}
                int code=process.waitFor(); if(code!=0) throw new IOException(text.isBlank()?"Scanner exited with code "+code:text); return text;
            }
            protected void done(){try{results.setText(get());status.setText("Scan complete. Review results before taking any manual action.");}catch(Exception ex){status.setText("Scan failed.");JOptionPane.showMessageDialog(DesktopApp.this,ex.getCause()!=null?ex.getCause().getMessage():ex.getMessage(),"DupliGuard error",JOptionPane.ERROR_MESSAGE);}finally{scan.setEnabled(true);}}
        }.execute();
    }

    public static void main(String[] args) {
        String binary=args.length>0?args[0]:(System.getProperty("os.name").toLowerCase().contains("win")?"dupliguard.exe":"dupliguard");
        SwingUtilities.invokeLater(() -> new DesktopApp(binary).setVisible(true));
    }
}
