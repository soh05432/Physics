#include <Base.hpp>

#include <algorithm>

///
/// Point class non-inline functions
Vector3::Vector3()
	: x(0.f), y(0.f), z(0.f)
{
	
}

Vector3::Vector3(const Real x, const Real y, const Real z)
	: x(x), y(y), z(z)
{

}

Vector3::Vector3(const Vector3& copy)
    : x(copy.x), y(copy.y), z(copy.z)
{ 
	/// Over-ride copy constructor
}

void Vector3::setTransformedPos(const Matrix3& t, const Vector3& v)
{
	Vector3 vCopy(v);
	vCopy(2) = 1.f; /// Add additional dimension to enable translation
	Real xbuf = t(0,0)*vCopy(0) + t(0,1)*vCopy(1) + t(0,2)*vCopy(2);
	y = t(1,0)*vCopy(0) + t(1,1)*vCopy(1) + t(1,2)*vCopy(2);
	x = xbuf;
}

void Vector3::setTransformedInversePos(const Matrix3& t, const Vector3& v)
{
	// Original matrix
	// t(0,0) t(0,1) t(0,2)
	// t(1,0) t(1,1) t(1,2)
	// t(2,0) t(2,1) t(2,2)

	// Matrix of minors
	// t(1,1)*t(2,2)-t(2,1)*t(1,2)  t(1,0)*t(2,2)-t(2,0)*t(1,2)  t(1,0)*t(2,1)-t(2,0)*t(1,1)
	// t(0,1)*t(2,2)-t(2,1)*t(0,2)  t(0,0)*t(2,2)-t(2,0)*t(0,2)  t(0,0)*t(2,1)-t(2,0)*t(0,1)
	// t(0,1)*t(1,2)-t(1,1)*t(0,2)  t(0,0)*t(1,2)-t(1,0)*t(0,2)  t(0,0)*t(1,1)-t(1,0)*t(0,1)

	// Matrix of cofactors
	// t(1,1)*t(2,2)-t(2,1)*t(1,2)  t(2,0)*t(1,2)-t(1,0)*t(2,2)  t(1,0)*t(2,1)-t(2,0)*t(1,1)
	// t(2,1)*t(0,2)-t(0,1)*t(2,2)  t(0,0)*t(2,2)-t(2,0)*t(0,2)  t(2,0)*t(0,1)-t(0,0)*t(2,1)
	// t(0,1)*t(1,2)-t(1,1)*t(0,2)  t(1,0)*t(0,2)-t(0,0)*t(1,2)  t(0,0)*t(1,1)-t(1,0)*t(0,1)

	// Adjugation
	// t(1,1)*t(2,2)-t(2,1)*t(1,2)  t(2,1)*t(0,2)-t(0,1)*t(2,2)  t(0,1)*t(1,2)-t(1,1)*t(0,2)
	// t(2,0)*t(1,2)-t(1,0)*t(2,2)  t(0,0)*t(2,2)-t(2,0)*t(0,2)  t(1,0)*t(0,2)-t(0,0)*t(1,2)
	// t(1,0)*t(2,1)-t(2,0)*t(1,1)  t(2,0)*t(0,1)-t(0,0)*t(2,1)  t(0,0)*t(1,1)-t(1,0)*t(0,1)

	Real detT =
		t(0,0)*( t(1,1)*t(2,2) - t(2,1)*t(1,2) ) -
		t(0,1)*( t(1,0)*t(2,2) - t(2,0)*t(1,2) ) +
		t(0,2)*( t(1,0)*t(2,1) - t(2,0)*t(1,1) );

	Assert(fabs(detT) > std::numeric_limits<Real>::epsilon(), "Transformation matrix doesn't have inverse");

	Real xbuf =
        ((t(1,1)*t(2,2)-t(2,1)*t(1,2))*v(0) + (t(2,1)*t(0,2)-t(0,1)*t(2,2))*v(1) + (t(0,1)*t(1,2)-t(1,1)*t(0,2))*v(2)) / detT;
	y =
        ((t(2,0)*t(1,2)-t(1,0)*t(2,2))*v(0) + (t(0,0)*t(2,2)-t(2,0)*t(0,2))*v(1) + (t(1,0)*t(0,2)-t(0,0)*t(1,2))*v(2)) / detT;
	x = xbuf;
}

void Vector3::setClampedLength(const Vector3& v, const Real length)
{
	Assert(!v.checkZero(), "Trying to give length to zero vector");
	Assert(length > std::numeric_limits<Real>::epsilon(), "Trying to scale to negative length");
	Real scale = length / v.length();
	scale = std::min(1.f, scale);
	x = v.x * scale;
	y = v.y * scale;
	z = v.z * scale;
}

std::ostream& operator<<(std::ostream& stream, Vector3& v)
{
	stream.precision(2);
	stream << std::fixed << v(0) << "," << v(1) << std::endl;
	return stream;
}