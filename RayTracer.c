#include "RayTracer.h"
#include <float.h>


static Ray MakeRay(const Camera* camera, float u, float v){
    Vector3 target = Add(camera->lower_left,
         Add(Scale(camera->horizontal, u), Scale(camera->vertical,v)));
    
    Ray ray;
    ray.origin = camera->origin;
    ray.direction = Norm(Subtract(target, camera->origin));
    return ray;
}


static int SphereIntersection(const Ray* ray, const Sphere* sphere, float t_min, float t_max, float *t_out) {

    Vector3 origin = Subtract(ray->origin, sphere->origin);

    float a = Dot(ray->direction, ray->direction);
    float b = 2.0f * Dot(origin, ray->direction);
    float c = Dot(origin, origin) - sphere->radius * sphere->radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return 0;
    }

    float t = (-b - sqrt(discriminant)) / (2.0f * a);
    if (t > t_min && t < t_max) { *t_out = t; return 1; }

    t = (-b + sqrt(discriminant)) / (2.0f * a);
    if (t > t_min && t < t_max) { *t_out = t; return 1; }

    
    return 0; 
}


static int SceneIntersection(const Scene* p_Scene, const Ray* ray, float t_min, float t_max, const Sphere** hit_sphere, float* hit_t) {
    int  foundIntersection   = 0;
    float closest = t_max;

    for (int i = 0; i < p_Scene->num_spheres; i++) {
        float t;
        if (SphereIntersection(ray, &p_Scene->spheres[i], t_min, closest, &t)) {
            foundIntersection       = 1;
            closest     = t;
            *hit_t      = t;
            *hit_sphere = &p_Scene->spheres[i];
        }
    }
    return foundIntersection;
}


static Vector3 RayColor(const Scene *scene, const Ray *rayIn, int depth, int *seed) {
    if (depth <= 0)
        return (Vector3){0,0,0};   /* recursion limit hit */

    Vector3 final_color   = {0.0f, 0.0f, 0.0f};
    Vector3 contributions = {1.0f, 1.0f, 1.0f};

    Ray ray = *rayIn;   /* local copy — updated each bounce */

    for (int bounce = 0; bounce < depth; bounce++) {
        *seed += bounce;   
        const Sphere *hit_sphere = NULL;
        float           hit_t      = 0.0f;

        if (!SceneIntersection(scene, &ray, 0.001f, FLT_MAX, &hit_sphere, &hit_t)) {
           
            Vector3 sky = {0.6f, 0.7f, 0.9f};
            final_color = Add(final_color, Mul(sky, contributions)); 
            break;
        }

        const Material *material = &hit_sphere->material;

        /* hit point in world space  */
        Vector3 hit_point = Add(ray.origin, Scale(ray.direction, hit_t));

        /* outward surface normal (sphere-local then normalise) */
        Vector3 normal = Norm(Subtract(hit_point, hit_sphere->origin));

        /* accumulate emission from this surface */
        Vector3 emission = GetEmission(material);
        final_color = Add(final_color, Mul(emission, contributions));

        /* tint contributions by albedo for future bounces */
        contributions = Mul(contributions, material->color);

        Vector3 scatter = Norm(Add(normal, RandomUnitVector(seed)));

        /* offset origin along normal to avoid self-intersection */
        ray.origin = Add(hit_point, Scale(normal, 0.001f));
        ray.direction    = scatter;

    }

    return final_color;

}


Pixel TracePixel(const Scene* p_Scene, int pixelX, int pixelY){
    Vector3 accumulations = {0.0f, 0.0f, 0.0f};

    int samplePerPixel = p_Scene->samples_per_pixel;

    for (int sample = 0; sample < samplePerPixel; sample++)
    {
        float u = ((float) pixelX + randf()) / (float)(p_Scene->image_width - 1);
        float v = ((float) pixelY + randf()) / (float)(p_Scene->image_height - 1);
        v = 1.0f - v;

        Ray ray = MakeRay(&p_Scene->camera, u, v);
        int seed = pixelX + pixelY * p_Scene->image_width;
        seed *= (sample+1); 

        Vector3 color = RayColor(p_Scene, &ray, p_Scene->max_depth, &seed);
        accumulations = Add(accumulations, color);

    }

    float scale = 1.0f / (float) samplePerPixel;

    float r = accumulations.x * scale;
    float g = accumulations.y * scale;
    float b = accumulations.z * scale;


    // Here I am applying some gamma correction; Will learn more to implement it better
    r = sqrtf(r < 0 ? 0 : r);
    g = sqrtf(g < 0 ? 0 : g);
    b = sqrtf(b < 0 ? 0 : b);

    Pixel pixelOut;
    pixelOut.r = (char)(255.99f * (r > 1.0f ? 1.0f : r));
    pixelOut.g = (char)(255.99f * (g > 1.0f ? 1.0f : g));
    pixelOut.b = (char)(255.99f * (b > 1.0f ? 1.0f : b));
    return pixelOut;
}