// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include "precomp.hpp"
#include "opencv2/core/mat.hpp"

namespace cv {

typedef Vec<int, 5> Vec5i;
typedef Vec<int, 7> Vec7i;
typedef Vec<int, 9> Vec9i;
typedef Vec<int, 10> Vec10i;
typedef Vec<int, 11> Vec11i;
typedef Vec<int, 12> Vec12i;
typedef Vec<int, 13> Vec13i;
typedef Vec<int, 14> Vec14i;
typedef Vec<int, 15> Vec15i;
typedef Vec<int, 16> Vec16i;
typedef Vec<int, 32> Vec32i;
typedef Vec<int, 64> Vec64i;
typedef Vec<int, 128> Vec128i;

#undef STD_VECTOR_SWITCH
#define STD_VECTOR_SWITCH(esz, func) \
    switch( esz ) \
    { \
    case 1: func(uchar); \
    case 2: func(Vec2b); \
    case 3: func(Vec3b); \
    case 4: func(int); \
    case 6: func(Vec3s); \
    case 8: func(Vec2i); \
    case 12: func(Vec3i); \
    case 16: func(Vec4i); \
    case 20: func(Vec5i); \
    case 24: func(Vec6i); \
    case 28: func(Vec7i); \
    case 32: func(Vec8i); \
    case 36: func(Vec9i); \
    case 40: func(Vec10i); \
    case 44: func(Vec11i); \
    case 48: func(Vec12i); \
    case 52: func(Vec13i); \
    case 56: func(Vec14i); \
    case 60: func(Vec15i); \
    case 64: func(Vec16i); \
    case 128: func(Vec32i); \
    case 256: func(Vec64i); \
    case 512: func(Vec128i); \
    default: \
        CV_Error_(cv::Error::StsBadArg, ("Vectors of vectors with element size %d are not supported. Please, modify STD_VECTOR_SWITCH() macro\n", esz)); \
    }

/*************************************************************************************************\
                                        Input/Output Array
\*************************************************************************************************/

Mat _InputArray::getMat_(int i) const
{
    _InputArray::KindFlag k = kind();
    AccessFlag accessFlags = flags & ACCESS_MASK;

    if( k == MAT )
    {
        const Mat* m = (const Mat*)obj;
        if( i < 0 )
            return *m;
        return m->row(i);
    }

    if (k == MATX)
    {
        CV_Assert( i < 0 );
        return Mat(sz, flags, obj);
    }

    if( k == STD_BOOL_VECTOR )
    {
        CV_Assert( i < 0 );
        int t = CV_8U;
        const std::vector<bool>& v = *(const std::vector<bool>*)obj;
        int j, n = (int)v.size();
        if( n == 0 )
            return Mat();
        Mat m(1, &n, t);
        uchar* dst = m.data;
        for( j = 0; j < n; j++ )
            dst[j] = (uchar)v[j];
        return m;
    }

    if( k == NONE )
        return Mat();

    if( k == STD_VECTOR || k == STD_VECTOR_VECTOR )
    {
        int t = CV_MAT_TYPE(flags);
        int esz = CV_ELEM_SIZE(t);

    #undef GET_MAT
    #define GET_MAT(T) \
        { \
            std::vector<T>* v; \
            if (k == STD_VECTOR_VECTOR) { \
                std::vector<std::vector<T> >* vv = \
                    reinterpret_cast<std::vector<std::vector<T> >*>(obj); \
                CV_Assert(size_t(i) < vv->size()); \
                v = &vv->at(i); \
            } else { \
                v = reinterpret_cast<std::vector<T>*>(obj); \
            } \
            int v_sz = int(v->size()); \
            return v->empty() ? Mat() : Mat(1, &v_sz, t, v->data()); \
        }

        STD_VECTOR_SWITCH(esz, GET_MAT)
    }

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& v = *(const std::vector<Mat>*)obj;
        CV_Assert( 0 <= i && i < (int)v.size() );

        return v[i];
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* v = (const Mat*)obj;
        CV_Assert( 0 <= i && i < sz.height );

        return v[i];
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

void _InputArray::getMatVector(std::vector<Mat>& mv) const
{
    _InputArray::KindFlag k = kind();
    AccessFlag accessFlags = flags & ACCESS_MASK;

    if( k == MAT )
    {
        const Mat& m = *(const Mat*)obj;
        int n = (int)m.size[0];
        mv.resize(n);
        CV_Assert(m.dims >= 2);

        for( int i = 0; i < n; i++ )
            mv[i] = m.dims <= 2 ? Mat(1, m.cols, m.type(), (void*)m.ptr(i)) :
                Mat(m.dims-1, &m.size[1], m.type(), (void*)m.ptr(i), &m.step[1]);
        return;
    }

    if (k == MATX)
    {
        size_t n = sz.height, esz = CV_ELEM_SIZE(flags);
        mv.resize(n);

        for( size_t i = 0; i < n; i++ )
            mv[i] = Mat(1, sz.width, CV_MAT_TYPE(flags), (uchar*)obj + esz*sz.width*i);
        return;
    }

    if( k == STD_VECTOR )
    {
        const std::vector<uchar>& v = *(const std::vector<uchar>*)obj;

        size_t n = total(), esz = CV_ELEM_SIZE(flags);
        int t = CV_MAT_DEPTH(flags), cn = CV_MAT_CN(flags);
        mv.resize(n);

        for( size_t i = 0; i < n; i++ )
            mv[i] = Mat(1, cn, t, (void*)(v.data() + esz*i));
        return;
    }

    if( k == NONE )
    {
        mv.clear();
        return;
    }

    if( k == STD_VECTOR_VECTOR )
    {
        int typ = CV_MAT_TYPE(flags);
        int esz = CV_ELEM_SIZE(typ);

        #undef GET_VEC_VEC_MAT_VEC
        #define GET_VEC_VEC_MAT_VEC(T) \
        { \
            const std::vector<std::vector<T > >* vv = (const std::vector<std::vector<T > >*)obj; \
            size_t n = vv->size(); \
            mv.resize(n); \
            for (size_t i = 0; i < n; i++) { \
                const std::vector<T >& vi = vv->at(i); \
                CV_Assert(vi.size() <= (size_t)INT_MAX); \
                int ni = (int)vi.size(); \
                mv[i] = ni > 0 ? Mat(1, &ni, typ, (void*)vi.data()) : Mat(); \
            } \
        } \
        break

        STD_VECTOR_SWITCH(esz, GET_VEC_VEC_MAT_VEC)
        return;
    }

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& v = *(const std::vector<Mat>*)obj;
        size_t n = v.size();
        mv.resize(n);

        for( size_t i = 0; i < n; i++ )
            mv[i] = v[i];
        return;
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* v = (const Mat*)obj;
        size_t n = sz.height;
        mv.resize(n);

        for( size_t i = 0; i < n; i++ )
            mv[i] = v[i];
        return;
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

_InputArray::KindFlag _InputArray::kind() const
{
    KindFlag k = flags & KIND_MASK;
#if CV_VERSION_MAJOR < 5
    CV_DbgAssert(k != EXPR);
    CV_DbgAssert(k != STD_ARRAY);
#endif
    return k;
}

int _InputArray::rows(int i) const
{

    return size(i).height;
}

int _InputArray::cols(int i) const
{

    return size(i).width;
}

Size _InputArray::size(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        const Mat* m = (const Mat*)obj;
        CV_Assert(m->dims <= 2);
        return Size(m->cols, m->rows);
    }

    if (k == MATX)
    {
        CV_Assert( i < 0 );
        return sz;
    }

    if( k == NONE )
        return Size();

    if( k == STD_VECTOR || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
    {
        size_t n = total(i);
        CV_Assert(n <= (size_t)INT_MAX);
        return Size((int)n, 1);
    }

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        if( i < 0 )
            return vv.empty() ? Size() : Size((int)vv.size(), 1);
        CV_Assert( i < (int)vv.size() );

        return vv[i].size();
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        if( i < 0 )
            return sz.height==0 ? Size() : Size(sz.height, 1);
        CV_Assert( i < sz.height );

        return vv[i].size();
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

int _InputArray::sizend(int* arrsz, int i) const
{
    int j, d = 0;
    _InputArray::KindFlag k = kind();

    if( k == NONE )
        ;
    else if( k == MAT )
    {
        CV_Assert( i < 0 );
        const Mat& m = *(const Mat*)obj;
        d = m.dims;
        if(arrsz)
            for(j = 0; j < d; j++)
                arrsz[j] = m.size.p[j];
    }
    else if( k == STD_VECTOR_MAT && i >= 0 )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert( i < (int)vv.size() );
        const Mat& m = vv[i];
        d = m.dims;
        if(arrsz)
            for(j = 0; j < d; j++)
                arrsz[j] = m.size.p[j];
    }
    else if( k == STD_ARRAY_MAT && i >= 0 )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert( i < sz.height );
        const Mat& m = vv[i];
        d = m.dims;
        if(arrsz)
            for(j = 0; j < d; j++)
                arrsz[j] = m.size.p[j];
    }    else if (((k == STD_VECTOR || k == STD_BOOL_VECTOR || k == STD_VECTOR_VECTOR) && i < 0) ||
             (k == STD_VECTOR_VECTOR && i >= 0))
    {
        size_t n = total(i);
        d = 1;
        if(arrsz)
        {
            arrsz[0] = int(n);
        }
    }
    else
    {
        CV_CheckLE(dims(i), 2, "Not supported");
        Size sz2d = size(i);
        d = 2;
        if(arrsz)
        {
            arrsz[0] = sz2d.height;
            arrsz[1] = sz2d.width;
        }
    }

    return d;
}

bool _InputArray::empty(int i) const
{
    _InputArray::KindFlag k = kind();
    if (i >= 0) {
        if (k == STD_VECTOR_MAT) {
            auto mv = reinterpret_cast<const std::vector<Mat>*>(obj);
            CV_Assert((size_t)i < mv->size());
            return mv->at(i).empty();
        }        else if (k == STD_VECTOR || k == STD_BOOL_VECTOR || k == STD_VECTOR_VECTOR) {
            size_t n = total(i);
            return n == 0;
        } else {
            CV_Error(Error::StsNotImplemented, "");
        }
    }
    return empty();
}

MatShape _InputArray::shape(int i) const
{
    _InputArray::KindFlag k = kind();
    MatShape shape;

    if( k == NONE )
        ;
    else if( k == MAT )
    {
        CV_Assert( i < 0 );
        const Mat& m = *(const Mat*)obj;
        shape = m.size;
    }    else if( k == STD_VECTOR_MAT && i >= 0 )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert( (size_t)i < vv.size() );
        const Mat& m = vv[i];
        shape = m.size;
    }
    else if( k == STD_ARRAY_MAT && i >= 0 )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert( i < sz.height );
        const Mat& m = vv[i];
        shape = m.size;
    }    else
    {
        int sizes[CV_MAX_DIM];
        int dims = sizend(sizes, i);
        shape = MatShape(dims, sizes);
    }

