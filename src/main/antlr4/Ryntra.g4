grammar Ryntra;

// Parsing Elements
program: statement+ EOF;
statement: functionCall ';';

functionCall: IDENTIFIER '(' expr? ')';

expr: STRING | INT;

// Lexical Elements
INT: [0-9]+;
STRING: '"' .*? '"';
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;
WS: [ \t\r\n]+ -> skip;