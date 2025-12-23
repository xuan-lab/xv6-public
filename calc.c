// calc.c - Simple Calculator for xv6
// Usage: 
//   calc <num1> <op> <num2>     - Single calculation
//   calc -i                     - Interactive mode (stays running)
// Operations: + - x / m(mod)

#include "types.h"
#include "stat.h"
#include "user.h"

int
calculate(int a, char op, int b)
{
  switch(op) {
  case '+':
    return a + b;
  case '-':
    return a - b;
  case 'x':
  case 'X':
  case '*':
    return a * b;
  case '/':
    if(b == 0) {
      printf(1, "Error: Division by zero!\n");
      return 0;
    }
    return a / b;
  case 'm':
  case 'M':
  case '%':
    if(b == 0) {
      printf(1, "Error: Division by zero!\n");
      return 0;
    }
    return a % b;
  default:
    printf(1, "Unknown operator. Use: + - x / m\n");
    return 0;
  }
}

void
interactive_mode(void)
{
  char buf[128];
  char num1_str[32], op_str[8], num2_str[32];
  int a, b, result;
  char op;
  
  printf(1, "\n");
  printf(1, "=================================\n");
  printf(1, "  Interactive Calculator (xv6)\n");
  printf(1, "  PID: %d\n", getpid());
  printf(1, "=================================\n");
  printf(1, "Enter: num1 op num2 (e.g., 10 + 5)\n");
  printf(1, "Operators: + - x / m(mod)\n");
  printf(1, "Type 'q' to quit\n\n");
  
  while(1) {
    printf(1, "calc> ");
    
    gets(buf, sizeof(buf));
    
    if(buf[0] == 'q' || buf[0] == 'Q') {
      printf(1, "Goodbye!\n");
      break;
    }
    
    if(buf[0] == '\n' || buf[0] == '\0') {
      continue;
    }
    
    int i = 0, j = 0;
    
    while(buf[i] == ' ') i++;
    
    j = 0;
    while(buf[i] && buf[i] != ' ' && buf[i] != '\n' && j < 31) {
      num1_str[j++] = buf[i++];
    }
    num1_str[j] = 0;
    
    while(buf[i] == ' ') i++;
    
    j = 0;
    while(buf[i] && buf[i] != ' ' && buf[i] != '\n' && j < 7) {
      op_str[j++] = buf[i++];
    }
    op_str[j] = 0;
    
    while(buf[i] == ' ') i++;
    
    j = 0;
    while(buf[i] && buf[i] != ' ' && buf[i] != '\n' && j < 31) {
      num2_str[j++] = buf[i++];
    }
    num2_str[j] = 0;
    
    if(num1_str[0] == 0 || op_str[0] == 0 || num2_str[0] == 0) {
      printf(1, "Usage: num1 op num2 (e.g., 10 + 5)\n");
      continue;
    }
    
    a = atoi(num1_str);
    op = op_str[0];
    b = atoi(num2_str);
    
    result = calculate(a, op, b);
    
    if(op == 'x' || op == 'X' || op == '*') {
      printf(1, "%d x %d = %d\n", a, b, result);
    } else if(op == 'm' || op == 'M' || op == '%') {
      printf(1, "%d mod %d = %d\n", a, b, result);
    } else {
      printf(1, "%d %s %d = %d\n", a, op_str, b, result);
    }
  }
}

int
main(int argc, char *argv[])
{
  int a, b, result;
  char op;

  if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'i') {
    interactive_mode();
    exit();
  }

  if(argc != 4) {
    printf(1, "Simple Calculator for xv6\n");
    printf(1, "Usage:\n");
    printf(1, "  calc <num1> <op> <num2>  - Single calculation\n");
    printf(1, "  calc -i                  - Interactive mode\n");
    printf(1, "\nOperations: + - x / m(mod)\n");
    printf(1, "Examples:\n");
    printf(1, "  calc 10 + 5\n");
    printf(1, "  calc 6 x 7\n");
    printf(1, "  calc -i    <- stays running, use sysinfo -p to see it!\n");
    exit();
  }

  a = atoi(argv[1]);
  op = argv[2][0];
  b = atoi(argv[3]);

  result = calculate(a, op, b);
  
  if(op == 'x' || op == 'X' || op == '*') {
    printf(1, "%d x %d = %d\n", a, b, result);
  } else if(op == 'm' || op == 'M' || op == '%') {
    printf(1, "%d mod %d = %d\n", a, b, result);
  } else {
    printf(1, "%d %s %d = %d\n", a, argv[2], b, result);
  }

  exit();
}
