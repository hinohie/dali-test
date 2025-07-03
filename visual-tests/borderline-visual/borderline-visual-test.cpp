/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/controls/control-devel.h>
#include <dali-toolkit/devel-api/visuals/visual-properties-devel.h>
#include <dali-toolkit/devel-api/visuals/image-visual-properties-devel.h>

#include <dali-toolkit/devel-api/visuals/animated-image-visual-actions-devel.h>

// INTERNAL INCLUDES
#include "visual-test.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
// Resource for drawing
const std::string JPG_FILENAME             = TEST_IMAGE_DIR "corner-radius-visual/gallery-medium-16.jpg";
const std::string SVG_FILENAME             = TEST_IMAGE_DIR "corner-radius-visual/Contacts.svg";
const std::string ANIMATED_WEBP_FILENAME   = TEST_IMAGE_DIR "corner-radius-visual/dog-anim.webp";

// Resource for visual comparison
const std::string EXPECTED_IMAGE_FILE = TEST_IMAGE_DIR "borderline-visual/expected-result.png";

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
constexpr static int NORMAL_VISUAL_SIZE = 100;
constexpr static int SMALL_VISUAL_SIZE  = 20;
constexpr static int MARGIN_VISUALS     = 11;

constexpr static int TESTSET_VISUAL_SIZE =  NORMAL_VISUAL_SIZE + SMALL_VISUAL_SIZE + MARGIN_VISUALS * 3;

// Want to test propeties about borderline offset list
const static Property::Value BORDERLINE_OFFSET_LIST[] =
{
  Property::Value(-1.0f),
  Property::Value(1.0f),
  Property::Value(0.0f),
};
const static Property::Value BORDERLINE_COLOR_LIST[] =
{
  Property::Value(Vector4(0.7f, 0.7f, 1.0f, 0.5f)),
  Property::Value(Vector4(0.0f, 1.0f, 0.5f, 1.0f)),
  Property::Value(Vector4(1.0f, 0.0f, 0.0f, 0.5f)),
};
const static Property::Value BORDERLINE_WIDTH_LIST[] =
{
  Property::Value(5.0f),
  Property::Value(5.0f),
  Property::Value(10.0f),
};
constexpr static Vector4 CORNER_RADIUS_RATES = Vector4(0.5f, 0.0f, 0.33f, 0.17f);
constexpr static int NUMBER_OF_PROPERTY_TYPES = sizeof(BORDERLINE_OFFSET_LIST) / sizeof(BORDERLINE_OFFSET_LIST[0]) * 2;

// Valid visual type list
constexpr static Visual::Type VALID_VISUAL_TYPES[] =
{
  Visual::Type::IMAGE,
  Visual::Type::COLOR,
  Visual::Type::GRADIENT,
  Visual::Type::SVG,
  Visual::Type::ANIMATED_IMAGE,
};
constexpr static int NUMBER_OF_VALID_VISUAL_TYPES = sizeof(VALID_VISUAL_TYPES) / sizeof(VALID_VISUAL_TYPES[0]);

constexpr static int TOTAL_RESOURCES = NUMBER_OF_PROPERTY_TYPES * NUMBER_OF_VALID_VISUAL_TYPES * 4; // Total amount of resource to ready

enum TestStep
{
  CREATE_STATIC_PREMULTIPLIED_STEP,
  CREATE_STATIC_NO_PREMULTIPLIED_STEP,
  CREATE_ANIMATE_PREMULTIPLIED_STEP,
  CREATE_ANIMATE_NO_PREMULTIPLIED_STEP,
  NUMBER_OF_STEPS
};
constexpr static int TERMINATE_RUNTIME = 10 * 1000; // 10 seconds

static int gTestStep = -1;
static int gResourceReadyCount = 0;
static bool gAnimationFinished = true;
static bool gTermiatedTest = false;

}  // namespace

/**
 * @brief This is to test the functionality of native image and image visual
 */
class BorderineVisualTest: public VisualTest
{
public:

  BorderineVisualTest( Application& application )
    : mApplication( application )
  {
  }

  ~BorderineVisualTest()
  {
  }

  void OnInit( Application& application )
  {
    mWindow = mApplication.GetWindow();
    mWindow.SetBackgroundColor(Color::BLACK); // Due to the dog-anim.webp is white background, we make window black.

    mTerminateTimer = Timer::New(TERMINATE_RUNTIME);
    mTerminateTimer.TickSignal().Connect(this, &BorderineVisualTest::OnTerminateTimer);
    mTerminateTimer.Start();

    PrepareNextTest();
  }

private:

  bool OnTerminateTimer()
  {
    // Visual Test Timout!
    printf("TIMEOUT borderine-visual.test spend more than %d ms\n",TERMINATE_RUNTIME);

    gTermiatedTest = true;
    gExitValue = -1;
    mApplication.Quit();

    exit(gExitValue);

    return false;
  }

