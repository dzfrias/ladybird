/*
 * Copyright (c) 2023, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define AK_DONT_REPLACE_STD

#include <core/SkTypeface.h>
#include <harfbuzz/hb.h>

#include <LibGfx/Font/ScaledFont.h>
#include <LibGfx/Font/Typeface.h>

namespace Gfx {

Typeface::Typeface() = default;

Typeface::~Typeface()
{
    if (m_harfbuzz_face)
        hb_face_destroy(m_harfbuzz_face);
    if (m_harfbuzz_blob)
        hb_blob_destroy(m_harfbuzz_blob);
}

NonnullRefPtr<ScaledFont> Typeface::scaled_font(float point_size) const
{
    auto it = m_scaled_fonts.find(point_size);
    if (it != m_scaled_fonts.end())
        return *it->value;

    // FIXME: It might be nice to have a global cap on the number of fonts we cache
    //        instead of doing it at the per-Typeface level like this.
    constexpr size_t max_cached_font_size_count = 128;
    if (m_scaled_fonts.size() > max_cached_font_size_count)
        m_scaled_fonts.remove(m_scaled_fonts.begin());

    auto scaled_font = adopt_ref(*new ScaledFont(*this, point_size, point_size));
    m_scaled_fonts.set(point_size, scaled_font);
    return scaled_font;
}

hb_face_t* Typeface::harfbuzz_typeface() const
{
    if (!m_harfbuzz_blob)
        m_harfbuzz_blob = hb_blob_create(reinterpret_cast<char const*>(buffer().data()), buffer().size(), HB_MEMORY_MODE_READONLY, nullptr, [](void*) {});
    if (!m_harfbuzz_face)
        m_harfbuzz_face = hb_face_create(m_harfbuzz_blob, ttc_index());
    return m_harfbuzz_face;
}

}
