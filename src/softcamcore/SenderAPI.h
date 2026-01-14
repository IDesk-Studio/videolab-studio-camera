#pragma once

#include "FrameBuffer.h"


namespace softcam {
namespace sender {

using CameraHandle = void*;

CameraHandle    CreateCamera(int width, int height, float framerate = 60.0f, PixelFormat format = PixelFormat::RGB24);
void            DeleteCamera(CameraHandle camera);
void            SendFrame(CameraHandle camera, const void* image_bits);
bool            WaitForConnection(CameraHandle camera, float timeout = 0.0f);
bool            IsConnected(CameraHandle camera);

} //namespace sender
} //namespace softcam
