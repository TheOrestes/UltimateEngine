#include "UltimateEnginePCH.h"
#include "FreeCamera.h"

//---------------------------------------------------------------------------------------------------------------------
FreeCamera::FreeCamera() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f }
{
    viewMatrix = XMMatrixIdentity();
}

//---------------------------------------------------------------------------------------------------------------------
FreeCamera::~FreeCamera() {}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::SetPosition(float x, float y, float z)
{
    position = XMFLOAT3(x, y, z);
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::SetRotation(float pitch, float yaw, float roll)
{
    rotation = XMFLOAT3(pitch, yaw, roll);
}

//---------------------------------------------------------------------------------------------------------------------
XMMATRIX FreeCamera::GetViewMatrix()
{
    UpdateViewMatrix(); // Recalculate view matrix before returning
    return viewMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
XMMATRIX FreeCamera::GetProjectionMatrix(float screenWidth, float screenHeight, float nearZ, float farZ) const
{
    float aspectRatio = screenWidth / screenHeight;
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), aspectRatio, nearZ, farZ);
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::UpdateViewMatrix() 
{
    XMVECTOR eyePosition = XMVectorSet(position.x, 1.0f, -25.0f, 0.0f);
    XMVECTOR focusPoint =  XMVectorSet( position.x + sinf(XMConvertToRadians(rotation.y)),
                                        position.y + sinf(XMConvertToRadians(rotation.x)),
                                        position.z + cosf(XMConvertToRadians(rotation.y)), 1.0f);
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::MoveForward(float distance)
{
    position.z += distance * cosf(XMConvertToRadians(rotation.y));
    position.x += distance * sinf(XMConvertToRadians(rotation.y));
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::MoveBackward(float distance)
{
    position.z -= distance * cosf(XMConvertToRadians(rotation.y));
    position.x -= distance * sinf(XMConvertToRadians(rotation.y));
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::MoveRight(float distance)
{
    position.x += distance * cosf(XMConvertToRadians(rotation.y));
    position.z -= distance * sinf(XMConvertToRadians(rotation.y));
}

//---------------------------------------------------------------------------------------------------------------------
void FreeCamera::MoveLeft(float distance)
{
    position.x -= distance * cosf(XMConvertToRadians(rotation.y));
    position.z += distance * sinf(XMConvertToRadians(rotation.y));
}