    return shape;
}

bool _InputArray::sameSize(const _InputArray& arr) const
{
    _InputArray::KindFlag k1 = kind(), k2 = arr.kind();
    Size sz1;

    if( k1 == MAT )
    {
        const Mat* m = ((const Mat*)obj);
        if( k2 == MAT )
            return m->size == ((const Mat*)arr.obj)->size;
        if( m->dims > 2 )
            return false;
        sz1 = m->size();
    }
    else
        sz1 = size();

    if( arr.dims() > 2 )
        return false;
    return sz1 == arr.size();
}

int _InputArray::dims(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        return ((const Mat*)obj)->dims;
    }

    if (k == MATX)
    {
        CV_Assert( i < 0 );
        return 2;
    }

    if( k == STD_VECTOR || k == STD_BOOL_VECTOR )
    {
        CV_Assert( i < 0 );
        return 1;
    }

    if( k == NONE )
        return 0;

    if( k == STD_VECTOR_VECTOR )
    {
        if( i < 0 )
            return 1;
        CV_Assert( size_t(i) < total() );
        return 1;
    }

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        if( i < 0 )
            return 1;
        CV_Assert( i < (int)vv.size() );

        return vv[i].dims;
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        if( i < 0 )
            return 1;
        CV_Assert( i < sz.height );

        return vv[i].dims;
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

