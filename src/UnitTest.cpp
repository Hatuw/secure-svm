#include "mpc.h"
#include "util.h"
#include <NTL/BasicThreadPool.h>
#include <NTL/ZZ.h>
#include <NTL/mat_ZZ.h>
#include <NTL/mat_ZZ_p.h>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace NTL;
using namespace std;

// template <typename T> T inline ABS(T a) { return (a < 0 ? -(a) : a); }
#define ABS(a) (((a) < 0) ? -(a) : (a))

bool unit_test(MPCEnv &mpc, int pid) {
  ZZ_p pi2fp;
  double fp2pi;
  double eps = 1e-6;
  const long double PI = 3.141592653589793238462643383279;

  // cout << "[Fixed-point ZZ_p <-> Double conversion]" << endl;
  // x = DoubleToFP(PI, Param::NBIT_K, Param::NBIT_F);
  // d = ABS(FPToDouble(x, Param::NBIT_K, Param::NBIT_F) - PI);
  if (pid == 0) {
    cout.precision(32);
    for (int i = -100; i <= 100; ++i) {
      pi2fp = DoubleToFP(PI - i, Param::NBIT_K, Param::NBIT_F);
      fp2pi = FPToDouble(pi2fp, Param::NBIT_K, Param::NBIT_F) + i;
      cout << "PI-(" << i << ") --> FP: " << pi2fp << "\t\t";
      cout << "FP --> Double + (" << i << "): " << fp2pi << endl;
    }

    // assert(d < eps);
    cout << "Success" << endl;
  }

  #undef PI
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "Usage: ShareData party_id param_file" << endl;
    return 1;
  }

  /* Load party id. */
  string pid_str(argv[1]);
  int pid;
  if (!Param::Convert(pid_str, pid, "party_id") || pid < 0 || pid > 3) {
    cout << "Error party_id should be 0, 1, 2, or 3" << endl;
    return 1;
  }

  /* Load values in parameter file. */
  if (!Param::ParseFile(argv[2])) {
    cout << "Could not finish parsing parmeter file" << endl;
    return 1;
  }

  /* Initalize MPC enviroment. */
  vector<pair<int, int>> pairs;
  // pairs.push_back(make_pair(0, 1));
  // pairs.push_back(make_pair(0, 2));
  // pairs.push_back(make_pair(1, 2));
  // pairs.push_back(make_pair(1, 3));
  // pairs.push_back(make_pair(2, 3));
  MPCEnv mpc;
  if (!mpc.Initialize(pid, pairs)) {
    cout << "MPC enviroment initialization failed" << endl;
    return 1;
  }

  /* Unit test. */
  bool success = true;
  if (pid == 0) {
    success = unit_test(mpc, pid);
  }

  mpc.CleanUp();

  if (success) {
    cout << "Protocol successfully completed." << endl;
    return 0;
  } else {
    cout << "Protocol abnormally terminated." << endl;
    return 1;
  }
}