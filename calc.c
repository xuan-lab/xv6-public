// calc.c - Simple Calculator for xv6
// Supports: + - * / operations
// Usage: calc <num1> <op> <num2>
// Example: calc 10 + 5

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int a, b, result;
  char op;

  if(argc != 4) {
    printf(1, "Simple Calculator for xv6\n");
    printf(1, "Usage: calc <num1> <op> <num2>\n");
    printf(1, "Operations: + - x /\n");
    printf(1, "Examples:\n");
    printf(1, "  calc 10 + 5\n");
    printf(1, "  calc 20 - 8\n");
    printf(1, "  calc 6 x 7\n");
    printf(1, "  calc 100 / 4\n");
    printf(1, "\nNote: Use 'x' for multiply (shell treats * specially)\n");
    exit();
  }

  a = atoi(argv[1]);
  op = argv[2][0];
  b = atoi(argv[3]);

  switch(op) {
  case '+':
    result = a + b;
    printf(1, "%d + %d = %d\n", a, b, result);
    break;
  case '-':
    result = a - b;
    printf(1, "%d - %d = %d\n", a, b, result);
    break;
  case 'x':
  case 'X':
  case '*':
    result = a * b;
    printf(1, "%d x %d = %d\n", a, b, result);
    break;
  case '/':
    if(b == 0) {
      printf(1, "Error: Division by zero!\n");
      exit();
    }
    result = a / b;
    printf(1, "%d / %d = %d\n", a, b, result);
    break;
  case '%':
  case 'm':
    if(b == 0) {
      printf(1, "Error: Division by zero!\n");
      exit();
    }
    result = a % b;
    printf(1, "%d mod %d = %d\n", a, b, result);
    break;
  default:
    printf(1, "Unknown operator: %s\n", argv[2]);
    printf(1, "Supported: + - x / m(mod)\n");
    exit();
  }

  exit();
}
