package org.remimwen.Ryntra.Interpreter.SemanticAnalyzer;

import org.remimwen.Ryntra.RyntraBaseVisitor;
import org.remimwen.Ryntra.RyntraParser;
import org.remimwen.Ryntra.Interpreter.BuiltinFunctions;

public class SemanticAnalyzer extends RyntraBaseVisitor<Void> {
    @Override
    public Void visitProgram(RyntraParser.ProgramContext ctx) {
        return visitChildren(ctx);
    }

    @Override
    public Void visitFunctionCall(RyntraParser.FunctionCallContext ctx) {
        String functionName = ctx.IDENTIFIER().getText();
        
        // Check the function is builtin func or not
        if (BuiltinFunctions.isValidBuiltinPrefix(functionName)) {
            if (BuiltinFunctions.isBuiltinFunction(functionName)) {
                validateBuiltinFunction(functionName, ctx);
            } else {
                System.err.println("[Error]: Unknown builtin function '" + functionName + "'");
            }
        } else {
            System.err.println("[Error]: Unknown function '" + functionName + "'");
        }
        
        return visitChildren(ctx);
    }

    private void validateBuiltinFunction(String functionName, RyntraParser.FunctionCallContext ctx) {
        switch (functionName) {
            case "__builtin_print":
                // `__builtin_print()` can have 0 or 1 arguments to call
                if (ctx.expr() != null) {
                    if (ctx.expr().INT() == null && ctx.expr().STRING() == null) {
                        System.err.println("[Error]: __builtin_print expected Int or String, but got invalid expression.");
                    }
                }
                break;
            default:
                System.err.println("[Error]: Unknown builtin function '" + functionName + "'");
        }
    }
}