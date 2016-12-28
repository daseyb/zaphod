#pragma once
#include "../../SimpleMath.h"
#include <random>
#include <memory>
#include <exception>
#include "../Cameras/Camera.h"

class Scene;

enum class OutputFormat {
  PNG,
  HDR,
  PFM
};

struct OutputImage {
  std::shared_ptr<DirectX::SimpleMath::Color> Data;
  std::string name;
  OutputFormat format;
};

class Integrator {
protected:
  Scene *m_Scene;
  Camera *m_Camera;
  int m_Width, m_Height;

  std::vector<OutputImage> m_Outputs;

  std::shared_ptr<DirectX::SimpleMath::Color> AddOutput(std::string name, OutputFormat format, int w, int h) {
    OutputImage img = { 
      std::shared_ptr<DirectX::SimpleMath::Color>(new DirectX::SimpleMath::Color[w * h]), 
      name,  
      format 
    };

    m_Outputs.push_back(img);
    std::fill(img.Data.get(), img.Data.get() + w * h, DirectX::SimpleMath::Color(0, 0, 0, 0));

    return img.Data;
  }

public:
  Integrator(Scene *scene, Camera *camera, int w, int h) : m_Scene(scene), m_Camera(camera), m_Width(w), m_Height(h) {}
  virtual DirectX::SimpleMath::Color
  Intersect(const DirectX::SimpleMath::Ray &_ray, int _depth, bool _isSecondary,
            std::default_random_engine &_rnd) const = 0;

  virtual DirectX::SimpleMath::Color Sample(float x, float y, int w, int h, std::default_random_engine& _rnd) const {
      float weight;
      DirectX::SimpleMath::Ray  ray = m_Camera->GetRay(x, y, w, h, _rnd, weight);

      if (weight > FLT_EPSILON) {
          return Intersect(ray, 8, false, _rnd);
      }
      return{ 0,0,0 };
  }

  virtual void Finalize(int totalSamples) const {
    float factor = 1.0 / totalSamples;
    for (auto& output : m_Outputs) {
      for (int i = 0; i < m_Width * m_Height; i++) {
        *(output.Data.get() + i) *= factor;
      }
    }
  }

  void Reset() {
    for (auto& output : m_Outputs) {
      std::fill(output.Data.get(), output.Data.get() + m_Width * m_Height, DirectX::SimpleMath::Color(0, 0, 0, 0));
    }
  }

  std::vector<OutputImage> getOutputs() { return m_Outputs; }
};
