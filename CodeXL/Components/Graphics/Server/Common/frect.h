//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  a template class that can be used to define rectangles where the four elements are of a particular data type
//==============================================================================

#ifndef GPS_FRECT_H
#define GPS_FRECT_H

//-----------------------------------------------------------------------------
/// Used to define a screen quad position and dimension.
/// This struct can be returned by the ToWinRect() function of a Rect and allows
/// users to convert between a Rect<T> and a windows RECT using
/// reinterpret_cast<RECT*>
//-----------------------------------------------------------------------------
struct WinRect
{
    /// left
    long left;

    /// top
    long top;

    /// right
    long right;

    ///bottom
    long bottom;
};

//-----------------------------------------------------------------------------
/// Rect<T> template class
//-----------------------------------------------------------------------------
template< class T > class Rect
{
public:
    /// left coordinate of the rectangle
    T left;

    /// top coordinate of the rectangle
    T top;

    /// right coordinate of the rectangle
    T right;

    /// bottom coordinate of the rectangle
    T bottom;

    //--------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------
    Rect< T > () { left = top = right = bottom = 0; };

    //--------------------------------------------------------------------------
    /// Constructor ( overloaded )
    //--------------------------------------------------------------------------
    Rect< T > (T tLeft, T tTop, T tRight, T tBottom)
    {
        left = tLeft; top = tTop; right = tRight; bottom = tBottom;
    };

    //--------------------------------------------------------------------------
    /// Constructor
    ///
    /// Allows construction of Rect<T> based on a Rect of a different type; also
    /// useful for casting
    /// \param rect input rect
    //--------------------------------------------------------------------------
    template <class S> Rect<T> (Rect<S> rect)
    {
        left = (T)rect.left;
        top = (T)rect.top;
        right = (T)rect.right;
        bottom = (T)rect.bottom;
    };

    //--------------------------------------------------------------------------
    /// Width
    /// \return width of the Rect (right - left)
    //--------------------------------------------------------------------------
    T Width() { return right - left; };

    //--------------------------------------------------------------------------
    /// Height
    /// \return height of the Rect (bottom - top)
    //--------------------------------------------------------------------------
    T Height() { return bottom - top; };

    //--------------------------------------------------------------------------
    /// Area
    /// \return Area of the Rect (Width() * Height())
    //--------------------------------------------------------------------------
    T Area() { return Width() * Height(); };

    //--------------------------------------------------------------------------
    /// ToWinRect
    ///
    /// \return structure that can be reinterpret_cast to RECT* for Windows OS
    //--------------------------------------------------------------------------
    WinRect ToWinRect()
    {
        WinRect wr;
        wr.left = (long)left;
        wr.right = (long)right;
        wr.top = (long)top;
        wr.bottom = (long)bottom;
        return wr;
    };

    /// Resizes the rect so that it has the supplied aspect ratio
    /// and fits inside of its previous area. ie: this can only make
    /// the rect smaller than it previously was.
    /// \param fAspectRatio the aspect ratio ( width / height ) to apply to the rect
    void ApplyAspectRatio(float fAspectRatio)
    {
        float fRectRatio = Width() / Height();

        if (fAspectRatio > fRectRatio)
        {
            // desired ratio is wider than rect
            // need to make rect less tall

            // store old height so that we can recenter the image
            T oldHeight = Height();

            bottom = top + (Width() / fAspectRatio);

            // recenter height
            T delta = (oldHeight - Height()) / 2;
            top += delta;
            bottom += delta;
        }
        else if (fAspectRatio < fRectRatio)
        {
            // desired ratio is taller than rect
            // need to make rect more narrow

            // store old width
            T oldWidth = Width();

            right = left + (Height() * fAspectRatio);

            // recenter width
            T delta = (oldWidth - Width()) / 2;
            left += delta;
            right += delta;
        }
        else
        {
            // they are equal, do nothing
        }
    }
};

typedef Rect< float > FRECT; ///< Typedefs to FRECT type
typedef Rect< long >  LRECT; ///< Typedefs to LRECT type
typedef Rect< int >   NRECT; ///< Typedefs to NRECT type

#endif // GPS_FRECT_H