size_t _InputArray::total(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        return ((const Mat*)obj)->total();
    }

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        if( i < 0 )
            return vv.size();

        CV_Assert( i < (int)vv.size() );
        return vv[i].total();
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        if( i < 0 )
            return sz.height;

        CV_Assert( i < sz.height );
        return vv[i].total();
    }

    if (k == STD_VECTOR || k == STD_VECTOR_VECTOR)
    {
        CV_Assert(i < 0 || k == STD_VECTOR_VECTOR);
        int esz = CV_ELEM_SIZE(flags);

        #undef GET_VEC_SIZE
        #define GET_VEC_SIZE(T) \
            if (k == STD_VECTOR_VECTOR) { \
                const std::vector<std::vector<T > >* vv = (const std::vector<std::vector<T > >*)obj; \
                size_t n = vv->size(); \
                if (i < 0) \
                    return n; \
                CV_Assert((size_t)i < n); \
                return vv->at(i).size(); \
            } else \
                return ((const std::vector<T >*)obj)->size()

        STD_VECTOR_SWITCH(esz, GET_VEC_SIZE)
    }

    return size(i).area();
}

int _InputArray::type(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
        return ((const Mat*)obj)->type();

    if( k == MATX || k == STD_VECTOR || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
        return CV_MAT_TYPE(flags);

    if( k == NONE )
        return -1;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        if( vv.empty() )
        {
            CV_Assert((flags & FIXED_TYPE) != 0);
            return CV_MAT_TYPE(flags);
        }
        CV_Assert( i < (int)vv.size() );
        return vv[i >= 0 ? i : 0].type();
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        if( sz.height == 0 )
        {
            CV_Assert((flags & FIXED_TYPE) != 0);
            return CV_MAT_TYPE(flags);
        }
        CV_Assert( i < sz.height );
        return vv[i >= 0 ? i : 0].type();
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

int _InputArray::depth(int i) const
{
    return CV_MAT_DEPTH(type(i));
}

int _InputArray::channels(int i) const
{
    return CV_MAT_CN(type(i));
}

bool _InputArray::empty() const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
        return ((const Mat*)obj)->empty();

    if (k == MATX)
        return false;

    if( k == STD_VECTOR || k == STD_BOOL_VECTOR || k == STD_VECTOR_VECTOR )
    {
        size_t n = total(-1);
        return n == 0;
    }

    if( k == NONE )
        return true;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        return vv.empty();
    }

    if( k == STD_ARRAY_MAT )
    {
        return sz.height == 0;
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

bool _InputArray::isContinuous(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
        return i < 0 ? ((const Mat*)obj)->isContinuous() : true;

    if( k == MATX || k == STD_VECTOR ||
        k == NONE || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
        return true;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert(i >= 0 && (size_t)i < vv.size());
        return vv[i].isContinuous();
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert(i >= 0 && i < sz.height);
        return vv[i].isContinuous();
    }

    CV_Error(cv::Error::StsNotImplemented, "Unknown/unsupported array type");
}

bool _InputArray::isSubmatrix(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
        return i < 0 ? ((const Mat*)obj)->isSubmatrix() : false;

    if( k == MATX || k == STD_VECTOR ||
        k == NONE || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
        return false;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert(i >= 0 && (size_t)i < vv.size());
        return vv[i].isSubmatrix();
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert(i >= 0 && i < sz.height);
        return vv[i].isSubmatrix();
    }

    CV_Error(cv::Error::StsNotImplemented, "");
}

size_t _InputArray::offset(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        const Mat * const m = ((const Mat*)obj);
        return (size_t)(m->ptr() - m->datastart);
    }

    if( k == MATX || k == STD_VECTOR ||
        k == NONE || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
        return 0;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert( i >= 0 && i < (int)vv.size() );

        return (size_t)(vv[i].ptr() - vv[i].datastart);
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert( i >= 0 && i < sz.height );
        return (size_t)(vv[i].ptr() - vv[i].datastart);
    }

    CV_Error(Error::StsNotImplemented, "");
}

size_t _InputArray::step(int i) const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        return ((const Mat*)obj)->step;
    }

    if( k == MATX || k == STD_VECTOR ||
        k == NONE || k == STD_VECTOR_VECTOR || k == STD_BOOL_VECTOR )
        return 0;

    if( k == STD_VECTOR_MAT )
    {
        const std::vector<Mat>& vv = *(const std::vector<Mat>*)obj;
        CV_Assert( i >= 0 && i < (int)vv.size() );
        return vv[i].step;
    }

    if( k == STD_ARRAY_MAT )
    {
        const Mat* vv = (const Mat*)obj;
        CV_Assert( i >= 0 && i < sz.height );
        return vv[i].step;
    }

    CV_Error(Error::StsNotImplemented, "");
}

