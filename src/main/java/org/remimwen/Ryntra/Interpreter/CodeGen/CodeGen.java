package org.remimwen.Ryntra.Interpreter.CodeGen;

import org.remimwen.Ryntra.RyntraBaseVisitor;
import org.remimwen.Ryntra.RyntraParser;
import org.remimwen.Ryntra.Interpreter.VM.Command;
import org.remimwen.Ryntra.Interpreter.VM.OpCodes;
import org.remimwen.Ryntra.Interpreter.VM.utils.ConstantPool;
import org.remimwen.Ryntra.Interpreter.BuiltinFunctions;

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
        // Visit all the statements
        visitChildren(ctx);
        
        // Add HALT command to terminate the program
        commands.add(new Command(OpCodes.HALT, new ArrayList<>()));
        
        return null;
    }

    @Override
    public Void visitStatement(RyntraParser.StatementContext ctx) {
        return visitChildren(ctx);
    }

    @Override
    public Void visitFunctionCall(RyntraParser.FunctionCallContext ctx) {
        String functionName = ctx.IDENTIFIER().getText();
        
        // Process builtin functions
        if (BuiltinFunctions.isBuiltinFunction(functionName)) {
            generateBuiltinFunctionCall(functionName, ctx);
        }
        
        return null;
    }

    private void generateBuiltinFunctionCall(String functionName, RyntraParser.FunctionCallContext ctx) {
        switch (functionName) {
            case "__builtin_print":
                // Check builtin print
                if (ctx.expr() != null) {
                    // Visit the expression
                    visit(ctx.expr());
                } else {
                    // If there's no arguments, simply print the empty line
                    // Which means the call [LOAD_CONST String ""]
                    int index = constantPool.addString("");
                    commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("String", index)));
                }
                
                // Generate NATIVE_CALL Command
                commands.add(new Command(OpCodes.NATIVE_CALL, Arrays.asList("stdout")));
                break;
        }
    }

    // Process literals
    @Override
    public Void visitExpr(RyntraParser.ExprContext ctx) {
        if (ctx.STRING() != null) {
            // Process string literals
            String stringValue = ctx.STRING().getText();
            // Remove the quotation mark, or this could affect the generation to `""abc""`
            stringValue = stringValue.substring(1, stringValue.length() - 1);
            
            int index = constantPool.addString(stringValue);
            commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("String", index)));
            
        } else if (ctx.INT() != null) {
            // Process integer literals
            int intValue = Integer.parseInt(ctx.INT().getText());
            int index = constantPool.addInt(intValue);
            commands.add(new Command(OpCodes.LOAD_CONST, Arrays.asList("Int", index)));
        }
        
        return null;
    }
}