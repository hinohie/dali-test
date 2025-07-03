/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// INTERNAL INCLUDES
#include "visual-test.h"
#include "image-util.h"
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <Magick++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <filesystem>

using namespace Dali;
namespace fs = std::filesystem;

char*       gTempDir;
char*       gTempFilename;
const char* gVirtualFramebuffer = "/var/tmp/Xvfb_screen0";
bool        gFB                 = false;
int         gExitValue          = 1;
int         gImageNumber        = 1;

bool ParseEnvironment(int argc, char** argv, int WindowWidth, int WindowHeight)
{
  int c = 1;
  while(c < argc)
  {
    if(!strcmp(argv[c], "--fb"))
    {
      gFB = true;
      ++c;
    }
    else if(!strcmp(argv[c], "--get-dimensions"))
    {
      printf("%dx%dx24\n", WindowWidth, WindowHeight);
      return false;
    }
    else if(!strcmp(argv[c], "--directory"))
    {
      if(c + 1 < argc)
      {
        gTempDir = strdup(argv[c+1]);
      }
      c += 2;
    }
    else
    {
      ++c; //ignore unknown args
    }
  }
  return true;
}

/**
 * @brief Constructor.
 */
VisualTest::VisualTest()
: mWindow()
{
}

void VisualTest::SetupNativeImage(Dali::Window window, Dali::CameraActor customCamera)
{
  window.ResizeSignal().Connect(this, &VisualTest::OnWindowResized);

  if(!mOffscreenRenderTask || window.GetRootLayer() != mWindow.GetHandle())
  {
    Layer rootLayer = window.GetRootLayer();
    mWindow         = rootLayer;

    mNativeImageSourcePtr = NativeImageSource::New(window.GetSize().GetWidth(), window.GetSize().GetHeight(), NativeImageSource::COLOR_DEPTH_DEFAULT);
    mNativeTexture        = Texture::New(*mNativeImageSourcePtr);

    mFrameBuffer = FrameBuffer::New(mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::STENCIL);
    mFrameBuffer.AttachColorTexture(mNativeTexture);

    RenderTaskList taskList = window.GetRenderTaskList();
    if(mOffscreenRenderTask)
    {
      taskList.RemoveTask(mOffscreenRenderTask);
    }
    mOffscreenRenderTask = taskList.CreateTask();
    mOffscreenRenderTask.SetSourceActor(window.GetRootLayer());
    mOffscreenRenderTask.SetClearColor(window.GetBackgroundColor());
    mOffscreenRenderTask.SetClearEnabled(true);
    mOffscreenRenderTask.SetFrameBuffer(mFrameBuffer);

    if(customCamera)
    {
      mOffscreenRenderTask.SetCameraActor(customCamera);
    }
    else
    {
      mCameraActor = CameraActor::New(Vector2(window.GetSize().GetWidth(), window.GetSize().GetHeight()));
      mCameraActor.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
      mCameraActor.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
      mCameraActor.SetInvertYAxis(true);
      window.Add(mCameraActor);

      mOffscreenRenderTask.SetCameraActor(mCameraActor);
    }
  }

  mOffscreenRenderTask.SetRefreshRate(RenderTask::REFRESH_ONCE);
}

void VisualTest::OnWindowResized(Dali::Window window, Dali::Window::WindowSize size)
{
  window.ResizeSignal().Disconnect(this, &VisualTest::OnWindowResized);

  // When the window is resized, recreate the offscreen render task
  RenderTaskList taskList = window.GetRenderTaskList();
  if(mOffscreenRenderTask)
  {
    taskList.RemoveTask(mOffscreenRenderTask);
    mOffscreenRenderTask.Reset();
  }
}
void VisualTest::CaptureWindowAfterFrameRendered(Dali::Window window, Dali::CameraActor customCamera)
{
  Debug::LogMessage(Debug::INFO, "Starting draw and check()\n");

  mCaptureRequestedWindow = window;
  mCaptureRequestedCamera = customCamera;

  Dali::Animation firstFrameAnimator = Dali::Animation::New(0);
  firstFrameAnimator.FinishedSignal().Connect(this, &VisualTest::OnAnimationFinished1);
  firstFrameAnimator.Play();
  firstFrameAnimator.Stop();
}

void VisualTest::OnAnimationFinished1(Dali::Animation& /* not used */)
{
  Debug::LogMessage(Debug::INFO, "First Update done()\n");
  Dali::Animation secondFrameAnimator = Dali::Animation::New(0);
  secondFrameAnimator.FinishedSignal().Connect(this, &VisualTest::OnAnimationFinished2);
  secondFrameAnimator.Play();
  secondFrameAnimator.Stop();
}

