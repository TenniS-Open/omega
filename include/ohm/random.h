//
// Created by kier on 2018/7/19.
//

#ifndef OMEGA_RANDOM_H
#define OMEGA_RANDOM_H

#include <ctime>
#include <cstdlib>
#include <cmath>

namespace ohm {
    class MT19937 {
    public:
        using self = MT19937;

        MT19937() {
            this->srand((int) ::time(nullptr));
        }

        explicit MT19937(int __seed) {
            this->srand(__seed);
        }

        void srand(int __seed) {
            auto constexpr N = self::N;
            this->m_seed = __seed;
            this->m_i = 0;
            MT[0] = __seed;
            for (int i = 1; i < N; i++) {
                MT[i] = 0x6c078965 * (MT[i - 1] ^ (MT[i - 1] >> 30)) + i;
                MT[i] &= 0xffffffff;   // get the last 32bits
            }
        }

        int rand() {
            int i = this->m_i;
            int generate = (MT[i] & 0x80000000) + (MT[(i + 1) % 624] & 0x7fffffff);
            MT[i] = MT[(i + 397) % 624] ^ (generate >> 1);
            if (generate & 1) MT[i] ^= 0x9908b0df;

            int y = MT[i];
            y = y ^ (y >> 11);
            y = y ^ ((y << 7) & 0x9d2c5680);
            y = y ^ ((y << 15) & 0xefc60000);
            y = y ^ (y >> 18);
            this->m_i = (i + 1) % 624;
            return y;
        }

        static constexpr int MAX = 0x7fffffff;     // 2 ^ 31 - 1
    private:
        static constexpr int N = 624;      //624 * 32 - 31 = 19937
        int MT[N];
        int m_i = 0;
        int m_seed;
    };

#pragma push_macro("ln")
#define ln ::log
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI

    class Random {
    public:
        using self = Random;

        Random()
                : mt() {
        }

        explicit Random(int __seed)
                : mt(__seed) {
        }

        // Set seed of random serial
        void seed(int __seed) {
            mt.srand(__seed);
        }

        // Uniformly distributed integer in [0, MT19937::MAX]
        int next() {
            return mt.rand();
        }

        // Uniformly distributed integer in [min, max]
        int next(int min, int max) {
            // return min + (int) ((max - min) * u());
            return min + (mt.rand() % (max - min + 1));
        }

        // Uniformly distributed number in [0, 1]
        double u() {
            return (double) mt.rand() / MT19937::MAX;
        }

        // Binomial distributed boolean(p)
        bool binomial(double p) {
            return u() < p;
        }

        // Exponential distribution
        double exp(double beta) {
            return -beta * ln(u());
        }

        // Ray distribution
        double ray(double mu) {
            return sqrt(-2 * mu * mu * ln(u()));
        }

        // Weibull distribution
        double weibull(double alpha, double beta) {
            return beta * pow(-ln(u()), 1.0 / alpha);
        }

        // Normal distribution: N(0, 1)
        double normal() {
            return sqrt(-2 * ln(u())) * sin(2 * M_PI * u());
        }

        // Normal distribution: N(mu, delta^2)
        double normal(double mu, double delta) {
            return mu + delta * normal();
        }

    private:
        MT19937 mt;
    };
#pragma pop_macro("ln")

}

#endif //OMEGA_RANDOM_H
