#ifndef __MYSVM_H_
#define __MYSVM_H_

#include "mpc.h"
#include <vector>
#include <NTL/mat_ZZ_p.h>
#include <NTL/mat_ZZ.h>
#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>
#include <iostream>

using namespace NTL;
using namespace std;

class MySVM
{
  public:
    MySVM(MPCEnv &mpc,
          int pid,
          Mat<ZZ_p> x,
          Mat<ZZ_p> lb,
          int c,
          float t);
    Mat<ZZ_p> kernelTrans(Mat<ZZ_p> feature, Mat<ZZ_p> line);
    ZZ_p calcEK(int k);
    void solve();

  private:
    MPCEnv &mpc;        // the mpc enviroment
    int pid;            // part id
    Mat<ZZ_p> X;        // features of data
    Mat<ZZ_p> labelMat; // ground-truth labels of data
    int C;              // soft margin
    float tol;          // threshold to stop
    int m;              // lines of data
    Mat<ZZ_p> alphas;   // the mat of alphs, refer to SVM algo
    int b;              // default = 0
    Mat<ZZ_p> eCache;   // the cache
    Mat<ZZ_p> K;        // the result of kernel function
};

MySVM::MySVM(
    MPCEnv &mpc,
    int pid,
    Mat<ZZ_p> x,
    Mat<ZZ_p> lb,
    int c = 200,
    float t = 0.0001)
    : mpc(mpc), pid(pid), X(x), labelMat(lb), C(c), tol(t), b(0)
{
    // init m, alphas, eCache and K
    m = x.NumRows();
    int n = x.NumCols();
    Init(alphas, m, 1);
    Init(eCache, m, 2);
    Init(K, m, m);

    // init kernel result
    Mat<ZZ_p> tmp_x, tmp_k;
    Init(tmp_x, 1, n);
    Init(tmp_k, m, 1);
    /**
     * The Python implementation:
     * ``` python
     * for i in range(m):
     *     self.K[:, i] = kernelTrans(X, X[i, :])
     * ```
    */
    cout << "Init kernel results" << endl;
    for (int i = 0; i < m; ++i)
    {
        for (int col = 0; col < n; ++col)
            tmp_x[0][col] = x[i][col];
        tmp_k = kernelTrans(X, tmp_x); // size: m * 1
        for (int row = 0; row < m; ++row)
            K[row][i] = tmp_k[row][0];
    }
    cout << "OptStruct has init" << endl;
}

/** 核函数
 * @param feature: 支持向量的特征数
 * @param line: 某一行的特征数据
 * return: 核函数计算结果
*/
Mat<ZZ_p> MySVM::kernelTrans(Mat<ZZ_p> feature, Mat<ZZ_p> line)
{
    int m = feature.NumRows();
    // int n = feature.NumCols();
    Mat<ZZ_p> K;
    Init(K, m, 1);

    // 线性核函数
    mpc.Transpose(line);
    mpc.MultMat(K, feature, line);
    return K;
}

// def calcEk(oS, k):  # 计算Ek（参考《统计学习方法》p127公式7.105）
//     fXk = float(multiply(oS.alphas, oS.labelMat).T*oS.K[:, k] + oS.b)
//     Ek = fXk - float(oS.labelMat[k])
//     return Ek

ZZ_p MySVM::calcEK(int k)
{
    Mat<ZZ_p> fXk;
    Mat<ZZ_p> tmp_mult, trans_k;
    Init(fXk, 1, 1);
    Init(tmp_mult, 1, m);
    Init(trans_k, m, 1);
    for (int i = 0; i < m; ++i)
    {
        tmp_mult[0][i] =
            alphas[i][0] * labelMat[i][0]; // already transpose
        trans_k[i][0] = K[i][k];           // K[:, k]
    }
    mpc.MultMat(fXk, tmp_mult, trans_k);
    fXk[0][0] = fXk[0][0] + b;
    ZZ_p Ek = fXk[0][0] - labelMat[k][0];
    return Ek;
}

void MySVM::solve()
{
    ZZ_p Ek = calcEK(1);
    cout << Ek << endl;
}

#endif