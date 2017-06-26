#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

#define MAXL 10000
#define MAXN 10000
#define MAXK 30

int la;
char a[MAXL + 1];


char db[MAXN + 1][MAXK + 1];
vector<int> e[MAXN + 1], e2[MAXN + 1];

int w[MAXN + 1];
void walk(int s)
{
    static char buf[MAXN + MAXK];
    char *ptr = buf;
    while (1) {
        assert(!w[s]);
        w[s] = 1;
        //printf("%-6d %s\n", s, db[s]);
        strcpy(ptr++, db[s]);
        if (e[s].empty()) {
            break;
        } else {
            assert(e[s].size() == 1);
            s = e[s][0];
        }
    }
    printf("%s\n", buf);
}

int main()
{
    int n, k;
    int i, j;

    freopen("../task2.in", "r", stdin);
    freopen("../task2_preprocessed.in", "w", stdout);
    scanf("%s", a); la = strlen(a);
    scanf("%d", &n);

    for (i = 1; i <= n; i++) {
        scanf("%s", db[i]);
    }

    k = strlen(db[1]);
    for (i = 1; i <= n; i++) {
        for (j = 1; j <= n; j++) {
            if (strncmp(db[i] + 1, db[j], k - 1) == 0) {
                e[i].push_back(j);
                e2[j].push_back(i);
            }
        }
    }

    for (i = 1; i <= n; i++) {
        assert(e[i].size() < 2 && e2[i].size() < 2);
        //printf("%d e:%d e2:%d\n", i, (int) e[i].size(), (int) e2[i].size());
    }
    for (i = 1; i <= n; i++) {
        if (e2[i].empty()) {
            walk(i);
        }
    }

    return 0;
}
