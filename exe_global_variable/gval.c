/*
int main() {
  return 42;
}

0000000000000000 <main>:
   0:   48 c7 c0 2a 00 00 00    mov    rax,0x2a
   7:   c3                      ret
*/

char main[] = "\x48\xc7\xc0\x2a\x00\x00\x00\xc3";
