/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/controls/control-devel.h>
#include <dali-toolkit/devel-api/visuals/visual-properties-devel.h>
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>

#include <dali-toolkit/devel-api/visuals/animated-image-visual-actions-devel.h>
#include <dali-toolkit/devel-api/visuals/animated-vector-image-visual-actions-devel.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
// Resource for drawing
const std::string NPATCH_FILENAME_1     = TEST_IMAGE_DIR "npatch-image/demo-tile-texture.9.png";
const std::string NPATCH_FILENAME_2     = TEST_IMAGE_DIR "npatch-image/heartsframe.9.png";
const std::string NPATCH_FILENAME_3     = TEST_IMAGE_DIR "npatch-image/blueheartsframe.9.png";
const std::string BORDER_IMAGE_FILENAME = TEST_IMAGE_DIR "npatch-image/cluster-image-frame.png";
const Rect<int> BORDER_IMAGE_RECT = Rect<int>(13, 13, 13, 13);

// Resource for Auxiliary
const std::string AUXILIARY_IMAGE_FILE = TEST_IMAGE_DIR "npatch-image/cluster-background.png";
constexpr float AUXILIARY_IMAGE_ALPHA = 0.9f;

// Resource for visual comparison
const std::string EXPECTED_IMAGE_FILE = TEST_IMAGE_DIR "npatch-image/expected-result.png";

/**
 * @brief Test area for each visuals
 * This visual test will create below 4 kinds of visual size per each testset.
 *
 * |---     TESTSET_VISUAL_SIZE    ---|
 * +---+-----+---+----------------+---+ -
 * |   |     |   |                |   | | MARGIN_VISUALS
 * | -   ---   -   --------------   - | -
 * |   | A   |   | B              |   | | SMALL_VISUAL_SIZE
 * |   |     |   |                |   | |
 * | -   ---   -   --------------   - | -
 * |   |     |   |                |   | | MARGIN_VISUALS
 * | -   ---   -   --------------   - | -
 * |   | C   |   | D              |   | |
 * |   |     |   |                |   | |
 * |   |     |   |                |   | | NORMAL_VISUAL_SIZE
 * |   |     |   |                |   | |
 * |   |     |   |                |   | |
 * |   |     |   |                |   | |
 * | -   ---   -   --------------   - | -
 * |   |     |   |                |   | | MARGIN_VISUALS
 * +---+-----+---+----------------+---+ -
 *
 * We will created 'NUMBER_OF_VALID_VISUAL_TYPES' rows.
 * Each row, we will make visual test set with 'NUMBER_OF_PROPERTY_TYPES' kind of properties
 *
 *        NUMBER_OF_PROPERTY_TYPES
 *   ---------------------------->
 * | +-----------+ +-----------+
 * | |ImageVisual| |ImageVisual|
 * | |with type 1| |with type 2|  ...
 * | +-----------+ +-----------+
 * | +-----------+ +-----------+
 * | |ColorVisual| |ColorVisual|
 * | |with type 1| |with type 2|  ...
 * | +-----------+ +-----------+
 * | ...           ...
 * V
 * NUMBER_OF_VALID_VISUAL_TYPES
 *
 */
constexpr static int NORMAL_VISUAL_SIZE = 150;
constexpr static int SMALL_VISUAL_SIZE  = 40;
constexpr static int MARGIN_VISUALS     = 5;

constexpr static int TESTSET_VISUAL_SIZE =  NORMAL_VISUAL_SIZE + SMALL_VISUAL_SIZE + MARGIN_VISUALS * 3;

constexpr static int NUMBER_OF_PROPERTY_TYPES = 4;
constexpr static int NUMBER_OF_VALID_VISUAL_TYPES = 4;

constexpr static int TOTAL_RESOURCES = NUMBER_OF_PROPERTY_TYPES * NUMBER_OF_VALID_VISUAL_TYPES * 4; // Total amount of resource to ready

enum TestStep
{
  CREATE_ASYNC_PREMULTILED_STEP,
  TEST_ASYNC_PREMULTILED_STEP,
  CREATE_SYNC_PREMULTILED_STEP,
  TEST_SYNC_PREMULTILED_STEP,
  CREATE_MIXED_PREMULTILED_STEP,
  TEST_MIXED_PREMULTILED_STEP,
  CREATE_ASYNC_NO_PREMULTILED_STEP,
  TEST_ASYNC_NO_PREMULTILED_STEP,
  CREATE_SYNC_NO_PREMULTILED_STEP,
  TEST_SYNC_NO_PREMULTILED_STEP,
  CREATE_MIXED_NO_PREMULTILED_STEP,
  TEST_MIXED_NO_PREMULTILED_STEP,
  NUMBER_OF_STEPS,
};