  void PrepareNextTest()
  {
    Window window = mApplication.GetWindow();

    gTestStep++;

    switch(gTestStep)
    {
      case CREATE_STATIC_PREMULTIPLIED_STEP:
      case CREATE_STATIC_NO_PREMULTIPLIED_STEP:
      case CREATE_ANIMATE_PREMULTIPLIED_STEP:
      case CREATE_ANIMATE_NO_PREMULTIPLIED_STEP:
      {
        CreateVisuals(
          (gTestStep == CREATE_ANIMATE_PREMULTIPLIED_STEP) || (gTestStep == CREATE_ANIMATE_NO_PREMULTIPLIED_STEP),
          (gTestStep == CREATE_STATIC_PREMULTIPLIED_STEP) || (gTestStep == CREATE_ANIMATE_PREMULTIPLIED_STEP));
        break;
      }
      default:
        break;
    }
  }

  void OnFinishedAnimation(Animation& animation)
  {
    // Animation done. Check we need to go to next step
    gAnimationFinished = true;
    if(gAnimationFinished && gResourceReadyCount == TOTAL_RESOURCES)
    {
      CaptureWindowAfterFrameRendered(mApplication.GetWindow());
    }
  }

  void OnReady(Dali::Toolkit::Control control)
  {
    // Resource ready done. Check we need to go to next step
    gResourceReadyCount++;
    if(gAnimationFinished && gResourceReadyCount == TOTAL_RESOURCES)
    {
      CaptureWindowAfterFrameRendered(mApplication.GetWindow());
    }
  }

  void PostRender(std::string outputFile, bool success)
  {
    CompareImageFile(EXPECTED_IMAGE_FILE, outputFile, 0.98f);
    if(gTestStep < NUMBER_OF_STEPS-1)
    {
      UnparentAllControls();
      PrepareNextTest();
    }
    else
    {
      // The last check has been done, so we can quit the test
      mTerminateTimer.Stop();
      mApplication.Quit();
    }
  }

private:
  void CreateVisuals(bool isAnimation, bool requiredPreMulitpliedAlpha)
  {
    // Reset resource ready count
    gResourceReadyCount = 0;
    gAnimationFinished = true;

    // If isAnimation, create new super-fast animation.
    if(isAnimation)
    {
      mAnimation = Animation::New(0.001f); // seconds
      gAnimationFinished = false;
    }

    // Create Visuals for each testset types.
    for(std::uint32_t visualTestTypeIndex = 0; visualTestTypeIndex < NUMBER_OF_VALID_VISUAL_TYPES; ++visualTestTypeIndex)
    {
      for(std::uint32_t propertyTestTypeIndex = 0; propertyTestTypeIndex < NUMBER_OF_PROPERTY_TYPES; ++propertyTestTypeIndex)
      {
        CreateTestSet(visualTestTypeIndex, propertyTestTypeIndex, isAnimation, requiredPreMulitpliedAlpha);
      }
    }

    if(isAnimation)
    {
      // Wait until all animations are finished.
      mAnimation.FinishedSignal().Connect(this, &BorderineVisualTest::OnFinishedAnimation);
      mAnimation.Play();
    }
  }

  // Main setup here!
  // Costumize here if you want
  void CreateTestSet(std::uint32_t visualTestTypeIndex, std::uint32_t propertyTestTypeIndex, bool isAnimation, bool requiredPreMulitpliedAlpha)
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
      control.ResourceReadySignal().Connect(this, &BorderineVisualTest::OnReady);

      auto visualType = VALID_VISUAL_TYPES[visualTestTypeIndex];

      // Basic informations of current visual expects
      Property::Map basicVisualMap = CreateBasicVisualMap(visualType);

      // Specific information of current property testset.
      Property::Map testVisualMap = CreateTestVisualMap(propertyTestTypeIndex, subSizeTestTypeIndex == 3 ? NORMAL_VISUAL_SIZE : SMALL_VISUAL_SIZE);

      if(isAnimation)
      {
        // Merge non-animatable properties first.
        if(testVisualMap.Find(DevelVisual::Property::CORNER_RADIUS_POLICY))
        {
          basicVisualMap.Insert(DevelVisual::Property::CORNER_RADIUS_POLICY, testVisualMap[DevelVisual::Property::CORNER_RADIUS_POLICY]);
        }

        // Set properties into background
        control[Control::Property::BACKGROUND] = basicVisualMap;

        // Create animation after create background visual
        for(std::uint32_t mapIndex = 0; mapIndex < testVisualMap.Count(); ++mapIndex)
        {
          KeyValuePair pair = testVisualMap.GetKeyValue(mapIndex);
          // We can assume that we only use index key in this sample
          if(pair.first.type == Property::Key::Type::INDEX)
          {
            // We want to animate CornerRadius or Borderline only in this sample
            if((pair.first.indexKey == DevelVisual::Property::BORDERLINE_WIDTH) ||
              (pair.first.indexKey == DevelVisual::Property::BORDERLINE_COLOR) ||
              (pair.first.indexKey == DevelVisual::Property::BORDERLINE_OFFSET) ||
              (pair.first.indexKey == DevelVisual::Property::CORNER_RADIUS))
            {
              mAnimation.AnimateTo(DevelControl::GetVisualProperty(control, Control::Property::BACKGROUND, pair.first.indexKey), pair.second);
            }
          }
        }
      }
      else
      {
        // if isAnimation false, just merge property values into basicVisualMap
        basicVisualMap.Merge(testVisualMap);

        // Set background
        control[Control::Property::BACKGROUND] = basicVisualMap;
      }

