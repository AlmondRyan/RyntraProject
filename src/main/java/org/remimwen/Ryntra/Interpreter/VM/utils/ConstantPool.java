package org.remimwen.Ryntra.Interpreter.VM.utils;

import java.util.ArrayList;
import java.util.List;

public class ConstantPool {
    private final List<String> stringConstants = new ArrayList<String>();
    private final List<Integer> intConstants = new ArrayList<Integer>();

    public int addInt(int targ) {
        intConstants.add(targ);
        return intConstants.size() - 1; // 返回新添加元素的索引
    }

    public int addString(String targ) {
        stringConstants.add(targ);
        return stringConstants.size() - 1; // 返回新添加元素的索引
    }

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
