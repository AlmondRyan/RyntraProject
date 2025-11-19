package org.remimwen.Ryntra.Interpreter.SemanticAnalyzer;

import org.remimwen.Ryntra.RyntraBaseVisitor;
import org.remimwen.Ryntra.RyntraParser;

public class SemanticAnalyzer extends RyntraBaseVisitor<Void> {
    @Override
    public Void visitPrintStmt(RyntraParser.PrintStmtContext ctx) {
        // 检查是否有表达式参数
        if (ctx.expr() != null) {
            // 检查表达式类型
            if (ctx.expr().INT() == null && ctx.expr().STRING() == null) {
                System.err.println("[Error]: Method 'print' expected Int or String, but got invalid expression.");
            }
        } else {
            System.out.println("[Warning]: print() without arguments will print an empty line.");
        }
        return visitChildren(ctx);
    }

    @Override
    public Void visitProgram(RyntraParser.ProgramContext ctx) {
        return visitChildren(ctx);
    }
}