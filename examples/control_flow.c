int main() {
    int total = 0;
    int i = 1;

    while (i <= 10) {
        if (i == 7) {
            total = total + 100;
        } else {
            total = total + i;
        }
        i = i + 1;
    }

    return total;
}