static int gTestStep = -1;
static int gResourceReadyCount = 0;
static bool gAnimationFinished = true;

}  // namespace

/**
 * @brief This is to test the functionality of native image and image visual
 */
class NPatchImageVisualTest: public VisualTest
{
public:

  NPatchImageVisualTest( Application& application )
    : mApplication( application )
  {
  }

  ~NPatchImageVisualTest()
  {
  }

  void OnInit( Application& application )
  {
    mWindow = mApplication.GetWindow();
    mWindow.SetBackgroundColor(Color::CRIMSON); // Due to the cluster-image-frame.png is white background, we make window not black/white color.

    // Start the test
    WaitForNextTest();
  }

private:

  void WaitForNextTest()
  {
    gTestStep++;

    mTimer = Timer::New(100); // ms
    mTimer.TickSignal().Connect(this, &NPatchImageVisualTest::OnTimer);
    mTimer.Start();
  }

  void OnReady(Dali::Toolkit::Control control)
  {
    // Resource ready done. Check we need to go to next step
    gResourceReadyCount++;
    if(gAnimationFinished && gResourceReadyCount == TOTAL_RESOURCES)
    {
      WaitForNextTest();
    }
  }

  bool OnTimer()
  {
    PerformTest();
    return false;
  }
  void OnFinishedAnimation(Animation& animation)
  {
    // Animation done. Check we need to go to next step
    gAnimationFinished = true;
    if(gAnimationFinished && gResourceReadyCount == TOTAL_RESOURCES)
    {
      WaitForNextTest();
    }
  }

  void PerformTest()
  {
    Window window = mApplication.GetWindow();

    switch(gTestStep)
    {
      case CREATE_ASYNC_PREMULTILED_STEP:
      case CREATE_SYNC_PREMULTILED_STEP:
      case CREATE_MIXED_PREMULTILED_STEP:
      case CREATE_ASYNC_NO_PREMULTILED_STEP:
      case CREATE_SYNC_NO_PREMULTILED_STEP:
      case CREATE_MIXED_NO_PREMULTILED_STEP:
      {
        CreateVisuals(
          gTestStep == CREATE_ASYNC_PREMULTILED_STEP || gTestStep == CREATE_MIXED_PREMULTILED_STEP || gTestStep == CREATE_ASYNC_NO_PREMULTILED_STEP || gTestStep == CREATE_MIXED_NO_PREMULTILED_STEP,
          gTestStep == CREATE_SYNC_PREMULTILED_STEP || gTestStep == CREATE_MIXED_PREMULTILED_STEP || gTestStep == CREATE_SYNC_NO_PREMULTILED_STEP || gTestStep == CREATE_MIXED_NO_PREMULTILED_STEP,
          gTestStep == CREATE_ASYNC_PREMULTILED_STEP || gTestStep == CREATE_SYNC_PREMULTILED_STEP || gTestStep == CREATE_MIXED_PREMULTILED_STEP);
        break;
      }
      case TEST_ASYNC_PREMULTILED_STEP:
      case TEST_SYNC_PREMULTILED_STEP:
      case TEST_MIXED_PREMULTILED_STEP:
      case TEST_ASYNC_NO_PREMULTILED_STEP:
      case TEST_SYNC_NO_PREMULTILED_STEP:
      case TEST_MIXED_NO_PREMULTILED_STEP:
      {
        CaptureWindow(window);
        break;
      }
      default:
        break;
    }
  }

