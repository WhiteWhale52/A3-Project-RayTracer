#include "RayTracer.h"


static Ray MakeRay(const Camera* camera, float u, float v){
    Vector3 target = Add(camera->lower_left,
         Add(Scale(camera->horizontal, u), Scale(camera->vertical,v)));
    
    Ray ray;
    ray.origin = camera->origin;
    ray.direction = Norm(Subtract(target, camera->origin));
    return ray;
}

/* ────────────────────────────────────────────────────────────
 * TODO 1 — Ray–sphere intersection
 *
 * Test whether ray `r` intersects sphere `s` at parameter t
 * in (t_min, t_max).  Return 1 on hit and fill *t_out with
 * the nearest positive t; return 0 on miss.
 *
 * Hint: solve |r.origin + t*r.dir - s.center|^2 = s.radius^2
 *        rearranges to a quadratic in t.
 * ──────────────────────────────────────────────────────────── */
static int intersect_sphere(const Ray *r, const Sphere *s,
                             float t_min, float t_max,
                             float *t_out) {
    /* TODO: compute a, b, c of the quadratic               */
    /* TODO: compute discriminant; return 0 if negative     */
    /* TODO: find the smaller root in (t_min, t_max)        */
    /* TODO: if no root in range try the larger root        */
    /* TODO: store result in *t_out and return 1            */
    (void)r; (void)s; (void)t_min; (void)t_max; (void)t_out;
    return 0; /* placeholder */
}

/* ────────────────────────────────────────────────────────────
 * TODO 2 — Scene intersection
 *
 * Walk all spheres; return the closest hit or 0 if none.
 * Fill *hit_sphere and *hit_t on success.
 * ──────────────────────────────────────────────────────────── */
static int intersect_scene(const Scene *p_Scene, const Ray *r,
                            float t_min, float t_max,
                            const Sphere **hit_sphere,
                            float *hit_t) {
    /* TODO: loop over p_Scene->spheres[0..num_spheres-1]     */
    /* TODO: call intersect_sphere() for each               */
    /* TODO: track the closest hit, shrink t_max each time  */
    (void)p_Scene; (void)r; (void)t_min; (void)t_max;
    (void)hit_sphere; (void)hit_t;
    return 0; /* placeholder */
}

/* ────────────────────────────────────────────────────────────
 * TODO 3 — Recursive colour / shading function
 *
 * Cast ray `r` into the scene.  On a hit, compute shading
 * and optionally recurse (reflection / diffuse bounce) up to
 * `depth` times.  Return the colour as a Vector3 in [0,1]^3.
 *
 * Minimum viable version: Lambertian diffuse + sky gradient.
 * Stretch goal: add mirror reflection using reflectance field.
 *
 * Hint for the sky: lerp between white and light-blue based
 *   on the normalised y-component of r.dir.
 * ──────────────────────────────────────────────────────────── */
static Vector3 RayColor(const Scene *scene, const Ray *r, int depth) {
    if (depth <= 0)
        return (Vector3){0,0,0};   /* recursion limit hit */

    /* TODO: call intersect_scene()                         */
    /* TODO: on hit — compute surface normal, scatter ray   */
    /*                return mat.color * ray_color(scatter) */
    /* TODO: on miss — return sky background colour         */

    (void)scene; (void)r; /* silence warnings until implemented */

    /* placeholder: solid grey so the image isn't all-black */
    return (Vector3){0.5f, 0.5f, 0.5f};
}


Pixel TracePixel(const Scene* p_Scene, int pixelX, int pixelY){
    Vector3 accumulations = {0.0f, 0.0f, 0.0f};

    int samplePerPixel = p_Scene->samples_per_pixel;

    for (int s = 0; s < samplePerPixel; s++)
    {
        float u = ((float) pixelX + randf()) / (float)(p_Scene->image_width - 1);
        float v = ((float) pixelY + randf()) / (float)(p_Scene->image_height - 1);
        v = 1.0f - v;

        Ray ray = MakeRay(&p_Scene->camera, u, v);
        Vector3 color = RayColor(p_Scene, &ray, p_Scene->max_depth);
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