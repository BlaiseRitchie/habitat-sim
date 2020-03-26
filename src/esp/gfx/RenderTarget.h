// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <Magnum/Magnum.h>

#include "esp/core/esp.h"

#include "esp/gfx/DepthUnprojection.h"
#include "esp/gfx/TriangleShader.h"

namespace esp {
namespace gfx {

/**
 * Holds a framebuffer and encapsulates the logic of retrieving rendering
 * results of various types (RGB, Depth, ObjectID) from the framebuffer.
 *
 * Reads the rendering results into either CPU or GPU, if compiled with CUDA,
 * memory
 */
class RenderTarget {
 public:
  /**
   * @brief Constructor
   * @param size               The size of the underlying framebuffers in WxH
   * @param depthUnprojection  Depth unprojection parameters.  See @ref
   *                           calculateDepthUnprojection()
   * @param depthShader        A DepthShader used to unproject depth on the GPU.
   *                           Unprojects the depth on the CPU if nullptr.
   *                           Must be not nullptr to use @ref
   *                           readFrameDepthGPU()
   */
  RenderTarget(const Magnum::Vector2i& size,
               const Magnum::Vector2& depthUnprojection,
               DepthShader* depthShader,
               TriangleShader* triangleShader);

  /**
   * @brief Constructor
   * @param size               The size of the underlying framebuffers in WxH
   * @param depthUnprojection  Depth unrpojection parameters.  See @ref
   *                           calculateDepthUnprojection()
   *
   * Equivalent to calling @ref RenderTarget(size, depthUnprojection, nullptr)
   */
  RenderTarget(const Magnum::Vector2i& size,
               const Magnum::Vector2& depthUnprojection)
      : RenderTarget{size, depthUnprojection, nullptr, nullptr} {};

  ~RenderTarget() { LOG(INFO) << "Deconstructing RenderTarget"; }

  /**
   * @brief Called before any draw calls that target this RenderTarget
   * Clears the framebuffer and binds it
   */
  void renderEnter();

  /**
   * @brief Called after any draw calls that target this RenderTarget
   */
  void renderExit();

  /**
   * @brief The size of the framebuffer in WxH
   */
  Magnum::Vector2i framebufferSize() const;

  /**
   * @brief Retrieve the RGBA rendering results.
   *
   * @param[in, out] view Preallocated memory that will be populated with the
   * result.  The result will be read as the pixel format of this view.
   */
  void readFrameRgba(const Magnum::MutableImageView2D& view);

  /**
   * @brief Retrieve the depth rendering results.
   *
   * @param[in, out] view Preallocated memory that will be populated with the
   * result.  The PixelFormat of the image must only specify the R channel,
   * generally @ref Magnum::PixelFormat::R32F
   */
  void readFrameDepth(const Magnum::MutableImageView2D& view);

  /**
   * @brief Reads the ObjectID rendering results into the memory specified by
   * view
   *
   * @param[in, out] view Preallocated memory that will be populated with the
   * result.  The PixelFormat of the image must only specify the R channel and
   * be a format which a uint16_t can be interpreted as, generally @ref
   * Magnum::PixelFormat::R32UI, @ref Magnum::PixelFormat::R32I, or @ref
   * Magnum::PixelFormat::R16UI
   */
  void readFrameObjectId(const Magnum::MutableImageView2D& view);

#ifdef ESP_BUILD_WITH_TRIANGLE_SENSOR
  /**
   * @brief Reads the TriangleID rendering results into the memory specified by
   * view
   *
   * @param[in, out] view Preallocated memory that will be populated with the
   * result.  The PixelFormat of the image must only specify the R channel and
   * be a format which a uint16_t can be interpreted as, generally @ref
   * Magnum::PixelFormat::R32UI, @ref Magnum::PixelFormat::R32I, or @ref
   * Magnum::PixelFormat::R16UI
   */
  void readFrameTriangleId(const Magnum::MutableImageView2D& view);
#endif

  /**
   * @brief Blits the rgba buffer from internal FBO to default frame buffer
   * which in case of EmscriptenApplication will be a canvas element.
   */
  void blitRgbaToDefault();

  // @brief Delete copy Constructor
  RenderTarget(const RenderTarget&) = delete;
  // @brief Delete copy operator
  RenderTarget& operator=(const RenderTarget&) = delete;

#ifdef ESP_BUILD_WITH_CUDA
  /**
   * @brief Reads the RGBA rendering result directly into CUDA memory. The
   * caller is responsible for allocating memory and ensuring that the OpenGL
   * context and the devPtr are on the same CUDA device.
   *
   * @param[in, out] devPtr CUDA memory pointer that points to a contiguous
   * memory region of at least W*H*sizeof(uint8_t) bytes.
   */
  void readFrameRgbaGPU(uint8_t* devPtr);

  /**
   * @brief Reads the depth rendering result directly into CUDA memory.  See
   * @ref readFrameRgbaGPU()
   *
   * Requires the rendering target to have a valid DepthShader
   *
   * @param[in, out] devPtr CUDA memory pointer that points to a contiguous
   * memory region of at least W*H*sizeof(float) bytes.
   */
  void readFrameDepthGPU(float* devPtr);

  /**
   * @brief Reads the ObjectID rendering result directly into CUDA memory.  See
   * @ref readFrameRgbaGPU()
   *
   * @param[in, out] devPtr CUDA memory pointer that points to a contiguous
   * memory region of at least W*H*sizeof(int32_t) bytes.
   */
  void readFrameObjectIdGPU(int32_t* devPtr);

#ifdef ESP_BUILD_WITH_TRIANGLE_SENSOR
  /**
   * @brief Reads the TriangleID rendering result directly into CUDA memory. See
   * @ref readFrameRgbaGPU()
   *
   * @param[in, out] devPtr CUDA memory pointer that points to a contiguous
   * memory region of at least W*H*sizeof(int32_t) bytes.
   */
  void readFrameTriangleIdGPU(int32_t* devPtr);
#endif
#endif

  ESP_SMART_POINTERS_WITH_UNIQUE_PIMPL(RenderTarget)
};

}  // namespace gfx
}  // namespace esp
