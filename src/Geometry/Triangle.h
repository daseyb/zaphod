#pragma once
#include "..\SimpleMath.h"

struct Triangle {
  size_t m_Indices[3];

  Triangle() {
    m_Indices[0] = 0;
    m_Indices[1] = 0;
    m_Indices[2] = 0;
  }

  Triangle(size_t i0, size_t i1, size_t i2) {
    m_Indices[0] = i0;
    m_Indices[1] = i1;
    m_Indices[2] = i2;
  }
};
