package org.remimwen.Ryntra.Interpreter;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class BuiltinFunctions {
    private static final Set<String> BUILTIN_FUNCTIONS = new HashSet<>(Arrays.asList(
        "__builtin_print"
        // TODO: MORE FUNCTIONS PLEASE
    ));

    public static boolean isBuiltinFunction(String functionName) {
        return functionName.startsWith("__builtin_") && BUILTIN_FUNCTIONS.contains(functionName);
    }

    public static Set<String> getAllBuiltinFunctions() {
        return new HashSet<>(BUILTIN_FUNCTIONS);
    }

    public static boolean isValidBuiltinPrefix(String functionName) {
        return functionName.startsWith("__builtin_");
    }
}