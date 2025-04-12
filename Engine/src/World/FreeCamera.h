#pragma once

#include "D3D12Renderer/D3DGlobals.h"
#include <DirectXMath.h>

class FreeCamera
{
public:
    FreeCamera();
    ~FreeCamera();

    void SetPosition(float x, float y, float z);
    void SetRotation(float pitch, float yaw, float roll);

    XMMATRIX GetViewMatrix();
    XMMATRIX GetProjectionMatrix(float screenWidth, float screenHeight, float nearZ, float farZ) const;

    void MoveForward(float distance);
    void MoveBackward(float distance);
    void MoveRight(float distance);
    void MoveLeft(float distance);

private:
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    XMMATRIX viewMatrix;

    void UpdateViewMatrix();
};