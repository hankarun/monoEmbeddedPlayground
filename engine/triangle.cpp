#include "triangle.h"

Triangle::Triangle(const vec3& p1, const vec3& p2, const vec3& p3, material* m)
    : p1(p1), p2(p2), p3(p3), mat_ptr(m)
{
    vec3 p12 = p1 - p2;
    vec3 p13 = p1 - p3;
    normal = cross(p12, p13);
}

bool Triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
    float area2 = normal.length();

    // Step 1: finding P
    float kEpsilon = 0.00000000000001;

    // check if ray and plane are parallel ?
    float NdotRayDirection = dot(normal, r.direction());
    if (fabs(NdotRayDirection) < kEpsilon)  //almost 0 
        return false;  //they are parallel so they don't intersect ! 

    // compute d parameter using equation 2
    float d = -dot(normal, p1);

    // compute t (equation 3)
    float t = -(dot(normal, r.origin()) + d) / NdotRayDirection;

    // check if the triangle is in behind the ray
    if (t < 0) return false;  //the triangle is behind 

    // compute the intersection point using equation 1
    vec3 P = r.origin() + t * r.direction();

    // Step 2: inside-outside test
    vec3 C;  //vector perpendicular to triangle's plane 

    // edge 0
    vec3 edge0 = p2 - p1;
    vec3 vp0 = P - p1;
    C = cross(edge0, vp0);
    if (dot(normal, C) < 0) return false;  //P is on the right side 

    // edge 1
    vec3 edge1 = p3 - p2;
    vec3 vp1 = P - p2;
    C = cross(edge1, vp1);
    if (dot(normal, C) < 0)  return false;  //P is on the right side 

    // edge 2
    vec3 edge2 = p1 - p3;
    vec3 vp2 = P - p3;
    C = cross(edge2, vp2);
    if (dot(normal, C) < 0) return false;  //P is on the right side; 

    rec.t = t;
    rec.p = r.point_at_parameter(rec.t);
    rec.normal = normal;
    rec.mat_ptr = mat_ptr;

    return true;  //this ray hits the triangle 
}