void VisualTest::OnAnimationFinished2(Dali::Animation& /* not used */)
{
  Debug::LogMessage(Debug::INFO, "Second Update done(). We can assume that at least 1 frame rendered now.\n");
  Debug::LogMessage(Debug::INFO, "But GPU might not be rendered to buffer well. Render 2 more frames.\n");
  Dali::Animation thirdFrameAnimator = Dali::Animation::New(0);
  thirdFrameAnimator.FinishedSignal().Connect(this, &VisualTest::OnAnimationFinished3);
  thirdFrameAnimator.Play();
  thirdFrameAnimator.Stop();
}

void VisualTest::OnAnimationFinished3(Dali::Animation& /* not used */)
{
  Debug::LogMessage(Debug::INFO, "Third Update done()\n");
  Dali::Animation fourthFrameAnimator = Dali::Animation::New(0);
  fourthFrameAnimator.FinishedSignal().Connect(this, &VisualTest::OnAnimationFinished4);
  fourthFrameAnimator.Play();
  fourthFrameAnimator.Stop();
}

void VisualTest::OnAnimationFinished4(Dali::Animation& /* not used */)
{
  Debug::LogMessage(Debug::INFO, "Fourth Update done(). We can assume that at least 1 frame rendered to surface now. Capturing window\n");

  auto window = mCaptureRequestedWindow;
  auto customCamera = mCaptureRequestedCamera;
  mCaptureRequestedWindow.Reset();
  mCaptureRequestedCamera.Reset();

  CaptureWindow(window, customCamera);
}

void VisualTest::CaptureWindow(Dali::Window window, Dali::CameraActor customCamera)
{
  if(gFB)
  {
    RenderTask renderTask = window.GetRenderTaskList().GetTask(0);
    OnOffscreenRenderFinished(renderTask);
  }
  else
  {
    SetupNativeImage(window, customCamera);
    mOffscreenRenderTask.FinishedSignal().Connect(this, &VisualTest::OnOffscreenRenderFinished);
  }
}

void VisualTest::OnOffscreenRenderFinished(RenderTask& task)
{
  Debug::LogMessage(Debug::INFO, "VisualTest::OnOffscreenRenderFinished(), capturing offscreen\n");

  // Ensure there's a directory to write to:
  if(!fs::exists(fs::path(gTempDir)))
  {
    fs::create_directory(fs::path(gTempDir));
  }

  char* imageName;
  int n = asprintf(&imageName, "%s%02d.png", gTempFilename, gImageNumber);
  bool success=false;
  if(n > 0)
  {
    gImageNumber++;

    if(gFB)
    {
      Magick::Image image;
      image.magick("xwd");
      image.read(gVirtualFramebuffer);
      image.magick("png");
      image.write(imageName);
      success = true;
    }
    else
    {
      success = mNativeImageSourcePtr->EncodeToFile(imageName);
    }
  }

  if(!gFB)
  {
    task.SetRefreshRate(RenderTask::REFRESH_ALWAYS);
    task.FinishedSignal().Disconnect(this, &VisualTest::OnOffscreenRenderFinished);
  }
  PostRender(imageName, success);
  free(imageName);
}

bool VisualTest::CompareImageFile(const std::string fileName1, const std::string fileName2, const float similarityThreshold, const Rect<uint16_t>& areaToCompare)
{
  cv::Scalar similarity;

  // Load the images
  cv::Mat matrixImg1 = cv::imread(fileName1);
  cv::Mat matrixImg2 = cv::imread(fileName2);

  if(areaToCompare != Rect<uint16_t>(0u, 0u, 0u, 0u))
  {
    // Crop the images
    cv::Rect roi;
    roi.x      = areaToCompare.x;
    roi.y      = areaToCompare.y;
    roi.width  = areaToCompare.width;
    roi.height = areaToCompare.height;

    cv::Mat croppedMatrixImg1 = matrixImg1(roi);
    cv::Mat croppedMatrixImg2 = matrixImg2(roi);

    similarity = ImageUtil::CalculateSSIM(croppedMatrixImg1, croppedMatrixImg2);
  }
  else
  {
    similarity = ImageUtil::CalculateSSIM(matrixImg1, matrixImg2);
  }

  // Check whether SSIM for all the three channels (RGB) are above the threshold
  bool passed = (similarity.val[0] >= similarityThreshold && similarity.val[1] >= similarityThreshold && similarity.val[2] >= similarityThreshold);

  printf(
    "Test similarity: R:%f G:%f B:%f\n"
    "Passed threshold of %f: %s\n",
    100.0f * similarity.val[0],
    100.0f * similarity.val[1],
    100.0f * similarity.val[2],
    100.0f * similarityThreshold,
    passed ? "TRUE" : "FALSE");

  gExitValue = 33.3f * (similarity.val[0] + similarity.val[1] + similarity.val[2]);
  if(passed) gExitValue = 0;

  return passed;
}

void VisualTest::EmitTouch( TouchPoint& touchPoint )
{
  touchPoint.state =Dali::PointState::DOWN;
  Adaptor::Get().FeedTouchPoint(touchPoint, 0);
  touchPoint.state =Dali::PointState::UP;
  Adaptor::Get().FeedTouchPoint(touchPoint, 1);
}
