#include "rectangle.h"

Rectangle Rectangle::Empty=Rectangle();
Rectangle Rectangle::FromLTRB(int left, int top, int right, int bottom)
      {
         return Rectangle(left,
                              top,
                              right - left,
                              bottom - top);
      }
Rectangle Rectangle::Inflate(Rectangle rect, int x, int y)
      {
         Rectangle r = rect;
         r.Inflate(x, y);
         return r;
      }
Rectangle Rectangle::Intersect(Rectangle a, Rectangle b)
      {
         int x1 = std::max(a.X(), b.X());
         int x2 = std::min(a.X() + a.Width(), b.X() + b.Width());
         int y1 = std::max(a.Y(), b.Y());
         int y2 = std::min(a.Y() + a.Height(), b.Y() + b.Height());

         if(x2 >= x1
                && y2 >= y1)
         {

            return  Rectangle(x1, y1, x2 - x1, y2 - y1);
         }
         return Rectangle::Empty;
      }
Rectangle Rectangle::Union(const Rectangle &a,const Rectangle &b)
      {
    int x1 = std::min(a.x, b.x);
    int x2 = std::max(a.x + a.width, b.x + b.width);
    int y1 = std::min(a.y, b.y);
    int y2 = std::max(a.y + a.height, b.y + b.height);

         return Rectangle(x1, y1, x2 - x1, y2 - y1);
      }
bool operator==(Rectangle const& lhs,Rectangle const& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height);
}
uint qHash(Rectangle const& rect)
      {
         return (int) ((quint32) rect.x ^
                        (((quint32) rect.y << 13) | ((quint32) rect.y >> 19)) ^
                        (((quint32) rect.width << 26) | ((quint32) rect.width >>  6)) ^
                        (((quint32) rect.height <<  7) | ((quint32) rect.height >> 25)));
      }
