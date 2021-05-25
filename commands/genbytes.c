#include <stdio.h>
#include <stdint.h>


struct {
    unsigned type : 2;
    unsigned act : 6;
    int16_t value;
} pack = {
    .type=0, 
    .act=1,
    .value=0
};


int main() {
    FILE *fp = fopen("gen", "wb");
    fwrite(&pack, 1, sizeof(pack), fp);
    fclose(fp);
}
