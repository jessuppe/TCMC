/*
 *  Copyright (C) 2007-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RendererDRMPRIMEGLES.h"

#include "ServiceBroker.h"
#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoPlayer/VideoRenderers/RenderCapture.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFlags.h"
#include "rendering/gles/RenderSystemGLES.h"
#include "utils/EGLFence.h"
#include "utils/GLUtils.h"
#include "utils/log.h"
#include "windowing/gbm/WinSystemGbmEGLContext.h"

using namespace KODI::WINDOWING::GBM;
using namespace KODI::UTILS::EGL;

CRendererDRMPRIMEGLES::~CRendererDRMPRIMEGLES()
{
  Flush(false);
}

CBaseRenderer* CRendererDRMPRIMEGLES::Create(CVideoBuffer* buffer)
{
  if (buffer && dynamic_cast<CVideoBufferDRMPRIME*>(buffer))
    return new CRendererDRMPRIMEGLES();

  return nullptr;
}

void CRendererDRMPRIMEGLES::Register()
{
  VIDEOPLAYER::CRendererFactory::RegisterRenderer("drm_prime_gles", CRendererDRMPRIMEGLES::Create);
}

bool CRendererDRMPRIMEGLES::Configure(const VideoPicture& picture,
                                      float fps,
                                      unsigned int orientation)
{
  CWinSystemGbmEGLContext* winSystem =
      dynamic_cast<CWinSystemGbmEGLContext*>(CServiceBroker::GetWinSystem());
  if (!winSystem)
    return false;

  m_format = picture.videoBuffer->GetFormat();
  m_sourceWidth = picture.iWidth;
  m_sourceHeight = picture.iHeight;
  m_renderOrientation = orientation;

  m_iFlags = GetFlagsChromaPosition(picture.chroma_position) |
             GetFlagsColorMatrix(picture.color_space, picture.iWidth, picture.iHeight) |
             GetFlagsColorPrimaries(picture.color_primaries) |
             GetFlagsStereoMode(picture.stereoMode);

  // Calculate the input frame aspect ratio.
  CalculateFrameAspectRatio(picture.iDisplayWidth, picture.iDisplayHeight);
  SetViewMode(m_videoSettings.m_ViewMode);
  ManageRenderArea();

  Flush(false);

  EGLDisplay eglDisplay = winSystem->GetEGLDisplay();
  for (auto&& buf : m_buffers)
  {
    if (!buf.fence)
    {
      buf.texture.Init(eglDisplay);
      buf.fence.reset(new CEGLFence(eglDisplay));
    }
  }

  m_clearColour = winSystem->UseLimitedColor() ? (16.0f / 0xff) : 0.0f;

  m_configured = true;
  return true;
}

void CRendererDRMPRIMEGLES::AddVideoPicture(const VideoPicture& picture, int index)
{
  BUFFER& buf = m_buffers[index];
  if (buf.videoBuffer)
  {
    CLog::LogF(LOGERROR, "unreleased video buffer");
    if (buf.fence)
      buf.fence->DestroyFence();
    buf.texture.Unmap();
    buf.videoBuffer->Release();
  }
  buf.videoBuffer = picture.videoBuffer;
  buf.videoBuffer->Acquire();
}

bool CRendererDRMPRIMEGLES::Flush(bool saveBuffers)
{
  if (!saveBuffers)
    for (int i = 0; i < NUM_BUFFERS; i++)
      ReleaseBuffer(i);

  return saveBuffers;
}

void CRendererDRMPRIMEGLES::ReleaseBuffer(int index)
{
  BUFFER& buf = m_buffers[index];

  if (buf.fence)
    buf.fence->DestroyFence();

  buf.texture.Unmap();

  if (buf.videoBuffer)
  {
    buf.videoBuffer->Release();
    buf.videoBuffer = nullptr;
  }
}

bool CRendererDRMPRIMEGLES::NeedBuffer(int index)
{
  return !m_buffers[index].fence->IsSignaled();
}

CRenderInfo CRendererDRMPRIMEGLES::GetRenderInfo()
{
  CRenderInfo info;
  info.max_buffer_size = NUM_BUFFERS;
  return info;
}

void CRendererDRMPRIMEGLES::Update()
{
  if (!m_configured)
    return;

  ManageRenderArea();
}

void CRendererDRMPRIMEGLES::RenderUpdate(
    int index, int index2, bool clear, unsigned int flags, unsigned int alpha)
{
  if (!m_configured)
    return;

  ManageRenderArea();

  if (clear)
  {
    glClearColor(m_clearColour, m_clearColour, m_clearColour, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
  }

  if (alpha < 255)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
  {
    glDisable(GL_BLEND);
  }

  Render(flags, index);

  VerifyGLState();
  glEnable(GL_BLEND);
}

bool CRendererDRMPRIMEGLES::RenderCapture(CRenderCapture* capture)
{
  capture->BeginRender();
  capture->EndRender();
  return true;
}

bool CRendererDRMPRIMEGLES::ConfigChanged(const VideoPicture& picture)
{
  if (picture.videoBuffer->GetFormat() != m_format)
    return true;

  return false;
}

void CRendererDRMPRIMEGLES::Render(unsigned int flags, int index)
{
  BUFFER& buf = m_buffers[index];

  CVideoBufferDRMPRIME* buffer = dynamic_cast<CVideoBufferDRMPRIME*>(buf.videoBuffer);
  if (!buffer || !buffer->IsValid())
    return;

  CRenderSystemGLES* renderSystem =
      dynamic_cast<CRenderSystemGLES*>(CServiceBroker::GetRenderSystem());
  if (!renderSystem)
    return;

  if (!buf.texture.Map(buffer))
    return;

  glBindTexture(GL_TEXTURE_EXTERNAL_OES, buf.texture.GetTexture());

  renderSystem->EnableGUIShader(SM_TEXTURE_RGBA_OES);

  GLubyte idx[4] = {0, 1, 3, 2}; // Determines order of triangle strip
  GLuint vertexVBO;
  GLuint indexVBO;
  struct PackedVertex
  {
    float x, y, z;
    float u1, v1;
  };

  std::array<PackedVertex, 4> vertex;

  GLint vertLoc = renderSystem->GUIShaderGetPos();
  GLint loc = renderSystem->GUIShaderGetCoord0();

  // top left
  vertex[0].x = m_rotatedDestCoords[0].x;
  vertex[0].y = m_rotatedDestCoords[0].y;
  vertex[0].z = 0.0f;
  vertex[0].u1 = 0.0f;
  vertex[0].v1 = 0.0f;

  // top right
  vertex[1].x = m_rotatedDestCoords[1].x;
  vertex[1].y = m_rotatedDestCoords[1].y;
  vertex[1].z = 0.0f;
  vertex[1].u1 = 1.0f;
  vertex[1].v1 = 0.0f;

  // bottom right
  vertex[2].x = m_rotatedDestCoords[2].x;
  vertex[2].y = m_rotatedDestCoords[2].y;
  vertex[2].z = 0.0f;
  vertex[2].u1 = 1.0f;
  vertex[2].v1 = 1.0f;

  // bottom left
  vertex[3].x = m_rotatedDestCoords[3].x;
  vertex[3].y = m_rotatedDestCoords[3].y;
  vertex[3].z = 0.0f;
  vertex[3].u1 = 0.0f;
  vertex[3].v1 = 1.0f;

  glGenBuffers(1, &vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex) * vertex.size(), vertex.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(vertLoc, 3, GL_FLOAT, 0, sizeof(PackedVertex),
                        reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, x)));
  glVertexAttribPointer(loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                        reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u1)));

  glEnableVertexAttribArray(vertLoc);
  glEnableVertexAttribArray(loc);

  glGenBuffers(1, &indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 4, idx, GL_STATIC_DRAW);

  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);

  glDisableVertexAttribArray(vertLoc);
  glDisableVertexAttribArray(loc);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &indexVBO);

  renderSystem->DisableGUIShader();

  glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

  buf.fence->CreateFence();
}

bool CRendererDRMPRIMEGLES::Supports(ERENDERFEATURE feature)
{
  switch (feature)
  {
    case RENDERFEATURE_STRETCH:
    case RENDERFEATURE_ZOOM:
    case RENDERFEATURE_VERTICAL_SHIFT:
    case RENDERFEATURE_PIXEL_RATIO:
    case RENDERFEATURE_ROTATION:
      return true;
    default:
      return false;
  }
}

bool CRendererDRMPRIMEGLES::Supports(ESCALINGMETHOD method)
{
  switch (method)
  {
    case VS_SCALINGMETHOD_LINEAR:
      return true;
    default:
      return false;
  }
}