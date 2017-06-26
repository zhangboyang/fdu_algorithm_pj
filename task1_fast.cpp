// compile with
//  g++ -O2 -g -fopenmp -funroll-loops task1_fast.cpp -o task1_fast -Wall -static

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <vector>
#include <cassert>
#include <omp.h>

using namespace std;

#define INPUT_FILE "../task3_to_task1.in"
#define OUTPUT_FILE "../task3.out"
#define MAXN 200000
int cp = 5000;

int la, lb;

char a[MAXN + 1], b[MAXN + 1];

// f[1...la][1...lb]
int *f[MAXN + 1];
int *g[MAXN + 1];
pair<int, int> *h2[MAXN + 1];
int *h3[MAXN + 1];
int *h4[MAXN + 1];

int u[] = {-1, 0, -1};
int v[] = {0, -1, -1};

FILE *ofp;

void print(int x, int y, int lx, int ly)
{
    int i, j, t;

    //printf("print: x=%d y=%d lx=%d ly=%d\n", x, y, lx, ly);
    
    for (i = x, j = y; i != lx && j != ly; t = g[i][j], i += u[t], j += v[t]) {
        //printf("i=%d j=%d\n", i, j);
        h4[i][j] = 1;
    }
    //printf(" -> i=%d j=%d\n", i, j);
    if (i == 0 || j == 0) {
        while (i--) fprintf(ofp, "DEL %d\n", i);
        for (i = 0; i < j; i++) fprintf(ofp, "INS 0 %c\n", b[i]);
    }
    for (i = lx + 1; i <= x; i++) {
        for (j = ly + 1; j <= y; j++) {
            if (h4[i][j]) {
                char ca = a[i - 1], cb = b[j - 1];
                switch (g[i][j]) {
                case 0:
                    fprintf(ofp, "DEL %d\n", i - 1);
                    break;
                case 1:
                    fprintf(ofp, "INS %d %c\n", i, cb);
                    break;
                case 2:
                    if (ca != cb) {
                        fprintf(ofp, "SUB %d %c\n", i - 1, cb);
                    }
                    break;
                }
            }
        }
    }
}

template<int pass>
void run()
{
    assert(pass == 1 || pass == 2);
    double wst, wed;
    int i, j, k;
    int lcp = 0;
    for (i = 0; i <= lb; i++) f[0][i] = i;
    for (i = 1; i <= la; i++) {
        //printf("i=%d\n", i);
        f[i] = new int[lb + 1];
        f[i][0] = i;
        g[i] = new int[lb + 1];
        h4[i] = new int[lb + 1];
        memset(h4[i], 0, sizeof(h4[i][0]) * (lb + 1));
        
        for (j = 1; j <= lb; j++) {
            //printf("i=%d j=%d\n", i, j);
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

        if (i % cp == 1 || i == la) {
            wst = omp_get_wtime();
            printf("pass %d checkpoint %d ... ", pass, i);
            fflush(stdout);
        }

        if (pass == 1) {
            if (i % cp == 1 || i == la) {
                
                h2[i] = new pair<int, int>[lb + 1];
                h3[i] = new int[lb + 1];
                memset(h3[i], 0, sizeof(h3[i][0]) * (lb + 1));
                #pragma omp parallel for
                for (k = 1; k <= lb; k++) {
                    int x, y, t;
                    for (x = i, y = k; x > 0 && x != lcp && y != 0; t = g[x][y], x += u[t], y += v[t]);
                    //printf("i=%d k=%d x=%d y=%d\n", i, k, x, y);
                    h2[i][k] = make_pair(x, y);
                }
            }
        }
    
        if (pass == 2) {
            if (i % cp == 1 || i == la) {
                for (k = 1; k <= lb; k++) {
                    if (h3[i][k]) break;
                }
                if (k <= lb) {
                    int x = i, y = k;
                    // print trace
                    print(x, y, lcp, 0);
                }
            }
        }

        if (i % cp == 1 || i == la) {
            wed = omp_get_wtime();
            printf("time = %.3f\n", wed - wst);
            lcp = i;
        }

        if (i - cp >= 1) {
            delete[] f[i - cp];
            f[i - cp] = NULL;
            delete[] g[i - cp];
            g[i - cp] = NULL;
            delete[] h4[i - cp];
            h4[i - cp] = NULL;
        }
    }

    printf("result = %d\n", f[la][lb]);



    if (pass == 1) {
        fprintf(ofp, "%d\n", f[la][lb]);
        int x = la, y = lb;
        while (x > 0 && y > 0) {
            //printf("x=%d y=%d\n", x, y);
            h3[x][y] = 1;
            pair<int, int> &next = h2[x][y];
            x = next.first, y = next.second;
        }
    }

    for (i = 1; i <= la; i++) {
        if (f[i]) {
            delete[] f[i];
            f[i] = NULL;
        }
        if (g[i]) {
            delete[] g[i];
            g[i] = NULL;
        }
        if (h4[i]) {
            delete[] h4[i];
            h4[i] = NULL;
        }
    }
}

int main()
{

    FILE *fp = fopen(INPUT_FILE, "r");
    fscanf(fp, "%s%s", a, b);
    fclose(fp);

    ofp = fopen(OUTPUT_FILE, "w");

    // copy b to final result
    fprintf(ofp, "%s\n", b);


    la = strlen(a);
    lb = strlen(b);
    printf("la=%d lb=%d\n", la, lb);
    
    
    f[0] = new int[lb + 1];
    
    run<1>();
    run<2>();

    fclose(ofp);
    return 0;
}
