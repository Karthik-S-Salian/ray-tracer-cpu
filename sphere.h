#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable
{
public:
  sphere(point3 _center, double _radius) : center1(_center), radius(_radius) {}

  // stationary shpere
  sphere(point3 _center, double _radius, shared_ptr<material> _material)
      : center1(_center), center_vec(vec3()), radius(fmax(0, _radius)), mat(_material)
  {
    auto rvec = vec3(radius, radius, radius);
    bbox = aabb(center1 - rvec, center1 + rvec);
  }

  // moving shpere
  sphere(point3 _center1, point3 _center2, double _radius, shared_ptr<material> _material)
      : center1(_center1), radius(fmax(0, _radius)), mat(_material)
  {
    center_vec = _center2 - _center1;

    auto rvec = vec3(radius, radius, radius);
    aabb box1(_center1 - rvec, _center1 + rvec);
    aabb box2(_center2 - rvec, _center2 + rvec);
    bbox = aabb(box1, box2);
  }

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override
  {
    point3 center = sphere_center(r.time());
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0)
      return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (!ray_t.surrounds(root))
    {
      root = (-half_b + sqrtd) / a;
      if (!ray_t.surrounds(root))
        return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);

    vec3 outward_normal = (rec.p - center1) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat = mat;

    return true;
  }

  point3 sphere_center(double time) const
  {
    return center1 + time * center_vec;
  }

  aabb bounding_box() const override { return bbox; }

  static void get_sphere_uv(point3 &p, double &u, double &v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //<1 0 0> yields <0.50 0.50> <-1 0 0> yields <0.00 0.50>
    //<0 1 0> yields <0.50 1.00> < 0 -1 0> yields <0.50 0.00>
    //<0 0 1> yields <0.25 0.50> < 0 0 -1> yields <0.75 0.50>

    auto theta =  acos(p.y());
    auto phi = atan2(-p.z(),p.x()) + pi;

    u = phi/(2*pi);
    v = theta/pi;
  }

private:
  point3 center1;
  double radius;
  shared_ptr<material> mat;
  vec3 center_vec;
  aabb bbox;
};

#endif