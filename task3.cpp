// compile with 
//   g++ -O2 -g -fopenmp -funroll-loops task3.cpp -o task3 -Wall -static

#include <cstdio>
#include <cstring>
#include <vector>
#include <climits>
#include <cassert>
#include <algorithm>
#include <map>
#include <string>
#include <deque>
#include <omp.h>
using namespace std;

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


#define OUTPUT_FILE "../task3_to_task1.in"
#define INPUT_FILE "../task3.in"
#define RESULT_DUMP "../task3_dump.txt"
#define PREPROCESS_DUMP "../task3_pre.tmp"
//#define PREPROCESS_DUMP_USE


#if 0 // use task2 data
#undef INPUT_FILE
#undef PREPROCESS_DUMP
#undef PREPROCESS_DUMP_USE
#define INPUT_FILE "../task2.in"
#endif

#define MAXN 100000
#define MAXM 1000000
#define MAXK 30
#define MAXSIGMA 4
int cp = 100;


#define INF 0x10000000


int n, m, k;
char a[MAXN + 1];
char db[MAXM + 1][MAXK + 1];
char lch[MAXM + 1];

void readdata()
{
    printf("reading data ...\n");
    FILE *fp = fopen(INPUT_FILE, "r");
    fscanf(fp, "%s", a); n = strlen(a);
    fscanf(fp, "%d", &m);
    for (int i = 1; i <= m; i++) {
        fscanf(fp, "%s", db[i]);
    }
    k = strlen(db[1]);
    fclose(fp);
    printf(" n=%d m=%d k=%d\n", n, m, k);
}



// fk[vertex][i] = f[vertex][i][k]
// fk is stripped
vector<int> fk[MAXM + 1];
#define _fk(x) (fk[x])
#define safeF(F, j) ( ((j) < (int) (F).size()) ? \
    ((F)[j]) : \
    ((F).back() + (j) - (int) (F).size() + 1) \
)
#define sFK(v, i) safeF(_fk(v), i)

static struct {
    int fk_size, fk_back;
    #define _fk_size(x) (vd[x].fk_size)
    #define _fk_back(x) (vd[x].fk_back)
    #define sFK_fast(v, j) ( unlikely((j) < _fk_size(v)) ? \
        (_fk(v)[j]) : \
        (_fk_back(v) + (j) - _fk_size(v) + 1) \
    )

    int next[MAXSIGMA];
    #define _next(x) (vd[x].next)
    int prev[MAXSIGMA];
    #define _prev(x) (vd[x].prev)


} vd[MAXM + 1];

void calc_fk(int v)
{

    vector<int> f[MAXK + 1];
#define sF(i, j) safeF(f[i], j)

    for (int i = 0; i <= k; i++) {
        f[i].clear(); f[i].push_back(i); // i.e. F[i][0] = i
    }
    for (int i = 1; i <= k; i++) {
        for (int j = 1; j <= n; j++) {
            int d[3];
            d[0] = sF(i - 1, j) + 1;
            d[1] = sF(i, j - 1) + 1;
            d[2] = sF(i - 1, j - 1) + (db[v][i - 1] != a[j - 1]);
            int m = INT_MAX;
            for (int k = 0; k < 3; k++) {
                if (d[k] < m) m = d[k];
            }
            //printf("%d i=%d j=%d m=%d\n", (j - i == m), i, j, m);
            f[i].push_back(m);
            if (j - i == m) break;
        }
    }

    _fk(v) = f[k];

#undef sF
}


void preprocess()
{
    #ifdef PREPROCESS_DUMP_USE
    {
        printf("reading preprocessed data ...\n");
        FILE *fp = fopen(PREPROCESS_DUMP, "rb");
        for (int v = 1; v <= m; v++) {
            int cnt;
            fread(&cnt, sizeof(int), 1, fp);
            _fk(v).clear();
            _fk(v).resize(cnt);
            fread(&_fk(v)[0], sizeof(int), cnt, fp);
        }
        fclose(fp);
        return;
    }
    #endif
    
    printf("preprocess ...\n");
    for (int v = 1; v <= m; v++) {
        calc_fk(v);
        if (v % 100000 == 0) printf(" v=%d\n", v);
    }

    #ifdef PREPROCESS_DUMP
    {
        printf(" dumping to file ...\n");
        FILE *fp = fopen(PREPROCESS_DUMP, "wb");
        for (int v = 1; v <= m; v++) {
            int cnt = _fk(v).size();
            fwrite(&cnt, sizeof(int), 1, fp);
            fwrite(&_fk(v)[0], sizeof(int), cnt, fp);
        }
        fclose(fp);
    }
    #endif
}

