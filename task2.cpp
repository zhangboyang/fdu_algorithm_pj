#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <vector>
#include <algorithm>

using namespace std;

#define MAXN 10000

char a[MAXN + 1], b[MAXN + 1], c[MAXN + 1];
char ansa[MAXN + 1], ansb[MAXN + 1];
int f[MAXN + 1][MAXN + 1];
int g[MAXN + 1][MAXN + 1];
int h[MAXN + 1][MAXN + 1];
int u[] = {-1, 0, -1};
int v[] = {0, -1, -1};

int calc()
{
    int la = strlen(a);
    int lb = strlen(b);

    int i, j, k;
    for (i = 0; i <= la; i++) f[i][0] = i;
    for (i = 0; i <= lb; i++) f[0][i] = i;
    for (i = 1; i <= la; i++) {
        for (j = 1; j <= lb; j++) {
            char ca = a[i - 1], cb = b[j - 1];
            int d[3];

            d[0] = f[i - 1][j] + 1;
            d[1] = f[i][j - 1] + 1;
            d[2] = f[i - 1][j - 1] + (ca != cb);
            
            int m = INT_MAX;
            for (k = 0; k < 3; k++) {
                if (d[k] < m) {
                    m = d[k];
                    g[i][j] = k;
                }
            }
            f[i][j] = m;
        }
    }

    return f[la][lb];
}
void print()
{
    int la = strlen(a);
    int lb = strlen(b);
    FILE *fp = fopen("../task2.out", "w");
#define printf(...) fprintf(fp, __VA_ARGS__)

    printf("%s\n", b);
    printf("%d\n", f[la][lb]);
    int i, j, t;
    memset(h, 0, sizeof(h));
    for (i = la, j = lb; i != 0 && j != 0; t = g[i][j], i += u[t], j += v[t]) {
        h[i][j] = 1;
    }
    while (i--) printf("DEL 0\n");
    for (i = 0; i < j; i++) printf("INS 0 %c\n", b[i]);
    //while (j--) printf("INS 0 %c\n", b[j]);
    for (i = 1; i <= la; i++) {
        for (j = 1; j <= lb; j++) {
            if (h[i][j]) {
                //printf("i=%d j=%d f=%d h=%d g=%d\n", i, j, f[i][j], h[i][j], g[i][j]);
                char ca = a[i - 1], cb = b[j - 1];
                switch (g[i][j]) {
                case 0:
                    //printf("DEL %d\n", j);
                    printf("DEL %d\n", i - 1);
                    break;
                case 1:
                    //printf("INS %d %c\n", j - 1, cb);
                    printf("INS %d %c\n", i, cb);
                    break;
                case 2:
                    if (ca != cb) {
                        //printf("SUB %d %c\n", j - 1, cb);
                        printf("SUB %d %c\n", i - 1, cb);
                    }
                    break;
                }
            }
        }
    }
#undef printf
    fclose(fp);
}

int main()
{
    FILE *fp = fopen("../task2.in", "r");
    fscanf(fp, "%s", a);
    fclose(fp);

    fp = fopen("../task2_preprocessed.in", "r");
    int result;
    int m = INT_MAX;
    while (fscanf(fp, "%s", c) == 1) {
        puts(c);
        int lc = strlen(c);
        for (int i = 0; i < lc; i++) {
            for (int j = 1; j <= lc - i; j++) {
                strncpy(b, c + i, j); b[j] = 0;
                result = calc();
                printf("i=%d j=%d result=%d\n", i, j, result);
                if (result < m) {
                    m = result;
                    strcpy(ansa, a);
                    strcpy(ansb, b);
                }
            }
        }
    }
    fclose(fp);

    strcpy(a, ansa);
    strcpy(b, ansb);
    calc();
    print();

    return 0;
}
