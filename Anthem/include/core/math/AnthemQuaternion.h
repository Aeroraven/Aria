#pragma once
#include "../base/AnthemBaseImports.h"
#include "./AnthemMatrix.h"

namespace Anthem::Core::Math {
    template<typename T>
    concept AnthemQuaternionLegalConcept = TmplDefs::ATpdAnyOf<T, float, int, double, unsigned int>;

    template<typename T>
    requires AnthemQuaternionLegalConcept<T>
    class AnthemQuaternion {
    public:
		T x, y, z, w;
        AnthemQuaternion() : x(0), y(0), z(0), w(1) {}
        AnthemQuaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
        AnthemQuaternion(const AnthemQuaternion<T>& p) : x(p.x), y(p.y), z(p.z), w(p.w) {}
        AnthemQuaternion(AnthemQuaternion<T>&& p) : x(p.x), y(p.y), z(p.z), w(p.w) {}   
        const AnthemQuaternion<T>& operator=(const AnthemQuaternion<T>& p) {
			x = p.x;
			y = p.y;
			z = p.z;
			w = p.w;
			return *this;
		}
        const AnthemQuaternion<T>& operator=(AnthemQuaternion<T>&& p) {
            x = p.x;
            y = p.y;
            z = p.z;
            w = p.w;
            return *this;
        }
        AnthemQuaternion<T> operator*(const AnthemQuaternion<T>& second) const {
            return AnthemQuaternion<T>(
				w * second.x + x * second.w + y * second.z - z * second.y,
				w * second.y + y * second.w + z * second.x - x * second.z,
				w * second.z + z * second.w + x * second.y - y * second.x,
				w * second.w - x * second.x - y * second.y - z * second.z
			);
		}
        AnthemQuaternion<T> operator*(T scalar) const {
			return AnthemQuaternion<T>(x * scalar, y * scalar, z * scalar, w * scalar);
		}
        AnthemQuaternion<T> operator+(const AnthemQuaternion<T>& second) const {
			return AnthemQuaternion<T>(x + second.x, y + second.y, z + second.z, w + second.w);
		}
        AnthemQuaternion<T> operator-(const AnthemQuaternion<T>& second) const {
			return AnthemQuaternion<T>(x - second.x, y - second.y, z - second.z, w - second.w);
		}
        AnthemQuaternion<T> operator/(T scalar) const {
			return AnthemQuaternion<T>(x / scalar, y / scalar, z / scalar, w / scalar);
		}
        T dot(const AnthemQuaternion<T>& second) const {
			return x * second.x + y * second.y + z * second.z + w * second.w;
		}
        T length() const {
			return std::sqrt(x * x + y * y + z * z + w * w);
		}
        AnthemQuaternion<T> normalize() const {
			T len = length();
			return AnthemQuaternion<T>(x / len, y / len, z / len, w / len);
		}
		AnthemQuaternion<T> conjugate() const {
			return AnthemQuaternion<T>(-x, -y, -z, w);
		}
		AnthemQuaternion<T> inverse() const {
			return conjugate() / (x * x + y * y + z * z + w * w);
		}
		AnthemQuaternion<T> slerp(const AnthemQuaternion<T>& second, T t) const {
			T dotProd = dot(second);
			T theta = std::acos(dotProd);
			T sinTheta = std::sin(theta);
			T w1 = std::sin((1 - t) * theta) / sinTheta;
			T w2 = std::sin(t * theta) / sinTheta;
			return (*this * w1) + (second * w2);
		}
		void setFromEuler(T pitch, T yaw, T roll) {
			T cy = std::cos(yaw * 0.5);
			T sy = std::sin(yaw * 0.5);
			T cp = std::cos(pitch * 0.5);
			T sp = std::sin(pitch * 0.5);
			T cr = std::cos(roll * 0.5);
			T sr = std::sin(roll * 0.5);
			x = cy * cp * sr - sy * sp * cr;
			y = sy * cp * sr + cy * sp * cr;
			z = sy * cp * cr - cy * sp * sr;
			w = cy * cp * cr + sy * sp * sr;
		}
		AnthemMatrix<T, 4, 4> toMatrix() const {

			T xx = x * x;
			T xy = x * y;
			T xz = x * z;
			T xw = x * w;
			T yy = y * y;
			T yz = y * z;
			T yw = y * w;
			T zz = z * z;
			T zw = z * w;

			AnthemMatrix<T, 4, 4> m;
			m[0][0] = 1 - 2 * (yy + zz);
			m[0][1] = 2 * (xy - zw);
			m[0][2] = 2 * (xz + yw);
			m[0][3] = 0;

			m[1][0] = 2 * (xy + zw);
			m[1][1] = 1 - 2 * (xx + zz);
			m[1][2] = 2 * (yz - xw);
			m[1][3] = 0;

			m[2][0] = 2 * (xz - yw);
			m[2][1] = 2 * (yz + xw);
			m[2][2] = 1 - 2 * (xx + yy);
			m[2][3] = 0;

			m[3][0] = 0;
			m[3][1] = 0;
			m[3][2] = 0;
			m[3][3] = 1;

			return m;
		}
    };
}