#include "EmbreeScene.h"
#include "../../Objects/RenderObject.h"
#include "../../Geometry/Intersection.h"
#include "../../Geometry/Triangle.h"

using namespace DirectX::SimpleMath;

void error_handler(const RTCError code, const char *str) {
  printf("Embree: ");
  switch (code) {
  case RTC_UNKNOWN_ERROR:
    printf("RTC_UNKNOWN_ERROR");
    break;
  case RTC_INVALID_ARGUMENT:
    printf("RTC_INVALID_ARGUMENT");
    break;
  case RTC_INVALID_OPERATION:
    printf("RTC_INVALID_OPERATION");
    break;
  case RTC_OUT_OF_MEMORY:
    printf("RTC_OUT_OF_MEMORY");
    break;
  case RTC_UNSUPPORTED_CPU:
    printf("RTC_UNSUPPORTED_CPU");
    break;
  case RTC_CANCELLED:
    printf("RTC_CANCELLED");
    break;
  default:
    printf("invalid error code");
    break;
  }
  if (str) {
    printf(" (");
    while (*str)
      putchar(*str++);
    printf(")\n");
  }
  exit(1);
}

EmbreeScene::EmbreeScene() {
  m_Device = rtcNewDevice(nullptr);
  rtcDeviceSetErrorFunction(m_Device, error_handler);
  m_Scene = rtcDeviceNewScene(m_Device, RTC_SCENE_STATIC, RTC_INTERSECT1);
}

void EmbreeScene::Clear()
{
	rtcDeleteScene(m_Scene);
	m_Scene = rtcDeviceNewScene(m_Device, RTC_SCENE_STATIC, RTC_INTERSECT1);
}

bool EmbreeScene::AddObject(RenderObject *obj) {
  if (!obj->HasBuffers()) {
    return false;
  }

  auto vertexPtr = (Vector3 *)obj->GetVertexBuffer();
  auto indexPtr = (Triangle *)obj->GetIndexBuffer();

  unsigned geomID =
      rtcNewTriangleMesh(m_Scene, RTC_GEOMETRY_STATIC, obj->GetTriangleCount(),
                         obj->GetVertexCount());
  auto transform = obj->GetTransform();

  Vector4 *vertices =
      (Vector4 *)rtcMapBuffer(m_Scene, geomID, RTC_VERTEX_BUFFER);

  for (size_t i = 0; i < obj->GetVertexCount(); i++) {
    Vector3 v = Vector3::Transform(vertexPtr[i], transform);
    vertices[i] = Vector4(v.x, v.y, v.z, 0);
  }

  rtcUnmapBuffer(m_Scene, geomID, RTC_VERTEX_BUFFER);

  rtcSetBuffer(m_Scene, geomID, RTC_INDEX_BUFFER, indexPtr, 0,
               3 * sizeof(uint32_t));

  rtcSetUserData(m_Scene, geomID, obj);

  return true;
}

void EmbreeScene::CommitScene() { rtcCommit(m_Scene); }

bool EmbreeScene::Trace(const DirectX::SimpleMath::Ray &_ray,
                        Intersection &minIntersect) const {
  RTCRay ray;
  ray.org[0] = _ray.position.x;
  ray.org[1] = _ray.position.y;
  ray.org[2] = _ray.position.z;

  ray.dir[0] = _ray.direction.x;
  ray.dir[1] = _ray.direction.y;
  ray.dir[2] = _ray.direction.z;

  ray.tnear = 0.001f;
  ray.tfar = FLT_MAX;
  ray.geomID = RTC_INVALID_GEOMETRY_ID;
  ray.primID = RTC_INVALID_GEOMETRY_ID;
  ray.instID = RTC_INVALID_GEOMETRY_ID;
  ray.mask = 0xFFFFFFFF;
  ray.time = 0.f;

  rtcIntersect(m_Scene, ray);

  if (ray.geomID == RTC_INVALID_GEOMETRY_ID) {
    return false;
  }


  Vector3 rayDir(ray.dir);
  rayDir.Normalize();
  Vector3 rayPos(ray.org);

  minIntersect.position = rayPos + ray.tfar * rayDir;

  minIntersect.normal = Vector3(ray.Ng);
  minIntersect.normal.Normalize();

  minIntersect.hitObject = (RenderObject *)rtcGetUserData(m_Scene, ray.geomID);

  auto face = minIntersect.hitObject->GetIndexBuffer()[ray.primID];

  auto uvBuffer = minIntersect.hitObject->GetUVBuffer();

  if (uvBuffer) {
    auto uv0 = uvBuffer[face.m_Indices[0]];
    auto uv1 = uvBuffer[face.m_Indices[1]];
    auto uv2 = uvBuffer[face.m_Indices[2]];

    minIntersect.uv = (1.0f - ray.u - ray.v) * uv0 + ray.u * uv1 + ray.v * uv2;
  } else {
    minIntersect.uv = {0, 0};
  }
  
  minIntersect.material = minIntersect.hitObject->GetMaterial();
  return true;
}

EmbreeScene::~EmbreeScene() {
  rtcDeleteScene(m_Scene);
  rtcDeleteDevice(m_Device);
}
