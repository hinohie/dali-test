#ifndef VISUAL_TEST_UTILS_H
#define VISUAL_TEST_UTILS_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <string>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

#define DALI_VISUAL_TEST( VisualTestName, InitFunction ) \
  int DALI_EXPORT_API main( int argc, char **argv ) \
  { \
    Application application = Application::New( &argc, &argv, "" ); \
    VisualTestName test( application ); \
    application.InitSignal().Connect( &test, &VisualTestName::InitFunction ); \
    application.MainLoop(); \
    return 0; \
  }

/**
 * @brief This class provides the functionality of visual testing by capturing the content
 *        rendered by GPU in the given window and compare it with a given image.
 */
class VisualTestUtils : public Dali::ConnectionTracker
{
public:

  /**
   * @brief Constructor.
   */
  VisualTestUtils();

protected:

  /**
   * @brief Default Destructor.
   */
  virtual ~VisualTestUtils() = default;

  /**
   * @brief Capture the content of the given window rendered by GPU
   * @param[in] window The window to be captured
   */
  void CaptureWindow( Dali::Window window );

  /**
   * @brief Compare the result of the window capture with the given image file
   * @param[in] fileName The image file to be compared with
   * @param[in] areaToCompare The area in the image to be compared (The default value is an empty rectangle which means the whole area of the image will be compared)
   */
  bool CheckImage( const std::string fileName, const Dali::Rect<uint16_t>& areaToCompare = Dali::Rect<uint16_t>(0u, 0u, 0u, 0u) );

private:

  /**
   * @brief This virtual function will be called after the offscreen window frame buffer has been rendered.
   * @note  The visual test case must implement this function to check the result of the offscreen frame buffer.
   */
  virtual void PostRender() = 0;

  /**
   * @brief Set up the native image for offscreen rendering.
   * @param[in] window The window to be rendered
   */
  void SetupNativeImage( Dali::Window window );

  /**
   * @brief Callback function when a RenderTask has finished
   * @param[in] task The render task
   */
  void OnOffscreenRenderFinished( Dali::RenderTask& task );

  /**
   * @brief Compare the given area in the two image files.
   * @param[in] fileName1 The first image file
   * @param[in] fileName2 The second image file
   * @param[in] areaToCompare The area to be compared
   * @return Whether the given area in the images of the two image files are identical
   */
  bool CompareImageFile( const std::string fileName1, const std::string fileName2, const Dali::Rect<uint16_t>& areaToCompare );

  /**
   * @brief Compare the image file with the given image buffer.
   * @param[in] fileName The image file
   * @param[in] imageBuffer The image buffer
   * @param[in] imageWidth The width of the image
   * @param[in] imageHeight The height of the image
   * @return Whether the image buffer and the image in the image file are identical
   */
  bool CompareImageFile( const std::string fileName, std::vector< unsigned char > imageBuffer, unsigned int imageWidth, unsigned int imageHeight );

private:

  Dali::NativeImageSourcePtr mNativeImageSourcePtr;           ///< The pointer of the native image source
  Dali::Texture              mNativeTexture;                  ///< The texture for the native image
  Dali::FrameBuffer          mFrameBuffer;                    ///< The frame buffer for offscreen rendering

  Dali::RenderTask mOffscreenRenderTask;                      ///< The offscreen render task
  Dali::CameraActor mCameraActor;                             ///< The camera actor for the offscreen render task
  Dali::WeakHandle< Dali::Layer > mWindow;                    ///< The weak handle of the window to be rendered
};

#endif // VISUAL_TEST_UTILS_H
