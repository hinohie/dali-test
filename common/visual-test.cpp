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

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;

namespace
{

const std::string TEMP_FILENAME = "/tmp/temp.png";

}  // namespace

/**
 * @brief Constructor.
 */
VisualTest::VisualTest()
: mNativeImageSourcePtr( nullptr ),
  mWindow()
{
}

void VisualTest::SetupNativeImage( Dali::Window window )
{
  if( !mOffscreenRenderTask || window.GetRootLayer() != mWindow.GetHandle() )
  {
    Layer rootLayer = window.GetRootLayer();
    mWindow = rootLayer;

    mNativeImageSourcePtr = NativeImageSource::New( window.GetSize().GetWidth(), window.GetSize().GetHeight(), NativeImageSource::COLOR_DEPTH_DEFAULT );
    mNativeTexture = Texture::New( *mNativeImageSourcePtr );

    mFrameBuffer = FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::NONE );
    mFrameBuffer.AttachColorTexture( mNativeTexture );

    mCameraActor = CameraActor::New( Vector2(window.GetSize().GetWidth(), window.GetSize().GetHeight()) );
    mCameraActor.SetParentOrigin( ParentOrigin::CENTER );
    mCameraActor.SetParentOrigin( AnchorPoint::CENTER );
    window.Add( mCameraActor );

    RenderTaskList taskList = DevelWindow::GetRenderTaskList( window );
    if ( mOffscreenRenderTask )
    {
      taskList.RemoveTask( mOffscreenRenderTask );
    }
    mOffscreenRenderTask = taskList.CreateTask();
    mOffscreenRenderTask.SetSourceActor( window.GetRootLayer() );
    mOffscreenRenderTask.SetClearColor( Color::WHITE );
    mOffscreenRenderTask.SetClearEnabled( true );
    mOffscreenRenderTask.SetCameraActor( mCameraActor );
    mOffscreenRenderTask.GetCameraActor().SetInvertYAxis( false );
    mOffscreenRenderTask.SetFrameBuffer( mFrameBuffer );
  }

  mOffscreenRenderTask.SetRefreshRate( RenderTask::REFRESH_ONCE );
}

void VisualTest::CaptureWindow( Dali::Window window )
{
  SetupNativeImage( window );

  mOffscreenRenderTask.FinishedSignal().Connect( this, &VisualTest::OnOffscreenRenderFinished );
}

void VisualTest::OnOffscreenRenderFinished( RenderTask& task )
{
  task.FinishedSignal().Disconnect( this, &VisualTest::OnOffscreenRenderFinished );

  PostRender();
}

bool VisualTest::CheckImage( const std::string fileName, const Rect<uint16_t>& areaToCompare )
{
  bool success = false;

//  mNativeImageSourcePtr->EncodeToFile( TEMP_FILENAME );

  if ( areaToCompare == Rect<uint16_t>(0u, 0u, 0u, 0u) )
  {
    std::vector< unsigned char > pixbuf;
    unsigned int width(0), height(0);
    Pixel::Format pixelFormat;

    // Compare the image in the whole area
    if ( mNativeImageSourcePtr->GetPixels( pixbuf, width, height, pixelFormat ) )
    {
      if ( CompareImageFile( fileName, pixbuf, width, height ) )
      {
        success = true;
      }
    }
  }
  else
  {
    // Compare the image in the given area
    if ( mNativeImageSourcePtr->EncodeToFile( TEMP_FILENAME ) )
    {
      if ( CompareImageFile( fileName, TEMP_FILENAME, areaToCompare ) )
      {
        success = true;
      }
    }
  }

  return success;
}

bool VisualTest::CompareImageFile( const std::string fileName1, const std::string fileName2, const Rect<uint16_t>& areaToCompare )
{
  Devel::PixelBuffer pixelBuffer1 = Dali::LoadImageFromFile( fileName1 );
  Devel::PixelBuffer pixelBuffer2 = Dali::LoadImageFromFile( fileName2 );

  if ( pixelBuffer1.GetWidth() != pixelBuffer2.GetWidth() || pixelBuffer1.GetHeight() != pixelBuffer2.GetHeight() )
  {
    return false;
  }

  pixelBuffer1.Crop( areaToCompare.x, areaToCompare.y, areaToCompare.width,areaToCompare.height );
  pixelBuffer2.Crop( areaToCompare.x, areaToCompare.y, areaToCompare.width,areaToCompare.height );

  // Check the bytes per pixel.
  const Pixel::Format pixelFormat = pixelBuffer1.GetPixelFormat();
  const unsigned int pixelSize = Pixel::GetBytesPerPixel( pixelFormat );
  const unsigned int bufferSize = pixelBuffer1.GetWidth() * pixelBuffer1.GetHeight() * pixelSize;

  // Compare the two buffers.
  unsigned char* pBitmapData1 = pixelBuffer1.GetBuffer();
  unsigned char* pBitmapData2 = pixelBuffer2.GetBuffer();
  for ( unsigned int i = 0; i < bufferSize; ++i, ++pBitmapData1, ++pBitmapData2 )
  {
    if ( *pBitmapData1 != *pBitmapData2 )
    {
      return false;
    }
  }

  return true;
}

bool VisualTest::CompareImageFile( const std::string fileName, std::vector< unsigned char > imageBuffer, unsigned int imageWidth, unsigned int imageHeight )
{
  Devel::PixelBuffer pixelBuffer = Dali::LoadImageFromFile( fileName );

  if ( pixelBuffer.GetWidth() != imageWidth || pixelBuffer.GetHeight() != imageHeight )
  {
    return false;
  }

  // Check the bytes per pixel.
  const Pixel::Format pixelFormat = pixelBuffer.GetPixelFormat();
  const unsigned int pixelSize = Pixel::GetBytesPerPixel( pixelFormat );
  const unsigned int bufferSize = pixelBuffer.GetWidth() * pixelBuffer.GetHeight() * pixelSize;

  // Compare the two buffers.
  unsigned char* pBitmapData = pixelBuffer.GetBuffer();
  for ( unsigned int i = 0; i < bufferSize; ++i, ++pBitmapData )
  {
    if ( *pBitmapData != imageBuffer[i] )
    {
      return false;
    }
  }

  return true;
}

