#include "Scene.h"
#include "Material.h"
#include "PointLight.h"
#include "Sphere.h"
#include "Plane.h"

#include <iostream>

void Scene::addObject(std::shared_ptr<const IntersectableObject> object) {
  sceneObjects.push_back(object);
}

void Scene::addLight(std::shared_ptr<const LightSource> ls) {
  lightSources.push_back(ls);
}

Vec3 Scene::getBackgroundcolor() const {
  return backgroundColor;
}

std::optional<Intersection> Scene::intersect(const Ray& ray, bool shadowRay) const {
  std::optional<Intersection> result{};
  for (std::shared_ptr<const IntersectableObject> object : sceneObjects) {
    if (shadowRay && !object->getMaterial().isShadowCaster())
      continue;

    std::optional<Intersection> i = object->intersect(ray);
    if (!i.has_value())
      continue;

    if (!result.has_value() || i.value().getT() < result.value().getT())
      result = i;
  }
  return result;
}

/// <summary>
/// Trace a ray through the scene and compute its color value.
/// </summary>
/// <param name="ray">to trace</param>
/// <param name="IOR">optical density of the material we are currently travelling in</param>
/// <param name="recDepth">recursion depth</param>
/// <returns>final color value computed for this ray</returns>
Vec3 Scene::traceRay(const Ray& ray, float IOR, int recDepth) const {
  
  if (recDepth == 0)
		return backgroundColor;
  
  // no intersection found
  std::optional<Intersection> opt_intersection = intersect(ray, false);
  if (!opt_intersection.has_value())
    return backgroundColor;

  // else intersection found, do recursive ray tracing
  Intersection inter = opt_intersection.value();
  Vec3 interPos = ray.getPosOnRay(inter.getT());
  Vec3 offSurfacePos = interPos + inter.getNormal() * OFFSET_EPSILON;
  Vec3 inSurfacePos = interPos + inter.getNormal() * -OFFSET_EPSILON;


  Vec3 localColor{ 0.0f, 0.0f, 0.0f };
  for (std::shared_ptr<const LightSource> ls : lightSources) {
    Ray shadowRay{ offSurfacePos, ls->getDirection(offSurfacePos) };
    std::optional<Intersection> shadowInter = intersect(shadowRay, true);

    Vec3 ambient = inter.getMaterial().getAmbient() * ls->getAmbient();

    if (!shadowInter.has_value() || shadowInter->getT() > ls->getDistance(offSurfacePos)) {
      float d = Vec3::dot(ls->getDirection(offSurfacePos), inter.getNormal());
      Vec3 diffuse = inter.getMaterial().getDiffuse() * ls->getDiffuse() * d;
      diffuse = Vec3::clamp(diffuse, 0.0f, 1.0f);

      Vec3 Rv = Vec3::reflect(ray.getDirection(), inter.getNormal());
      float s = pow(std::max(0.0f, Vec3::dot(Rv, ls->getDirection(offSurfacePos))), inter.getMaterial().getExp());
      Vec3 specular = inter.getMaterial().getSpecular() * ls->getSpecular() * s;
      specular = Vec3::clamp(specular, 0.0f, 1.0f);

      localColor = localColor + ambient + diffuse + specular;
    } else {
      localColor = localColor + ambient;
    }
  }


  // Firstly decide if it is entering or exiting the material
  float dotDN = Vec3::dot(ray.getDirection(), inter.getNormal());
  bool rayEntering = dotDN < 0;
  float materialIOR = inter.getMaterial().getIndexOfRefraction().value_or(1.0f);
  float IOR_from = IOR;
  float IOR_to = rayEntering ? materialIOR : 1.0f; // Assume: When the ray is exiting the material, it is going to air with IOR = 1.0f
  
  // When doing reflexion, if entering then offSurfacePos else inSurfacePos
  Vec3 difSurfacePos = rayEntering ? offSurfacePos : inSurfacePos;

  // Calculating Reflected Color
  Vec3 reflectedColor{ 0.0f, 0.0f, 0.0f };
  if (inter.getMaterial().reflects()) {
    Ray reflectedRay{ difSurfacePos, Vec3::reflect(ray.getDirection(), inter.getNormal()) };
    // Since the medium does not change when reflecting, we keep the same IOR for the reflected ray
    reflectedColor = traceRay(reflectedRay, IOR, recDepth - 1);
  }

  // Calculating Refracted Color

  // When doing refraction, if entering then inSurfacePos else offSurfacePos
  difSurfacePos = rayEntering ? inSurfacePos : offSurfacePos; // When refracting, if entering then inSurfacePos else offSurfacePos, since we want to make sure the refracted ray starts in the medium it is going into to avoid numerical issues with intersecting with the same surface again immediately

  
  Vec3 refractedColor{ 0.0f, 0.0f, 0.0f };
  if (inter.getMaterial().refracts()) {
    // Since the refract function detects if we are entering or exiting
    // based on the sign of the dot product,
    // Only the materialIOR of the intersection medium needs to be passed to it
    std::optional<Vec3> refractedDir = Vec3::refract(ray.getDirection(), inter.getNormal(), materialIOR);

    if (refractedDir.has_value()) {
      Ray refractedRay{ difSurfacePos, refractedDir.value() };
      // Since we are changing medium when refracting, we need to update the IOR for the refracted ray
      refractedColor = traceRay(refractedRay, IOR_to, recDepth - 1);
    }else{
      std::cout << "Total internal reflection occurred. No refraction possible." << std::endl;
    }
  }


  // Calculating the weights using Schlick's approximation of the Fresnel equations
  float localWeight, reflectedWeight, refractedWeight;
  float local = inter.getMaterial().getLocalRefectivity();
  
  Vec3 IncomingDir = ray.getDirection() * -1.0f;  // To calculate the angle between the incoming ray and the normal, we need the incoming direction vector pointing towards the surface
  Vec3 N = rayEntering ? inter.getNormal() : inter.getNormal() * -1.0f; // We need the normal to point against the incoming ray for the calculations, if the ray is exiting the material we need to invert the normal
  float cosTheta = Vec3::dot(IncomingDir, N);
  cosTheta = std::clamp(cosTheta, 0.0f, 1.0f); // Clamp cosTheta to the range [0, 1] to avoid numerical issues

  if (inter.getMaterial().reflects() && !inter.getMaterial().refracts()) {
    float R_0 = 1.0f - local;
    float Rtheta = R_0 + (1.0f - R_0) * std::pow(1.0f - cosTheta, 5);

    reflectedWeight = Rtheta;
    localWeight = 1.0f - reflectedWeight;
    refractedWeight = 0.0f;
  }
  else if (inter.getMaterial().reflects() && inter.getMaterial().refracts()) {

    float R_0 = pow((IOR_from - IOR_to) / (IOR_from + IOR_to), 2);
    float Rtheta = R_0 + (1.0f - R_0) * std::pow(1.0f - cosTheta, 5);
    float Ttheta = 1.0f - Rtheta;
    
    localWeight = local;
    float remainingWeight = 1.0f - localWeight;

    reflectedWeight = remainingWeight * Rtheta;
    refractedWeight = remainingWeight * Ttheta;
  } 
  else {
    localWeight = 1.0f;
    reflectedWeight = 0.0f;
    refractedWeight = 0.0f;
  }

  return localColor * localWeight + reflectedColor * reflectedWeight + refractedColor * refractedWeight;
  
}

