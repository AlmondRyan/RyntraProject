package org.remimwen.Ryntra.Interpreter.VM;

import org.remimwen.Ryntra.Interpreter.VM.utils.ConstantPool;

import java.util.List;
import java.util.Objects;
import java.util.Stack;

public class VM {
    private Stack<Object> stack = new Stack<>();
    private List<Command> operationCodes;
    private ConstantPool constantPool;

    public void execute(List<Command> opCodes, ConstantPool pool) {
        this.operationCodes = opCodes;
        this.constantPool = pool;

        for (var i : operationCodes) {
            OpCodes opCode = i.getOperation();
            switch (opCode) {
                case LOAD_CONST: // LOAD_CONST [type] [index]
                    String type = i.getArguments().get(0).toString();
                    Object obj = null;

                    if (Objects.equals(type, "String")) {
                        obj = constantPool.getConstant((Integer) i.getArguments().get(1), String.class);
                    } else if (Objects.equals(type, "Int")) {
                        obj = constantPool.getConstant((Integer) i.getArguments().get(1), Integer.class);
                    }

                    stack.push(obj);
                    break; // 修复：添加缺失的 break

                case NATIVE_CALL: // NATIVE_CALL [call] [arguments]
                    String call = i.getArguments().get(0).toString();
                    if (Objects.equals(call, "stdout")) {
                        // 检查栈是否为空，防止 EmptyStackException
                        if (stack.isEmpty()) {
                            System.err.println("[Runtime Error]: Attempted to pop from empty stack during stdout call");
                            return;
                        }
                        // 从栈中弹出参数并打印
                        Object arg = stack.pop();
                        System.out.println(arg);
                    }
                    break;

                case HALT:
                    return; // 停止执行

                default:
                    break;
            }
        }
    }
}
