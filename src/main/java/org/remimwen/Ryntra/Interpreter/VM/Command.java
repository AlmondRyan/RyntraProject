package org.remimwen.Ryntra.Interpreter.VM;

import java.util.List;

public class Command {
    private OpCodes operation = OpCodes.HALT;
    private List<Object> arguments;

    public Command(OpCodes operation, List<Object> arguments) {
        this.operation = operation;
        this.arguments = arguments;
    }

    OpCodes getOperation() {
        return operation;
    }

    List<Object> getArguments() {
        return arguments;
    }

    Object queryArgument(int index) {
        return arguments.get(index);
    }
}
