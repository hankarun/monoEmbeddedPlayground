#pragma once
#include "hitable.h"
#include "vec3.h"

class Triangle : public hitable
{
public:
	Triangle(const vec3& p1, const vec3& p2, const vec3& p3, material* m);
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

	material* mat_ptr;
private:
	vec3 p1;
	vec3 p2;
	vec3 p3;
	vec3 normal;
};