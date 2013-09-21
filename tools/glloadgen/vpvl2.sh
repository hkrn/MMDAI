#!/bin/sh

lua LoadGen.lua \
    -version 2.1 \
    -ext ARB_debug_output \
    -ext ARB_depth_buffer_float \
    -ext ARB_draw_elements_base_vertex \
    -ext ARB_framebuffer_object \
    -ext ARB_half_float_pixel \
    -ext ARB_map_buffer_range \
    -ext ARB_occlusion_query \
    -ext ARB_occlusion_query2 \
    -ext ARB_sampler_objects \
    -ext ARB_texture_float \
    -ext ARB_texture_rg \
    -ext ARB_texture_storage \
    -ext ARB_vertex_array_object \
    -ext APPLE_vertex_array_object \
    -ext EXT_framebuffer_blit \
    -ext EXT_framebuffer_multisample \
    -ext EXT_framebuffer_object \
    -indent space \
    -prefix vpvl2_ vpvl2