  void PostRender()
  {
    switch(gTestStep)
    {
      case CREATE_ASYNC_PREMULTILED_STEP:
      case CREATE_SYNC_PREMULTILED_STEP:
      case CREATE_MIXED_PREMULTILED_STEP:
      case CREATE_ASYNC_NO_PREMULTILED_STEP:
      case CREATE_SYNC_NO_PREMULTILED_STEP:
      case CREATE_MIXED_NO_PREMULTILED_STEP:
      {
        // Just Create step. ignore
        break;
      }
      case TEST_ASYNC_PREMULTILED_STEP:
      case TEST_SYNC_PREMULTILED_STEP:
      case TEST_MIXED_PREMULTILED_STEP:
      case TEST_ASYNC_NO_PREMULTILED_STEP:
      case TEST_SYNC_NO_PREMULTILED_STEP:
      case TEST_MIXED_NO_PREMULTILED_STEP:
      {
        // All steps will have same result.
        if(!CheckImage(EXPECTED_IMAGE_FILE)) // should be identical
        {
          mTimer.Stop();
          mApplication.Quit();
        }
        else if(gTestStep + 1u == NUMBER_OF_STEPS)
        {
          // The last check has been done, so we can quit the test
          mApplication.Quit();
        }
        else
        {
          // Test done. Let's do next test!
          UnparentAllControls();
          WaitForNextTest();
        }
        break;
      }
      default:
        break;
    }
  }

private:
  void CreateVisuals(bool requiredAsyncLoading, bool requiredSyncLoading, bool requiredPreMulitpliedAlpha)
  {
    // Reset resource ready count
    gResourceReadyCount = 0;

    // Create Visuals for each testset types.
    for(std::uint32_t visualTestTypeIndex = 0; visualTestTypeIndex < NUMBER_OF_VALID_VISUAL_TYPES; ++visualTestTypeIndex)
    {
      for(std::uint32_t propertyTestTypeIndex = 0; propertyTestTypeIndex < NUMBER_OF_PROPERTY_TYPES; ++propertyTestTypeIndex)
      {
        CreateTestSet(visualTestTypeIndex, propertyTestTypeIndex, requiredAsyncLoading, requiredSyncLoading, requiredPreMulitpliedAlpha);
      }
    }
  }

  // Main setup here!
  // Costumize here if you want
  void CreateTestSet(std::uint32_t visualTestTypeIndex, std::uint32_t propertyTestTypeIndex, bool requiredAsyncLoading, bool requiredSyncLoading, bool requiredPreMulitpliedAlpha)
  {
    Vector2 topLeftPosition = Vector2(propertyTestTypeIndex * TESTSET_VISUAL_SIZE, visualTestTypeIndex * TESTSET_VISUAL_SIZE);

    // subSizeTestTypeIndex = 0, 1, 2, 3 --> A, B, C, D which explained at brief
    for(std::uint32_t subSizeTestTypeIndex = 0; subSizeTestTypeIndex < 4; ++subSizeTestTypeIndex)
    {
      // Calculate controls size and position from TOP_LEFT
      Vector2 controlSize     = Vector2(subSizeTestTypeIndex & 1 ? NORMAL_VISUAL_SIZE : SMALL_VISUAL_SIZE,
                                        subSizeTestTypeIndex & 2 ? NORMAL_VISUAL_SIZE : SMALL_VISUAL_SIZE);
      Vector2 controlPosition = topLeftPosition + 
                                Vector2(subSizeTestTypeIndex & 1 ? SMALL_VISUAL_SIZE + MARGIN_VISUALS * 2 : MARGIN_VISUALS,
                                        subSizeTestTypeIndex & 2 ? SMALL_VISUAL_SIZE + MARGIN_VISUALS * 2 : MARGIN_VISUALS);
      // Create new Control and setup default data
      ImageView control = ImageView::New();
      control[Actor::Property::PARENT_ORIGIN] = ParentOrigin::TOP_LEFT;
      control[Actor::Property::ANCHOR_POINT]  = AnchorPoint::TOP_LEFT;
      control[Actor::Property::SIZE]          = controlSize;
      control[Actor::Property::POSITION]      = controlPosition;

      control[ImageView::Property::PRE_MULTIPLIED_ALPHA] = requiredPreMulitpliedAlpha;


      // Attach resource ready signal
      control.ResourceReadySignal().Connect(this, &NPatchImageVisualTest::OnReady);

      // Basic informations of current visual expects
      Property::Map basicVisualMap = CreateBasicVisualMap(visualTestTypeIndex);

      // Specific information of current property testset.
      Property::Map testVisualMap = CreateTestVisualMap(propertyTestTypeIndex);

      bool synchronousLoading = false;
      if(requiredAsyncLoading ^ requiredSyncLoading)
      {
        synchronousLoading = requiredSyncLoading;
      }
      else
      {
        int id = (visualTestTypeIndex * NUMBER_OF_PROPERTY_TYPES + propertyTestTypeIndex) % 6;
        uint32_t requiedSubSize1, requiedSubSize2;
        switch(id)
        {
          case 0: requiedSubSize1 = 0; requiedSubSize2 = 1; break;
          case 1: requiedSubSize1 = 0; requiedSubSize2 = 2; break;
          case 2: requiedSubSize1 = 0; requiedSubSize2 = 3; break;
          case 3: requiedSubSize1 = 1; requiedSubSize2 = 2; break;
          case 4: requiedSubSize1 = 1; requiedSubSize2 = 3; break;
          case 5: requiedSubSize1 = 2; requiedSubSize2 = 3; break;
          default: requiedSubSize1 = 0; requiedSubSize2 = 1; break;
        }
        if(subSizeTestTypeIndex == requiedSubSize1 || subSizeTestTypeIndex == requiedSubSize2)
        {
          synchronousLoading = true;
        }
      }

      // Append one more informations / sync load or not.
      testVisualMap.Insert(ImageVisual::Property::SYNCHRONOUS_LOADING, synchronousLoading);

      {
        // Just merge property values into basicVisualMap
        basicVisualMap.Merge(testVisualMap);

        // Set background
        control[Control::Property::BACKGROUND] = basicVisualMap;
      }
      mWindow.Add(control);
      // For clean up
      mControlList.emplace_back(control);
    }
  }

