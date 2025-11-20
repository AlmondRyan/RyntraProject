package org.remimwen.Ryntra.Interpreter.VM.utils;

import java.util.ArrayList;
import java.util.List;

public class ConstantPool {
    private final List<String> stringConstants = new ArrayList<String>();
    private final List<Integer> intConstants = new ArrayList<Integer>();

    /**
     * Adds an integer to the constant pool.
     * @param targ The integer value to add
     * @return The index of the newly added integer in the constant pool
     */
    public int addInt(int targ) {
        intConstants.add(targ);
        return intConstants.size() - 1;
    }

    /**
     * Adds a string to the constant pool.
     * @param targ The string to add
     * @return The index of the newly added string in the constant pool
     */
    public int addString(String targ) {
        stringConstants.add(targ);
        return stringConstants.size() - 1;
    }

    /**
     * Gets a constant by its index and type.
     *
     * @param index The index of the constant to retrieve; typically obtained from a prior call
     *              to {@code addString()}, {@code addInt()}, or something similar.
     * @param type  The type of the constant to retrieve
     * @return The constant value corresponding to the given index and type
     * @throws IllegalArgumentException if the index or type is not stored.
     */
    public Object getConstant(int index, Class<?> type) {
        if (type == Integer.class) {
            if (index >= 0 && index < intConstants.size()) {
                return intConstants.get(index);
            }
        } else if (type == String.class) {
            if (index >= 0 && index < stringConstants.size()) {
                return stringConstants.get(index);
            }
        }
        throw new IllegalArgumentException("Invalid constant index or type: " + index + ", " + type);
    }
}