void _InputArray::copyTo(const _OutputArray& arr) const
{
    _InputArray::KindFlag k = kind();

    if( k == NONE )
        arr.release();
    else if( k == MAT || k == MATX || k == STD_VECTOR || k == STD_BOOL_VECTOR )
    {
        Mat m = getMat();
        m.copyTo(arr);
    }
    else
        CV_Error(Error::StsNotImplemented, "");
}

void _InputArray::copyTo(const _OutputArray& arr, const _InputArray & mask) const
{
    _InputArray::KindFlag k = kind();

    if( k == NONE )
        arr.release();
    else if( k == MAT || k == MATX || k == STD_VECTOR || k == STD_BOOL_VECTOR )
    {
        Mat m = getMat();
        m.copyTo(arr, mask);
    }
    else
        CV_Error(Error::StsNotImplemented, "");
}

bool _OutputArray::fixedSize() const
{
    return (flags & FIXED_SIZE) == FIXED_SIZE;
}

bool _OutputArray::fixedType() const
{
    return (flags & FIXED_TYPE) == FIXED_TYPE;
}

void _OutputArray::create(Size _sz, int mtype, int i, bool allowTransposed, _OutputArray::DepthMask fixedDepthMask) const
{
    _InputArray::KindFlag k = kind();
    if( k == MAT && i < 0 && !allowTransposed && fixedDepthMask == 0 )
    {
        CV_Assert(!fixedSize() || ((Mat*)obj)->size.operator()() == _sz);
        CV_Assert(!fixedType() || ((Mat*)obj)->type() == mtype);
        ((Mat*)obj)->create(_sz, mtype);
        return;
    }
    int sizes[] = {_sz.height, _sz.width};
    create(2, sizes, mtype, i, allowTransposed, fixedDepthMask);
}

