int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }
