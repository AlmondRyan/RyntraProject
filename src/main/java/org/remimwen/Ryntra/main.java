package org.remimwen.Ryntra;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.ParseTree;
import org.remimwen.Ryntra.Interpreter.SemanticAnalyzer.SemanticAnalyzer;
import org.remimwen.Ryntra.Interpreter.CodeGen.CodeGen;
import org.remimwen.Ryntra.Interpreter.VM.VM;
import org.remimwen.Ryntra.Interpreter.VM.Command;
import org.remimwen.Ryntra.Interpreter.VM.utils.ConstantPool;

import java.util.List;
import java.util.Scanner;

public class main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        String input = scanner.nextLine();
        Compilation(input);
        scanner.close();
    }
    
    private static void Compilation(String sourceCode) {
        try {
            // Lexing and Parsing
            CharStream charStream = CharStreams.fromString(sourceCode);
            RyntraLexer lexer = new RyntraLexer(charStream);
            CommonTokenStream tokenStream = new CommonTokenStream(lexer);
            RyntraParser parser = new RyntraParser(tokenStream);
            
            final boolean[] hasErrors = {false};
            parser.removeErrorListeners();
            parser.addErrorListener(new BaseErrorListener() {
                @Override
                public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol,
                                        int line, int charPositionInLine, String msg,
                                        RecognitionException e) {
                    hasErrors[0] = true;
                    if (msg.contains("missing ';'")) {
                        System.err.println("[Error] Expect ';' at position " + line + ":" + charPositionInLine + ".");
                    } else {
                        System.err.println("[Error]: " + msg + " at line " + line + ", position " + charPositionInLine);
                    }
                }
            });

            ParseTree tree = parser.program();
            
            if (hasErrors[0]) {
                System.err.println("Compilation failed due to syntax errors. Execution aborted.");
                return;
            }
            
            // Semantic Analyzing
            SemanticAnalyzer analyzer = new SemanticAnalyzer();
            analyzer.visit(tree);
            
            // CodeGen
            CodeGen codeGen = new CodeGen();
            codeGen.visit(tree);
            
            // Get Commands and Constant Pool
            List<Command> commands = codeGen.getCommands();

            ConstantPool constantPool = codeGen.getConstantPool();
            
            // Execute Commands
            VM vm = new VM();
            vm.execute(commands, constantPool);
        } catch (Exception e) {
            System.err.println("Unexpected error during compilation and/or execution: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
