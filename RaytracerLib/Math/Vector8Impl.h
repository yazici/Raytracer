#pragma once

namespace rt {
namespace math {

// Constructors ===================================================================================

Vector8::Vector8()
{
    v = _mm256_setzero_ps();
}

Vector8::Vector8(const Vector4& lo)
{
    v = _mm256_castps128_ps256(lo);
}

Vector8::Vector8(const Vector4& lo, const Vector4& hi)
{
    const __m256 tmp = _mm256_castps128_ps256(lo);
    v = _mm256_insertf128_ps(tmp, hi, 1);
}

Vector8::Vector8(const __m256& m)
    : v(m)
{
}

Vector8::Vector8(Float e0, Float e1, Float e2, Float e3, Float e4, Float e5, Float e6, Float e7)
{
    v = _mm256_set_ps(e0, e1, e2, e3, e4, e5, e6, e7);
}

Vector8::Vector8(int e0, int e1, int e2, int e3, int e4, int e5, int e6, int e7)
{
    v = _mm256_castsi256_ps(_mm256_set_epi32(e0, e1, e2, e3, e4, e5, e6, e7));
}

Vector8::Vector8(const Float* src)
{
    v = _mm256_loadu_ps(src);
}

void Vector8::Set(Float scalar)
{
    v = _mm256_set1_ps(scalar);
}

// Elements rearrangement =========================================================================

Vector8 Vector8::Splat(Float f)
{
    return Vector8(_mm256_set1_ps(f));
}

Vector8 Vector8::SelectBySign(const Vector8& a, const Vector8& b, const Vector8& sel)
{
    return _mm256_blendv_ps(a, b, sel);
}

// Logical operations =============================================================================

Vector8 Vector8::operator& (const Vector8& b) const
{
    return _mm256_and_ps(v, b);
}

Vector8 Vector8::operator| (const Vector8& b) const
{
    return _mm256_or_ps(v, b);
}

Vector8 Vector8::operator^ (const Vector8& b) const
{
    return _mm256_xor_ps(v, b);
}

Vector8& Vector8::operator&= (const Vector8& b)
{
    v = _mm256_and_ps(v, b);
    return *this;
}

Vector8& Vector8::operator|= (const Vector8& b)
{
    v = _mm256_or_ps(v, b);
    return *this;
}

Vector8& Vector8::operator^= (const Vector8& b)
{
    v = _mm256_xor_ps(v, b);
    return *this;
}

// Simple arithmetics =============================================================================

Vector8 Vector8::operator- () const
{
    return Vector8() - (*this);
}

Vector8 Vector8::operator+ (const Vector8& b) const
{
    return _mm256_add_ps(v, b);
}

Vector8 Vector8::operator- (const Vector8& b) const
{
    return _mm256_sub_ps(v, b);
}

Vector8 Vector8::operator* (const Vector8& b) const
{
    return _mm256_mul_ps(v, b);
}

Vector8 Vector8::operator/ (const Vector8& b) const
{
    return _mm256_div_ps(v, b);
}

Vector8 Vector8::operator* (Float b) const
{
    return _mm256_mul_ps(v, _mm256_set1_ps(b));
}

Vector8 Vector8::operator/ (Float b) const
{
    return _mm256_div_ps(v, _mm256_set1_ps(b));
}

Vector8 operator*(Float a, const Vector8& b)
{
    return _mm256_mul_ps(b, _mm256_set1_ps(a));
}


Vector8& Vector8::operator+= (const Vector8& b)
{
    v = _mm256_add_ps(v, b);
    return *this;
}

Vector8& Vector8::operator-= (const Vector8& b)
{
    v = _mm256_sub_ps(v, b);
    return *this;
}

Vector8& Vector8::operator*= (const Vector8& b)
{
    v = _mm256_mul_ps(v, b);
    return *this;
}

Vector8& Vector8::operator/= (const Vector8& b)
{
    v = _mm256_div_ps(v, b);
    return *this;
}

Vector8& Vector8::operator*= (Float b)
{
    v = _mm256_mul_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vector8& Vector8::operator/= (Float b)
{
    v = _mm256_div_ps(v, _mm256_set1_ps(b));
    return *this;
}

Vector8 Vector8::MulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

Vector8 Vector8::MulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

Vector8 Vector8::NegMulAndAdd(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

Vector8 Vector8::NegMulAndSub(const Vector8& a, const Vector8& b, const Vector8& c)
{
#ifdef RT_USE_FMA
    return _mm256_fnmsub_ps(a, b, c);
#else
    return c - a * b;
#endif
}


Vector8 Vector8::Floor(const Vector8& V)
{
    Vector8 vResult = _mm256_sub_ps(V, _mm256_set1_ps(0.49999f));
    __m256i vInt = _mm256_cvtps_epi32(vResult);
    vResult = _mm256_cvtepi32_ps(vInt);
    return vResult;
}


Vector8 Vector8::Sqrt(const Vector8& V)
{
    return _mm256_sqrt_ps(V);
}

Vector8 Vector8::Reciprocal(const Vector8& V)
{
    return _mm256_div_ps(_mm256_set1_ps(1.0f), V);
}

Vector8 Vector8::FastReciprocal(const Vector8& v)
{
    return _mm256_rcp_ps(v);
}

Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, const Vector8& weight)
{
    __m256 vTemp = _mm256_sub_ps(v2, v1);
    vTemp = _mm256_mul_ps(vTemp, weight);
    return _mm256_add_ps(v1, vTemp);
}

Vector8 Vector8::Lerp(const Vector8& v1, const Vector8& v2, Float weight)
{
    __m256 vWeight = _mm256_set1_ps(weight);
    __m256 vTemp = _mm256_sub_ps(v2, v1);
    vTemp = _mm256_mul_ps(vTemp, vWeight);
    return _mm256_add_ps(v1, vTemp);
}

Vector8 Vector8::Min(const Vector8& a, const Vector8& b)
{
    return _mm256_min_ps(a, b);
}

Vector8 Vector8::Max(const Vector8& a, const Vector8& b)
{
    return _mm256_max_ps(a, b);
}

Vector8 Vector8::Abs(const Vector8& v)
{
    return _mm256_and_ps(v, VECTOR8_MASK_ABS);
}

Vector8 Vector8::Clamped(const Vector8& min, const Vector8& max) const
{
    return Min(max, Max(min, *this));
}

int Vector8::GetSignMask() const
{
    return _mm256_movemask_ps(v);
}

Vector8 Vector8::HorizontalMin() const
{
    __m256 temp;
    temp = _mm256_min_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_min_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_min_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

Vector8 Vector8::HorizontalMax() const
{
    __m256 temp;
    temp = _mm256_max_ps(v, _mm256_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm256_max_ps(temp, _mm256_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    temp = _mm256_max_ps(temp, _mm256_permute2f128_ps(temp, temp, 1));
    return temp;
}

// Comparison functions ===========================================================================

int Vector8::EqualMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_EQ_OQ));
}

int Vector8::LessMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_LT_OQ));
}

int Vector8::LessEqMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_LE_OQ));
}

int Vector8::GreaterMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_GT_OQ));
}

int Vector8::GreaterEqMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_GE_OQ));
}

int Vector8::NotEqualMask(const Vector8& v1, const Vector8& v2)
{
    return _mm256_movemask_ps(_mm256_cmp_ps(v1, v2, _CMP_NEQ_OQ));
}

bool Vector8::operator== (const Vector8& b) const
{
    return EqualMask(*this, b) == 0xF;
}

bool Vector8::operator< (const Vector8& b) const
{
    return LessMask(*this, b) == 0xF;
}

bool Vector8::operator<= (const Vector8& b) const
{
    return LessEqMask(*this, b) == 0xF;
}

bool Vector8::operator> (const Vector8& b) const
{
    return GreaterMask(*this, b) == 0xF;
}

bool Vector8::operator>= (const Vector8& b) const
{
    return GreaterEqMask(*this, b) == 0xF;
}

bool Vector8::operator!= (const Vector8& b) const
{
    return NotEqualMask(*this, b) == 0xF;
}


} // namespace math
} // namespace rt