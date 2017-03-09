{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'follow_trade',
    'type' : '<(component)',
    'variables' : {
    },
    'sources' : [
      'follow_trade_actor.h',
      'follow_trade_actor.cc',
      'instrument_follow.h',
      'instrument_follow.cc',
      'order_follow.h',
      'order_follow.cc',
      'caf_defines.h',
    ],
    'dependencies' : [
      #'<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:libcaf_build_config',
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
    ],
  },
  {
    'target_name' : 'follow_trade_unittest',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      'follow_trade_unittest.cc',
      'instrument_follow_unittest.cc',
    ],
    'dependencies' : [
      'follow_trade',
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
    ],
  }
  ]
}     
