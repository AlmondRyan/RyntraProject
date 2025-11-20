package org.remimwen.Ryntra.Interpreter.VM;

public enum OpCodes {
    LOAD_CONST,               // LOAD_CONST [Type] [Index]
    NATIVE_CALL,              // NATIVE_CALL [Native Handle]
    HALT                      // HALT
}