      // Send STOP action for animate image and animate vector image.
      if(visualType == Visual::ANIMATED_IMAGE)
      {
        DevelControl::DoAction(control, Control::Property::BACKGROUND, DevelAnimatedImageVisual::Action::STOP, Property::Value());
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
  Property::Map CreateBasicVisualMap(Visual::Type type)
  {
    Property::Map visualMap;
    switch(type)
    {
      case Visual::Type::IMAGE:
      {
        visualMap[Visual::Property::TYPE]     = Visual::IMAGE;
        visualMap[ImageVisual::Property::URL] = JPG_FILENAME;
        break;
      }
      case Visual::Type::COLOR:
      {
        visualMap[Visual::Property::TYPE]           = Visual::COLOR;
        visualMap[ColorVisual::Property::MIX_COLOR] = Color::DODGER_BLUE;
        break;
      }
      case Visual::Type::GRADIENT:
      {
        visualMap[Visual::Property::TYPE] = Visual::GRADIENT;

        Property::Array stopOffsets;
        stopOffsets.PushBack(0.0f);
        stopOffsets.PushBack(0.3f);
        stopOffsets.PushBack(0.6f);
        stopOffsets.PushBack(0.8f);
        stopOffsets.PushBack(1.0f);
        visualMap[GradientVisual::Property::STOP_OFFSET] = stopOffsets;

        Property::Array stopColors;
        stopColors.PushBack(Vector4(129.f, 198.f, 193.f, 255.f) / 255.f);
        stopColors.PushBack(Vector4(196.f, 198.f, 71.f, 122.f) / 255.f);
        stopColors.PushBack(Vector4(214.f, 37.f, 139.f, 191.f) / 255.f);
        stopColors.PushBack(Vector4(129.f, 198.f, 193.f, 150.f) / 255.f);
        stopColors.PushBack(Color::YELLOW);
        visualMap[GradientVisual::Property::STOP_COLOR] = stopColors;

        // linear gradient with units as objectBoundingBox
        visualMap[GradientVisual::Property::START_POSITION] = Vector2(0.5f, 0.5f);
        visualMap[GradientVisual::Property::END_POSITION]   = Vector2(-0.5f, -0.5f);
        break;
      }
      case Visual::Type::SVG:
      {
        visualMap[Visual::Property::TYPE]     = Visual::SVG;
        visualMap[ImageVisual::Property::URL] = SVG_FILENAME;
        break;
      }
      case Visual::Type::ANIMATED_IMAGE:
      {
        visualMap[Visual::Property::TYPE]     = Visual::ANIMATED_IMAGE;
        visualMap[ImageVisual::Property::URL] = ANIMATED_WEBP_FILENAME;

        // We will control the animation by StopAction.
        visualMap[Toolkit::DevelImageVisual::Property::STOP_BEHAVIOR] = DevelImageVisual::StopBehavior::FIRST_FRAME;
        break;
      }
      default:
        break;
    }
    return visualMap;
  }

  Property::Map CreateTestVisualMap(std::uint32_t propertyTestTypeIndex, float minSizeOfVisual)
  {
    Property::Map visualMap;

    auto offsetValue = BORDERLINE_OFFSET_LIST[propertyTestTypeIndex %  (NUMBER_OF_PROPERTY_TYPES / 2)];
    auto colorValue = BORDERLINE_COLOR_LIST[propertyTestTypeIndex %  (NUMBER_OF_PROPERTY_TYPES / 2)];
    auto widthValue = BORDERLINE_WIDTH_LIST[propertyTestTypeIndex %  (NUMBER_OF_PROPERTY_TYPES / 2)];

    visualMap[DevelVisual::Property::BORDERLINE_WIDTH]  = widthValue;
    visualMap[DevelVisual::Property::BORDERLINE_COLOR]  = colorValue;
    visualMap[DevelVisual::Property::BORDERLINE_OFFSET] = offsetValue;
    if(propertyTestTypeIndex >= (NUMBER_OF_PROPERTY_TYPES / 2))
    {
      // Note : We should set CornerRadius as Vector4 type due to the animation TC.
      visualMap[DevelVisual::Property::CORNER_RADIUS]        = CORNER_RADIUS_RATES;
      visualMap[DevelVisual::Property::CORNER_RADIUS_POLICY] = Visual::Transform::Policy::RELATIVE;
    }
    return visualMap;
  }

private:
  Application&         mApplication;
  Window               mWindow;
  Timer                mTerminateTimer;
  Animation            mAnimation;
  std::vector<Control> mControlList;
};

DALI_VISUAL_TEST_WITH_WINDOW_SIZE( BorderineVisualTest, OnInit, TESTSET_VISUAL_SIZE * NUMBER_OF_PROPERTY_TYPES, TESTSET_VISUAL_SIZE * NUMBER_OF_VALID_VISUAL_TYPES )