Scene Scene::genSimpleScene() {
  // create an empty scene
  Scene s;

  // and God said, let there be light and there was light
  const auto l = std::make_shared<const PointLight>(Vec3{ 0, 4, -2 },
                                                    Vec3{ 1, 1, 1 },
                                                    Vec3{ 1, 1, 1 },
                                                    Vec3{ 1, 1, 1 });

  // attach the light source to the scene
  s.addLight(l);

  // create the bluish material for the right sphere
  // vec3 are treated as color values in the range [0, 1]
  Material m(Vec3(0.0f, 0.0f, 0.3f),
             Vec3(0.0f, 0.0f, 0.5f),
             Vec3(1.0f, 1.0f, 1.0f),
             8, 0.2f, 1.52f);
  // create a sphere, apply the material above to it and attach it to the scene
  s.addObject(std::make_shared<Sphere>(Vec3{ 0.7f, -0.4f, -2.0f }, 0.9f, m));

  // create the red material and apply it to the left sphere
  m = Material(Vec3{ 0.3f, 0.0f, 0.0f },
               Vec3{ 0.5f, 0.0f, 0.0f },
               Vec3{ 1.0f, 1.0f, 1.0f }, 8, 1);
  s.addObject(std::make_shared<Sphere>(Vec3{ -0.9f, -0.1f, -2.2f }, 0.6f, m));

  // create the yellowish material and apply it to the big sphere in the back
  m = Material(Vec3{ 0.3f, 0.3f, 0.0f },
               Vec3{ 0.7f, 0.7f, 0.0f },
               Vec3{ 1.0f, 1.0f, 0.0f }, 8, 0.3f);
  s.addObject(std::make_shared<Sphere>(Vec3{ 0.0f, 4.0f, -8.0f }, 3.9f, m));

  // create the white ground plane
  m = Material(Vec3{ 0.3f, 0.3f, 0.3f },
               Vec3{ 0.5f, 0.5f, 0.5f },
               Vec3{ 1.0f, 1.0f, 1.0f }, 32, 0.5f);
  s.addObject(std::make_shared<Plane>(Vec3{ 0.0f, 1.0f, 0.0f }, 1.5f, m));

  return s;
}
