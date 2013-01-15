# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'app_list',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../skia/skia.gyp:skia',
        '../compositor/compositor.gyp:compositor',
        '../ui.gyp:ui',
        '../views/views.gyp:views',
      ],
      'defines': [
        'APP_LIST_IMPLEMENTATION',
      ],
      'sources': [
        'app_list_bubble_border.cc',
        'app_list_bubble_border.h',
        'app_list_export.h',
        'app_list_item_model.cc',
        'app_list_item_model.h',
        'app_list_item_model_observer.h',
        'app_list_item_view.cc',
        'app_list_item_view.h',
        'app_list_model.cc',
        'app_list_model.h',
        'app_list_view.cc',
        'app_list_view.h',
        'app_list_view_delegate.h',
        'apps_grid_view.cc',
        'apps_grid_view.h',
        'contents_view.cc',
        'contents_view.h',
        'drop_shadow_label.cc',
        'drop_shadow_label.h',
        'icon_cache.cc',
        'icon_cache.h',
        'page_switcher.cc',
        'page_switcher.h',
        'pagination_model.cc',
        'pagination_model.h',
        'pagination_model_observer.h',
        'search_box_model.h',
        'search_box_model.cc',
        'search_box_model_observer.h',
        'search_box_view.cc',
        'search_box_view.h',
        'search_box_view_delegate.h',
        'search_result.cc',
        'search_result.h',
        'search_result_view.cc',
        'search_result_view.h',
        'search_result_list_view.cc',
        'search_result_list_view.h',
        'search_result_list_view_delegate.h',
      ],
    },
    {
      'target_name': 'app_list_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:test_support_base',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../compositor/compositor.gyp:compositor',
        '../compositor/compositor.gyp:compositor_test_support',
        '../views/views.gyp:views',
        'app_list',
      ],
      'sources': [
        'apps_grid_view_unittest.cc',
        'pagination_model_unittest.cc',
        'test/app_list_test_suite.cc',
        'test/app_list_test_suite.h',
        'test/run_all_unittests.cc',
      ],
    },
  ],
}