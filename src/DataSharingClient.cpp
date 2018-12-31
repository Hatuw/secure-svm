#include "mpc.h"
#include "protocol.h"
#include "util.h"
#include "NTL/ZZ_p.h"
#include "svmiter.h"

#include <iostream>

using namespace NTL;
using namespace std;

bool send_stream(string data_dir, MPCEnv &mpc)
{
  return true;
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cout << "Usage: DataSharingClient party_id param_file [data_fir (for P3/SP)]" << endl;
  }

  /* Load party id */
  string pid_str(argv[1]);
  int pid;
  if (!Param::Convert(pid_str, pid, "party_id") || pid < 0 || pid > 3)
  {
    cout << "Error: party_id should be 0, 1, 2, or 3" << endl;
    return 1;
  }

  /* Load values in parameter file. */
  if (!Param::ParseFile(argv[2]))
  {
    cout << "Could not finish parsing parameter file" << endl;
    return 1;
  }

  /* Load data directory name. */
  string data_dir;
  // pid == 3 --> P3/SP
  if (pid == 3)
  {
    if (argc < 4)
    {
      cout << "Error: for P3/SP, data directory should be probided as the last argument" << endl;
      return 1;
    }
    data_dir = argv[3];
    // ex:  bin/DataSharingClient 3 ../par/test.par.3.txt ../test_data/
    if (data_dir[data_dir.size() - 1] != '/')
    {
      data_dir += "/";
    }

    cout << "Data directory: " << data_dir << endl;
  }

  /* Initialize MPC enviroment */
  vector<pair<int, int>> pairs;
  pairs.push_back(make_pair(0, 1));
  pairs.push_back(make_pair(0, 2));
  pairs.push_back(make_pair(1, 2));
  pairs.push_back(make_pair(1, 3));
  pairs.push_back(make_pair(2, 3));

  MPCEnv mpc;
  if (!mpc.Initialize(pid, pairs))
  {
    cout << "MPC environment initialization failed" << endl;
    return 1;
  }

  /* Mask the data and save to file. */
  bool success;
  if (pid < 3)
  {
    success = svm_data_sharing_protocol(mpc, pid);
  }
  else
  {
    // pid == 3 --> P3/SP
    int signal = mpc.ReceiveInt(1);

    while (signal != SVMIterator::TERM_CODE)
    {
      success = send_stream(data_dir, mpc);
      if (!success)
        break;

      signal = mpc.ReceiveInt(1);
    }

    cout << "Done with streaming data" << endl;
    success = true;
  }

  // Thiss is here just to keep P0 online until the end for data transfer
  // In pratice, P0 would send data in advance before each phase and go oofline
  if (pid == 0)
  {
    mpc.ReceiveBool(2);
  }
  else if (pid == 2)
  {
    mpc.SendBool(true, 0);
  }

  mpc.CleanUp();

  if (success)
  {
    cout << "Protocol successfully completed" << endl;
    return 0;
  }
  else
  {
    cout << "Protocol abnormally terminated" << endl;
    return 1;
  }
}
