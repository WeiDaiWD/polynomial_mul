#include "simd_poly.h"

uint64_t rdtsc() {
  unsigned int lo,hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

//#define rdtscp(t) {asm volatile ("rdtscp" : "=A"(t));}

class Data {

public:

  inline Data();

  inline Data(uint16_t N) {
    n = N;
    a = (uint16_t *)malloc(sizeof(uint16_t) * 2 * n);
    b = (uint16_t *)malloc(sizeof(uint16_t) * 2 * n);
    t = (uint16_t *)malloc(sizeof(uint16_t) * 4 * n);
    r = (uint16_t *)malloc(sizeof(uint64_t) * 4 * n);
  }

  inline ~Data() {}

  uint16_t *a;
  uint16_t *b;
  uint16_t *r;
  uint16_t *t;
  uint16_t  n;

  inline void SetZero() {
    memset(a, 0, sizeof(uint16_t) * 2 * n);
    memset(b, 0, sizeof(uint16_t) * 2 * n);
    memset(t, 0, sizeof(uint16_t) * 4 * n);
    memset(r, 0, sizeof(uint16_t) * 4 * n);
  }

  inline void Rand(uint16_t *x, uint16_t N) {
    for (int i = 0; i < N; i ++)
      x[i] = rand() & 0x07ff;
  }

  inline void Randa() { Rand(a, n); }

  inline void Randb() { Rand(b, n); }

  inline void PolyMul() {
    karatsuba_toom4(r, t, a, b, n);
    //__mm256i_karatsuba__mm256_toom4(r, t, a, b, n);
  }

  inline void Clean() {
    SetZero();
    free(a);
    free(b);
    free(r);
    free(t);
    n = 0;
  }

};

#define start() { asm volatile ("mfence"); startc = rdtsc();}
#define stop() { asm volatile ("mfence"); stopc = rdtsc();}

void Measure(uint16_t N) {
  Data data(N);
  uint64_t startc, stopc;
  uint16_t count = 10;//000;
  uint64_t sum[4] = {0, 0, 0, 0};
  ofstream fout;

  // warm up
  fout.open("res3.txt");
  for (int i = 0; i < count; i ++) {
    data.SetZero();
    data.Randa();
    data.Randb();
    data.PolyMul(); // extra warm up
    start();
    data.PolyMul();
    stop();
    sum[3] += stopc - startc;
    fout<< stopc - startc <<endl;
  }
  fout.close();

  // fixed * fixed
  uint64_t max = 0, min = -1;
  fout.open("res0.txt");
  for (int i = 0; i < count; i ++) {
//    data.SetZero();
    data.PolyMul(); // extra warm up
    start();
    data.PolyMul();
    stop();
    sum[0] += stopc - startc;
    max = max > (stopc - startc) ? max : (stopc - startc);
    min = min < (stopc - startc) ? min : (stopc - startc);
    fout<< stopc - startc <<endl;
  }
  fout.close();
  cout<< max - min <<endl;

  // rand * fixed
  fout.open("res1.txt");
  for (int i = 0; i < count; i ++) {
//    data.SetZero();
    data.Randa();
    data.PolyMul(); // extra warm up
    start();
    data.PolyMul();
    stop();
    sum[1] += stopc - startc;
    fout<< stopc - startc <<endl;
  }
  fout.close();

  // rand * rand
  fout.open("res2.txt");
  for (int i = 0; i < count; i ++) {
//    data.SetZero();
    data.Randa();
    data.Randb();
    data.PolyMul(); // extra warm up
    start();
    data.PolyMul();
    stop();
    sum[2] += stopc - startc;
    fout<< stopc - startc <<endl;
  }
  fout.close();

  for (int i = 0; i < 4; i ++)
    printf("%lu\n", sum[i] / count);

  data.Clean();
}

int main() {
  srand(3);
  uint16_t N = 768;
  Measure(N);
  return 0;
}