void makegraph()
{
    printf("makegraph ...\n");

    for (int i = 1; i <= m; i++) {
        memset(_next(i), 0, sizeof(_next(i)));
        memset(_prev(i), 0, sizeof(_prev(i)));
    }
    static int nc[MAXM + 1];
    static int pc[MAXM + 1];
    memset(nc, 0, sizeof(nc));
    memset(pc, 0, sizeof(pc));

    map<string, vector<int> > b;
    map<string, vector<int> >::iterator it;
    for (int i = 1; i <= m; i++) {
        b[string(db[i]).substr(0, k - 1)].push_back(i);
    }
    for (int i = 1; i <= m; i++) {
        string t = string(db[i]).substr(1);
        if ((it = b.find(t)) != b.end()) {
            vector<int> &v = it->second;
            vector<int>::iterator vit;
            for (vit = v.begin(); vit != v.end(); vit++) {
                int j = *vit;
                assert(nc[i] < MAXSIGMA);
                _next(i)[nc[i]++] = j;
                assert(pc[j] < MAXSIGMA);
                _prev(j)[pc[j]++] = i;
            }
        }
    }
}

struct row_data {
    
    // g[i][vertex]
    int g[MAXM + 1];
    #define G(x) (rd[x]->g)

    int h[MAXM + 1];
    #define H(x) (rd[x]->h)

} *rd[MAXN + 1];

int *h2[MAXN + 1];
int *h3[MAXN + 1];

