#include "../../../include/components/math/AnthemNoise.h"

namespace Anthem::Components::Math {
	AnthemImprovedNoise::AnthemImprovedNoise() {
		for (int i = 0; i < 256; i++) {
			p[256 + i]  = permutation[i];
			p[i] = permutation[i];
		}
	}
	double AnthemImprovedNoise::fade(double t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	double AnthemImprovedNoise::lerp(double t, double a, double b) {
		return a + t * (b - a);
	}
	double AnthemImprovedNoise::grad(int hash, double x, double y, double z) {
			int h = hash & 15;
			double u = h < 8 ? x : y,
				v = h < 4 ? y : h == 12 || h == 14 ? x : z;
			return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}

	double AnthemImprovedNoise::noise(double x, double y, double z) {
		int X = (int)std::floor(x) & 255,
			Y = (int)std::floor(y) & 255,
			Z = (int)std::floor(z) & 255;
		x -= std::floor(x);
		y -= std::floor(y);
		z -= std::floor(z);
		double u = fade(x),
			v = fade(y),
			w = fade(z);
		int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z,
			B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

		return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
			grad(p[BA], x - 1, y, z)),
			lerp(u, grad(p[AB], x, y - 1, z),
				grad(p[BB], x - 1, y - 1, z))),
			lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
				grad(p[BA + 1], x - 1, y, z - 1)),
				lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
					grad(p[BB + 1], x - 1, y - 1, z - 1))));
	}

	AnthemFractalNoise::AnthemFractalNoise(IAnthemNoiseGenerator* baseGenerator,int iterations) {
		this->base = baseGenerator;
		this->its = iterations;
	}
	double AnthemFractalNoise::noise(double x, double y, double z) {
		double val = 0, ampl = 1, freq = 1, csamp = 0;
		for (auto i = 0; i < this->its; i++) {
			val += this->base->noise(freq * x, freq * y, freq * z) * ampl;
			csamp += ampl;
			freq *= 2;
			ampl /= 2;
		}
		return val / csamp;
	}
}