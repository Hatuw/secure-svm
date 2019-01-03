#ifndef __SVMITER_H_
#define __SVMITER_H_

#include "assert.h"
#include "mpc.h"
#include <NTL/mat_ZZ_p.h>
#include <vector>

using namespace std;

class SVMIterator {
public:
  static const int TERM_CODE = 0;
  static const int G_CODE = 1;
  static const int GP_CODE = 2;
  static const int GM_CODE = 3;
  static const int GMP_CODE = 4;

  explicit SVMIterator(MPCEnv &mpc, int pid) {
    this->mpc = &mpc;
    this->pid = pid;
  }

  void Init(bool pheno_flag, bool missing_flag) {
    this->pheno_flag = pheno_flag;
    this->missing_flag = missing_flag;
    if (pid == 1)
      mpc->SendInt(TransferMode(), 3);

    cout << "Initialized SVMIterator" << endl;
  }

  void Terminate() {
    if (pid == 1)
      mpc->SendInt(TERM_CODE, 3);
  }

  int TransferMode() {
    if (missing_flag) {
      if (pheno_flag) {
        return GMP_CODE;
      } else {
        return GM_CODE;
      }
    } else {
      if (pheno_flag) {
        return GP_CODE;
      } else {
        return G_CODE;
      }
    }
  }

  bool NotDone() { return num_left > 0; }

  /* Return genotypes in dosage format (0, 1, or 2), assume no missing */
  void GetNextGP(Vec<ZZ_p> &g, Vec<ZZ_p> &p) {
    assert(TransferMode() == GP_CODE);
    Vec<ZZ_p> m;
    Mat<ZZ_p> gmat;
    GetNextAux(gmat, m, p);
    g = gmat[0];
  }

  void GetNextG(Vec<ZZ_p> &g) {
    assert(TransferMode() == G_CODE);
    Vec<ZZ_p> p;
    Vec<ZZ_p> m;
    Mat<ZZ_p> gmat;
    GetNextAux(gmat, m, p);
    g = gmat[0];
  }

  /* Return genotype probabilities with missingness information */
  void GetNextGM(Mat<ZZ_p> &g, Vec<ZZ_p> &m) {
    assert(TransferMode() == GM_CODE);
    Vec<ZZ_p> p;
    GetNextAux(g, m, p);
  }

  void GetNextGMP(Mat<ZZ_p> &g, Vec<ZZ_p> &m, Vec<ZZ_p> &p) {
    assert(TransferMode() == GMP_CODE);
    GetNextAux(g, m, p);
  }

private:
  MPCEnv *mpc;
  int pid;
  int num_left;
  int index;
  bool pheno_flag; // labels(y) flag ??
  bool missing_flag;

  void GetNextAux(Mat<ZZ_p> &g, Vec<ZZ_p> &m, Vec<ZZ_p> &p) {
    assert(NotDone());

    if (missing_flag) {
    } else {
    }

    if (pid == 2) {
      if (pheno_flag) {
      }
      if (missing_flag) {
      } else {
      }
    } else if (pid == 1) {
      mpc->SwitchSeed(3);
      if (pheno_flag) {
      }
      if (missing_flag) {
      } else {
      }
      mpc->RestoreSeed();
    }

    index++;
    num_left--;
  }
};

#endif
