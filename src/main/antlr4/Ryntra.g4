grammar Ryntra;

// Parsing Elements
program: printStmt+ EOF;
printStmt: PRINT '(' expr ')' ';';

expr: STRING | INT;

// Lexical Elements
PRINT: 'print';
INT: [0-9]+;
STRING: '"' .*? '"';
WS: [ \t\r\n]+ -> skip;