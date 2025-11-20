package org.remimwen.Ryntra.Interpreter.VM;

import java.util.List;

public class Command {
    private OpCodes operation = OpCodes.HALT;
    private List<Object> arguments;

    /**
     * Constructor of class {@code Command}. Which accepts ONE operation and arguments.
     * <p>
     * The Command should like this below:
     * <pre>
     * {@code
     * LOAD_CONST Int 0
     * }
     * </pre>
     * which means load constant index 0, and typed Int into stack. For the commands accept
     * one argument:
     * <pre>
     * {@code
     * NATIVE_CALL stdout
     * }
     * </pre>
     * which means call stdout handle to output the top of stack.
     *
     * @param operation The operation that current command has.
     * @param arguments The arguments that current operation need.
     */
    public Command(OpCodes operation, List<Object> arguments) {
        this.operation = operation;
        this.arguments = arguments;
    }

    public OpCodes getOperation() {
        return operation;
    }

    public List<Object> getArguments() {
        return arguments;
    }

    Object queryArgument(int index) {
        return arguments.get(index);
    }
}
