#ifndef __SVMITER_H_
#define __SVMITER_H_

#include "mpc.h"
#include "assert.h"
#include <vector>
#include <NTL/mat_ZZ_p.h>

using namespace std;

class SVMIterator {
    public:
    static const int TERM_CODE = 0;
    static const int G_CODE = 1;
    static const int GP_CODE = 2;
    static const int GM_CODE = 3;
    static const int GMP_CODE = 4;

    explicit SVMIterator(MPCEnv& mpc, int pid) {
        this -> mpc = &mpc;
        this -> pid = pid;
    }

    void Init(bool pheno_flag, bool missing_flag) {
        cout << "Initialized SVMIterator" << endl;
    }

    void Terminate() {
        if (pid == 1) {
            mpc -> SendInt(TERM_CODE, 3);
        }
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

    bool NotDone() {
      return num_left > 0;
    }

    void GetNextGP(Vec<ZZ_p>& g, Vec<ZZ_p>& p) {

    }

    void GetNextG(Vec<ZZ_p>& g) {

    }

    void GetNextGM(Mat<ZZ_p>& g, Vec<ZZ_p>& m) {

    }

    void GetNextGMP(Mat<ZZ_p>& g, Vec<ZZ_p>& m, Vec<ZZ_p>& p) {
      assert(TransferMode() == GMP_CDOE);
      GetNextAux(g, m, p);
    }

private:
  MPCEnv *mpc;
  int pid;
  int num_left;
  int index;
  bool pheno_flag;
  bool missing_flat;

  void GetNextAut(Mat<ZZ_p>& g, Vec<ZZ_p>& m, Vec<ZZ_p>& p) {
    assert(NotDone());

    if (missing_flat) {

    } else {

    }

    if (pid == 2) {
      if (pheno_flag) {

      }
      if (missing_flag) {

      } else {

      }
    } else if (pid == 1) {
      mpc -> SwitchSeed(3);
      if (pheno_flag) {

      }
      if (missing_flat) {

      } else {

      }
      mpc -> RestoreSeed();
    }

    index++;
    num_left--;
  }
};

#endif