#define SAMEROW_BIT 0x40000000
template<int pass>
void run()
{
    vector<int> p2r; // pass 2 result
    assert(pass == 1 || pass == 2);

    int lcp = 0;

    for (int i = 1; i <= n; i++) {

        printf("pass %d, iteration %d (%.2f%%) ...", pass, i, double(i) / n * 100.0);
        fflush(stdout);

        clock_t st = clock();
        double wst = omp_get_wtime();

        rd[i] = new row_data;
        
        #define MAXQ (MAXM * 10)
        static int q[MAXQ];
        int fr = 0, bk = 0;

        memset(H(i), -1, sizeof(H(i)[0]) * (m + 1)); 
        
        G(i - 1)[0] = INF;
        #pragma omp parallel for
        for (int v = 1; v <= m; v++) {
            int r = G(i - 1)[v] + 1, f = v;
            int t = sFK_fast(v, i);
            if (t < r) r = t, f = -v;

            int r2, f2;
            r2 = G(i - 1)[f2 = _prev(v)[0]];
            for (int s = 1; s < MAXSIGMA; s++) {
                int pv = _prev(v)[s];
                t = G(i - 1)[pv];
                if (t < r2) r2 = t, f2 = pv;
            }
            r2 += (lch[v] != a[i - 1]);
            if (r2 < r) r = r2, f = f2;

            G(i)[v] = r;
            if (pass == 1) H(i)[v] = f <= 0 ? f : H(i - 1)[f];
            if (pass == 2) H(i)[v] = f;
        }
        double wst2;
        printf(" s1 = %.3f,", (wst2 = omp_get_wtime()) - wst);

        G(i)[0] = INF;
        #pragma omp parallel for
        for (int v = 1; v <= m; v++) {
            int r, f;
            r = G(i)[f = _prev(v)[0]];
            for (int s = 1; s < MAXSIGMA; s++) {
                int pv = _prev(v)[s];
                int t = G(i)[pv]; if (t < r) r = t, f = pv;
            }
            r++;
            if (r < G(i)[v]) {
                G(i)[v] = r;
                if (pass == 1) H(i)[v] = H(i)[f];
                if (pass == 2) H(i)[v] = f | SAMEROW_BIT;
                for (int s = 0; s < MAXSIGMA; s++) {
                    int nv = _next(v)[s];
                    if (nv <= 0) break;
                    int old;
                    #pragma omp atomic capture
                    old = bk++;
                    q[old] = nv;
                }
            }
        }
        
        double wst3;
        printf(" s2 = %.3f,", (wst3 = omp_get_wtime()) - wst2);
        
        int loop_cnt = 0;
        while (fr < bk) {
            loop_cnt++;
            int v = q[fr++];
            
            int r, f;
            r = G(i)[f = _prev(v)[0]];
            for (int s = 1; s < MAXSIGMA; s++) {
                int pv = _prev(v)[s];
                int t = G(i)[pv]; if (t < r) r = t, f = pv;
            }
            r++;
            
            if (r < G(i)[v]) {
                G(i)[v] = r;
                if (pass == 1) H(i)[v] = H(i)[f];
                if (pass == 2) H(i)[v] = f | SAMEROW_BIT;

                for (int s = 0; s < MAXSIGMA; s++) {
                    int nv = _next(v)[s];
                    if (nv <= 0) break;
                    q[bk++] = nv;
                }
            }
        }

        assert(bk <= MAXQ);

        double wst4;
        printf(" s3 = %.3f,", (wst4 = omp_get_wtime()) - wst3);

        clock_t ed = clock();

        printf(" time = %.3fs, cpu = %.3fs, loop = %d\n", wst4 - wst, (double) (ed - st) / CLOCKS_PER_SEC, loop_cnt);
        

        if (pass == 1) {
            // checkpoint
            if (i % cp == 1 || i == n) {
                h2[i] = new int[m + 1];
                memcpy(h2[i], H(i), sizeof(H(i)[0]) * (m + 1));
                h3[i] = new int[m + 1];
                memset(h3[i], 0, sizeof(h3[i][0]) * (m + 1));
                for (int v = 0; v <= m; v++) {
                    H(i)[v] = v;
                }
            }
        }
        if (pass == 2) {
            if (i % cp == 1 || i == n) {
                printf("check point %d ...\n", i);
                int sv = 0;
                for (int v = 1; v <= m; v++) {
                    if (h3[i][v]) {
                        sv = v;
                        break;
                    }
                }
                if (sv != 0) {
                    vector<int> vl;
                    for (int r = i; r > lcp;) {
                        vl.push_back(sv);
                        int nv = H(r)[sv];
                        if (nv <= 0) {
                            vl.push_back(nv);
                            break;
                        }
                        if ((nv & SAMEROW_BIT)) {
                            nv &= ~SAMEROW_BIT;
                        } else {
                            r--;
                        }
                        sv = nv;
                    }
                    while (!vl.empty()) {
                        sv = vl.back(); vl.pop_back();
                        p2r.push_back(sv);
                    }
                }
            }
        }
        
        if (i % cp == 1 || i == n) {
            lcp = i;
        }

        // free memory
        if (i - cp >= 1) {
            delete rd[i - cp];
            rd[i - cp] = NULL;
        }
    }

    int final_result = INT_MAX;
    int final_vertex = 0;
    for (int i = 1; i <= m; i++) {
        if (G(n)[i] < final_result) {
            final_result = G(n)[i];
            final_vertex = i;
        }
    }
    printf("final result = %d, final vertex = %d\n", final_result, final_vertex);

    if (pass == 1) {

        vector<int> q;
        for (int i = 1; i <= n; i++) {
            if (i % cp == 1 || i == n) {
                q.push_back(i);
            }
        }
        int v = final_vertex;
        while (!q.empty()) {
            int r = q.back(); q.pop_back();
            h3[r][v] = 1;
            int nv = h2[r][v];
            printf("check point: %d -> %d\n", v, nv);
            v = nv;
            if (v <= 0) break;
        }
    }
    
    if (pass == 2) {
        printf("writing final data to '%s' ...\n", OUTPUT_FILE);
        p2r.resize(unique(p2r.begin(), p2r.end()) - p2r.begin());
        vector<int>::iterator it;
        FILE *fp = fopen(RESULT_DUMP, "w");
        for (it = p2r.begin(); it != p2r.end(); it++) {
            fprintf(fp, "%d\n", *it);
        }
        fclose(fp);
        fp = fopen(OUTPUT_FILE, "w");
        // write string 1
        fprintf(fp, "%s\n", a);
        // write string 2 (from graph)
        int negv = 0;
        for (it = p2r.begin(); it != p2r.end(); it++) {
            //fprintf(fp, "%d\n", *it);
            int v = *it;
            if (v < 0) {
                negv = -v;
                fprintf(fp, "%.*s", k, db[-v]);
            } else {
                if (v == negv) {
                    negv = -1;
                } else {
                    fprintf(fp, "%c", db[v][k - 1]);
                }
            }
        }
        fprintf(fp, "\n");
        fclose(fp);
    }

    for (int i = 1; i <= n; i++) {
        if (rd[i]) {
            delete rd[i];
            rd[i] = NULL;
        }
    }
}

int main()
{
    readdata();
    makegraph();
    preprocess();

    rd[0] = new row_data;
    memset(G(0), 1, sizeof(G(0)));

    vector<int> sv; // start vertex
    for (int i = 1; i <= m; i++) {
        if (_prev(i)[0] == -1) {
            sv.push_back(i);
        }
    }

    for (int i = 1; i <= m; i++) {
        lch[i] = db[i][k - 1];
    }
    
    for (int i = 1; i <= m; i++) {
        _fk_size(i) = _fk(i).size();
        _fk_back(i) = _fk(i).back();
    }

    printf("total %d start vertex.\n", (int) sv.size());

    run<1>(); 
    run<2>();

    return 0;
}
