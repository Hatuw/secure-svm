#include "NTL/ZZ_p.h"
#include "connect.h"
#include "mpc.h"
#include "protocol.h"
#include "util.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
// #include <sstream>
#include <string>

using namespace NTL;
using namespace std;

/**
 * mask_matrix
 * data_dir {string}
 * &mpc {MPCEnv}
 * name {string} : file's name
 * n_rows {size_t} : rows of data (batch or all)
 * n_cols {size_t} : columns of data
 */
bool mask_matrix(string data_dir, MPCEnv &mpc, string name, size_t n_rows,
                 size_t n_cols) {
  /* Open file. */
  string fname = data_dir + name;
  ifstream fin(fname.c_str());
  if (!fin.is_open()) {
    cout << "Error: could not open " << fname << endl;
    return false;
  }

  /* Read in matrix. */
  Mat<ZZ_p> matrix;
  Init(matrix, n_rows, n_cols);

  string line;
  int i = 0;
  vector<string> tmp_line;
  while (getline(fin, line)) {
    if (i % 1000 == 0) {
      cout << "Reading line " << i << endl;
    }
    // split line with '\t' flag
    split(line, tmp_line, '\t');
    for (int j = 0; j < n_cols; j++) {
      // XXX: why it should -48?
      // comment: 数据如果都是负数的话，那么DoubleToFP处理后的结果的长度应该是一样的
      //  当然，如果数据都是正数的话，结果的长度也一样（不过较都是负数的长度会短很多）
      //  猜测这是因为存储的精度问题。同时也是安全起见，将数据padding到同一长度
      // double val = (stod(tmp_line[j])) - 48;
      double val = (stod(tmp_line[j])) - 4;

      ZZ_p val_fp;
      DoubleToFP(val_fp, val, Param::NBIT_K, Param::NBIT_F);
      matrix[i][j] = val_fp;
    }
    ++i;
  }
  cout << matrix << endl;

  if (i != n_rows) {
    cout << "Error: Invalid number of rows: " << i << endl;
    return false;
  }
  fin.close();

  /* Mask matrix. */
  Mat<ZZ_p> mask;
  mpc.RandMat(mask, n_rows, n_cols);
  matrix -= mask; /* Masked `matrix` should be sent to CP2. */

  /* Save data to file. */
  fstream fs;
  fs.open((data_dir + name + "_masked.bin").c_str(), ios::out | ios::binary);
  mpc.WriteToFile(matrix, fs);
  fs.close();
  cout << "Finished writing to file." << endl;

  return true;
}

/**
 * mask_data
 * data_dir {string}
 * &mpc {MPCEnv}
 */
bool mask_data(string data_dir, MPCEnv &mpc) {
  vector<string> suffixes;

  // load suffixes file
  string suffixes_file_dir = "../res/data/";
  // suffixes = load_suffixes(suffixes_file);
  suffixes.push_back("traindata.txt");

  mpc.SwitchSeed(1); /* Use CP1's seed. */

  fstream fs;
  string fname;
  for (int i = 0; i < suffixes.size(); i++) {
    fname = cache(1, "seed_" + suffixes[i]);
    fs.open(fname.c_str(), ios::out | ios::binary);
    if (!fs.is_open()) {
      cout << "Error: could not open " << fname << endl;
      return false;
    }
    mpc.ExportSeed(fs);
    fs.close();

    /* Write batch to file. */
    // n_file_batch, feature_rank  --> (rows cols)
    if (!mask_matrix(data_dir, mpc, suffixes[i], 100, 3)) {
      return false;
    }
    // if (!mask_matrix(data_dir, mpc, "Y" + suffixes[i], 100, 2 - 1)) {
    //   return false;
    // }
  }

  mpc.RestoreSeed();

  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "Usage: ShareData party_id param_file [data_dir (for P3/SP)]"
         << endl;
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

  /* Load data directory name. */
  string data_dir;
  if (pid == 3) {
    if (argc < 4) {
      cout << "Error: for P3/SP, data directory should be provided as the last "
              "argument"
           << endl;
      return 1;
    }
    data_dir = argv[3];
    if (data_dir[data_dir.size() - 1] != '/') {
      data_dir += "/";
    }
    cout << "Data directory: " << data_dir << endl;
  }

  /* Initalize MPC enviroment. */
  vector<pair<int, int>> pairs;
  pairs.push_back(make_pair(0, 1));
  pairs.push_back(make_pair(0, 2));
  pairs.push_back(make_pair(1, 2));
  pairs.push_back(make_pair(1, 3));
  pairs.push_back(make_pair(2, 3));
  MPCEnv mpc;
  if (!mpc.Initialize(pid, pairs)) {
    cout << "MPC enviroment initialization failed" << endl;
    return 1;
  }

  /* Mask the data and save to file. */
  bool success = true;
  if (pid == 3) {
    success = mask_data(data_dir, mpc);
    if (!success) {
      cout << "Data masking failed." << endl;
    } else {
      cout << "Party 3 done streaming data." << endl;
    }
    mpc.SendBool(true, 2);

  } else if (pid == 2) {
    /* Keep CP2 alive until SP has shared data with it. */
    mpc.ReceiveBool(3);
    success = true;

  } else if (pid == 1) {
    /* CP1 needs to save its seed so it can reconstruct the masked data. */
    string fname = cache(pid, "initial_seed");
    fstream fs;
    fs.open(fname.c_str(), ios::out | ios::binary);
    if (!fs.is_open()) {
      cout << "Error: could not open " << fname << endl;
      // return false;
      return 1;
    }
    mpc.SwitchSeed(3);
    mpc.ExportSeed(fs);
    mpc.RestoreSeed();
    fs.close();
    success = true;
  }

  /* Keep party 0 online until end of data masking. */
  if (pid == 0) {
    mpc.ReceiveBool(2);
  } else if (pid == 2) {
    mpc.SendBool(true, 0);
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