void _OutputArray::create(int _rows, int _cols, int mtype, int i, bool allowTransposed, _OutputArray::DepthMask fixedDepthMask) const
{
    _InputArray::KindFlag k = kind();
    if( k == MAT && i < 0 && !allowTransposed && fixedDepthMask == 0 )
    {
        CV_Assert(!fixedSize() || ((Mat*)obj)->size.operator()() == Size(_cols, _rows));
        CV_Assert(!fixedType() || ((Mat*)obj)->type() == mtype);
        ((Mat*)obj)->create(_rows, _cols, mtype);
        return;
    }
    int sizes[] = {_rows, _cols};
    create(2, sizes, mtype, i, allowTransposed, fixedDepthMask);
}

void _OutputArray::create(int d, const int* sizes, int mtype, int i,
                          bool allowTransposed, _OutputArray::DepthMask fixedDepthMask) const
{
    int size0 = d > 0 ? sizes[0] : 1, size1 = d > 1 ? sizes[1] : 1;
    _InputArray::KindFlag k = kind();
    mtype = CV_MAT_TYPE(mtype);

    if( k == MAT )
    {
        CV_Assert( i < 0 );
        Mat& m = *(Mat*)obj;
        CV_Assert(!(m.empty() && fixedType() && fixedSize()) && "Can't reallocate empty Mat with locked layout (probably due to misused 'const' modifier)");
        if (!m.empty() && d <= 2 && m.dims <= 2 &&
            m.type() == mtype &&
            ((m.rows == size0 && m.cols == size1) ||
            (allowTransposed && m.rows == size1 && m.cols == size0 && m.isContinuous())))
        {
            return;
        }

        if(fixedType())
        {
            if(CV_MAT_CN(mtype) == m.channels() && ((1 << CV_MAT_DEPTH(flags)) & fixedDepthMask) != 0 )
                mtype = m.type();
            else
                CV_CheckTypeEQ(m.type(), CV_MAT_TYPE(mtype), "Can't reallocate Mat with locked type (probably due to misused 'const' modifier)");
        }
        if(fixedSize())
        {
            CV_CheckEQ(m.dims, d, "Can't reallocate Mat with locked size (probably due to misused 'const' modifier)");
            for(int j = 0; j < d; ++j)
                CV_CheckEQ(m.size[j], sizes[j], "Can't reallocate Mat with locked size (probably due to misused 'const' modifier)");
        }
        m.create(d, sizes, mtype);
        return;
    }

    if( k == MATX )
    {
        CV_Assert( i < 0 );
        int type0 = CV_MAT_TYPE(flags);
        CV_Assert( mtype == type0 || (CV_MAT_CN(mtype) == 1 && ((1 << type0) & fixedDepthMask) != 0) );
        CV_CheckLE(d, 2, "");
        Size requested_size(d == 2 ? sizes[1] : 1, d >= 1 ? sizes[0] : 1);
        if (sz.width == 1 || sz.height == 1)
        {
            // NB: 1D arrays assume allowTransposed=true (see #4159)
            int total_1d = std::max(sz.width, sz.height);
            CV_Check(requested_size, std::max(requested_size.width, requested_size.height) == total_1d, "");
        }
        else
        {
            if (!allowTransposed)
            {
                CV_CheckEQ(requested_size, sz, "");
            }
            else
            {
                CV_Check(requested_size,
                        (requested_size == sz || (requested_size.height == sz.width && requested_size.width == sz.height)),
                        "");
            }
        }
        return;
    }

    if( k == STD_VECTOR || k == STD_VECTOR_VECTOR )
    {
        CV_Assert(!fixedSize());
        CV_Assert(k == STD_VECTOR_VECTOR || i < 0);
        CV_Assert( d <= 2 && (size0 == 1 || size1 == 1 || size0*size1 == 0) );
        size_t len = size0*size1 > 0 ? size0 + size1 - 1 : 0;
        int esz = CV_ELEM_SIZE(flags);

        if( k == STD_VECTOR || (k == STD_VECTOR_VECTOR && i >= 0) )
        {
            int type0 = CV_MAT_TYPE(flags);
            CV_Assert( mtype == type0 || (CV_MAT_CN(mtype) == CV_MAT_CN(type0) && ((1 << type0) & fixedDepthMask) != 0) );
        }

    #undef RESIZE_VEC
    #define RESIZE_VEC(T) \
        { \
            std::vector<T>* v; \
            if (k == STD_VECTOR_VECTOR) { \
                std::vector<std::vector<T> >* vv = reinterpret_cast<std::vector<std::vector<T> >*>(obj); \
                if (i < 0) { \
                    CV_Assert(!fixedSize() || len == vv->size()); \
                    vv->resize(len); \
                    return; \
                } \
                CV_Assert(size_t(i) < vv->size()); \
                v = &vv->at(i); \
            } else { \
                v = reinterpret_cast<std::vector<T>*>(obj); \
            } \
            CV_Assert(!fixedSize() || len == v->size()); \
            v->resize(len); \
        } \
        break

        STD_VECTOR_SWITCH(esz, RESIZE_VEC)
        return;
    }

    if( k == NONE )
    {
        CV_Error(cv::Error::StsNullPtr, "create() called for the missing output array" );
    }

    if( k == STD_VECTOR_MAT )
    {
        std::vector<Mat>& v = *(std::vector<Mat>*)obj;

        if( i < 0 )
        {
            CV_Assert( d == 2 && (sizes[0] == 1 || sizes[1] == 1 || sizes[0]*sizes[1] == 0) );
            size_t len = sizes[0]*sizes[1] > 0 ? sizes[0] + sizes[1] - 1 : 0, len0 = v.size();

            CV_Assert(!fixedSize() || len == len0);
            v.resize(len);
            if( fixedType() )
            {
                int _type = CV_MAT_TYPE(flags);
                for( size_t j = len0; j < len; j++ )
                {
                    if( v[j].type() == _type )
                        continue;
                    CV_Assert( v[j].empty() );
                    v[j].flags = (v[j].flags & ~CV_MAT_TYPE_MASK) | _type;
                }
            }
            return;
        }

        CV_Assert( i < (int)v.size() );
        Mat& m = v[i];

        if( allowTransposed )
        {
            if( !m.isContinuous() )
            {
                CV_Assert(!fixedType() && !fixedSize());
                m.release();
            }

            if( d == 2 && m.dims == 2 && m.data &&
                m.type() == mtype && m.rows == sizes[1] && m.cols == sizes[0] )
                return;
        }

        if(fixedType())
        {
            if(CV_MAT_CN(mtype) == m.channels() && ((1 << CV_MAT_TYPE(flags)) & fixedDepthMask) != 0 )
                mtype = m.type();
            else
                CV_Assert(CV_MAT_TYPE(mtype) == m.type());
        }
        if(fixedSize())
        {
            CV_Assert(m.dims == d);
            for(int j = 0; j < d; ++j)
                CV_Assert(m.size[j] == sizes[j]);
        }

        m.create(d, sizes, mtype);
        return;
    }

    if( k == STD_ARRAY_MAT )
    {
        Mat* v = (Mat*)obj;

        if( i < 0 )
        {
            CV_Assert( d == 2 && (sizes[0] == 1 || sizes[1] == 1 || sizes[0]*sizes[1] == 0) );
            size_t len = sizes[0]*sizes[1] > 0 ? sizes[0] + sizes[1] - 1 : 0, len0 = sz.height;

            CV_Assert(len == len0);
            if( fixedType() )
            {
                int _type = CV_MAT_TYPE(flags);
                for( size_t j = len0; j < len; j++ )
                {
                    if( v[j].type() == _type )
                        continue;
                    CV_Assert( v[j].empty() );
                    v[j].flags = (v[j].flags & ~CV_MAT_TYPE_MASK) | _type;
                }
            }
            return;
        }

        CV_Assert( i < sz.height );
        Mat& m = v[i];

        if( allowTransposed )
        {
            if( !m.isContinuous() )
            {
                CV_Assert(!fixedType() && !fixedSize());
                m.release();
            }

            if( d == 2 && m.dims == 2 && m.data &&
                m.type() == mtype && m.rows == sizes[1] && m.cols == sizes[0] )
                return;
        }

        if(fixedType())
        {
            if(CV_MAT_CN(mtype) == m.channels() && ((1 << CV_MAT_TYPE(flags)) & fixedDepthMask) != 0 )
                mtype = m.type();
            else
                CV_Assert(CV_MAT_TYPE(mtype) == m.type());
        }

        if(fixedSize())
        {
            CV_Assert(m.dims == d);
            for(int j = 0; j < d; ++j)
                CV_Assert(m.size[j] == sizes[j]);
        }

        m.create(d, sizes, mtype);
        return;
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

void _OutputArray::create(const MatShape& shape, int mtype, int i,
                          bool allowTransposed, _OutputArray::DepthMask fixedDepthMask) const
{
    if (shape.dims < 0) {
        release();
    } else {
        create(shape.dims, shape.p, mtype, i, allowTransposed, fixedDepthMask);
    }
}

Mat _OutputArray::reinterpret(int mtype) const
{
    mtype = CV_MAT_TYPE(mtype);
    return getMat().reinterpret(mtype);
}

void _OutputArray::createSameSize(const _InputArray& arr, int mtype) const
{
    int arrsz[CV_MAX_DIM], d = arr.sizend(arrsz);
    create(d, arrsz, mtype);
}

void _OutputArray::fit(int d, const int* sizes, int mtype, int i,
                       bool allowTransposed, _OutputArray::DepthMask fixedDepthMask) const
{
    int size0 = d > 0 ? sizes[0] : 1, size1 = d > 1 ? sizes[1] : 1;
    _InputArray::KindFlag k = kind();
    mtype = CV_MAT_TYPE(mtype);

    if( (k == MAT && i < 0) || (k == STD_VECTOR_MAT && i >= 0) )
    {
        Mat* m;
        if (k == MAT)
            m = (Mat*)obj;
        else {
            std::vector<Mat>& v = *(std::vector<Mat>*)obj;
            CV_Assert((size_t)i < v.size());
            m = &v[i];
        }
        CV_Assert(!(m->empty() && fixedType() && fixedSize()) && "Can't reallocate empty Mat with locked layout (probably due to misused 'const' modifier)");
        if (!m->empty() && d <= 2 && m->dims <= 2 &&
            m->type() == mtype &&
            ((m->rows == size0 && m->cols == size1) ||
             (allowTransposed && m->rows == size1 && m->cols == size0 && m->isContinuous())))
        {
            return;
        }

        if(fixedType())
        {
            if(CV_MAT_CN(mtype) == m->channels() && ((1 << CV_MAT_DEPTH(flags)) & fixedDepthMask) != 0 )
                mtype = m->type();
            else
                CV_CheckTypeEQ(m->type(), CV_MAT_TYPE(mtype), "Can't reallocate Mat with locked type (probably due to misused 'const' modifier)");
        }
        if(fixedSize())
        {
            CV_CheckEQ(m->dims, d, "Can't reallocate Mat with locked size (probably due to misused 'const' modifier)");
            for(int j = 0; j < d; ++j)
                CV_CheckEQ(m->size[j], sizes[j], "Can't reallocate Mat with locked size (probably due to misused 'const' modifier)");
        }
        m->fit(d, sizes, mtype);
        return;
    }

    create(d, sizes, mtype, i, allowTransposed, fixedDepthMask);
}

void _OutputArray::release() const
{
    CV_Assert(!fixedSize());

    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        ((Mat*)obj)->release();
        return;
    }

    if( k == NONE )
        return;

    if( k == STD_VECTOR || k == STD_VECTOR_VECTOR )
    {
    #undef CLEAR_VEC
    #define CLEAR_VEC(T) \
        if (k == STD_VECTOR_VECTOR) { \
            std::vector<std::vector<T> >* vv = \
                reinterpret_cast<std::vector<std::vector<T> >*>(obj); \
            vv->clear(); \
        } else { \
            std::vector<T>* v = reinterpret_cast<std::vector<T>*>(obj); \
            v->clear(); \
        } \
        break

        int esz = CV_ELEM_SIZE(flags);
        STD_VECTOR_SWITCH(esz, CLEAR_VEC);
        return;
    }

    if( k == STD_VECTOR_MAT )
    {
        ((std::vector<Mat>*)obj)->clear();
        return;
    }

    if( k == STD_BOOL_VECTOR )
    {
        ((std::vector<bool>*)obj)->clear();
        return;
    }

    CV_Error(Error::StsNotImplemented, "Unknown/unsupported array type");
}

void _OutputArray::clear() const
{
    _InputArray::KindFlag k = kind();

    if( k == MAT )
    {
        CV_Assert(!fixedSize());
        ((Mat*)obj)->resize(0);
        return;
    }

    release();
}

bool _OutputArray::needed() const
{
    return kind() != NONE;
}

Mat& _OutputArray::getMatRef(int i) const
{
    _InputArray::KindFlag k = kind();
    if( i < 0 )
    {
        CV_Assert( k == MAT );
        return *(Mat*)obj;
    }

    CV_Assert( k == STD_VECTOR_MAT || k == STD_ARRAY_MAT );

    if( k == STD_VECTOR_MAT )
    {
        std::vector<Mat>& v = *(std::vector<Mat>*)obj;
        CV_Assert( i < (int)v.size() );
        return v[i];
    }
    else
    {
        Mat* v = (Mat*)obj;
        CV_Assert( 0 <= i && i < sz.height );
        return v[i];
    }
}

void _OutputArray::setTo(const _InputArray& arr, const _InputArray & mask) const
{
    _InputArray::KindFlag k = kind();

    if( k == NONE )
        ;
    else if (k == MAT || k == MATX || k == STD_VECTOR)
    {
        Mat m = getMat();
        m.setTo(arr, mask);
    }
    else
        CV_Error(Error::StsNotImplemented, "");
}

void _OutputArray::setZero() const
{
    _InputArray::KindFlag k = kind();

    if( k == NONE )
        ;
    else if (k == MAT || k == MATX || k == STD_VECTOR)
    {
        Mat m = getMat();
        m.setZero();
    }
    else
    {
        setTo(Scalar::all(0), noArray());
    }
}


void _OutputArray::assign(const Mat& m) const
{
    _InputArray::KindFlag k = kind();
    if (k == MAT)
    {
        *(Mat*)obj = m;
    }
    else if (k == MATX)
    {
        m.copyTo(getMat());
    }
    else
    {
        CV_Error(Error::StsNotImplemented, "");
    }
}


void _OutputArray::move(Mat& m) const
{
    if (fixedSize())
    {
        // TODO Performance warning
        assign(m);
        return;
    }
    int k = kind();
    if (k == MAT)
    {
        *(Mat*)obj = std::move(m);
    }
    else if (k == MATX)
    {
        m.copyTo(getMat());
        m.release();
    }
    else
    {
        CV_Error(Error::StsNotImplemented, "");
    }
}


void _OutputArray::assign(const std::vector<Mat>& v) const
{
    _InputArray::KindFlag k = kind();
    if (k == STD_VECTOR_MAT)
    {
        std::vector<Mat>& this_v = *(std::vector<Mat>*)obj;
        CV_Assert(this_v.size() == v.size());

        for (size_t i = 0; i < v.size(); i++)
        {
            const Mat& m = v[i];
            Mat& this_m = this_v[i];
            if (this_m.u != NULL && this_m.u == m.u)
                continue; // same object (see dnn::Layer::forward_fallback)
            m.copyTo(this_m);
        }
    }
    else
    {
        CV_Error(Error::StsNotImplemented, "");
    }
}


static _InputOutputArray _none;
InputOutputArray noArray() { return _none; }

} // cv::
