package org.remimwen.Ryntra.Interpreter.VM;

import org.remimwen.Ryntra.Interpreter.VM.utils.ConstantPool;

import java.util.List;
import java.util.Objects;
import java.util.Stack;

public class VM {
    private final Stack<Object> stack = new Stack<>();
    private List<Command> operationCodes;
    private ConstantPool constantPool;

    public void execute(List<Command> opCodes, ConstantPool pool) {
        this.operationCodes = opCodes;
        this.constantPool = pool;

        for (var i : operationCodes) {
            OpCodes opCode = i.getOperation();
            switch (opCode) {
                case LOAD_CONST: // LOAD_CONST [type] [index]
                    // Get type and set the constant object
                    String type = i.getArguments().get(0).toString();
                    Object obj = null;

                    // I don't know whether this is the best approach of coding or not. But this simply
                    // works.
                    if (Objects.equals(type, "String")) {
                        obj = constantPool.getConstant((Integer) i.getArguments().get(1), String.class);
                    } else if (Objects.equals(type, "Int")) {
                        obj = constantPool.getConstant((Integer) i.getArguments().get(1), Integer.class);
                    }

                    stack.push(obj);
                    break;

                case NATIVE_CALL: // NATIVE_CALL [call] [arguments]
                    String call = i.getArguments().getFirst().toString();
                    if (Objects.equals(call, "stdout")) {
                        if (stack.isEmpty()) {
                            System.err.println("[Error]: Attempted to pop from empty stack during NATIVE_CALL command");
                            return;
                        }

                        Object arg = stack.pop();
                        System.out.println(arg);
                    }
                    break;

                case HALT: // HALT
                    // This means stop executing.
                    return;

                default:
                    break;
            }
        }
    }
}
