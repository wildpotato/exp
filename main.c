void child() {
    int i;
    for (i=0; i < 0xFFFFFFF; i++) {
        asm("nop;");
    }
}

int main() {
    int i;
    for (i=0; i < 0xFFFFFFF; i++) {
        asm("nop;");
    }
    child();
}
