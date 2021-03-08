#include <iostream>
#include <cstdlib>
#include <vector>

int main() {
    std::vector<int> size {128, 512, 4096, 8192};
    std::vector<char *> buffer;
    int sz = size.size();
    buffer.resize(sz);

    for (int i = 0; i < sz; ++i) {
        buffer[i] = (char *) malloc(size[i]);
        if (buffer[i] == NULL) exit (1);
        std::cout << "New'd " << size[i] << "bytes\n";
    }

    for (int i = 0; i < sz; ++i) {
        free(buffer[i]);
    }
    return 0;
}