  void UnparentAllControls()
  {
    for(auto&& actor : mControlList)
    {
      actor.Unparent();
      actor.Reset();
    }
    mControlList.clear();
  }

private:
  Property::Map CreateBasicVisualMap(std::uint32_t visualTestTypeIndex)
  {
    Property::Map visualMap;
    switch(visualTestTypeIndex)
    {
      case 0:
      {
        visualMap[Visual::Property::TYPE]     = Visual::IMAGE;
        visualMap[ImageVisual::Property::URL] = NPATCH_FILENAME_1;
        break;
      }
      case 1:
      {
        visualMap[Visual::Property::TYPE]     = Visual::IMAGE;
        visualMap[ImageVisual::Property::URL] = NPATCH_FILENAME_2;
        break;
      }
      case 2:
      {
        visualMap[Visual::Property::TYPE]     = Visual::IMAGE;
        visualMap[ImageVisual::Property::URL] = NPATCH_FILENAME_3;
        break;
      }
      case 3:
      {
        visualMap[Visual::Property::TYPE]        = Visual::N_PATCH;
        visualMap[ImageVisual::Property::URL]    = BORDER_IMAGE_FILENAME;
        visualMap[ImageVisual::Property::BORDER] = BORDER_IMAGE_RECT;
        break;
      }
      default:
        break;
    }
    return visualMap;
  }

  Property::Map CreateTestVisualMap(std::uint32_t propertyTestTypeIndex)
  {
    Property::Map visualMap;
    switch(propertyTestTypeIndex)
    {
      case 0:
      {
        // Nothing.
        break;
      }
      case 1:
      {
        visualMap[ImageVisual::Property::BORDER_ONLY] = true;
        break;
      }
      case 2:
      {
        visualMap[DevelImageVisual::Property::AUXILIARY_IMAGE] = AUXILIARY_IMAGE_FILE;
        visualMap[DevelImageVisual::Property::AUXILIARY_IMAGE_ALPHA] = AUXILIARY_IMAGE_ALPHA;
        break;
      }
      case 3:
      {
        visualMap[DevelImageVisual::Property::AUXILIARY_IMAGE] = AUXILIARY_IMAGE_FILE;
        visualMap[DevelImageVisual::Property::AUXILIARY_IMAGE_ALPHA] = AUXILIARY_IMAGE_ALPHA;
        visualMap[ImageVisual::Property::BORDER_ONLY] = true;
        break;
      }
      default:
        break;
    }
    return visualMap;
  }

private:
  Application&         mApplication;
  Window               mWindow;
  Timer                mTimer;
  Animation            mAnimation;
  std::vector<Control> mControlList;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( NPatchImageVisualTest, OnInit, TESTSET_VISUAL_SIZE * NUMBER_OF_PROPERTY_TYPES, TESTSET_VISUAL_SIZE * NUMBER_OF_VALID_VISUAL_TYPES )
