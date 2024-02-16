// Copyright (c) 2023 UltiMaker
// CuraEngine is released under the terms of the AGPLv3 or higher

#ifndef GEOMETRY_POLYGON_H
#define GEOMETRY_POLYGON_H

#include "generic_closed_polyline.h"

namespace cura
{

class Shape;
class ListPolyIt;
class AngleDegrees;

class Polygon : public GenericClosedPolyline<PolylineType::Filled>
{
public:
    Polygon() = default;

    Polygon(const Polygon& other) = default;

    Polygon(Polygon&& other) = default;

    Polygon(const std::initializer_list<Point2LL>& initializer)
        : GenericClosedPolyline<PolylineType::Filled>(initializer)
    {
    }

    Polygon(const std::vector<Point2LL>& points)
        : GenericClosedPolyline<PolylineType::Filled>(points)
    {
    }

    Polygon& operator=(const Polygon& other)
    {
        GenericClosedPolyline::operator=(other);
        return *this;
    }

    /*!
     * Compute the morphological intersection between this polygon and another.
     *
     * Note that the result may consist of multiple polygons, if you have bad
     * luck.
     *
     * \param other The polygon with which to intersect this polygon.
     */
    Shape intersection(const Polygon& other) const;

    double area() const
    {
        return ClipperLib::Area(*this);
    }

    Point2LL centerOfMass() const;

    Shape offset(int distance, ClipperLib::JoinType joinType = ClipperLib::jtMiter, double miter_limit = 1.2) const;

    /*!
     * Smooth out small perpendicular segments and store the result in \p result.
     * Smoothing is performed by removing the inner most vertex of a line segment smaller than \p remove_length
     * which has an angle with the next and previous line segment smaller than roughly 150*
     *
     * Note that in its current implementation this function doesn't remove line segments with an angle smaller than 30*
     * Such would be the case for an N shape.
     *
     * \param remove_length The length of the largest segment removed
     * \param result (output) The result polygon, assumed to be empty
     */
    void smooth(int remove_length, Polygon& result) const;

    /*!
     * Smooth out sharp inner corners, by taking a shortcut which bypasses the corner
     *
     * \param angle The maximum angle of inner corners to be smoothed out
     * \param shortcut_length The desired length of the shortcut line segment introduced (shorter shortcuts may be unavoidable)
     * \param result The resulting polygon
     */
    void smooth_outward(const AngleDegrees angle, int shortcut_length, Polygon& result) const;

    /*!
     * Smooth out the polygon and store the result in \p result.
     * Smoothing is performed by removing vertices for which both connected line segments are smaller than \p remove_length
     *
     * \param remove_length The length of the largest segment removed
     * \param result (output) The result polygon, assumed to be empty
     */
    void smooth2(int remove_length, Polygon& result) const;

    /*!
     * Smooth out a simple corner consisting of two linesegments.
     *
     * Auxiliary function for \ref smooth_outward
     *
     * \param p0 The point before the corner
     * \param p1 The corner
     * \param p2 The point after the corner
     * \param p0_it Iterator to the point before the corner
     * \param p1_it Iterator to the corner
     * \param p2_it Iterator to the point after the corner
     * \param v10 Vector from \p p1 to \p p0
     * \param v12 Vector from \p p1 to \p p2
     * \param v02 Vector from \p p0 to \p p2
     * \param shortcut_length The desired length ofthe shortcutting line
     * \param cos_angle The cosine on the angle in L 012
     */
    static void smooth_corner_simple(
        const Point2LL& p0,
        const Point2LL& p1,
        const Point2LL& p2,
        const ListPolyIt& p0_it,
        const ListPolyIt& p1_it,
        const ListPolyIt& p2_it,
        const Point2LL& v10,
        const Point2LL& v12,
        const Point2LL& v02,
        const int64_t shortcut_length,
        double cos_angle);

    /*!
     * Smooth out a complex corner where the shortcut bypasses more than two line segments
     *
     * Auxiliary function for \ref smooth_outward
     *
     * \warning This function might try to remove the whole polygon
     * Error code -1 means the whole polygon should be removed (which means it is a hole polygon)
     *
     * \param p1 The corner point
     * \param[in,out] p0_it Iterator to the last point checked before \p p1 to consider cutting off
     * \param[in,out] p2_it Iterator to the last point checked after \p p1 to consider cutting off
     * \param shortcut_length The desired length ofthe shortcutting line
     * \return Whether this whole polygon whould be removed by the smoothing
     */
    static bool smooth_corner_complex(const Point2LL& p1, ListPolyIt& p0_it, ListPolyIt& p2_it, const int64_t shortcut_length);

    /*!
     * Try to take a step away from the corner point in order to take a bigger shortcut.
     *
     * Try to take the shortcut from a place as far away from the corner as the place we are taking the shortcut to.
     *
     * Auxiliary function for \ref smooth_outward
     *
     * \param[in] p1 The corner point
     * \param[in] shortcut_length2 The square of the desired length ofthe shortcutting line
     * \param[in,out] p0_it Iterator to the previously checked point somewhere beyond \p p1. Updated for the next iteration.
     * \param[in,out] p2_it Iterator to the previously checked point somewhere before \p p1. Updated for the next iteration.
     * \param[in,out] forward_is_blocked Whether trying another step forward is blocked by the smoothing outward condition. Updated for the next iteration.
     * \param[in,out] backward_is_blocked Whether trying another step backward is blocked by the smoothing outward condition. Updated for the next iteration.
     * \param[in,out] forward_is_too_far Whether trying another step forward is blocked by the shortcut length condition. Updated for the next iteration.
     * \param[in,out] backward_is_too_far Whether trying another step backward is blocked by the shortcut length condition. Updated for the next iteration.
     */
    static void smooth_outward_step(
        const Point2LL& p1,
        const int64_t shortcut_length2,
        ListPolyIt& p0_it,
        ListPolyIt& p2_it,
        bool& forward_is_blocked,
        bool& backward_is_blocked,
        bool& forward_is_too_far,
        bool& backward_is_too_far);
};

} // namespace cura

namespace std
{
#if 0
template<>
struct hash<cura::Polygon*>
{
    size_t operator()(const cura::PolygonPointer& poly) const
    {
        const cura::ConstPolygonRef ref = *static_cast<cura::PolygonPointer>(poly);
        return std::hash<const ClipperLib::Path*>()(&*ref);
    }
};
#endif
} // namespace std

#endif // GEOMETRY_POLYGON_H
