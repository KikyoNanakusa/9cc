#! /bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  echo "$input" >./test_cases/test.c
  ./9cc ./test_cases/test.c >tmp.s
  gcc -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input" = "$actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "int main() { char *a[] = {\"aaa\", \"bbb\", \"ccc\" }; return 0; }"
assert 97 "int main() { char *a[] = {\"aaa\", \"bbb\", \"ccc\" }; char *b = a[0]; return b[0]; }"

assert 97 "int main() { char a[] = {'a', 'b', 'c'}; return a[0]; }"
assert 98 "int main() { char a[] = {'a', 'b', 'c'}; return a[1]; }"
assert 99 "int main() { char a[] = {'a', 'b', 'c'}; return a[2]; }"

assert 0 "int main() { int a[] = {1, 2, 3}; return 0; }"
assert 1 "int main() { int a[] = { 1, 2, 3 }; return a[0]; }"
assert 2 "int main() { int a[] = { 1, 2, 3 }; return a[1]; }"
assert 3 "int main() { int a[] = { 1, 2, 3 }; return a[2]; }"

assert 0 "int main() { int a[3] = { 1, 2, 3 }; return 0; }"
assert 1 "int main() { int a[3] = { 1, 2, 3 }; return a[0]; }"
assert 2 "int main() { int a[3] = { 1, 2, 3 }; return a[1]; }"
assert 3 "int main() { int a[3] = { 1, 2, 3 }; return a[2]; }"

assert 0 "// test comment 
          int main() { return 0; }"

assert 0 "/* aaaa
bbbb
cccc
dddd
*/  
int main() { return 0; }"

assert 0 "char *c = \"aaa\"; int main() { return 0; }"
assert 97 "char *c = \"abc\"; int main() { return c[0]; }"
assert 98 "char *c = \"abc\"; int main() { return c[1]; }"

assert 0 "int main() {char *a = \"aaa\"; return 0;}"
assert 97 "int main() { char *a = \"abc\"; return a[0];}"
assert 98 "int main() { char *a = \"abc\"; return a[1];}"
assert 99 "int main() { char *a = \"abc\"; return a[2];}"
assert 97 "int main() { return \"abc\"[0]; }"
assert 98 "int main() { return \"abc\"[1]; }"
assert 99 "int main() { return \"abc\"[2]; }"
assert 4 "int main() { return sizeof(\"aaa\"); }"

assert 97 "int main() { char a = 'a'; return a;}"
assert 97 "char a = 'a'; int main() {return a;}"
assert 0 "int main() {char a = 'a'; char a_2 = 'a'; return a - a_2;}"
assert 1 "char a = 'a'; char b = 'b'; int main() { return b - a; }"

assert 0 'int main() {char a; return 0; }'
assert 0 'int main() {char a = 1; return 0; }'
assert 1 'int main() {char a = 1; return a; }'
assert 4 'int main() {char a = 1; char b = 3; char c = a+b; return c; }'
assert 1 'int main() {char a[3]; a[0] = 1; return a[0]; }'
assert 3 'int main() {char a[3]; a[0] = -1; a[1] = 2; int b = 4; return a[0] + b; }'
assert 1 'char a = 1; int main() { return a; }'
assert 2 'char a = 1; int main() { int b = 1; return a+b; }'
assert 3 'char a = 1; char b = 2; int main() { return a+b; }'

assert 0 'int a[20]; int main() { a[0] = 1; return 0; }'
assert 1 'int a[20]; int main() { a[0] = 1; return a[0]; }'
assert 4 'int a[20]; int main() { a[0] = 1; a[1] = 3; return a[0] + a[1]; }'
assert 3 'int a[20]; int b[10]; int main() { a[0] = 1; b[9] = 2; return a[0] + b[9]; }'

assert 1 'int x = 1; int main() { return x; }'
assert 3 'int x = 1; int y = 2; int main() { return x+y; }'
assert 4 'int x = 2; int main() { int y = 2; return x + y; }'
assert 2 'int x; int y; int main() { x = 1; y = 2; return y;}'
assert 3 'int x; int y; int main() { x = 1; y = 2; return x+y;}'
assert 1 'int x; int main() { x = 1; return x;}'
assert 1 'int x; int main() { x = 1; return x;} int y;'
assert 0 'int a = 0; int main() { return 0; }'
assert 0 'int a[20]; int main() { return 0; }'

assert 5 'int main() { int x=3; int y=5; int i=1; return *(&x+i); }'
assert 0 'int main() {int a[10]; return 0;}'
assert 1 'int main() { int a[2]; *a = 1; int *p; p = a; return *p;}'
assert 2 'int main() { int a[2]; *(a + 1) = 2; int *p; p = a; return *(p + 1);}'
assert 3 'int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}'

assert 1 "int main() { int a[2]; a[0]=1; return a[0]; }"
assert 2 "int main() { int a[2]; a[1]=2; return a[1]; }"
assert 3 "int main() { int a[2]; a[0]=1; a[1]=2; return a[0] + a[1]; }"

assert 1 "int main() { int i; for (i=0; i<3; i=i+1) { if (i==1) return i;} }"
assert 1 "int main() { int a[2]; int i; i = 1; a[1] = 1; return a[i];}"
assert 3 "int main() { int a[2]; int i; for (i=0; i<2; i=i+1) { a[i]=i+1; } return a[0]+a[1]; }"
assert 6 "int main() { int a=1; int b=2; int c=3; return a+b+c;}"
assert 12 "int main() { int a[3]; return sizeof(a);}"

assert 6 "int main() {int a[3]; a[0]=1; a[1]=2; a[2]=3; return a[0]+a[1]+a[2];}"

assert 4 'int main() { int x; return sizeof(x); }'
assert 4 'int main() { int x; return sizeof x; }'
assert 8 'int main() { int *x; return sizeof(x); }'
assert 8 'int main() {int x; int y; return sizeof(x) + sizeof(y); }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 5 'int main() { int x=3; int y=5; return *(1+&x); }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 5 'int main() { int x=3; int y=5; int *z=&x; return *(z+1); }'
assert 3 'int main() { int x=3; int y=5; int *z=&y; return *(z-1); }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-1)=7; return x; }'

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'
assert 21 'int main() { return 5+20-4; }'
assert 41 'int main() { return  12 + 34 - 5 ; }'
assert 47 'int main() { return 5+6*7; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'
assert 10 'int main() { return -10+20; }'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 42==42; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 42!=42; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 3 'int main() { int a; a=3; return a; }'
assert 8 'int main() { int a; int z; a=3; z=5; return a+z; }'
assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=3; int z=5; return a+z; }'

assert 1 'int main() { return 1; 2; 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 3 'int main() { 1; 2; return 3; }'

assert 3 'int main() { int foo=3; return foo; }'
assert 8 'int main() { int foo123=3; int bar=5; return foo123+bar; }'

assert 3 'int main() { if (0) return 2; return 3; }'
assert 3 'int main() { if (1-1) return 2; return 3; }'
assert 2 'int main() { if (1) return 2; return 3; }'
assert 2 'int main() { if (2-1) return 2; return 3; }'

assert 3 'int main() { {1; {2;} return 3;} }'

assert 10 'int main() { int i=0; i=0; while(i<10) i=i+1; return i; }'
assert 55 'int main() { int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 55 'int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main() { for (;;) return 3; return 5; }'

assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'
assert 8 'int main() { return add(3, 5); }'
assert 2 'int main() { return sub(5, 3); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

echo OK
