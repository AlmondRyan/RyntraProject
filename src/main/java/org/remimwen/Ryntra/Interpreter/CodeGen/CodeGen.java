package org.remimwen.Ryntra.Interpreter.CodeGen;

import org.remimwen.Ryntra.RyntraBaseVisitor;
import org.remimwen.Ryntra.RyntraParser;
import org.remimwen.Ryntra.Interpreter.VM.Command;
import org.remimwen.Ryntra.Interpreter.VM.OpCodes;
import org.remimwen.Ryntra.Interpreter.VM.utils.ConstantPool;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class CodeGen extends RyntraBaseVisitor<Void> {
    private List<Command> commands = new ArrayList<>();
    private ConstantPool constantPool = new ConstantPool();

    public List<Command> getCommands() {
        return commands;
    }

    public ConstantPool getConstantPool() {
        return constantPool;
    }

    @Override
    public Void visitProgram(RyntraParser.ProgramContext ctx) {
        // 访问所有 print 语句
        visitChildren(ctx);
        
        // 添加 HALT 指令结束程序
        commands.add(new Command(OpCodes.HALT, new ArrayList<>()));
        
        return null;
    }

    @Override
    public Void visitPrintStmt(RyntraParser.PrintStmtContext ctx) {
        // 检查是否有表达式参数
        if (ctx.expr() != null) {
            // 访问表达式，生成加载常量的指令
            visit(ctx.expr());
            
            // 生成 NATIVE_CALL 指令来打印
            commands.add(new Command(OpCodes.NATIVE_CALL, Arrays.asList("stdout")));
        } else {
            // 如果没有参数，打印空行
            // 加载空字符串到常量池
            int index = constantPool.addString("");
            commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("String", index)));
            commands.add(new Command(OpCodes.NATIVE_CALL, Arrays.asList("stdout")));
        }
        
        return null;
    }

    @Override
    public Void visitExpr(RyntraParser.ExprContext ctx) {
        if (ctx.STRING() != null) {
            // 处理字符串常量
            String stringValue = ctx.STRING().getText();
            // 移除引号
            stringValue = stringValue.substring(1, stringValue.length() - 1);
            
            // 添加到常量池并获取索引
            int index = constantPool.addString(stringValue);
            
            // 生成 LOAD_CONST 指令
            commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("String", index)));
            
        } else if (ctx.INT() != null) {
            // 处理整数常量
            int intValue = Integer.parseInt(ctx.INT().getText());
            
            // 添加到常量池并获取索引
            int index = constantPool.addInt(intValue);
            
            // 生成 LOAD_CONST 指令
            commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("Int", index)));
        }
        
        return null;
    }
}