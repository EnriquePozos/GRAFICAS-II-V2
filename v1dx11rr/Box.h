#pragma once
#include <vector>

#include "Billboard.h"

class Box
{
private:
  D3DXVECTOR3 position;
  D3DXVECTOR3 size;
  D3DXVECTOR3 rotation;
public:
  Box() : position(0.0f, 0.0f, 0.0f), size(1.0f, 1.0f, 1.0f), rotation(0.0f, 0.0f, 0.0f) {}
    
  Box(const D3DXVECTOR3& pos, const D3DXVECTOR3& boxSize) 
      : position(pos), size(boxSize), rotation(0.0f, 0.0f, 0.0f) {}
    
  Box(const D3DXVECTOR3& pos, const D3DXVECTOR3& boxSize, const D3DXVECTOR3& rot) 
      : position(pos), size(boxSize), rotation(rot) {}

  Box(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, bool fromPoints) {
    D3DXVECTOR3 minP(
      min(p1.x, p2.x),
      min(p1.y, p2.y),
      min(p1.z, p2.z)
    );
    D3DXVECTOR3 maxP(
      max(p1.x, p2.x),
      max(p1.y, p2.y),
      max(p1.z, p2.z)
    );
    position = (minP + maxP) * 0.5f;
    size = maxP - minP;

        
    rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  }

  static Box FromPoints(const D3DXVECTOR3& point1, const D3DXVECTOR3& point2) {
    return Box(point1, point2, true);
  }
    
  static Box FromCoords(float x1, float y1, float z1, float x2, float y2, float z2) {
    return FromPoints(D3DXVECTOR3(x1, y1, z1), D3DXVECTOR3(x2, y2, z2));
  }

  D3DXVECTOR3 GetPosition() const { return position; }

  D3DXVECTOR3 GetSize() const { return size; }
  
  D3DXVECTOR3 GetExtents() const {
    return D3DXVECTOR3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f);
  }
    
  D3DXVECTOR3 GetMin() const {
    D3DXVECTOR3 extents = GetExtents();
    return D3DXVECTOR3(position.x - extents.x, position.y - extents.y, position.z - extents.z);
  }
    
  D3DXVECTOR3 GetMax() const {
    D3DXVECTOR3 extents = GetExtents();
    return D3DXVECTOR3(position.x + extents.x, position.y + extents.y, position.z + extents.z);
  }


  
  void SetPosition(const D3DXVECTOR3& pos) { position = pos; }
  void SetSize(const D3DXVECTOR3& newSize) { size = newSize; }
  void SetRotation(const D3DXVECTOR3& rot) { rotation = rot; }

  void Translate(const D3DXVECTOR3& offset) {
    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;
  }

  void Scale(float factor) {
    size.x *= factor;
    size.y *= factor;
    size.z *= factor;
  }

  void Scale(const D3DXVECTOR3& factors) {
    size.x *= factors.x;
    size.y *= factors.y;
    size.z *= factors.z;
  }

  float GetVolume() const {
    return size.x * size.y * size.z;
  }
  
  float GetSurfaceArea() const {
    return 2.0f * (size.x * size.y + size.y * size.z + size.z * size.x);
  }

  bool ContainsPoint(const D3DXVECTOR3& point) const {
    D3DXVECTOR3 min = GetMin();
    D3DXVECTOR3 max = GetMax();
        
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
  }

  std::vector<D3DXVECTOR3> GetVertices() const {
    D3DXVECTOR3 min = GetMin();
    D3DXVECTOR3 max = GetMax();
        
    return {
      D3DXVECTOR3(min.x, min.y, min.z),
      D3DXVECTOR3(max.x, min.y, min.z),
      D3DXVECTOR3(max.x, max.y, min.z),
      D3DXVECTOR3(min.x, max.y, min.z),
      D3DXVECTOR3(min.x, min.y, max.z),
      D3DXVECTOR3(max.x, min.y, max.z),
      D3DXVECTOR3(max.x, max.y, max.z),
      D3DXVECTOR3(min.x, max.y, max.z)
    };
  }
  
  D3DXVECTOR3 GetFaceCenter(int face) const {
    D3DXVECTOR3 extents = GetExtents();
    switch(face) {
    case 0: return D3DXVECTOR3(position.x - extents.x, position.y, position.z); // Left
    case 1: return D3DXVECTOR3(position.x + extents.x, position.y, position.z); // Right
    case 2: return D3DXVECTOR3(position.x, position.y - extents.y, position.z); // Bottom
    case 3: return D3DXVECTOR3(position.x, position.y + extents.y, position.z); // Top
    case 4: return D3DXVECTOR3(position.x, position.y, position.z - extents.z); // Back
    case 5: return D3DXVECTOR3(position.x, position.y, position.z + extents.z); // Front
    default: return position;
    }
  }
  
};


struct Hitbox
{
  Box box;
  bool active;

  Hitbox(const Box& boxShape) : box(boxShape), active(true) {}

  Hitbox(const D3DXVECTOR3& pos, const D3DXVECTOR3& size) 
        : box(pos, size), active(true) {}

  D3DXVECTOR3 GetMin() const { return box.GetMin(); }
  D3DXVECTOR3 GetMax() const { return box.GetMax(); }
  D3DXVECTOR3 GetPosition() const { return box.GetPosition(); }
  D3DXVECTOR3 GetSize() const { return box.GetSize(); }
  D3DXVECTOR3 GetExtents() const { return box.GetExtents(); }
    
  void SetPosition(const D3DXVECTOR3& pos) { box.SetPosition(pos); }
  void SetSize(const D3DXVECTOR3& size) { box.SetSize(size); }
  void Translate(const D3DXVECTOR3& offset) { box.Translate(offset); }
    
  bool ContainsPoint(const D3DXVECTOR3& point) const { 
    return box.ContainsPoint(point); 
  }
  
};



