#include "connect.h"
#include "mpc.h"
#include "protocol.h"
#include "util.h"
#include "NTL/ZZ_p.h"

#include <cstdlib>
#include <fstream>
#include <map>
#include <iostream>
#include <sstream>

using namespace NTL;
using namespace std;

bool mask_matrix(string data_dir, MPCEnv &mpc, string name,
                 size_t n_rows, size_t n_cols)
{
    /* Open file. */

    /* Read in matrix. */

    /* Mask matrix. */

    /* Save data to file. */

    return true;
}

bool mask_data(string data_dir, MPCEnv &mpc)
{
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: ShareData party_id param_file [data_dir (for P3/SP)]" << endl;
        return 1;
    }

    /* Load party id. */
    string pid_str(argv[1]);
    int pid;
    if (!Param::Convert(pid_str, pid, "party_id") || pid < 0 || pid > 3)
    {
        cout << "Error party_id should be 0, 1, 2, or 3" << endl;
        return 1;
    }

    /* Load values in parameter file. */
    if (!Param::ParseFile(argv[2]))
    {
        cout << "Could not finish parsing parmeter file" << endl;
        return 1;
    }

    /* Load data directory name. */
    string data_dir;
    if (pid == 3)
    {
        if (argc < 4)
        {
            cout << "Error: for P3/SP, data directory should be provided as the last argument" << endl;
            return 1;
        }
        data_dir = argv[3];
        if (data_dir[data_dir.size() - 1] != '/')
        {
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
    if (!mpc.Initialize(pid, pairs))
    {
        cout << "MPC enviroment initialization failed" << endl;
        return 1;
    }

    /* Mask the data and save to file. */
    bool success = true;
    if (pid == 3)
    {
        success = mask_data(data_dir, mpc);
        if (!success)
        {
            cout << "Data masking failed." << endl;
        }
        else
        {
            cout << "Party 3 done streaming data." << endl;
        }
        mpc.SendBool(true, 2);
    }
    else if (pid == 2)
    {
        /* Keep CP2 alive until SP has shared data with it. */
        mpc.ReceiveBool(3);
        success = true;
    }
    else if (pid == 1)
    {
        /* CP1 needs to save its seed so it can reconstruct the masked data. */
        string fname = cache(pid, "initial_seed");
        fstream fs;
        fs.open(fname.c_str(), ios::out | ios::binary);
        if (!fs.is_open())
        {
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
        cout << "Protocol successfully completed." << endl;
        return 0;
    }
    else
    {
        cout << "Protocol abnormally terminated." << endl;
        return 1;
    }
}